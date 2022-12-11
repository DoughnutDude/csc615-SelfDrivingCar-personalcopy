/**************************************************************
* Class: CSC-615-01 Fall 2022
* Team Name: Data Pi-rats
* Name: Cameron Yee, Elisa Chih
* Student ID: 920699179, 920541866
* Github ID: DoughnutDude, elisachih
* Project: Term Project - Drive On
*
* File: motorcontroller.c
*
* Description:

* Sources referenced: https://www.waveshare.com/wiki/Motor_Driver_HAT
* https://elinux.org/RPi_GPIO_Code_Samples#Direct_register_access
* https://elinux.org/RPi_GPIO_Code_Samples
* https://abyz.me.uk/rpi/pigpio/ 
**************************************************************/

#include "motorcontroller.h"

int  mem_fd = -1;
void* gpio_map = NULL;
volatile unsigned* gpio = NULL;

void motorInit() {
    PCA9685_Init(FRONT_WHEELS, 0x40);
    PCA9685_Init(REAR_WHEELS, 0x54); // change to new address of 2nd motor driver hat
    PCA9685_SetPWMFreq(FRONT_WHEELS, 100);
    PCA9685_SetPWMFreq(REAR_WHEELS, 100);
}

void motorSetSpeed(int deviceNum, UBYTE motor, UWORD speed) {
    DEBUG("%d Setting motor speed.\r\n", deviceNum);
    if (speed > 100) {
        speed = 100;
    }
    if (motor == MOTORA) {
        DEBUG("Motor A Speed = %d\r\n", speed);
        PCA9685_SetPwmDutyCycle(deviceNum, PWMA, speed);
    } else {
        DEBUG("Motor B Speed = %d\r\n", speed);
        PCA9685_SetPwmDutyCycle(deviceNum, PWMB, speed);
    }
}

// dir should only be 1 or 0, forward or backward respectively.
void motorSetDir(int deviceNum, UBYTE motor, int dir) {
    DEBUG("%d Setting motor direction.\r\n", deviceNum);

    UBYTE chann1 = AIN1;
    UBYTE chann2 = AIN2;
    if (motor == MOTORA) {
        DEBUG("Motor A ");
    } else {
        DEBUG("Motor B ");
        chann1 = BIN1;
        chann2 = BIN2;
    }

    if (dir) {
        DEBUG("forward...\r\n");
        PCA9685_SetLevel(deviceNum, chann1, 0);
        PCA9685_SetLevel(deviceNum, chann2, 1);
    }
    else {
        DEBUG("backward...\r\n");
        PCA9685_SetLevel(deviceNum, chann1, 1);
        PCA9685_SetLevel(deviceNum, chann2, 0);
    }
}

void motorStop(int deviceNum, UBYTE motor) {
    if (motor == MOTORA) {
        PCA9685_SetPwmDutyCycle(deviceNum, PWMA, 0);
    } else {
        PCA9685_SetPwmDutyCycle(deviceNum, PWMB, 0);
    }
}

// Set up a memory regions to access GPIO
void setup_io() {
    /* open /dev/mem */
    if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        printf("can't open /dev/mem \n");
        exit(-1);
    }

    /* mmap GPIO */
    gpio_map = mmap(
        NULL,             //Any adddress in our space will do
        BLOCK_SIZE,       //Map length
        PROT_READ | PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       //Shared with other processes
        mem_fd,           //File to map
        GPIO_BASE         //Offset to GPIO peripheral
    );

    close(mem_fd); //No need to keep mem_fd open after mmap

    if (gpio_map == MAP_FAILED) {
        printf("mmap error %d\n", (int)gpio_map);//errno also set!
        exit(-1);
    }

    // Always use volatile pointer!
    gpio = (volatile unsigned*)gpio_map;
}

void sysExit(int signo) {
    //System Exit
    printf("\r\nHandler: Motor Stop\r\n");
    motorStop(FRONT_WHEELS, MOTORA);
    motorStop(FRONT_WHEELS, MOTORB);
    motorStop(REAR_WHEELS, MOTORB);
    motorStop(REAR_WHEELS, MOTORA);
    DEV_ModuleExit();

    exit(0);
}

