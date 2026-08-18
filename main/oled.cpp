#include "oled.h"

#include <Arduino.h>
#include <U8g2lib.h>

#include "pet.h"
#include "ActionManager.h"
#include "mpu.h"
#include "auto_level.h"

extern MPU6050 mpu;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C
u8g2(U8G2_R0, U8X8_PIN_NONE, 9, 8);

void OLED_Init()
{
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x12_tf);
}

const char* ActionToString(Action action)
{
    switch(action)
    {
    case ACTION_NONE:
        return "NULL";

    case ACTION_PLAY:
        return "PLAY";

    case ACTION_WANDER:
        return "IDLE";

    case ACTION_TIRED:
        return "TIRE";

    case ACTION_FORWARD:
        return "FOWD";

    default:
        return "NULL";
    }
}

//==============================
// 害怕表情（右下角）
//==============================
void DrawScaredFace()
{
    // 左眼
    u8g2.drawCircle(104, 48, 2, U8G2_DRAW_ALL);

    // 右眼
    u8g2.drawCircle(120, 48, 2, U8G2_DRAW_ALL);

    // 嘴（横线）
    u8g2.drawLine(108, 57, 116, 57);

    // 左眉
    u8g2.drawLine(101, 42, 106, 44);

    // 右眉
    u8g2.drawLine(118, 44, 123, 42);
}

void OLED_Update()
{
    if(Pet_IsSleeping())
    {
        u8g2.clearBuffer();
        u8g2.sendBuffer();
        return;
    }

    u8g2.clearBuffer();

    char str[32];

    if(AutoLevel_IsActive()&&(AutoLevel_GetAngle()>5||AutoLevel_GetAngle()<-5))
    {
        u8g2.drawStr(0, 12, "AUTO LEVEL");

        snprintf(str, sizeof(str), "ANG:%.1f", AutoLevel_GetAngle());
        u8g2.drawStr(0, 32, str);

        snprintf(str, sizeof(str), "OUT:%.1f", AutoLevel_GetOutput());
        u8g2.drawStr(0, 52, str);

        u8g2.sendBuffer();
        return;
    }
+
    sprintf(str, "EMOTION:%d", Pet_GetMood());
    u8g2.drawStr(0, 12, str);

    if(Pet_IsFull())
    {
        int remain = Pet_GetFullRemain();

        sprintf(str, "FULL %dMin %ds", remain / 60, remain % 60);

        u8g2.drawStr(0, 28, str);
    }
    else
    {
        u8g2.drawStr(0, 28, "HUNGRY");
    }

    strcpy(str, ActionToString(ActionManager_GetAction()));

    u8g2.drawStr(0, 44, str);

    //==============================
    // MPU害怕表情
    //==============================
    if(mpu.getState() == MPU_STATE_SCARED)
    {
        DrawScaredFace();
    }

    u8g2.sendBuffer();
}
