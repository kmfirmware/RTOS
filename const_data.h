#ifndef CONST_DATA_H
#define CONST_DATA_H
#include "global_decls.h"
const static LCD_SENTINAL LCD_Configure_Data[]={{.flag=COMMAND,  .data=DISCTL},
{.flag=DATA,.data=0x00}, //P1: 0x00=2 divisions, switching period=8 (default)
{.flag=DATA,.data=0x20}, //P2: 0x20=nlines/4 -1 =132/4-1 = 32)
{.flag=DATA,.data=0x00}, //P3: 0x00=no inversely highlighted lines
{.flag=COMMAND,  .data=COMSCN}, // COM Scan
{.flag=DATA,.data=0x01}, //P1: 0x01 = Scan 1->80, 160<-81
{.flag=COMMAND,  .data=OSCON},  // Internal oscillator ON
{.flag=COMMAND,  .data=SLPOUT}, //Sleep out
{.flag=COMMAND,  .data=PWRCTR}, //Power control
{.flag=DATA,  .data=0x0f},      // reference voltage regulator on, circuit voltage follower on, 
                               // BOOST ON
{.flag=COMMAND,  .data=DISINV},   // Inverse display
{.flag=COMMAND,  .data=DATCTL},   // Data Control
{.flag=DATA,  .data=0x00},      // P1: 0x0 = normal display of page/column address, 
                               // page scan direction
{.flag=DATA,  .data=0x00},      // normal RGB arrangement
{.flag=DATA,  .data=0x01},     // 8-bit grayscale
{.flag=COMMAND,  .data=VOLCTR},// Voltage control (contrast setting)
{.flag=DATA,  .data=32},       // P1 = 32 volume value (adjust this setting for your display 0 .. 63)
{.flag=DATA,  .data=3},      // P2 = 3 resistance ratio (determined by experiment)
{.flag=COMMAND,  .data=RGBSET8}, //setup 8-bit color lookup table  [RRRGGGBB]
{.flag=DATA,  .data=0},      //RED
{.flag=DATA,  .data=2},
{.flag=DATA,  .data=4},
{.flag=DATA,  .data=6},
{.flag=DATA,  .data=8},
{.flag=DATA,  .data=10},
{.flag=DATA,  .data=12},
{.flag=DATA,  .data=15},
{.flag=DATA,  .data=0},     //GREEN
{.flag=DATA,  .data=2},
{.flag=DATA,  .data=4},
{.flag=DATA,  .data=6},
{.flag=DATA,  .data=8},
{.flag=DATA,  .data=10},
{.flag=DATA,  .data=12},
{.flag=DATA,  .data=15},
{.flag=DATA,  .data=0},    // BLUE
{.flag=DATA,  .data=4},
{.flag=DATA,  .data=9},
{.flag=DATA,  .data=15}};


#endif