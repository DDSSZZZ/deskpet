#ifndef PET_H
#define PET_H

#include <Arduino.h>

void Pet_Init();
void Pet_Feed();
void Pet_Update();
void Pet_Touch();          // 摸头
void Pet_Wakeup();
bool Pet_IsSleeping();
int Pet_GetMood();
bool Pet_IsFull();
int Pet_GetFullRemain();
void Pet_EnterSleep();
void Pet_SetMood(int mood);
void Pet_SetFull(bool full);

#endif
