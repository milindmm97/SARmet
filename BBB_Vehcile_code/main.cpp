#include <stdio.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include <math.h>
#include "utils.h"
//#include "videoio.hpp"



#define USE_VIDEO 1

#undef MIN
#undef MAX
#define MAX(a,b) ((a)<(b)?(b):(a))
#define MIN(a,b) ((a)>(b)?(b):(a))

void crop(IplImage* src, IplImage* dest, CvRect rect) {
	cvSetImageROI(src, rect);
	cvCopy(src, dest);
	cvResetImageROI(src);
}

struct Lane {


};

struct Status {
	Status() :reset(true), lost(0) {}
	ExpMovingAverage k, b;
	bool reset;
	int lost;
};

struct Vehicle {
	CvPoint bmin, bmax;
	int symmetryX;
	bool valid;
	unsigned int lastUpdate;
	float len;
	int y1;
	int y2;
	float l1[10][30];
	//float l2[10][30];
	float vel[10];

	int num[10];
	
};

struct VehicleSample {
	CvPoint center;
	float radi;
	unsigned int frameDetected;
	int vehicleIndex;
};

#define GREEN CV_RGB(0,255,0)
#define RED CV_RGB(255,0,0)
#define BLUE CV_RGB(255,0,255)
#define PURPLE CV_RGB(255,0,255)

Status laneR, laneL;
std::vector<Vehicle> vehicles;
std::vector<VehicleSample> samples;

enum {
	SCAN_STEP = 5,              // in pixels
	LINE_REJECT_DEGREES = 10, // in degrees
	BW_TRESHOLD = 250,          // edge response strength to recognize for 'WHITE'
	BORDERX = 10,              // px, skip this much from left & right borders
	MAX_RESPONSE_DIST = 5,      // px

	CANNY_MIN_TRESHOLD = 1,      // edge detector minimum hysteresis threshold
	CANNY_MAX_TRESHOLD = 100, // edge detector maximum hysteresis threshold

	HOUGH_TRESHOLD = 50,        // line approval vote threshold
	HOUGH_MIN_LINE_LENGTH = 50,    // remove lines shorter than this treshold
	HOUGH_MAX_LINE_GAP = 100,   // join lines to one with smaller than this gaps

	CAR_DETECT_LINES = 4,    // minimum lines for a region to pass validation as a 'CAR'
	CAR_H_LINE_LENGTH = 10,  // minimum horizontal line length from car body in px

	MAX_VEHICLE_SAMPLES = 30,      // max vehicle detection sampling history
	CAR_DETECT_POSITIVE_SAMPLES = MAX_VEHICLE_SAMPLES - 2, // probability positive matches for valid car
	MAX_VEHICLE_NO_UPDATE_FREQ = 15 // remove car after this much no update frames
};

#define K_VARY_FACTOR 0.2f
#define B_VARY_FACTOR 20
#define MAX_LOST_FRAMES 30

void FindResponses(IplImage *img, int startX, int endX, int y, std::vector<int>& list)
{
	// scans for single response: /^\_

	const int row = y * img->width * img->nChannels;
	unsigned char* ptr = (unsigned char*)img->imageData;

	int step = (endX < startX) ? -1 : 1;
	int range = (endX > startX) ? endX - startX + 1 : startX - endX + 1;

	for (int x = startX; range>0; x += step, range--)
	{
		if (ptr[row + x] <= BW_TRESHOLD) continue; // skip black: loop until white pixels show up

												   // first response found
		int idx = x + step;

		// skip same response(white) pixels
		while (range > 0 && ptr[row + idx] > BW_TRESHOLD) {
			idx += step;
			range--;
		}

		// reached black again
		if (ptr[row + idx] <= BW_TRESHOLD) {
			list.push_back(x);
		}

		x = idx; // begin from new pos
	}
}

unsigned char pixel(IplImage* img, int x, int y) {
	return (unsigned char)img->imageData[(y*img->width + x)*img->nChannels];
}

