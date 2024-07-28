#ifndef PINS_H
#define PINS_H

#include "Arduino.h"
namespace Pins{
    struct Quad{
        int a;
        int b;
    };
    Quad quad1{A1,A0};
    Quad quad2{A3,A2};

    struct {
        int p1=1;
        int p2=0;
        int p3=A5;
        int p4=A4;
        int p5=6; //Solder Jumpered IO
        int p6=5; //Solder jumpered IO
        // int p7=not assigned to board;
    } pnuematics ;

    struct {
        int m1=12;
        int m2=11;
        int m3=10;
        int m4=9;
        int m5=6; //Solder Jumpered IO
        int m6=5; //Solder jumpered IO
    } motors ;

    int led = LED_BUILTIN;
}

#endif