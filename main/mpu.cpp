#include "mpu.h"

static constexpr float ACC_SCALE = 16384.0f;
static constexpr float GYRO_SCALE = 131.0f;

#if defined(ARDUINO_ARCH_ESP32)
static TwoWire mpuWire(1);
#else
#define mpuWire Wire
#endif

MPU6050::MPU6050()
{
    ax = ay = az = 0;
    gx = gy = gz = 0;

    filtAx = 0.0f;
    filtAy = 0.0f;
    filtAz = 0.0f;

    gxBias = 0.0f;
    gyBias = 0.0f;
    gzBias = 0.0f;

    gyroX = 0.0f;
    gyroY = 0.0f;
    gyroZ = 0.0f;

    totalRotation = 0.0f;

    lastUpdate = 0;
    stableStart = 0;

    state = MPU_STATE_NORMAL;

    calibrated = false;
    initialized = false;
    rotateTriggered = false;

    shakeCounter = 0;

    wire = &mpuWire;
}

bool MPU6050::begin(int sdaPin,int sclPin)
{
#if defined(ARDUINO_ARCH_ESP32)
    wire = &mpuWire;
#else
    wire = &Wire;
#endif

    if(sdaPin >= 0 && sclPin >= 0)
        wire->begin(sdaPin,sclPin);
    else
        wire->begin();

    wire->setClock(100000);
    delay(100);

    calibrated = false;
    initialized = false;

    if(!checkDevice() || !wakeDevice() || !configureDevice())
    {
        Serial.println("MPU6050 Init Failed on GPIO SDA=21 SCL=19");
        return false;
    }

    calibrateGyro();

    if(!readRaw())
    {
        Serial.println("MPU6050 Read Failed");
        return false;
    }

    filtAx = ax;
    filtAy = ay;
    filtAz = az;

    lastUpdate = millis();
    stableStart = 0;
    totalRotation = 0.0f;
    rotateTriggered = false;
    shakeCounter = 0;
    state = MPU_STATE_NORMAL;
    initialized = true;

    Serial.println("MPU6050 Ready");

    return true;
}

bool MPU6050::writeRegister(uint8_t reg, uint8_t value)
{
    wire->beginTransmission(MPU_ADDR);
    wire->write(reg);
    wire->write(value);

    return wire->endTransmission() == 0;
}

bool MPU6050::readRegister(uint8_t reg, uint8_t &value)
{
    wire->beginTransmission(MPU_ADDR);
    wire->write(reg);

    if(wire->endTransmission(false) != 0)
        return false;

    if(wire->requestFrom(MPU_ADDR, (uint8_t)1, true) != 1)
        return false;

    value = wire->read();

    return true;
}

bool MPU6050::checkDevice()
{
    uint8_t who = 0;

    if(!readRegister(WHO_AM_I, who))
        return false;

    Serial.print("MPU6050 WHO_AM_I=0x");
    Serial.println(who, HEX);

    return who == 0x68 || who == 0x69 || who == 0x70;
}

bool MPU6050::wakeDevice()
{
    writeRegister(PWR_MGMT_1, 0x80);
    delay(100);

    if(!writeRegister(PWR_MGMT_1, 0x01))
        return false;

    delay(100);

    return true;
}

bool MPU6050::configureDevice()
{
    // Sample Rate = 1kHz/(7+1)=125Hz
    if(!writeRegister(SMPLRT_DIV, 0x07))
        return false;

    // DLPF
    if(!writeRegister(CONFIG_REG, 0x06))
        return false;

    // +/-250 deg/s
    if(!writeRegister(GYRO_CONFIG, 0x00))
        return false;

    // +/-2g
    if(!writeRegister(ACCEL_CONFIG, 0x00))
        return false;

    return true;
}

void MPU6050::calibrateGyro()
{
    Serial.println("Calibrating Gyro...");

    float sx = 0;
    float sy = 0;
    float sz = 0;
    int count = 0;

    const int SAMPLE = 500;

    for(int i=0;i<SAMPLE;i++)
    {
        if(readRaw())
        {
            sx += gx;
            sy += gy;
            sz += gz;
            count++;
        }

        delay(2);
    }

    if(count > 0)
    {
        gxBias = sx / count;
        gyBias = sy / count;
        gzBias = sz / count;
        calibrated = true;
    }

    Serial.println("Calibration OK");
}

bool MPU6050::readRaw()
{
    wire->beginTransmission(MPU_ADDR);
    wire->write(ACCEL_XOUT_H);

    if(wire->endTransmission(false)!=0)
        return false;

    if(wire->requestFrom(MPU_ADDR,(uint8_t)14,true)!=14)
        return false;

    ax = (int16_t)(wire->read()<<8 | wire->read());
    ay = (int16_t)(wire->read()<<8 | wire->read());
    az = (int16_t)(wire->read()<<8 | wire->read());

    wire->read();
    wire->read();

    gx = (int16_t)(wire->read()<<8 | wire->read());
    gy = (int16_t)(wire->read()<<8 | wire->read());
    gz = (int16_t)(wire->read()<<8 | wire->read());

    // 丢弃明显错误的数据
    if(abs(gx)>=32760 ||
       abs(gy)>=32760 ||
       abs(gz)>=32760)
    {
        return false;
    }

    return true;
}

void MPU6050::updateFilter()
{
    filtAx = filtAx*(1.0f-ALPHA_FILTER) + ax*ALPHA_FILTER;
    filtAy = filtAy*(1.0f-ALPHA_FILTER) + ay*ALPHA_FILTER;
    filtAz = filtAz*(1.0f-ALPHA_FILTER) + az*ALPHA_FILTER;
}

