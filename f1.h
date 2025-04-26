#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/uart.h"
#include "nextion.h"

//Font definitions (this dash uses Eurostile)
#define PX64 0
#define PX56 1
#define PX48 2
#define PX40 3
#define PX32 4
#define PX24 5
#define PX16 6

int drawbg();

//Component-based drawing
int updateTacho(int rpm);
int updateSpeedo(int spd);
int updateOilTemp(int oilT);