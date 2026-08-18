#ifndef LIGHT_H
#define LIGHT_H

void Light_Init();

bool Light_IsDark();

void Light_Update();

bool Light_SleepRequest();
void Light_ClearSleepRequest();

#endif
