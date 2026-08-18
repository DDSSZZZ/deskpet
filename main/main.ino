#include <Arduino.h>
#include "ir_remote.h"
#include "button.h"
#include "light.h"
#include "encoder.h"
#include "servo.h"
#include "ActionManager.h"
#include "pet.h"
#include "oled.h"
#include "serialcmd.h"
#include "debug.h"
#include "mpu.h"
#include "auto_level.h"

//==============================
// 全局对象
//==============================

MPU6050 mpu;

void setup()
{
    Serial.begin(115200);

    Button_Init();
    Light_Init();
    Encoder_Init();

    OLED_Init();

    Servo_Init();

    ActionManager_Init();
    Pet_Init();

    bool mpuReady = mpu.begin();    // MPU初始化

    AutoLevel_Init(mpuReady);

    IR_Init();             // 最后初始化

    SerialCmd_Init();
    Debug_Init();
}

void loop()
{
    //==============================
    // 输入
    //==============================

    Light_Update();

    Button_Update();

    IR_Update();

    if (!Pet_IsSleeping())
    {
        Encoder_Update();
        mpu.update();
    }

    SerialCmd_Update();

    //==============================
    // 系统逻辑
    //==============================

    Pet_Update();

    ActionManager_Update();

    AutoLevel_Update();

    //==============================
    // 输出
    //==============================

    OLED_Update();

    Debug_PrintStatus();

    delay(2);
}
