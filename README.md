# SolarBeam
SolarBeam is an open-source multi-channel interface and amplifier for vibrotactile haptics. 
Demo video:
How to make a SolarBeam:
Academic paper:
Really interested in using a SolarBeam but can't make one yourself? Contact lloyd [at] ccrma [dot] stanford [dot] edu with the subject line "SolarBeam" and we'll try our best to get you one.


## Setup
Once you have a SolarBeam in hand, you'll need to flash it (i.e. upload the required software to the Teensy)
Connect the Teensy to a computer with Arduino through USB and supply power to the device via plugging in a 9V power supply. With the software open, make sure the Teensy is identified through label or active USB port usage. Additionally, set the audio output device for your computer's system as the Teensy.

Download the latest BrainBox version and open the latest BrainBox.ino file in Arduino. Upload the file onto the board, which when complete, will automatically run. For any issues that arise during the run a re-upload should fix them.

## Using a SolarBeam

With the BrainBox running, the device will take any audio input in the computer and separate them into the 8 output channels. Connect any output devices like a speaker or motor to any of the 8 aux ports using aux jacks to show the isolated audio in converted form, like vibration. Adjust the strength of the output channels with the dials associated with the channels.

Currently we have a way to interleave signals written in Max/MSP and a project template for the Reaper DAW. 

See the "how to interleave signals" portion of the documentation for more information. 
