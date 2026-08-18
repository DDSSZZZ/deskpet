#include "button.h"
#include <Arduino.h>

const int buttonPin = 17;

const unsigned long DEBOUNCE_DELAY = 20;

bool buttonState = HIGH;         
bool lastReading = HIGH;         

unsigned long lastDebounceTime = 0;

bool buttonPressed = false;

void Button_Init()
{
    pinMode(buttonPin, INPUT_PULLUP);
}

void Button_Update()
{
    bool currentReading = digitalRead(buttonPin);


    if (currentReading != lastReading)
    {
        lastDebounceTime = millis();
    }


    if (millis() - lastDebounceTime >= DEBOUNCE_DELAY)
    {
       
        if (currentReading != buttonState)
        {
            buttonState = currentReading;

            if (buttonState == LOW)
            {
                buttonPressed = true;
            }
        }
    }

    lastReading = currentReading;
}

bool Button_IsPressed()
{
    return buttonPressed;
}

void Button_ClearPressed()
{
    buttonPressed = false;
}
