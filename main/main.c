/**************************************************************
* Class: CSC-615-01 Fall 2022
* Team Name: Data Pi-rats
* Name: Cameron Yee, Elisa Chih
* Student ID: 920699179, 920541866
* Github ID: DoughnutDude, elisachih
* Project: Term Project - Drive On
*
* File: main.c
*
* Description: Contains the main driving logic

* Sources referenced: https://www.waveshare.com/wiki/Motor_Driver_HAT
* https://elinux.org/RPi_GPIO_Code_Samples#Direct_register_access
* https://elinux.org/RPi_GPIO_Code_Samples
* https://abyz.me.uk/rpi/pigpio/ 
**************************************************************/

#include "main.h"
#include <pigpio.h>
#include <pthread.h>
#include "motorcontroller.h"


#define OBSTACLE_GPIO 23
#define LINE_LEFT_GPIO 17
#define LINE_RIGHT_GPIO 27

int _quit = 0;

#define HIGH 1
#define LOW 0

#define YES 1
#define NO 0

int _isObstacle = NO;
int _isRightOnLine = NO;
int _isLeftOnLine = NO; 
int _isDrifting = NO;

void *myThreadFunLine(void *vargp) {
    // need to loop on the line sensor pin until a signal of 1 is received
    // then, save the start time
    while(_quit == 0) {
        int readLeft = gpioRead(LINE_LEFT_GPIO);
        //DEBUG("left line sensor: %d\r\n", readLeft);
        if(readLeft == LOW) {
            //printf("off the line\n");
            // motorSetDir(FRONT_WHEELS, MOTORA, FORWARD);
            // motorSetSpeed(FRONT_WHEELS, MOTORA, 100);
        } else {
            // motorSetDir(FRONT_WHEELS, MOTORA, BACKWARD);
            // motorSetSpeed(FRONT_WHEELS, MOTORA, 100);
            //printf("on the line\n");
        }

        int readRight = gpioRead(LINE_RIGHT_GPIO);
        DEBUG("right line sensor: %d\r\n", readRight);
        if(readRight == LOW) {
            printf("off the line\n");
            // motorSetDir(FRONT_WHEELS, MOTORA, FORWARD);
            // motorSetSpeed(FRONT_WHEELS, MOTORA, 100);
        } else {
            // motorSetDir(FRONT_WHEELS, MOTORA, BACKWARD);
            // motorSetSpeed(FRONT_WHEELS, MOTORA, 100);
            printf("on the line\n");
        }

        if (readLeft == HIGH) {
            _isLeftOnLine = YES;
        } 
        if (readLeft == LOW) {
            _isLeftOnLine = NO;
        } 

        if (readRight == HIGH) {
            _isRightOnLine = YES;
        }
        if (readRight == LOW) {
            _isRightOnLine = NO;
        }

        sleep(0.2);
    }
    return NULL;
}

void *myThreadFunObstacle(void *vargp) {

    while(_quit == 0) {
        int read = gpioRead(OBSTACLE_GPIO);
        DEBUG("%d\r\n", read);
        // is obstacle
        if(read == LOW) {
            printf("obstacle detected\n");
            _isObstacle = YES;
        } else {
            _isObstacle = NO;
        }
        sleep(0.2);
    }
    return NULL;
}

void *myThreadFunMain(void *vargp) {

    while(_quit == 0) {
        if (_isObstacle == YES) {
            motorStopAll();
            _isDrifting = YES;
            DriftLeft();
            sleep(1);

        } else if (_isDrifting == YES) {
            //goForward(30);
            motorStopAll();
            sleep(0.5);
            DriftRight();
            sleep(1);
        } else {
            if (_isLeftOnLine == YES && _isRightOnLine == YES) {
                goForward(30);
                sleep(0.5);
                if (_isLeftOnLine == YES && _isRightOnLine == YES) {
                    goForward(30);
                    sleep(0.5);                
                }

                // goForward();
            }
            if (_isLeftOnLine == NO && _isRightOnLine == NO) {
                goForward(SPEED_NORMAL);
                sleep(0.5);
            }
            if (_isLeftOnLine == NO && _isRightOnLine == YES) {
                turnRight();
                sleep(0.5);
            }
            if (_isLeftOnLine == YES  && _isRightOnLine == NO) {
                turnLeft();
                sleep(0.5);
            }
        }
        
    }
    return NULL;
}