int findSymmetryAxisX(IplImage* half_frame, CvPoint bmin, CvPoint bmax) {

	float value = 0;
	int axisX = -1; // not found

	int xmin = bmin.x;
	int ymin = bmin.y;
	int xmax = bmax.x;
	int ymax = bmax.y;
	int half_width = half_frame->width / 2;
	int maxi = 1;

	for (int x = xmin, j = 0; x<xmax; x++, j++) {
		float HS = 0;
		for (int y = ymin; y<ymax; y++) {
			int row = y*half_frame->width*half_frame->nChannels;
			for (int step = 1; step<half_width; step++) {
				int neg = x - step;
				int pos = x + step;
				unsigned char Gneg = (neg < xmin) ? 0 : (unsigned char)half_frame->imageData[row + neg*half_frame->nChannels];
				unsigned char Gpos = (pos >= xmax) ? 0 : (unsigned char)half_frame->imageData[row + pos*half_frame->nChannels];
				HS += abs(Gneg - Gpos);
			}
		}

		if (axisX == -1 || value > HS) { // find minimum
			axisX = x;
			value = HS;
		}
	}

	return axisX;
}

bool hasVertResponse(IplImage* edges, int x, int y, int ymin, int ymax) {
	bool has = (pixel(edges, x, y) > BW_TRESHOLD);
	if (y - 1 >= ymin) has &= (pixel(edges, x, y - 1) < BW_TRESHOLD);
	if (y + 1 < ymax) has &= (pixel(edges, x, y + 1) < BW_TRESHOLD);
	return has;
}

int horizLine(IplImage* edges, int x, int y, CvPoint bmin, CvPoint bmax, int maxHorzGap) {

	// scan to right
	int right = 0;
	int gap = maxHorzGap;
	for (int xx = x; xx<bmax.x; xx++) {
		if (hasVertResponse(edges, xx, y, bmin.y, bmax.y)) {
			right++;
			gap = maxHorzGap; // reset
		}
		else {
			gap--;
			if (gap <= 0) {
				break;
			}
		}
	}

	int left = 0;
	gap = maxHorzGap;
	for (int xx = x - 1; xx >= bmin.x; xx--) {
		if (hasVertResponse(edges, xx, y, bmin.y, bmax.y)) {
			left++;
			gap = maxHorzGap; // reset
		}
		else {
			gap--;
			if (gap <= 0) {
				break;
			}
		}
	}

	return left + right;
}

bool vehicleValid(IplImage* half_frame, IplImage* edges, Vehicle* v, int& index) {

	index = -1;

	// first step: find horizontal symmetry axis
	v->symmetryX = findSymmetryAxisX(half_frame, v->bmin, v->bmax);
	if (v->symmetryX == -1) return false;

	// second step: cars tend to have a lot of horizontal lines
	int hlines = 0;
	v->y1 = v->bmin.y;
	v->y2 =v->bmax.y;
	for (int y = v->bmin.y; y < v->bmax.y; y++) {
		if (horizLine(edges, v->symmetryX, y, v->bmin, v->bmax, 2) > CAR_H_LINE_LENGTH) {
#if _DEBUG
			if (v->y1 <= y) {
				v->y1 = y;
			}
			if (v->y2 >= y) {
				v->y2 = y;
			}
	
			//cvCircle(half_frame, cvPoint(v->symmetryX, y), 2, PURPLE);
			
			v->len = (v->y1) - (v->y2);
			
			
#endif
			hlines++;
		}
	}
	cvCircle(half_frame, cvPoint(v->symmetryX, v->y1), 2, PURPLE);
	cvCircle(half_frame, cvPoint(v->symmetryX, v->y2), 2, PURPLE);

	int midy = (v->bmax.y + v->bmin.y) / 2;

	// third step: check with previous detected samples if car already exists
	int numClose = 0;
	float closestDist = 0;
	for (int i = 0; i < samples.size(); i++) {
		int dx = samples[i].center.x - v->symmetryX;
		int dy = samples[i].center.y - midy;
		float Rsqr = dx*dx + dy*dy;

		if (Rsqr <= samples[i].radi*samples[i].radi) {
			numClose++;
			if (index == -1 || Rsqr < closestDist) {
				index = samples[i].vehicleIndex;
				closestDist = Rsqr;
			}
		}
	}

	return (hlines >= CAR_DETECT_LINES || numClose >= CAR_DETECT_POSITIVE_SAMPLES);
}

