#!/usr/bin/env python

from pocketsphinx.pocketsphinx import *
from sphinxbase.sphinxbase import *

import os
import pyaudio
import audioop
from collections import deque
import math


"""
Listens to Microphone, extracts phrases from it and calls pocketsphinx
to decode the sound
"""
class SpeechDetector:
    def __init__(self):
        self.setup_mic()
        
        #Open stream
        p = pyaudio.PyAudio()
        stream = p.open(format=self.FORMAT, 
                        channels=self.CHANNELS, 
                        rate=self.RATE, 
                        input=True, 
                        frames_per_buffer=self.CHUNK)
        print ("* Mic set up and listening. ")
        
        audio2send = []
        cur_data = ''  # current chunk of audio data
        rel = self.RATE/self.CHUNK
        slid_win = deque(maxlen=self.SILENCE_LIMIT * rel)
        #Prepend audio from 0.5 seconds before noise was detected
        prev_audio = deque(maxlen=self.PREV_AUDIO * rel)
        started = False
        
        while True:
            cur_data = stream.read(self.CHUNK)
            slid_win.append(math.sqrt(abs(audioop.avg(cur_data, 4))))
        
            if sum([x > self.THRESHOLD for x in slid_win]) > 0:
                if started == False:
                    print ("Starting recording of phrase")
                    started = True
                audio2send.append(cur_data)
        
            elif started:
                print ("Finished recording, decoding phrase")
                filename = self.save_speech(list(prev_audio) + audio2send, p)
                r = self.decode_phrase(filename)
                print ("DETECTED: ", r)
        
                # Removes temp audio file
                os.remove(filename)
                # Reset all
                started = False
                slid_win = deque(maxlen=self.SILENCE_LIMIT * rel)
                prev_audio = deque(maxlen=0.5 * rel)
                audio2send = []
                print ("Listening ...")
        
            else:
                prev_audio.append(cur_data)
        
        print ("* Done listening")
        stream.close()
        p.terminate()

if __name__ == "__main__":
    sd = SpeechDetector()
    sd.run()
