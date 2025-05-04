#include "f1.h"
#include "nextion.h"

int updateTacho(int rpm){
    set_value("tachometer", rpm);
    return 0;
}

int updateSpeedo(int spd){
    set_value("speedo", spd);
    return 0;
}

int updateOilTemp(int oilT){
    set_value("oilt", oilT);
    return 0;
}

int updateOilPressure(int oilP){
    set_value("oilp", oilP);
}

int updateTPS(int tps){
    set_value("tps", tps);
    return 0;
}

int updateAFR(float afr){
    set_value("afr", (int) (afr*100));
    return 0;
}

int updateCLT(float clt){
    set_value("clt", (int) clt);
    return 0;
}

int updateIAT(float iat){
    set_value("iat", (int) iat);
    return 0;
}

//Function to draw background [DEPRECATED]
/*int drawbg(){
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

    updateTPS(0);

    return 0;
}*/         