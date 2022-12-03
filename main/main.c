/**************************************************************
* Class: CSC-615-01 Fall 2022
* Team Name: Data Pi-rats
* Name: Cameron Yee
* Student ID: 920699179
* Github ID: DoughnutDude
* Project: Term Project - Drive On
*
* File: main.c
*
* Description: Contains the main driving logic
* Sources referenced: https://www.waveshare.com/wiki/Motor_Driver_HAT
* https://elinux.org/RPi_GPIO_Code_Samples#Direct_register_access
*
**************************************************************/

#include "main.h"

#define PIN_BUTTON 17

int main(void) {
    //1.System Initialization
    /*if (DEV_ModuleInit()) {
        exit(0);
    }*/
    // Exception handling:ctrl + c
    signal(SIGINT, sysExit);
    
    // Set up gpi pointer for direct register access
    setup_io();

    // Set GPIO button pin as input
    INP_GPIO(PIN_BUTTON);
    //DEV_GPIO_Mode(PIN_BUTTON, 0); //dev config stuff for sysfs GPIO

    DEBUG("setting pulldown: %d\r\n", GPIO_PULL);

    // Set pi GPIO pull to pulldown
    GPIO_PULL = 1; // set the type of pull we want, 1 = pulldown
    usleep(10); // wait 150 cycles
    GPIO_PULLCLK0 = 1<<PIN_BUTTON; // signify which pin which receives the pulldown change
    usleep(10); // wait 150 cycles
    GPIO_PULL = 0;
    GPIO_PULLCLK0 = 0;


    //2.Motor Initialization
    PCA9685_Init(FRONT_WHEELS, 0x40);
    PCA9685_Init(REAR_WHEELS, 0x50);
    PCA9685_SetPWMFreq(FRONT_WHEELS, 100);
    PCA9685_SetPWMFreq(REAR_WHEELS, 100);

    //3.Motor Run
    //DEBUG("running it\r\n");
    //motorSetDir(FRONT_WHEELS, MOTORB, FORWARD);
    //motorSetSpeed(FRONT_WHEELS, MOTORB, 100);
    //motorSetDir(FRONT_WHEELS, MOTORA, FORWARD);
    //motorSetSpeed(FRONT_WHEELS, MOTORA, 100);
    //motorSetDir(REAR_WHEELS, MOTORB, FORWARD);
    //motorSetSpeed(REAR_WHEELS, MOTORB, 100);
    //motorSetDir(REAR_WHEELS, MOTORA, FORWARD);
    //motorSetSpeed(REAR_WHEELS, MOTORA, 100);
    //DEV_Delay_ms(1000); //wait for 2 seconds

    //DEBUG("slowing it down\r\n");
    ////gradually slow down to 15%
    //for (int i = 100; i >= 15; i--) {
    //    motorSetSpeed(FRONT_WHEELS, MOTORA, i);
    //    motorSetSpeed(FRONT_WHEELS, MOTORB, i);
    //    motorSetSpeed(REAR_WHEELS, MOTORA, i);
    //    motorSetSpeed(REAR_WHEELS, MOTORB, i);
    //    DEV_Delay_ms(50);
    //}
    //
    ////gradually accelerate only front wheels to max reverse
    //DEBUG("front wheels drive");
    //motorSetDir(FRONT_WHEELS, MOTORA, BACKWARD);
    //motorSetDir(FRONT_WHEELS, MOTORB, BACKWARD);
    //for (int j = 0; j <= 100; j++) {
    //    motorSetSpeed(FRONT_WHEELS, MOTORA, j);
    //    motorSetSpeed(FRONT_WHEELS, MOTORB, j);
    //    DEV_Delay_ms(50);
    //}
    motorSetDir(FRONT_WHEELS, MOTORB, FORWARD);
    motorSetSpeed(FRONT_WHEELS, MOTORB, 100);
    
    //4.System Exit
    printf("\r\nEnd Reached: Motor Stop\r\n");
    motorStop(FRONT_WHEELS, MOTORA);
    motorStop(FRONT_WHEELS, MOTORB);
    motorStop(REAR_WHEELS, MOTORA);
    motorStop(REAR_WHEELS, MOTORB);
    //DEV_GPIO_Unexport(PIN_BUTTON);
    DEV_ModuleExit();
    return 0;
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

//dir should only be 1 or 0, forward or backward respectively.
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
void setup_io()
{
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
