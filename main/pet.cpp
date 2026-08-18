#include "pet.h"
#include "servo.h"
#include "button.h"
#include "light.h"
#include "ActionManager.h"
#include "encoder.h"
#include "debug.h"
#include "ir_remote.h"
#include "mpu.h"

// 使用main.cpp里的MPU对象
extern MPU6050 mpu;

int mood = 5;

bool full = false;

bool sleeping = false;

unsigned long moodTimer = 0;

unsigned long wanderTimer = 0;

unsigned long fullTimer = 0;

unsigned long autoActionTimer = 0;

unsigned long sleepstarttime = 0;

unsigned long lasttouch = 0;

void Pet_Init()
{
    mood = 5;
    sleeping = false;
    full = false;

    moodTimer = millis();
    autoActionTimer = millis();

    randomSeed(millis());
}

void Pet_Touch()
{
    if (sleeping)
        return;

    if (millis() - lasttouch < 10000 && lasttouch != 0)
        return;

    if (millis() < 500)
        return;

    lasttouch = millis();

    if (mood < 10)
        mood++;

    ActionManager_SetAction(ACTION_PLAY);

    Debug_Print("Head Touch");

    moodTimer = millis();
}

void Pet_Wakeup()
{
    sleeping = false;

    // 清除MPU累计状态
    mpu.reset();

    ActionManager_SetAction(ACTION_NONE);

    Serial.print("[");
    Serial.print(millis());
    Serial.println(" ms] Sleep Mode Off");

    Servo_SetLeft(90);
    Servo_SetRight(90);

    if (full)
    {
        fullTimer += millis() - sleepstarttime;
    }
}

void Pet_SetMood(int value)
{
    if (value < 1)
        value = 1;

    if (value > 10)
        value = 10;

    mood = value;
}

void Pet_SetFull(bool state)
{
    full = state;

    if (full)
    {
        Pet_Feed();
    }
    else
    {
        moodTimer = millis();
    }
}

void Pet_Feed()
{
    full = true;

    fullTimer = millis();

    moodTimer = millis();

    Debug_Print("Feed");
}

bool Pet_IsSleeping()
{
    return sleeping;
}

int Pet_GetMood()
{
    return mood;
}

bool Pet_IsFull()
{
    return full;
}

int Pet_GetFullRemain()
{
    long remain = 120000 - (millis() - fullTimer);

    return constrain(remain, 0, 120000) / 1000;
}
void Pet_EnterSleep()
{
    sleeping = true;

    // 清空MPU状态，睡眠期间不保留累计角度
    mpu.reset();

    sleepstarttime = millis();

    ActionManager_SetAction(ACTION_SLEEP);

    Light_ClearSleepRequest();

    Serial.print("[");
    Serial.print(millis());
    Serial.println(" ms] Sleep Mode On");

    Button_ClearPressed();
}

void Pet_Update()
{
    //-------------------------
    // 睡眠请求
    //-------------------------

    if (Light_SleepRequest() && !sleeping)
    {
        Pet_EnterSleep();
    }

    //-------------------------
    // 按键
    //-------------------------

    if (Button_IsPressed())
    {
        if (sleeping)
            Pet_Wakeup();
        else
            Pet_Touch();

        Button_ClearPressed();
    }

    //-------------------------
    // 喂食
    //-------------------------

    if (Encoder_FeedRequest())
    {
        Encoder_ClearFeedRequest();

        if (!sleeping)
        {
            Pet_Feed();
        }
    }

    //-------------------------
    // 睡眠优先级最高
    //-------------------------

    if (sleeping)
    {
        return;
    }

    //-------------------------
    // MPU害怕状态
    //-------------------------

    if (mpu.getState() == MPU_STATE_SCARED)
    {
        // 保持害怕动作，不执行其它动作


        return;
    }

    //-------------------------
    // 心情变化
    //-------------------------

    if (full)
    {
        if (millis() - fullTimer >= 120000)
        {
            full = false;
            moodTimer = millis();
        }
    }
    else
    {
        if (millis() - moodTimer >= 30000)
        {
            moodTimer = millis();

            if (mood > 1)
            {
                mood--;
            }
        }
    }

    //-------------------------
    // 随机动作
    //-------------------------

    if (ActionManager_GetAction() == ACTION_NONE)
    {
        if (millis() - autoActionTimer >= 1000)
        {
            autoActionTimer = millis();

            if (random(300) < 10)
            {
                if (mood >= 5)
                {
                    ActionManager_SetAction(ACTION_WANDER);
                }
                else
                {
                    ActionManager_SetAction(ACTION_TIRED);
                }
            }
        }
    }
}
