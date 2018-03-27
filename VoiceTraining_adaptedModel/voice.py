
#!/usr/bin/python

import time
import datetime
import math
from Adafruit_8x8 import EightByEight
import sys, select, subprocess


   
#runstring("Hello My name is Inigo Montoya. You killed my father. Prepare to die");

proc = subprocess.Popen(['sh', '-c', 'pocketsphinx_continuous -adcdev hw:1,0 -nfft 2048 -samprate 48000 2>/dev/null'],stdout=subprocess.PIPE)
while True:
    line = proc.stdout.readline()
    if line != '':
        #the real code does filtering here
        output = line.rstrip()
        print output
        if (len(output.split("READY"))>1):
            runstring("Speak")
        if (len(output.split("please wait"))>1):
            runstring("Please wait")
        if (len(output.split(":"))>1):
            runstring(str(output.split(":")[1])+'.')
    else:
        break
 
runVisibleVoice.py
Open with
Displaying runVisibleVoice.py.