/*****************************************************************************
* | File        :   dev_hardware_i2c.c
* | Author      :   Waveshare team
* | Function    :   Read and write /dev/i2C,  hardware I2C
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2019-06-26
* | Info        :   Basic version
*
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/

/**************************************************************
* Class: CSC-615-01 Fall 2022
* Team Name: Data Pi-rats
* Name: Cameron Yee
* Student ID: 920699179
* Github ID: DoughnutDude
* Project: Term Project - Drive On
*
* File: dev_hardware_iwc.c
*
* Description: low level functions for reading and writing to the i2c device.
* Sources referenced: https://www.waveshare.com/wiki/Motor_Driver_HAT
*
**************************************************************/

#include "dev_hardware_i2c.h"

#include <stdio.h>
#include <stdlib.h>   //exit()
#include <fcntl.h>    //define O_RDWR  
#include <linux/i2c-dev.h>  
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>


HARDWARE_I2C hardware_i2c[2];

/******************************************************************************
function: I2C device initialization
parameter:
    i2c_device : Device name
Info:   /dev/i2c-*
******************************************************************************/
void DEV_HARDWARE_I2C_begin(int deviceNum, char *i2c_device)
{
    printf("i2c_device: %s\r\n", i2c_device);
    //device
    if((hardware_i2c[deviceNum].fd = open(i2c_device, O_RDWR)) < 0)  { //打开I2C 
        perror("Failed to open i2c device.\n");  
        printf("Failed to open i2c device\r\n");
        exit(1); 
    } else {
        DEV_HARDWARE_I2C_Debug("open : %s\r\n", i2c_device);
    }
}

/******************************************************************************
function: I2C device End
parameter:
Info:
******************************************************************************/
void DEV_HARDWARE_I2C_end(int deviceNum)
{
    if (close(hardware_i2c[deviceNum].fd) != 0){
        perror("Failed to close i2c device.\n");  
    }
}

/******************************************************************************
function: Set the device address for I2C access
parameter:
    addr : Device address accessed by I2C
Info:
******************************************************************************/
void DEV_HARDWARE_I2C_setSlaveAddress(int deviceNum, uint8_t addr)
{
    printf("setting address: %d\r\n", addr);
    if(ioctl(hardware_i2c[deviceNum].fd, I2C_SLAVE, addr) < 0)  {
        printf("Failed to access bus.\n");  
        exit(1);  
    }
    printf("devnum%d fd: %d\r\n", deviceNum, hardware_i2c[deviceNum].fd);
}

/******************************************************************************
function:   I2C Send data
parameter:
    buf  : Send data buffer address
    len  : Send data length
Info:
******************************************************************************/
uint8_t DEV_HARDWARE_I2C_write(int deviceNum, const char * buf, uint32_t len)
{
    write(hardware_i2c[deviceNum].fd, buf, len);
    return 0;
}

/******************************************************************************
function:   I2C read data
parameter:
    reg  : Read data register address
    buf  : Sead data buffer address
    len  : Sead data length
Info:
******************************************************************************/
uint8_t DEV_HARDWARE_I2C_read(int deviceNum, uint8_t reg, char* buf, uint32_t len)
{
    uint8_t temp[1] = {reg};
    write(hardware_i2c[deviceNum].fd, temp, 1);
    read(hardware_i2c[deviceNum].fd, buf, len);
    return 0;
}