void removeOldVehicleSamples(unsigned int currentFrame) {
	// statistical sampling - clear very old samples
	std::vector<VehicleSample> sampl;
	for (int i = 0; i < samples.size(); i++) {
		if (currentFrame - samples[i].frameDetected < MAX_VEHICLE_SAMPLES) {
			sampl.push_back(samples[i]);
		}
	}
	samples = sampl;
}

void removeSamplesByIndex(int index) {
	// statistical sampling - clear very old samples
	std::vector<VehicleSample> sampl;
	for (int i = 0; i < samples.size(); i++) {
		if (samples[i].vehicleIndex != index) {
			sampl.push_back(samples[i]);
		}
	}
	samples = sampl;
}

void removeLostVehicles(unsigned int currentFrame) {
	// remove old unknown/false vehicles & their samples, if any
	for (int i = 0; i<vehicles.size(); i++) {
		if (vehicles[i].valid && currentFrame - vehicles[i].lastUpdate >= MAX_VEHICLE_NO_UPDATE_FREQ) {
			printf("\tremoving inactive car, index = %d\n", i);
			removeSamplesByIndex(i);
			vehicles[i].valid = false;
		}
	}
}

void vehicleDetection(IplImage* half_frame, CvHaarClassifierCascade* cascade, CvMemStorage* haarStorage) {

	
}

void drawVehicles(IplImage* half_frame) {

	
}

void processSide(std::vector<Lane> lanes, IplImage *edges, bool right) {

}


void processLanes(CvSeq* lines, IplImage* edges, IplImage* temp_frame) {


}


