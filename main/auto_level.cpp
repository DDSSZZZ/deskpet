#include "auto_level.h"

#include <Arduino.h>

#include "ActionManager.h"
#include "mpu.h"
#include "pet.h"
#include "servo.h"

extern MPU6050 mpu;

static constexpr unsigned long CONTROL_INTERVAL_MS = 20;
static constexpr unsigned long SERIAL_INTERVAL_MS = 2000;

static constexpr float TARGET_ANGLE = 0.0f;
static constexpr float KP = 1.5f;
static constexpr float KD = 0.12f;

static float zeroOffset = 0.0f;
static float lastError = 0.0f;
static float lastAngle = 0.0f;
static float lastOutput = 0.0f;
static int lastServoAngle = 90;

static bool calibrated = false;
static bool active = false;

static unsigned long lastControl = 0;
static unsigned long lastSerial = 0;

static bool canRunAutoLevel()
{
    return !Pet_IsSleeping() &&
           mpu.getState() != MPU_STATE_SCARED &&
           ActionManager_GetAction() == ACTION_NONE;
}

void AutoLevel_Init(bool mpuReady)
{
    calibrated = false;
    active = false;

    lastError = 0.0f;
    lastAngle = 0.0f;
    lastOutput = 0.0f;
    lastServoAngle = 90;
    lastControl = millis();
    lastSerial = 0;

    Serial.println();
    Serial.println("========== AUTO LEVEL ==========");

    Servo_SetLeft(90);
    Servo_SetRight(90);

    if(!mpuReady)
    {
        Serial.println("AutoLevel skipped: MPU not ready");
        return;
    }

    delay(1000);

    Serial.println("Calibrating AutoLevel... Keep Level");

    float sum = 0.0f;

    for(int i = 0; i < 200; i++)
    {
        mpu.update();
        sum += mpu.getPitch();
        delay(10);
    }

    zeroOffset = sum / 200.0f;
    calibrated = true;
    lastControl = millis();

    Serial.print("AutoLevel Zero Offset = ");
    Serial.println(zeroOffset, 3);

    Serial.println("AutoLevel Ready");
}

void AutoLevel_Update()
{
    unsigned long now = millis();

    if(!calibrated || !canRunAutoLevel())
    {
        active = false;
        lastError = 0.0f;
        lastOutput = 0.0f;
        lastControl = now;
        return;
    }

    if(now - lastControl < CONTROL_INTERVAL_MS)
        return;

    float dt = (now - lastControl) / 1000.0f;
    lastControl = now;

    if(dt <= 0.0f)
        dt = CONTROL_INTERVAL_MS / 1000.0f;

    float currentAngle = mpu.getPitch() - zeroOffset;
    float error = TARGET_ANGLE - currentAngle;

    if(fabsf(error) < 1.5f)
        error = 0.0f;

    float derivative = (error - lastError) / dt;
    float output = KP * error + KD * derivative;

    lastError = error;

    output = constrain(output, -45.0f, 45.0f);

    if(fabsf(output) < 1.5f)
        output = 0.0f;

    int leftServo = 90 - (int)output;
    int rightServo = 90 + (int)output;

    Servo_SetLeft(leftServo);
    Servo_SetRight(rightServo);

    lastAngle = currentAngle;
    lastOutput = output;
    lastServoAngle = rightServo;
    active = true;

    if(now - lastSerial >= SERIAL_INTERVAL_MS)
    {
        lastSerial = now;

        Serial.print("ANGLE: ");
        Serial.print(currentAngle, 2);

        Serial.print(" TARGET: ");
        Serial.print(TARGET_ANGLE, 2);

        Serial.print(" ERROR: ");
        Serial.print(error, 2);

        Serial.print(" OUT: ");
        Serial.print(output, 2);

        Serial.print(" SERVO:");
        Serial.println(rightServo);
    }
}

bool AutoLevel_IsActive()
{
    return active;
}

float AutoLevel_GetAngle()
{
    return lastAngle;
}

float AutoLevel_GetOutput()
{
    return lastOutput;
}

int AutoLevel_GetServoAngle()
{
    return lastServoAngle;
}