int main(void) {
    // System Initialization for I2C
    // if failed, just end program    
    if (DEV_ModuleInit()) {
        printf("DEV_ModuleInit Error\n");
        exit(0);
    }
    printf("DEV_ModuleInit initialised okay.! \n");

    // Initialize gpio library
    // if failed, just end program    
    if (gpioInitialise() < 0) {
        printf("gpioInitialise failed, Exit Program\n");
        exit(0);     
    }
    printf("pigpio initialised okay.! \n");

    // Exception handling:ctrl + c
    signal(SIGINT, sysExit);
    
    // // Set up gpi pointer for direct register access
    // setup_io();

    // Set GPIO pin as input
      // Set the GPIO 
    if (gpioGetMode(OBSTACLE_GPIO) != PI_INPUT)
        gpioSetMode(OBSTACLE_GPIO, PI_INPUT); 

    // set the GPIO 
    if (gpioGetMode(LINE_RIGHT_GPIO) != PI_INPUT)
        gpioSetMode(LINE_RIGHT_GPIO, PI_INPUT); 

    // set the GPIO 
    if (gpioGetMode(LINE_LEFT_GPIO) != PI_INPUT)
        gpioSetMode(LINE_LEFT_GPIO, PI_INPUT); 

    //DEV_GPIO_Mode(PIN_BUTTON, 0); //dev config stuff for sysfs GPIO

    // Set pi GPIO pull to pulldown
    // GPIO_PULL = 1; // set the type of pull we want, 1 = pulldown
    // usleep(10); // wait 150 cycles
    // GPIO_PULLCLK0 = 1<<LINE_GPIO | 1<<OBSTACLE_GPIO; // signify which pin which receives the pulldown change
    // usleep(10); // wait 150 cycles
    // GPIO_PULL = 0;
    // GPIO_PULLCLK0 = 0;

    /*printf("waiting for button...\r\n");
    while (GET_GPIO(PIN_BUTTON) == 0) {
    }
    DEBUG("button pushed, continuing.\r\n");*/

    //2.Motor Initialization
    motorInit();

    //3.Motor Run
    DEBUG("running it\r\n");

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
    // motorSetDir(FRONT_WHEELS, MOTORB, FORWARD); // right
    // motorSetSpeed(FRONT_WHEELS, MOTORB, 100);
    // motorSetDir(FRONT_WHEELS, MOTORA, FORWARD);
    // motorSetSpeed(FRONT_WHEELS, MOTORA, 100);
    // motorSetDir(REAR_WHEELS, MOTORA, FORWARD); // left
    // motorSetSpeed(REAR_WHEELS, MOTORA, 100);
    // motorSetDir(REAR_WHEELS, MOTORB, FORWARD); // right
    // motorSetSpeed(REAR_WHEELS, MOTORB, 100);
    // DEV_Delay_ms(5000);
    // DEBUG("stopping");
    // motorStop(FRONT_WHEELS, MOTORA);
    // motorStop(FRONT_WHEELS, MOTORB);


    printf("start...");

    pthread_t line_thread_id, obstacle_thread_id, obstacle_thread_main;
    printf("before thread\n");
    pthread_create(&line_thread_id, NULL, myThreadFunLine, NULL);
    pthread_create(&obstacle_thread_id, NULL, myThreadFunObstacle, NULL);
    pthread_create(&obstacle_thread_main, NULL, myThreadFunMain, NULL);

    char c = getchar();
    _quit = 1;
    pthread_join(line_thread_id, NULL);
    pthread_join(obstacle_thread_id, NULL);
    pthread_join(obstacle_thread_main, NULL);

    printf("after thread\n");

    //4.System Exit
    printf("\r\nEnd Reached: Motor Stop\r\n");
    motorStopAll();

    // shutdown gpio library
    gpioTerminate();

    DEV_ModuleExit();
    return 0;
}