int main(void)
{
//	FILE *f = fopen("output.txt", "w");
//	if (f == NULL)
//	{
//		printf("Error opening file!\n");
//		exit(1);
//	}

#ifdef USE_VIDEO
	CvCapture *input_video = cvCreateFileCapture("road.avi");
#else
	CvCapture *input_video = cvCaptureFromCAM(0);
#endif

	if (input_video == NULL) {
		fprintf(stderr, "Error: Can't open video\n");
		return -1;
	}

	CvFont font;
	cvInitFont(&font, CV_FONT_VECTOR0, 0.25f, 0.25f);

	CvSize video_size;
	video_size.height = (int)cvGetCaptureProperty(input_video, CV_CAP_PROP_FRAME_HEIGHT);
	video_size.width = (int)cvGetCaptureProperty(input_video, CV_CAP_PROP_FRAME_WIDTH);

	long current_frame = 0;
	int key_pressed = 0;
	IplImage *frame = NULL;
	int frm = 0;

	CvSize frame_size = cvSize(video_size.width, video_size.height/2 );
	IplImage *temp_frame = cvCreateImage(frame_size, IPL_DEPTH_8U, 3);
	IplImage *half_frame = cvCreateImage(cvSize(video_size.width/2 , video_size.height/2 ), IPL_DEPTH_8U, 3);

	CvMemStorage* houghStorage = cvCreateMemStorage(0);
	CvMemStorage* haarStorage = cvCreateMemStorage(0);
	CvHaarClassifierCascade* cascade = (CvHaarClassifierCascade*)cvLoad("haar/cars3.xml");

	cvSetCaptureProperty(input_video, CV_CAP_PROP_POS_FRAMES, current_frame);
	Vehicle v;
	while (key_pressed != 27) {
		//OpenCV Error: Unspecified error (The node does not represent a user object (unknown type?)) in cvRead, file C:\builds\2_4_PackSlave-win64-vc12-shared\opencv\modules\core\src\persistence.cpp, line 4991

		
		
		frame = cvQueryFrame(input_video);
		if (frame == NULL) {
			fprintf(stderr, "Error: null frame received\n");
			return -1;
		}

		cvPyrDown(frame, half_frame, CV_GAUSSIAN_5x5); // Reduce the image by 2     
													   //cvCvtColor(temp_frame, grey, CV_BGR2GRAY); // convert to grayscale

													   // we're interested only in road below horizont - so crop top image portion off
		//crop(frame, temp_frame, cvRect(0, frame_size.height, frame_size.width, frame_size.height));

		// process vehicles

		static unsigned int frame = -1;
		frame++;
		printf("*** vehicle detector frame: %d ***\n", frame);
	


		removeOldVehicleSamples(frame);

		// Haar Car detection
		const double scale_factor = 1.05; // every iteration increases scan window by 0% supposed to be 5%
		const int min_neighbours = 2; // minus 1, number of rectangles, that the object consists of
		CvSeq* rects = cvHaarDetectObjects(half_frame, cascade, haarStorage, scale_factor, min_neighbours, CV_HAAR_DO_CANNY_PRUNING);

		// Canny edge detection of the minimized frame
		if (rects->total > 0) {
			printf("\thaar detected %d car hypotheses\n", rects->total);
			IplImage *edges = cvCreateImage(cvSize(half_frame->width, half_frame->height), IPL_DEPTH_8U, 1);
			cvCanny(half_frame, edges, CANNY_MIN_TRESHOLD, CANNY_MAX_TRESHOLD);

			/* validate vehicles */
			for (int i = 0; i < rects->total; i++) {
				CvRect* rc = (CvRect*)cvGetSeqElem(rects, i);

				Vehicle v;
				v.bmin = cvPoint(rc->x, rc->y);
				v.bmax = cvPoint(rc->x + rc->width, rc->y + rc->height);
				v.valid = true;

				int index;
				if (vehicleValid(half_frame, edges, &v, index)) { // put a sample on that position

					if (index == -1) { // new car detected

						v.lastUpdate = frame;

						
						// re-use already created but inactive vehicles
						for (int j = 0; j<vehicles.size(); j++) {
							if (vehicles[j].valid == false) {
								index = j;
								break;
							}


						}
						if (index == -1) { // all space used
							index = vehicles.size();


							vehicles.push_back(v);
						}
						printf("\tnew car detected, index = %d\n", index);
						printf("llllllllllll %flllllllllllll\n", v.len);
						v.l1[index][0] = v.len;
						v.num[index] = 0;
						
					}
					
					else {
						// update the position from new data
						vehicles[index] = v;
						vehicles[index].lastUpdate = frame;
						printf("\tcar updated, index = %d\n", index);
						

						

						if (v.num[index] % 30 == 0) {
							v.num[index] = 0;
						}
						v.l1[index][v.num[index]] = v.len;
						
						if (v.num[index] % 30 == 29) {
							v.vel[index] = v.l1[index][29] - v.l1[index][0];
							printf("vvvvvvvvvvv %fvvvvvvvvvvvv\n", v.vel[index]);
						}
						

						printf("llllllllllll %flllllllllllll\n", v.len);
						//printf("1111111111 %f111111111111\n", v.l1[index][v.num[index]]);
						//printf("vvvvvvvvvvv %fvvvvvvvvvvvv\n", v.vel[index]);
						//v.l2[index] = v.l1[index];


						//if (v.vel[index] >= 18) {
							cvRectangle(half_frame, v.bmin, v.bmax, GREEN, 1);
						//}

					}
					v.num[index]++;
					VehicleSample vs;
					vs.frameDetected = frame;
					vs.vehicleIndex = index;
					vs.radi = (MAX(rc->width, rc->height)) / 4; // radius twice smaller - prevent false positives
					vs.center = cvPoint((v.bmin.x + v.bmax.x) / 2, (v.bmin.y + v.bmax.y) / 2);
					samples.push_back(vs);
				}
			}

			cvShowImage("Half-frame[edges]", edges);
			cvMoveWindow("Half-frame[edges]", half_frame->width * 2 + 10, half_frame->height);
			cvReleaseImage(&edges);
		}
		else {
			printf("\tno vehicles detected in current frame!\n");
		}

		removeLostVehicles(frame);

		printf("\ttotal vehicles on screen: %d\n", vehicles.size());




	

		drawVehicles(half_frame);
		cvShowImage("Half-frame", half_frame);
		//cvMoveWindow("Half-frame", half_frame->width * 2 + 10, 0);

		// show middle line
		cvLine(temp_frame, cvPoint(frame_size.width / 2, 0),
			cvPoint(frame_size.width / 2, frame_size.height), CV_RGB(255, 255, 0), 1);

		key_pressed = cvWaitKey(5);

	}
	//cv2.resizeWindow('half_frame',640,360)
	cvReleaseHaarClassifierCascade(&cascade);
	cvReleaseMemStorage(&haarStorage);
	cvReleaseMemStorage(&houghStorage);
	cvReleaseImage(&temp_frame);
	cvReleaseImage(&half_frame);

	cvReleaseCapture(&input_video);
}
