#include "debug.h"

#include <Arduino.h>

#include "pet.h"
#include "ActionManager.h"
#include "auto_level.h"

void Debug_Init()
{
}

void Debug_Print(const char *msg)
{
    if(Pet_IsSleeping())
        return;

    Serial.print("[");
    Serial.print(millis());
    Serial.print(" ms] ");

    Serial.println(msg);
}

void Debug_PrintMotion(int action)
{
    if(action == ACTION_NONE)
        return;

    if(Pet_IsSleeping())
        return;

    Serial.print("[");
    Serial.print(millis());
    Serial.print(" ms] ");

    Serial.print("Motion ");

    Serial.println(action);
}

void Debug_PrintStatus()
{
    if(Pet_IsSleeping())
        return;

    static unsigned long lastPrint = 0;

    if(millis() - lastPrint < 5000)
        return;

    lastPrint = millis();

    Serial.print("[");
    Serial.print(millis());
    Serial.print(" ms] ");

    Serial.print("EMOTION:");
    Serial.println(Pet_GetMood());

    if(Pet_IsFull())
    {
        int remain = Pet_GetFullRemain();

        Serial.print("FULL ");
        Serial.print(remain / 60);
        Serial.print("Min ");
        Serial.print(remain % 60);
        Serial.println("s");
    }
    else
    {
        Serial.println("HUNGRY");
    }

    Serial.print("ACTION:");

    switch(ActionManager_GetAction())
    {
        case ACTION_NONE:    Serial.println("NULL"); break;
        case ACTION_PLAY:    Serial.println("PLAY"); break;
        case ACTION_WANDER:  Serial.println("IDLE"); break;
        case ACTION_TIRED:   Serial.println("TIRE"); break;
        case ACTION_FORWARD: Serial.println("FOWD"); break;
        case ACTION_SLEEP:   Serial.println("SLEEP"); break;
    }

    Serial.println();
}
