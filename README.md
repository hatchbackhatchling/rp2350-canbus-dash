# rp2350-canbus-dash

This program uses a Waveshare-sourced RP2350-CAN and a Nextion HMI to display canbus statistics in an easily readable format.

The CANBUS session is setup with CAN2.0b at 1000KBPS and extended id.

The aim is to eventually make the program use multicore for as little latency as possible, and add speedometer and TPMS functionality among others.

Core 0 (Main Core) will handle CANbus message receiving and decoding, while Core 1 will handle Nextion updating.
Potential features that might be added down the line may also include Neopixels for Shift Light.

Program is written in C, but may change languages depending on how difficult it is to make everything work together.

NEXTION COMMANDS
draw 0,0,800,480,WHITE

draw 0,0,275,100,WHITE 
draw 275,0,525,100,WHITE
draw 525,0,800,100,WHITE

draw 0,100,190,210,WHITE
draw 190,100,290,210,WHITE

draw 0,210,190,320,WHITE
draw 190,210,290,320,WHITE