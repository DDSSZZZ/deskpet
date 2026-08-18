#include "ir_remote.h"
#include "mpu.h"
#define IR_SEND_PIN 4
#include <IRremote.hpp>

#include <string.h>

#include "pet.h"

#define IR_RECV_PIN 5

#ifndef IR_STUDENT_ID
#define IR_STUDENT_ID "2025010277"
#endif


// 舵机动作屏蔽时间
static unsigned long irBlockUntil = 0;

// 上电后的接收屏蔽时间，避开接收头上电毛刺
#define IR_BOOT_BLOCK_MS 1500

// 两次触发最小间隔
#define IR_INTERVAL 1500

// UNKNOWN 协议遥控器至少要满足的原始脉冲数量/总时长
#define IR_MIN_RAW_LEN 24
#define IR_MIN_RAW_US 8000UL

// 心情过低时自主发射的间隔
#define IR_LOW_MOOD_LEVEL 2
#define IR_LOW_MOOD_SEND_INTERVAL 15000UL
#define IR_NEC_ADDRESS 0x00

// 上次触发时间
static unsigned long lastIRTime = 0;

static unsigned long lastAutoSendTime = 0;

static char studentId[24] = IR_STUDENT_ID;

extern MPU6050 mpu;

// 原始数据总时长（微秒）
static unsigned long rawDurationMicros()
{
    unsigned long total = 0;

    if (IrReceiver.irparams.rawlen == 0)
        return 0;

    for (uint_fast16_t i = 1; i < IrReceiver.irparams.rawlen; i++)
    {
        total += IrReceiver.irparams.rawbuf[i] * MICROS_PER_TICK;
    }

    return total;
}

// 判断接收到的帧是否可用
static bool isUsableReceiveFrame()
{
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)
        return false;

    decode_type_t protocol = IrReceiver.decodedIRData.protocol;
    uint16_t bits = IrReceiver.decodedIRData.numberOfBits;
    uint16_t rawLen = IrReceiver.irparams.rawlen;

    if (protocol == UNKNOWN)
    {
        return rawLen >= IR_MIN_RAW_LEN && rawDurationMicros() >= IR_MIN_RAW_US;
    }


    return bits >= 8;
}

//========================
// 初始化
//========================

void IR_Init()
{
    pinMode(IR_RECV_PIN, INPUT_PULLUP);

    IrReceiver.begin(
        IR_RECV_PIN,
        DISABLE_LED_FEEDBACK
    );

    // 修改点：使用无参 begin()，库会使用 IR_SEND_PIN
    IrSender.begin();

    IR_Block(IR_BOOT_BLOCK_MS);

    Serial.println("IR Init OK");
}

//========================
// 屏蔽红外
//========================

void IR_Block(unsigned long time)
{
    irBlockUntil = millis() + time;
}

//========================
// 红外更新
//========================

void IR_Update()
{
    unsigned long now = millis();

    if(!Pet_IsSleeping() &&
       Pet_GetMood() <= IR_LOW_MOOD_LEVEL &&
       now - lastAutoSendTime >= IR_LOW_MOOD_SEND_INTERVAL)
    {
        IR_SendStudentId();
        lastAutoSendTime = millis();
        IR_Block(300);
    }

    /*
        舵机动作期间忽略红外
    */

    if(now < irBlockUntil)
    {

        if(IrReceiver.decode())
        {
            IrReceiver.resume();
        }

        return;
    }

    if (!isUsableReceiveFrame())
        {
            // 噪声信号：短暂屏蔽，避免后续毛刺干扰
            IR_Block(100);  // 屏蔽 100ms
            IrReceiver.resume();
            return;
        }

    if(IrReceiver.decode())
    {
        //========================
        // 调试输出
        //========================

        if((uint32_t)IrReceiver.decodedIRData.decodedRawData != 0xD7E4F398)
        {
           IrReceiver.resume();
           return;
        }

        Serial.print("[IR] ");

        Serial.print("Protocol=");
        Serial.print(getProtocolString(IrReceiver.decodedIRData.protocol));

        Serial.print("  Bits=");
        Serial.print(IrReceiver.decodedIRData.numberOfBits);

        Serial.print("  Flags=0x");
        Serial.print(IrReceiver.decodedIRData.flags, HEX);

        Serial.print("  Addr=0x");
        Serial.print(IrReceiver.decodedIRData.address, HEX);

        Serial.print("  Cmd=0x");
        Serial.print(IrReceiver.decodedIRData.command, HEX);

        Serial.print("  Raw=0x");

#if (__SIZEOF_LONG_LONG__ == 8)
        Serial.println((uint64_t)IrReceiver.decodedIRData.decodedRawData, HEX);
#else
        Serial.println((uint32_t)IrReceiver.decodedIRData.decodedRawData, HEX);
#endif

        /*
            过滤上电毛刺、短脉冲和重复码。
            有些遥控器会被识别为 UNKNOWN，所以不能只按协议名过滤。
        */

        if(!isUsableReceiveFrame())
        {
            IrReceiver.resume();
            return;
        }

        /*
            防止连续触发
        */

        if(now - lastIRTime < IR_INTERVAL)
        {
            IrReceiver.resume();
            return;
        }
        if(mpu.getState() == MPU_STATE_SCARED)
        {
            IrReceiver.resume( );
            return;
        }
        Serial.println("IR Touch");

        // 红外 = 摸头
        Pet_Touch();

        lastIRTime = now;

        IrReceiver.resume();
    }
}

void IR_SendStudentId()
{
    Serial.print("[IR SEND] ");
    Serial.println(studentId);

    for(uint8_t i = 0; studentId[i] != '\0'; i++)
    {
        IrSender.sendNEC(
            IR_NEC_ADDRESS,
            (uint8_t)studentId[i],
            0
        );

        delay(110);
    }
}

void IR_SetStudentId(const char *id)
{
    if(id == nullptr || id[0] == '\0')
        return;

    size_t len = strlen(id);

    if(len >= sizeof(studentId))
        len = sizeof(studentId) - 1;

    memcpy(studentId, id, len);
    studentId[len] = '\0';

    Serial.print("[IR ID] ");
    Serial.println(studentId);
}