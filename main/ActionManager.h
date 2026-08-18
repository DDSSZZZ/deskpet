#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <Arduino.h>

// 动作编号
enum Action
{
    ACTION_NONE,      // 无动作
    ACTION_PLAY,      // 玩耍
    ACTION_WANDER,    // 闲逛
    ACTION_TIRED,     // 疲惫
    ACTION_FORWARD,   // 前进
    ACTION_SLEEP      // 睡眠
};

// 初始化
void ActionManager_Init();

// 每次loop都调用一次
void ActionManager_Update();

// 开始一个新动作（可打断当前动作）
void ActionManager_SetAction(Action action);

// 获取当前动作
Action ActionManager_GetAction();

#endif