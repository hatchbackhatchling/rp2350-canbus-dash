#include "f1.h"
#include "nextion.h"

int updateTacho(int rpm){
    char rpmbuf[5]; //Create RPM buffer
    itoa(rpm, rpmbuf, 10);

    draw_rect(0,0,275,100,WHITE); //Draw the tachometer border.
    
    text_box(2,2,90,30,PX24,WHITE,BLACK,HORIZONTAL_LEFT,VERTICAL_TOP,STA_NONE,"RPM"); //Tachometer Text
    fill(40,25,150,50,BLACK); //Fill before refresh
    text_box(40,25,150,50,PX48,WHITE,BLACK,HORIZONTAL_RIGHT,VERTICAL_CENTER,STA_NONE,rpmbuf); //Draw the actual tachometer
    return 0;
}

int updateSpeedo(int spd){
    char spdbuf[3]; //Create speed buffer
    itoa(spd, spdbuf, 10);

    draw_rect(525,0,800,100,WHITE); //Draw the speedo border.
    text_box(527,2,200,30,PX16,WHITE,BLACK,HORIZONTAL_LEFT,VERTICAL_TOP,STA_NONE,"Speedometer (km/h)"); //Speedometer Text
    fill(560,25,150,50,BLACK); //Fill before refresh
    text_box(560,25,150,50,PX64,WHITE,BLACK,HORIZONTAL_RIGHT,VERTICAL_CENTER,STA_NONE,spdbuf); //Draw the actual speedometer
    return 0;
}

int updateOilTemp(int oilT){
    char tbuf[3]; //Create Buffer
    itoa(oilT, tbuf, 10);
    draw_rect(0,100,190,210,WHITE); //Draw the oilT border.
    text_box(2,102,190,210,PX24,WHITE,BLACK,HORIZONTAL_LEFT,VERTICAL_TOP,STA_NONE,"OIL TEMP °C"); //Oil Temp Text
    fill(30,135,150,50,BLACK); //Fill before refresh
    text_box(30,135,150,50,PX56,WHITE,BLACK,HORIZONTAL_RIGHT,VERTICAL_CENTER,STA_NONE,tbuf);
}

int updateOilPressure(int oilP){
    char pbuf[3]; //Create Buffer
    itoa(oilP, pbuf, 10);
    draw_rect(0,210,190, 320,WHITE); //Draw the oilP border.
    text_box(2,212,190,210,PX24,WHITE,BLACK,HORIZONTAL_LEFT,VERTICAL_TOP,STA_NONE,"OIL PRESS. (Bar)"); //Oil Pressure Text
    fill(30,245,150,50,BLACK); //Fill before refresh
    text_box(30,245,150,50,PX56,WHITE,BLACK,HORIZONTAL_RIGHT,VERTICAL_CENTER,STA_NONE,pbuf);
}

int updateAFR(float afr){
    char buf[4]; //Create buffer
    sprintf(buf, "%g", afr);
    draw_rect(0,320,190,430,WHITE); //Draw the AFR border.
    text_box(2,212,190,210,PX24,WHITE,BLACK,HORIZONTAL_LEFT,VERTICAL_TOP,STA_NONE,"Air-Fuel Ratio"); //Oil Pressure Text
    fill(30,245,150,50,BLACK); //Fill before refresh
    text_box(30,245,150,50,PX56,WHITE,BLACK,HORIZONTAL_RIGHT,VERTICAL_CENTER,STA_NONE,buf);    
}

int updateTPS(int tps){
    draw_rect(0,430,800,480,WHITE);
    fill(0,432,800,478,BLACK);
    fill(0,432,tps*8,48,GREEN);
}

//Function to draw background
int drawbg(){

    /*
        cls 0
        draw 0,0,800,480,WHITE
        draw 0,0,275,100,WHITE 
        draw 275,0,525,100,WHITE
        draw 525,0,800,100,WHITE

        draw 0,100,190,210,WHITE
        draw 0,210,190,320,WHITE
        draw 0,320,190,430,WHITE
        draw 610,100,800,210,WHITE
        draw 610,210,800,320,WHITE
        draw 610,320,800,430,WHITE
            
        draw 190,100,290,210,WHITE   
        draw 190,210,290,320,WHITE
        draw 510,100,610,210,WHITE
        draw 510,210,610,320,WHITE

        draw 190,320,270,430,WHITE
        draw 530,320,610,430,WHITE
        draw 270,320,530,430,WHITE
    */

    clear_screen(BLACK);

    draw_rect(0,0,800,480,WHITE); //Start by drawing outer border.

    updateTacho(0);
    draw_rect(275,0,525,100,WHITE); //Draw the BLANK border.
    updateSpeedo(0);

    updateOilTemp(0);
    updateOilPressure(0);
    updateAFR(0.0f);

    draw_rect(610,100,800,210,WHITE); //Draw the CLT border.
    draw_rect(610,210,800,320,WHITE); //Draw the IAT border.
    draw_rect(610,320,800,430,WHITE); //Draw the Fuel border.
    
    draw_rect(190,100,290,210,WHITE); //LF Tire Temp Border.     
    draw_rect(190,210,290,320,WHITE); //LR Tire Temp Border.
    draw_rect(510,100,610,210,WHITE); //RF Tire Temp Border.
    draw_rect(510,210,610,320,WHITE); //RR Tire Temp Border.

    draw_rect(190,320,270,430,WHITE); //N/A Border.
    draw_rect(530,320,610,430,WHITE); //N/A Border

    draw_rect(270,320,530,430,WHITE); //N/A Border

    updateTPS(0);

    return 0;
}