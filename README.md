# SARmet
Master code repo for SARmet project. 

1. Involves Vehicle detection and tracking code

2. App for notifications and server on BBB

3. Voice commands ( Trial) 

4. Miscellanous codes (for GUI) 




#For OpenCv Installation in BeagleBone Black 

1 cd /opt/scripts/tools

2 git pull

3 sudo ./grow_partition.sh

4 sudo reboot

//For expanding partition on SD Card 

1 sudo apt-get install build-essential cmake pkg-config

2 sudo apt-get install libtiff4-dev libjpeg-dev libjasper-dev libpng12-dev

3 sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev

// Installing the OpenCv pre- Requisities 

1 cd opencv

2 mkdir build && cd build

3 cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_CUDA=OFF -D WITH_CUFFT=OFF -D WITH_CUBLAS=OFF -D WITH_NVCUVID=OFF -D WITH_OPENCL=OFF -D WITH_OPENCLAMDFFT=OFF -D WITH_OPENCLAMDBLAS=OFF -D BUILD_opencv_apps=OFF -D BUILD_DOCS=OFF -D BUILD_PERF_TESTS=OFF -D BUILD_TESTS=OFF -D ENABLE_NEON=on ..

4 make

5 sudo make install

6 sudo ldconfig

// Building OpenCv 




#To set display to DLP 

i2cset -y 2 0x1b 0x0b 0x00 0x00 0x00 0x00 i

i2cset -y 2 0x1b 0x0c 0x00 0x00 0x00 0x1b i 

// writing to i2c bus 

export DISPLAY=: 0
// exporting all display to dlp 

