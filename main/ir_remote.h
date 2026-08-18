#ifndef IR_REMOTE_H
#define IR_REMOTE_H

#include <Arduino.h>

void IR_Init();

void IR_Update();

void IR_Block(unsigned long time);

void IR_SendStudentId();

void IR_SetStudentId(const char *id);

#endif