float MPU6050::getAccelMagnitude() const
{
    float x = filtAx / ACC_SCALE;
    float y = filtAy / ACC_SCALE;
    float z = filtAz / ACC_SCALE;

    return sqrtf(
        x*x +
        y*y +
        z*z
    );
}

float MPU6050::getGyroMagnitude() const
{
    return sqrtf(
        gyroX*gyroX +
        gyroY*gyroY +
        gyroZ*gyroZ
    );
}

float MPU6050::getRotation() const
{
    return totalRotation;
}

void MPU6050::integrateGyro(float dt)
{
    if(!calibrated)
        return;

    gyroX = (gx - gxBias) / GYRO_SCALE;
    gyroY = (gy - gyBias) / GYRO_SCALE;
    gyroZ = (gz - gzBias) / GYRO_SCALE;

    if(fabsf(gyroX) < GYRO_DEAD_ZONE)
        gyroX = 0.0f;

    if(fabsf(gyroY) < GYRO_DEAD_ZONE)
        gyroY = 0.0f;

    if(fabsf(gyroZ) < GYRO_DEAD_ZONE)
        gyroZ = 0.0f;

    float gyroMagnitude = sqrtf(
        gyroX * gyroX +
        gyroY * gyroY +
        gyroZ * gyroZ
    );

    totalRotation += gyroMagnitude * dt;

    // 静止时较快衰减，抑制小幅日常移动造成的累计漂移
    if(gyroMagnitude < STABLE_GYRO)
    {
        totalRotation *= 0.98f;

        if(totalRotation < 2.0f)
            totalRotation = 0.0f;
    }
}

void MPU6050::update()
{
    if(!initialized)
    {
        static unsigned long lastRetry = 0;

        if(millis() - lastRetry >= 2000)
        {
            lastRetry = millis();
            begin();
        }

        return;
    }

    unsigned long now = millis();

    float dt = (now - lastUpdate) / 1000.0f;

    if(dt <= 0.0f || dt > 0.05f)
        dt = 0.01f;

    lastUpdate = now;

    if(!readRaw())
        return;

    updateFilter();

    integrateGyro(dt);

    //-------------------------
    // 摇晃检测
    //-------------------------

    float acc = getAccelMagnitude();

    float accDiff = fabsf(acc - 1.0f);

    bool shake = false;

    if(accDiff > SHAKE_THRESHOLD)
    {
        if(shakeCounter < 6)
            shakeCounter++;
    }
    else
    {
        if(shakeCounter > 0)
            shakeCounter--;
    }

    if(shakeCounter >= SHAKE_COUNT_TRIGGER)
    {
        shake = true;
        shakeCounter = 0;
    }

    //-------------------------
    // 旋转检测
    //-------------------------

    bool rotate = false;

    if(!rotateTriggered &&
       totalRotation >= ROTATE_TRIGGER)
    {
        rotate = true;
        rotateTriggered = true;
    }

    //-------------------------
    // 状态机
    //-------------------------

    if(state == MPU_STATE_NORMAL)
    {
        if(shake || rotate)
        {
            state = MPU_STATE_SCARED;

            // 清空累计角度，避免再次立即触发
            totalRotation = 0.0f;

            stableStart = 0;

            Serial.println("MPU -> SCARED");
        }
    }
    else
    {
        bool stable =
            (getGyroMagnitude() < STABLE_GYRO) &&
            (accDiff < STABLE_ACCEL);

        if(stable)
        {
            if(stableStart == 0)
            {
                stableStart = now;
            }
            else if(now - stableStart >= STABLE_TIME_MS)
            {
                reset();
                

                Serial.println("MPU -> NORMAL");
            }
        }
        else
        {
            stableStart = 0;
        }
    }

#ifdef MPU_DEBUG

    Serial.print("ACC:");
    Serial.print(ax);
    Serial.print(",");
    Serial.print(ay);
    Serial.print(",");
    Serial.print(az);

    Serial.print("   GYRO:");
    Serial.print(gx);
    Serial.print(",");
    Serial.print(gy);
    Serial.print(",");
    Serial.print(gz);

    Serial.print("   ROT:");
    Serial.print(totalRotation,1);

    Serial.print("   STATE:");

    if(state == MPU_STATE_NORMAL)
        Serial.println("NORMAL");
    else
        Serial.println("SCARED");

#endif
}

MpuState MPU6050::getState() const
{
    return state;
}

void MPU6050::reset()
{
    state = MPU_STATE_NORMAL;

    totalRotation = 0.0f;

    shakeCounter = 0;

    rotateTriggered = false;

    stableStart = 0;
}

void MPU6050::getRaw(
    int16_t &outAx,
    int16_t &outAy,
    int16_t &outAz,
    int16_t &outGx,
    int16_t &outGy,
    int16_t &outGz
)
{
    outAx = ax;
    outAy = ay;
    outAz = az;

    outGx = gx;
    outGy = gy;
    outGz = gz;
}

float MPU6050::getPitch() const
{
    float axg = filtAx / ACC_SCALE;
    float ayg = filtAy / ACC_SCALE;
    float azg = filtAz / ACC_SCALE;

    return atan2f(
        azg,
        sqrtf(axg * axg + ayg * ayg)
    ) * 180.0f / PI;
}

float MPU6050::getRoll() const
{
    float axg = filtAx / ACC_SCALE;
    float ayg = filtAy / ACC_SCALE;
    float azg = filtAz / ACC_SCALE;

    return atan2f(
        ayg,
        sqrtf(axg * axg + azg * azg)
    ) * 180.0f / PI;
}
