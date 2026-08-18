#ifndef AUTO_LEVEL_H
#define AUTO_LEVEL_H

#include <Arduino.h>

void AutoLevel_Init(bool mpuReady);
void AutoLevel_Update();

bool AutoLevel_IsActive();
float AutoLevel_GetAngle();
float AutoLevel_GetOutput();
int AutoLevel_GetServoAngle();

#endif
