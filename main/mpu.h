#ifndef MPU_H
#define MPU_H

#include <Arduino.h>
#include <Wire.h>

#define MPU_ADDR            0x68

#define PWR_MGMT_1          0x6B
#define SMPLRT_DIV          0x19
#define CONFIG_REG          0x1A
#define GYRO_CONFIG         0x1B
#define ACCEL_CONFIG        0x1C

#define ACCEL_XOUT_H        0x3B
#define WHO_AM_I            0x75

//======================
// 参数（验收推荐）
//======================

// 一阶低通滤波
#define ALPHA_FILTER            0.20f

// 摇晃阈值(g)
#define SHAKE_THRESHOLD         0.35f

// 连续检测次数
#define SHAKE_COUNT_TRIGGER     3

// 陀螺仪死区(°/s)
#define GYRO_DEAD_ZONE          3.0f

// 累计旋转角度(°)
#define ROTATE_TRIGGER          90.0f

// 静止角速度(°/s)
#define STABLE_GYRO             5.0f

// 静止加速度(g)
#define STABLE_ACCEL            0.2f

// 恢复时间(ms)
#define STABLE_TIME_MS          10000

// 开启串口调试可取消注释
// #define MPU_DEBUG

enum MpuState
{
    MPU_STATE_NORMAL = 0,
    MPU_STATE_SCARED
};

class MPU6050
{
public:

    MPU6050();

    bool begin(
        int sdaPin = 21,
        int sclPin = 11
    );

    void update();

    

    MpuState getState() const;

    void reset();

    //-----------------------
    // 调试接口
    //-----------------------

    void getRaw(
        int16_t &outAx,
        int16_t &outAy,
        int16_t &outAz,
        int16_t &outGx,
        int16_t &outGy,
        int16_t &outGz
    );

    float getPitch() const;
    float getRoll() const;

    float getAccelMagnitude() const;

    float getGyroMagnitude() const;

    float getRotation() const;

private:

    bool wakeDevice();

    bool writeRegister(uint8_t reg, uint8_t value);

    bool readRegister(uint8_t reg, uint8_t &value);

    bool checkDevice();

    bool configureDevice();

    bool readRaw();

    void calibrateGyro();

    void updateFilter();

    void integrateGyro(float dt);

private:

    //-----------------------
    // 原始数据
    //-----------------------

    int16_t ax;
    int16_t ay;
    int16_t az;

    int16_t gx;
    int16_t gy;
    int16_t gz;

    //-----------------------
    // 滤波数据
    //-----------------------

    float filtAx;
    float filtAy;
    float filtAz;

    //-----------------------
    // 陀螺仪零偏
    //-----------------------

    float gxBias;
    float gyBias;
    float gzBias;

    //-----------------------
    // 当前角速度(°/s)
    //-----------------------

    float gyroX;
    float gyroY;
    float gyroZ;

    //-----------------------
    // 累计总旋转角(°)
    //-----------------------

    float totalRotation;

    //-----------------------
    // 时间
    //-----------------------

    unsigned long lastUpdate;

    unsigned long stableStart;

    //-----------------------
    // 状态
    //-----------------------

    MpuState state;

    bool calibrated;

    bool initialized;

    bool rotateTriggered;

    uint8_t shakeCounter;

    TwoWire *wire;
};

#endif
