#include "ActionManager.h"
#include "servo.h"
#include "ir_remote.h"      // 新增
#include <Arduino.h>
#include "debug.h"


//========================
// 私有变量（只有本文件能访问）
//========================

static Action currentAction = ACTION_NONE;

static unsigned long actionStartTime = 0;

static unsigned long actionDuration = 0;



//========================
// 初始化
//========================

void ActionManager_Init()
{
    ActionManager_SetAction(ACTION_NONE);

    Servo_SetLeft(90);
    Servo_SetRight(90);
}



//========================
// 设置动作
//========================

void ActionManager_SetAction(Action action)
{
    currentAction = action;

    actionStartTime = millis();


    /*
        动作开始时屏蔽红外

        防止舵机启动瞬间产生干扰
    */

    if(action != ACTION_NONE)
    {
        IR_Block(4200);
    }


    Debug_PrintMotion(action);
}

//========================
// 设置动作
//========================


Action ActionManager_GetAction()
{
    return currentAction;
}

void ActionManager_Update()
{
    unsigned long now=millis();
    if (currentAction==ACTION_PLAY)
    {
        if(now-actionStartTime<=250)
        {
            Servo_SetLeft(90-45*(now-actionStartTime)/250);
            Servo_SetRight(90-45*(now-actionStartTime)/250);
        }
        else if(250<=now-actionStartTime&&now-actionStartTime<=750)
        {
            Servo_SetLeft(45+90*(now-actionStartTime-250)/500);
            Servo_SetRight(45+90*(now-actionStartTime-250)/500);
        }
        else if(750<=now-actionStartTime&&now-actionStartTime<=1250)
        {
            Servo_SetLeft(135-90*(now-actionStartTime-750)/500);
            Servo_SetRight(135-90*(now-actionStartTime-750)/500);
        }
        else if(1250<=now-actionStartTime&&now-actionStartTime<=1750)
        {
            Servo_SetLeft(45+90*(now-actionStartTime-1250)/500);
            Servo_SetRight(45+90*(now-actionStartTime-1250)/500);
        }
        else if(1750<=now-actionStartTime&&now-actionStartTime<=2000)
        {
            Servo_SetLeft(135-45*(now-actionStartTime-1750)/250);
            Servo_SetRight(135-45*(now-actionStartTime-1750)/250);
        }
        else if(now-actionStartTime>2000)
        {
            currentAction=ACTION_NONE;
        }
    }
    else if(currentAction==ACTION_WANDER)
    {
        if(now-actionStartTime<=666)
        {
            Servo_SetLeft(90-60*(now-actionStartTime)/666);
            Servo_SetRight(90-60*(now-actionStartTime)/666);
        }
        else if(666<=now-actionStartTime&&now-actionStartTime<=1333)
        {
            Servo_SetLeft(30+60*(now-actionStartTime-666)/667);
            Servo_SetRight(30+60*(now-actionStartTime-666)/667);
        }
        else if(1333<=now-actionStartTime&&now-actionStartTime<=2000)
        {
            Servo_SetLeft(90-60*(now-actionStartTime-1333)/667);
            Servo_SetRight(90-60*(now-actionStartTime-1333)/667);
        }
        else if(2000<=now-actionStartTime&&now-actionStartTime<=2667)
        {
            Servo_SetLeft(30+60*(now-actionStartTime-2000)/667);
            Servo_SetRight(30+60*(now-actionStartTime-2000)/667);
        }
        else if(2667<=now-actionStartTime&&now-actionStartTime<=3333)
        {
            Servo_SetLeft(90-60*(now-actionStartTime-2667)/666);
            Servo_SetRight(90-60*(now-actionStartTime-2667)/666);
        }
        else if(3333<=now-actionStartTime&&now-actionStartTime<=4000)
        {
            Servo_SetLeft(30+60*(now-actionStartTime-3333)/667);
            Servo_SetRight(30+60*(now-actionStartTime-3333)/667);
        }
        else if(now-actionStartTime>4000)
        {
            currentAction=ACTION_NONE;
        }
    }
    else if(currentAction==ACTION_TIRED)
    {
        if(now-actionStartTime<=200)
        {
            Servo_SetLeft(90+30*(now-actionStartTime)/200);
        }
        else if(200<=now-actionStartTime&&now-actionStartTime<=850)
        {
            Servo_SetLeft(120+20*(now-actionStartTime-200)/650);
        }
        else if(850<=now-actionStartTime&&now-actionStartTime<=1500)
        {
            Servo_SetLeft(140-20*(now-actionStartTime-850)/650);
        }
        else if(1500<=now-actionStartTime&&now-actionStartTime<=2150)
        {
            Servo_SetLeft(120+20*(now-actionStartTime-1500)/650);
        }
        else if(2150<=now-actionStartTime&&now-actionStartTime<=2800)
        {
            Servo_SetLeft(140-20*(now-actionStartTime-2150)/650);
        }
        else if(2800<=now-actionStartTime&&now-actionStartTime<=3000)
        {
            Servo_SetLeft(120-30*(now-actionStartTime-2800)/200);
        }
        else if(now-actionStartTime>=3000)
        {
            currentAction=ACTION_NONE;
        }
    }
    else if(currentAction==ACTION_FORWARD)
    {
        if(now-actionStartTime<=666)
        {
            Servo_SetLeft(90-70*(now-actionStartTime)/666);
            Servo_SetRight(90+70*(now-actionStartTime)/666);
        }
        else if(666<=now-actionStartTime&&now-actionStartTime<=1333)
        {
            Servo_SetLeft(20+70*(now-actionStartTime-666)/667);
            Servo_SetRight(160-70*(now-actionStartTime-666)/667);
        }
        else if(1333<=now-actionStartTime&&now-actionStartTime<=2000)
        {
            Servo_SetLeft(90-70*(now-actionStartTime-1333)/667);
            Servo_SetRight(90+70*(now-actionStartTime-1333)/667);
        }
        else if(2000<=now-actionStartTime&&now-actionStartTime<=2667)
        {
            Servo_SetLeft(20+70*(now-actionStartTime-2000)/667);
            Servo_SetRight(160-70*(now-actionStartTime-2000)/667);
        }
        else if(2667<=now-actionStartTime&&now-actionStartTime<=3333)
        {
            Servo_SetLeft(90-70*(now-actionStartTime-2667)/666);
            Servo_SetRight(90+70*(now-actionStartTime-2667)/666);
        }
        else if(3333<=now-actionStartTime&&now-actionStartTime<=4000)
        {
            Servo_SetLeft(20+70*(now-actionStartTime-3333)/667);
            Servo_SetRight(160-70*(now-actionStartTime-3333)/667);
        }
        else if(now-actionStartTime>4000)
        {
            currentAction=ACTION_NONE;
        }
    }
    else if(currentAction==ACTION_SLEEP)
    {
        Servo_SetLeft(170);
        Servo_SetRight(170);
    }
}
