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
* Description: Contains the main driving logic for the sensors and car as a whole.

* Sources referenced: https://www.waveshare.com/wiki/Motor_Driver_HAT
* https://elinux.org/RPi_GPIO_Code_Samples#Direct_register_access
* https://elinux.org/RPi_GPIO_Code_Samples
* https://abyz.me.uk/rpi/pigpio/ 
**************************************************************/

#include "main.h"
#include <pigpio.h>
#include <pthread.h>
#include "motorcontroller.h"

#define OBSTACLE_RR_GPIO        21
#define OBSTACLE_CENTER_GPIO    23
#define OBSTACLE_RF_GPIO        24
#define LINE_LEFT_GPIO          17
#define LINE_RIGHT_GPIO         27

#define REAR_RIGHT_TRIGGER_GPIO  19
#define REAR_RIGHT_ECHO_GPIO     26

int _quit = 0;

#define HIGH 1
#define LOW 0

#define YES 1
#define NO 0

// constants for ON/OFF
#define ON  1
#define OFF 0

int _isRightFrontObstacle = NO;
int _isRightRearObstacle = NO;
int _isRightRearObstacleYet = NO;
int _isObstacle = NO;
int _isRightOnLine = NO;
int _isRightOnLineYet = NO;
int _isLeftOnLine = NO; 
int _isDrifting = NO;

float _rearObstacleDistance = 0;

