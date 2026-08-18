#include "light.h"
#include <Arduino.h>

const int lightPin = 2;   

bool darktimeflag = false;
unsigned long darkStart = 0;
bool sleepTriggered = false;    

bool sleepRequest = false;

void Light_Init()
{
    pinMode(lightPin, INPUT);
}

bool Light_IsDark()
{
    return digitalRead(lightPin);
}

void Light_Update()
{
    if(Light_IsDark())
    {
       if(darktimeflag==false)
       {
        darkStart=millis();
        darktimeflag=true;
       }
       else
       {
        if(millis()-darkStart>5000&&!sleepTriggered)
        {
            sleepRequest=true;
            sleepTriggered = true;    
        }
       }
    }
    else
    {
        darktimeflag=false;
        sleepTriggered = false;
    }
}

bool Light_SleepRequest()
{
    return sleepRequest;
}

void Light_ClearSleepRequest()
{
    sleepRequest = false;
}
