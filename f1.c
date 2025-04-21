#include "f1.h"
#include "nextion.h"

//Function to draw background
int drawbg(){
    
    /*
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
    */

    clear_screen(BLACK);

    draw_rect(0,0,800,480,WHITE); //Start by drawing outer border.

    draw_rect(0,0,275,100,WHITE); //Draw the lap time border.
    draw_rect(275,0,525,100,WHITE); //Draw the tachometer border.
    draw_rect(525,0,800,100,WHITE); //Draw the interval border.

    draw_rect(0,100,190,210,WHITE); //Draw the speedometer border.
    draw_rect(0,210,190, 320,WHITE); //Draw the lap border.
    draw_rect(0,320,190,430,WHITE); //Draw the SOC (State of Charge) border.

    draw_rect(610,100,800,210,WHITE); //Draw the brake balance border.
    draw_rect(610,210,800,320,WHITE); //Draw the pit limiter border.
    draw_rect(610,320,800,430,WHITE); //Draw the variable speed control border.
    
    draw_rect(190,100,290,210,WHITE); //LF Tire Temp Border.     
    draw_rect(190,210,290,320,WHITE); //LR Tire Temp Border.
    draw_rect(510,100,610,210,WHITE); //RF Tire Temp Border.
    draw_rect(510,210,610,320,WHITE); //RR Tire Temp Border.

    draw_rect(190,320,270,430,WHITE); //FUEL MIX Border.
    draw_rect(530,320,610,430,WHITE); //ERS Border
    return 0;
}