// Thread function used for the line sensors
void *myThreadFunLine(void *vargp) {
    // need to loop on the line sensor pin until a signal of 1 is received
    // then, save the start time
    while(_quit == 0) {
        int readLeft = gpioRead(LINE_LEFT_GPIO);
        //DEBUG("left line sensor: %d\r\n", readLeft);
        if(readLeft == LOW) {
            //printf("off the line\n");
        } else {
            //printf("on the line\n");
        }

        int readRight = gpioRead(LINE_RIGHT_GPIO);
        //DEBUG("right line sensor: %d\r\n", readRight);
        if(readRight == LOW) {
            //printf("off the line\n");
        } else {
            //printf("on the line\n");
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
    }
    return NULL;
}

// Thread function used to detect obstacle
void *myThreadFunObstacle(void *vargp) {

    while(_quit == 0) {
        int read = gpioRead(OBSTACLE_CENTER_GPIO);
        //DEBUG("%d\r\n", read);
        // is obstacle
        if(read == LOW) {
            //printf("obstacle detected\n");
            _isObstacle = YES;
        } else {
            _isObstacle = NO;
        }

        read = gpioRead(OBSTACLE_RR_GPIO);
        //DEBUG("%d\r\n", read);
        // is obstacle
        if(read == LOW) {
            //printf("Right Rear obstacle detected\n");
            _isRightRearObstacle = YES;
        } else {
            _isRightRearObstacle = NO;
        }

        read = gpioRead(OBSTACLE_RF_GPIO);
        //DEBUG("%d\r\n", read);
        // is obstacle
        if(read == LOW) {
            //printf("Right Front obstacle detected\n");
            _isRightFrontObstacle = YES;
        } else {
            _isRightFrontObstacle = NO;
        }
    }
    return NULL;
}

// Thread function used to detect distance for the rear ultrasonic sensor
void *myThreadFunMeasure(void *vargp) {

    while(_quit == 0) {
        // trigger the ultrasonic
        gpioWrite(REAR_RIGHT_TRIGGER_GPIO, ON); 
        // 10 micros delay
        gpioDelay(10);  
        // stop trigger
        gpioWrite(REAR_RIGHT_TRIGGER_GPIO, OFF); 

        uint32_t startTick, endTick;
        float diffTick;

        // we need to loop on the echo pin until we received a signal of 1
        // Then we can save the StartTime
        while (gpioRead(REAR_RIGHT_ECHO_GPIO) == 0) {
            startTick = gpioTick();
        }

        // we need to loop on the echo pin until the signal is off
        // Then we can save the end time
        while (gpioRead(REAR_RIGHT_ECHO_GPIO) == 1) {
            endTick = gpioTick();
        }

        diffTick = endTick - startTick;

        //printf("Total time: %f microseconds\n", diffTick);

        // multiply with the sonic speed (34300 cm/s)
        // and divide by 2, because there and back
        float distance = (diffTick/1000000 * 34300) / 2.0;
        //printf("Distance: %f cm\n", distance);
        _rearObstacleDistance = distance;

        time_sleep(0.2);
    }
    return NULL;
}

#define FOLLOW_LINE         0
#define DRIFT_TO_LEFT       1
#define DRIFT_TO_FORWARD    2 
#define DRIFT_TO_RIGHT      3
int state = FOLLOW_LINE;

// Main thread for running the follow line and obstacle detection logics
void *myThreadFunMain(void *vargp) {

    while(_quit == 0) {

        // if there is obstacle and we are not drifting yet, stop
        // and start drifting to the left
        if ((_isObstacle == YES || _isRightFrontObstacle == YES) && state != DRIFT_TO_LEFT) { 
            motorStopAll();
            sleep(1);
            _isDrifting = YES;
            state = DRIFT_TO_LEFT;
            DriftLeft();
            continue;
        } 
        // if the front obstacle is cleared, but there is still obstacle on the right
        // we will continue to drift for 1 second
        if (_isObstacle == NO && _isRightFrontObstacle == YES && state == DRIFT_TO_LEFT) { 
            DriftLeft();
            sleep(DRIFT_LEFT_SLEEP);
            continue;
        }                
        // if there are no more obstacles, and we are still in drifting state, 
        // we will now continue to go forward
        if ((_isObstacle == NO && _isRightFrontObstacle == NO) && state == DRIFT_TO_LEFT) { 
            state = DRIFT_TO_FORWARD;
            goForward(SPEED_SLOW);
            continue;
        } 
 
        // if we are in a state of going forward but in an off course
        // state (drifting), we will check if the right rear 
        // obstacle is cleared.
        if (state == DRIFT_TO_FORWARD) {
            // if the rear right detects an obstalce, we will mark
            // it as entering obstacle.
            if (_rearObstacleDistance <= 10 && _isRightRearObstacleYet ==  NO) { 
                _isRightRearObstacleYet = YES;
            } 
            // if there are no more obstacles at the rear right and
            // we previouslly detected that there is an obstacle
            // this means we have cleared the back
            // we will continue forward for 1 second, and stop
            // and set the state to be ready to drift to the right
            if (_rearObstacleDistance > 10 && _isRightRearObstacleYet == YES) {
                sleep(RIGHT_REAR_SLEEP);
                motorStopAll();
                state = DRIFT_TO_RIGHT;
                _isRightRearObstacleYet = NO;
            }
        }
        // now we are in the state of drifting to the right
        // we want to make sure the right line sensor is over the line
        if (state == DRIFT_TO_RIGHT) {
            // if we are on the line, but have not crossed it
            // continue to drift to the right and mark that we just entered the line
            if (_isRightOnLine == YES && _isRightOnLineYet == NO) {
                _isRightOnLineYet = YES;
                DriftRight();
            }
            // we are not on line yet, continue to drift right
            if (_isRightOnLineYet == NO) {
                DriftRight();
            }
            // we are not on the line anymore, but we were previously on the line
            // this means we crossed the line, we will stop and
            // resume line following.
            if  (_isRightOnLine == NO && _isRightOnLineYet == YES) {
                _isRightOnLineYet = NO;
                motorStopAll();
                state = FOLLOW_LINE;
            }
        }
        if (state == FOLLOW_LINE) {
            // if both sensors are on the line, we will just go forward a little
            if (_isLeftOnLine == YES && _isRightOnLine == YES) {
                goForward(SPEED_SLOW);
                sleep(0.5);
                // if both sensors are still on the line, we will just go forward a little
                if (_isLeftOnLine == YES && _isRightOnLine == YES) {
                    goForward(SPEED_SLOW);
                    sleep(0.5);                
                }
            }
            // if both sensors are not on the line, we will just go forward
            if (_isLeftOnLine == NO && _isRightOnLine == NO) {
                goForward(SPEED_NORMAL);
                sleep(0.5);
            }
            // if only right is on the line, we will turn right
            if (_isLeftOnLine == NO && _isRightOnLine == YES) {
                turnRight();
                sleep(0.5);
            }
            // if only left is on the line, we will turn left
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

    if (gpioGetMode(REAR_RIGHT_TRIGGER_GPIO) != PI_OUTPUT)
        gpioSetMode(REAR_RIGHT_TRIGGER_GPIO, PI_OUTPUT); 
    
    // set the GPIO for echo pins as INPUT 
    if (gpioGetMode(REAR_RIGHT_ECHO_GPIO) != PI_INPUT)
        gpioSetMode(REAR_RIGHT_ECHO_GPIO, PI_INPUT); 

    // Send OFF to the trigger pin
    gpioWrite(REAR_RIGHT_TRIGGER_GPIO, OFF); 
    time_sleep(1);
    
    // Set GPIO pin as input
      // Set the GPIO 
    if (gpioGetMode(OBSTACLE_CENTER_GPIO) != PI_INPUT)
        gpioSetMode(OBSTACLE_CENTER_GPIO, PI_INPUT); 

    if (gpioGetMode(OBSTACLE_RF_GPIO) != PI_INPUT)
        gpioSetMode(OBSTACLE_RF_GPIO, PI_INPUT); 

    if (gpioGetMode(OBSTACLE_RR_GPIO) != PI_INPUT)
        gpioSetMode(OBSTACLE_RR_GPIO, PI_INPUT); 

    // set the GPIO 
    if (gpioGetMode(LINE_RIGHT_GPIO) != PI_INPUT)
        gpioSetMode(LINE_RIGHT_GPIO, PI_INPUT); 

    // set the GPIO 
    if (gpioGetMode(LINE_LEFT_GPIO) != PI_INPUT)
        gpioSetMode(LINE_LEFT_GPIO, PI_INPUT); 

    //2.Motor Initialization
    motorInit();

    //3.Motor Run

    printf("start...");

    // set up thread for the sensors
    pthread_t line_thread_id, obstacle_thread_id, obstacle_thread_main, measure_thread_main;
    printf("before thread\n");
    pthread_create(&line_thread_id, NULL, myThreadFunLine, NULL);
    pthread_create(&obstacle_thread_id, NULL, myThreadFunObstacle, NULL);
    pthread_create(&obstacle_thread_main, NULL, myThreadFunMain, NULL);
    pthread_create(&measure_thread_main, NULL, myThreadFunMeasure, NULL);

    char c = getchar();
    _quit = 1;
    pthread_join(line_thread_id, NULL);
    pthread_join(obstacle_thread_id, NULL);
    pthread_join(obstacle_thread_main, NULL);
    pthread_join(measure_thread_main, NULL);

    printf("after thread\n");

    //4.System Exit
    printf("\r\nEnd Reached: Motor Stop\r\n");
    motorStopAll();

    // shutdown gpio library
    gpioTerminate();

    DEV_ModuleExit();
    return 0;
}
