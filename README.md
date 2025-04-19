# rp2350-canbus-dash

This program uses a Waveshare-sourced RP2350-CAN and a Nextion HMI to display canbus statistics in an easily readable format.

The CANBUS session is setup with CAN2.0b at 1000KBPS and extended id.

The aim is to eventually make the program use multicore for as little latency as possible, and add speedometer and TPMS functionality among others.

Core 0 (Main Core) will handle CANbus message receiving and decoding, while Core 1 will handle Nextion updating.
Potential features that might be added down the line may also include Neopixels for Shift Light.

Program is written in C, but may change languages depending on how difficult it is to make everything work together.