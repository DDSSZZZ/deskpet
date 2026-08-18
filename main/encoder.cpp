#include "encoder.h"
#include <Arduino.h>

const int clkPin = 13;
const int dtPin  = 14;

const unsigned long DEBOUNCE_DELAY = 5;     // 建议5ms
const unsigned long STEP_TIMEOUT   = 600;   // 放宽一点

static bool feedRequest = false;

static int stepCount = 0;
static int reverseCount = 0;

static int lastCLK;

static unsigned long lastDebounceTime = 0;
static unsigned long lastStepTime = 0;

void Encoder_Init()
{
    pinMode(clkPin, INPUT_PULLUP);
    pinMode(dtPin, INPUT_PULLUP);

    lastCLK = digitalRead(clkPin);
}

void Encoder_Update()
{
    int currentCLK = digitalRead(clkPin);

    // 超时重新开始
    if(stepCount > 0 && millis() - lastStepTime > STEP_TIMEOUT)
    {
        stepCount = 0;
        reverseCount = 0;
    }

    // 只检测CLK下降沿
    if(lastCLK == HIGH &&
       currentCLK == LOW &&
       millis() - lastDebounceTime > DEBOUNCE_DELAY)
    {
        lastDebounceTime = millis();
        lastStepTime = millis();

        // 读取DT判断方向
        bool clockwise = digitalRead(dtPin);

        if(clockwise)
        {
            reverseCount = 0;

            stepCount++;

            if(stepCount >= 15)
            {
                feedRequest = true;
                stepCount = 0;

            }
        }
        else
        {
            reverseCount++;

            if(reverseCount >= 3)     // 连续3次才认为真的反转
            {
                reverseCount = 0;
                stepCount = 0;
            }
        }
    }

    lastCLK = currentCLK;
}

bool Encoder_FeedRequest()
{
    return feedRequest;
}

void Encoder_ClearFeedRequest()
{
    feedRequest = false;
}