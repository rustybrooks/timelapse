#include <Bounce.h>

const int b1in = 7;
const int b2in = 8;
const int ledPin = 13;

Bounce pushbutton1 = Bounce(b1in, 10);  // 10 ms debounce
Bounce pushbutton2 = Bounce(b2in, 10);  // 10 ms debounce


void setup() {
    pinMode(b1in, INPUT_PULLUP);
    pinMode(b2in, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);

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
        } else if (pushbutton1.risingEdge()) {  
            keyboard_keys[0] = 0;
            usb_keyboard_send();
        }
    }

    if (pushbutton2.update()) {
        if (pushbutton2.fallingEdge()) {  
            keyboard_keys[0] = KEY_B;
            usb_keyboard_send();
        } else if (pushbutton2.risingEdge()) {  
            keyboard_keys[0] = 0;
            usb_keyboard_send();
        }
    }
}

