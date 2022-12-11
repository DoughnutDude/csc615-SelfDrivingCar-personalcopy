/**************************************************************
* Class: CSC-615-01 Fall 2022
* Team Name: Data Pi-rats
* Name: Cameron Yee, Elisa Chih
* Student ID: 920699179, 920541866
* Github ID: DoughnutDude, elisachih
* Project: Term Project - Drive On
*
* File: motorcontroller.h
*
* Description: Includes motor driver files and DEV_config files.

* Sources referenced: https://www.waveshare.com/wiki/Motor_Driver_HAT
* https://elinux.org/RPi_GPIO_Code_Samples#Direct_register_access
*
**************************************************************/

#ifndef __MOTORCONTROLLER_H__
#define __MOTORCONTROLLER_H__

#include <stdlib.h>     //exit()
#include <fcntl.h>
#include <sys/mman.h>

#include "DEV_Config.h"
#include "DEV_Config.h"
#include "PCA9685.h"


//GPIO config
#define PWMA        PCA_CHANNEL_0
#define AIN1        PCA_CHANNEL_1
#define AIN2        PCA_CHANNEL_2
#define PWMB        PCA_CHANNEL_5
#define BIN1        PCA_CHANNEL_3
#define BIN2        PCA_CHANNEL_4


#define FRONT_WHEELS	0
#define REAR_WHEELS		1
#define MOTORA			0
#define MOTORB			1
#define BACKWARD		0
#define FORWARD			1

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

extern int  mem_fd;
extern void* gpio_map;

// I/O access
extern volatile unsigned* gpio;

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

#define SPEED_NORMAL 45
#define SPEED_SLOW   40

#define DRIFT_LEFT_SLEEP 1
#define RIGHT_REAR_SLEEP 1 

void motorInit();
void motorSetSpeed(int, UBYTE, UWORD);
void motorSetDir(int, UBYTE, int);
void motorStop(int, UBYTE);
void motorStopAll();

// helper functions
void goForward(int speed);
void turnLeft();
void turnRight();
void DriftLeft();
void DriftRight();

void setup_io();
void sysExit(int);

#endif 