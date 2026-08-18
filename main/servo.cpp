#include "servo.h"

#include <Arduino.h>

const int leftPin  = 6;
const int rightPin = 7;

#define SERVO_FREQ 50
#define SERVO_RES  12

static int lastLeft = -1;
static int lastRight = -1;

// 0.5ms~2.5ms
static inline uint32_t AngleToDuty(int angle)
{
    angle = constrain(angle, 0, 180);

    return map(angle, 0, 180, 102, 512);
}

void Servo_Init()
{
    bool ok1 = ledcAttach(leftPin, 50, 12);
    bool ok2 = ledcAttach(rightPin, 50, 12);

    Serial.print("Left=");
    Serial.println(ok1);

    Serial.print("Right=");
    Serial.println(ok2);

    delay(100);

    ledcWrite(leftPin, 307);
    ledcWrite(rightPin, 307);
}

void Servo_SetLeft(int angle)
{
    angle = constrain(angle,0,180);

    if(angle==lastLeft)
        return;

    lastLeft=angle;

    ledcWrite(leftPin,AngleToDuty(angle));
}

void Servo_SetRight(int angle)
{
    angle = constrain(angle,0,180);

    if(angle==lastRight)
        return;

    lastRight=angle;

    ledcWrite(rightPin,AngleToDuty(angle));
}