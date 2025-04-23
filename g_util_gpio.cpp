/**
 * Copyright © 2025-2025 Gking,All Rights Reserved.
 * https://github.com/Gking-a/Gking_Milkvduo_Tools
 * Mozilla Public License Version 2.0
 */
#include "g_u_h.h"
#include "wiringx.h"
int hardcode_milkv_gp_pwm[]={2,10,3,11,4,5,5,6,6,9,7,8,8,7,9,4,12,4,13,5,26,3,27,4};
int wiringSetup(){
    if(wiringXSetup("milkv_duo", NULL) == -1) {
        wiringXGC();
        return -1;
    }
    return 0;
}
int wiringOpr(int gpioid,int hl){
    if(wiringXValidGPIO(gpioid) != 0) {
        printf("Invalid GPIO %d\n", gpioid);
        return -1;
    }
    
    digitalWrite(gpioid, digital_value_t(hl));
    return 0;
}
void standardPWM(int pwm_pin,int duty);
L298NController::L298NController(char ap,char a1,char a2,char bp,char b3,char b4){
    port.chan[0][0]=ap;
    port.chan[0][1]=a1;
    port.chan[0][2]=a2;
    port.chan[1][0]=bp;
    port.chan[1][1]=b3;
    port.chan[1][2]=b4;
}
void helpmethodtopwm(int gp){
    for (char i = 0; i < 12; i++)
    {
        if(gp==hardcode_milkv_gp_pwm[i*2]){
            char pwm=hardcode_milkv_gp_pwm[i*2+1];
            char cmd[100];
            snprintf(cmd,100,"duo-pinmux -w GP%d/PWM_%d",gp,pwm);
            system(cmd);
            return;
        }
    }
}
void helpmethodtogp(int gp)
{
    char cmd[100];
    snprintf(cmd, 100, "duo-pinmux -w GP%d/GP%d", gp, gp);
    system(cmd);
    return;
}
L298NController::L298NController(){}
void L298NController::setConfig(L298NPort port){
    this->port=port;   
}
void L298NController::setMode(){
    helpmethodtogp(port.chan[0][1]);
    helpmethodtogp(port.chan[0][2]);
    helpmethodtogp(port.chan[1][1]);
    helpmethodtogp(port.chan[1][2]);
    helpmethodtopwm(port.chan[0][0]);
    helpmethodtopwm(port.chan[1][0]);
    pinMode(port.chan[0][1],PINMODE_OUTPUT);
    pinMode(port.chan[0][2],PINMODE_OUTPUT);
    pinMode(port.chan[1][1],PINMODE_OUTPUT);
    pinMode(port.chan[1][2],PINMODE_OUTPUT);
    // 设置 PWM 周期为 1000 ns; Set PWM Period 1000 ns long.
    wiringXPWMSetPeriod(port.chan[0][0], 1000);
    // 设置 PWM 极性并使能; Set PWM Polarity and enable.
    wiringXPWMSetPolarity(port.chan[0][0], 0);
    wiringXPWMEnable(port.chan[0][0], 1);
    // 设置 PWM 周期为 1000 ns; Set PWM Period 1000 ns long.
    wiringXPWMSetPeriod(port.chan[1][0], 1000);
    // 设置 PWM 极性并使能; Set PWM Polarity and enable.
    wiringXPWMSetPolarity(port.chan[1][0], 0);
    wiringXPWMEnable(port.chan[1][0], 1);
}
L298NController::~L298NController(){}
void L298NController::setspeed(int chan,int speed){
    int pwm_pin=port.chan[chan][0];
    if(speed>1000||speed<-1000){
        standardPWM(pwm_pin,1000);
        wiringOpr(this->port.chan[chan][2],1);
        wiringOpr(this->port.chan[chan][1],1);
    }else if(speed==0){
        standardPWM(pwm_pin,0);
        wiringOpr(this->port.chan[chan][2],0);
        wiringOpr(this->port.chan[chan][1],0);
    }else if (speed>0)
    {
        standardPWM(pwm_pin,speed);
        wiringOpr(this->port.chan[chan][1],1);
        wiringOpr(this->port.chan[chan][2],0);
    }else if(speed<0){
        standardPWM(pwm_pin,-speed);
        wiringOpr(this->port.chan[chan][1],0);
        wiringOpr(this->port.chan[chan][2],1);
    }   
    else{
        
    }
    
}
void standardPWM(int pwm_pin, int duty)
{
    
    // 设置 PWM 占空比; Set PWM Duty.
    wiringXPWMSetDuty(pwm_pin, duty);
    
    printf("pin %d -> duty %d\n",pwm_pin ,duty);
}