void motorStopAll() {
    motorStop(FRONT_WHEELS, MOTORA);
    motorStop(FRONT_WHEELS, MOTORB);
    motorStop(REAR_WHEELS, MOTORA);
    motorStop(REAR_WHEELS, MOTORB);
}

void goForward(int speed) {
    motorSetDir(FRONT_WHEELS, MOTORB, FORWARD); // right
    motorSetSpeed(FRONT_WHEELS, MOTORB, speed);

    motorSetDir(FRONT_WHEELS, MOTORA, FORWARD); // left
    motorSetSpeed(FRONT_WHEELS, MOTORA, speed);

    motorSetDir(REAR_WHEELS, MOTORB, FORWARD); // right
    motorSetSpeed(REAR_WHEELS, MOTORB, speed);
    
    motorSetDir(REAR_WHEELS, MOTORA, FORWARD); // left
    motorSetSpeed(REAR_WHEELS, MOTORA, speed);    
}

void turnLeft() {
    motorSetDir(FRONT_WHEELS, MOTORB, FORWARD); // right
    motorSetSpeed(FRONT_WHEELS, MOTORB, SPEED_NORMAL );

    motorSetDir(FRONT_WHEELS, MOTORA, BACKWARD); // left
    motorSetSpeed(FRONT_WHEELS, MOTORA, SPEED_NORMAL);

    motorSetDir(REAR_WHEELS, MOTORB, FORWARD); // right
    motorSetSpeed(REAR_WHEELS, MOTORB, SPEED_NORMAL);

    motorSetDir(REAR_WHEELS, MOTORA, BACKWARD); // left
    motorSetSpeed(REAR_WHEELS, MOTORA, SPEED_NORMAL);       
}

void turnRight() {
    motorSetDir(FRONT_WHEELS, MOTORB, BACKWARD); // right
    motorSetSpeed(FRONT_WHEELS, MOTORB, SPEED_NORMAL);

    motorSetDir(FRONT_WHEELS, MOTORA, FORWARD); // left
    motorSetSpeed(FRONT_WHEELS, MOTORA, SPEED_NORMAL );

    motorSetDir(REAR_WHEELS, MOTORB, BACKWARD); // right
    motorSetSpeed(REAR_WHEELS, MOTORB, SPEED_NORMAL);

    motorSetDir(REAR_WHEELS, MOTORA, FORWARD); // left
    motorSetSpeed(REAR_WHEELS, MOTORA, SPEED_NORMAL);       
}

void DriftLeft() {
    motorSetDir(FRONT_WHEELS, MOTORB, FORWARD); // front right
    motorSetSpeed(FRONT_WHEELS, MOTORB, SPEED_SLOW );

    motorSetDir(FRONT_WHEELS, MOTORA, BACKWARD); // left
    motorSetSpeed(FRONT_WHEELS, MOTORA, SPEED_SLOW );

    motorSetDir(REAR_WHEELS, MOTORB, BACKWARD); // rear right
    motorSetSpeed(REAR_WHEELS, MOTORB, SPEED_SLOW );

    motorSetDir(REAR_WHEELS, MOTORA, FORWARD); // rear left
    motorSetSpeed(REAR_WHEELS, MOTORA, SPEED_SLOW);       
}

void DriftRight() {
    motorSetDir(FRONT_WHEELS, MOTORB, BACKWARD); // front right
    motorSetSpeed(FRONT_WHEELS, MOTORB, SPEED_SLOW );

    motorSetDir(FRONT_WHEELS, MOTORA, FORWARD); // left
    motorSetSpeed(FRONT_WHEELS, MOTORA, SPEED_SLOW );

    motorSetDir(REAR_WHEELS, MOTORB, FORWARD); // rear right
    motorSetSpeed(REAR_WHEELS, MOTORB, SPEED_SLOW );

    motorSetDir(REAR_WHEELS, MOTORA, BACKWARD); // left
    motorSetSpeed(REAR_WHEELS, MOTORA, SPEED_SLOW );           
}