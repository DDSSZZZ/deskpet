#include "serialcmd.h"

#include <Arduino.h>

#include "pet.h"
#include "ActionManager.h"
#include "ir_remote.h"

void SerialCmd_Init()
{

}

void SerialCmd_Update()
{
    if (!Serial.available())
        return;

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    // 睡眠状态下只允许 setslp false
    if (Pet_IsSleeping())
    {
        if (cmd == "setslp false")
        {
            Pet_Wakeup();
        }
        return;
    }

    // ---------------- setemo ----------------
    if (cmd.startsWith("setemo "))
    {
        int mood = cmd.substring(7).toInt();
        Pet_SetMood(mood);
        return;
    }

    // ---------------- setful ----------------
    if (cmd == "setful true")
    {
        Pet_SetFull(true);
        return;
    }

    if (cmd == "setful false")
    {
        Pet_SetFull(false);
        return;
    }

    // ---------------- ir ----------------
    if (cmd.startsWith("setir "))
    {
        IR_SetStudentId(cmd.substring(6).c_str());
        return;
    }

    if (cmd == "sendir")
    {
        IR_SendStudentId();
        return;
    }

    // ---------------- setmot ----------------
    if (cmd == "setmot 0")
    {
        ActionManager_SetAction(ACTION_NONE);
        return;
    }
    if (cmd == "setmot 1")
    {
        ActionManager_SetAction(ACTION_PLAY);
        return;
    }

    if (cmd == "setmot 2")
    {
        ActionManager_SetAction(ACTION_WANDER);
        return;
    }

    if (cmd == "setmot 3")
    {
        ActionManager_SetAction(ACTION_TIRED);
        return;
    }

    if (cmd == "setmot 4")
    {
        ActionManager_SetAction(ACTION_FORWARD);
        return;
    }

    // ---------------- setslp ----------------
    if (cmd == "setslp true")
    {
        Pet_EnterSleep();
        return;
    }

    if (cmd == "setslp false")
    {
        Pet_Wakeup();
        return;
    }

    Serial.println("Unknown Command");
}
