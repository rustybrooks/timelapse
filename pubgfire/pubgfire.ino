#include <Bounce.h>
#include <math.h>    


const int b1in = 7;
const int apin = 14;
const int zpsmax = 1000;

float e = 2.71828;
float mousez = 0;
int d = 10;
int p = 0;
int val = 0;
float slope1 = 1;
float slope2 = .3;
int cutoff = 1024;

Bounce pushbutton1 = Bounce(b1in, 10);  // 10 ms debounce


void setup() {
    pinMode(b1in, INPUT_PULLUP);

    keyboard_keys[0] = 0;
    keyboard_keys[1] = 0;
    keyboard_keys[2] = 0;
    keyboard_keys[3] = 0;
    keyboard_keys[4] = 0;
    keyboard_keys[5] = 0;
    usb_keyboard_send();
} 

void loop() {
    if (pushbutton1.update()) {
        if (pushbutton1.fallingEdge()) {  
            keyboard_keys[0] = KEY_A;
            usb_keyboard_send();
            val = analogRead(0);
            if (val < cutoff) {
                
            } else {
                val = 250*(1-slope2) + val*slope2;
            }
            mousez = zpsmax*val/1024;
            p = max(1, mousez / 100);
            d = max(1, (1000*p/mousez));                 
            Serial.print("val is "); Serial.println(val);
            Serial.print("rate is "); Serial.println(mousez);
            Serial.print("d = "); Serial.println(d);
            Serial.print("p = "); Serial.println(p);
        } else if (pushbutton1.risingEdge()) {  
            keyboard_keys[0] = 0;
            usb_keyboard_send();
            mousez = 0;
        }
    }

    if (mousez > 0.001) {
        Mouse.move(0, p);
        delay(max(0, d-2));
    }
}

