###############################################################
# Class: CSC-615-01 Fall 2022
# Team Name: Data Pi-rats
# Name:	Cameron Yee
# Student ID: 920699179
# Github ID: DoughnutDude
# Project: Term Project - Drive On
#
# File: Makefile
#
# Description: Compiles the program when make is executed from the terminal.
#
###############################################################

DIR_OBJ = ./lib
DIR_BIN = ./bin
DIR_Config = ./lib/Config
DIR_PCA9685 = ./lib/PCA9685
DIR_Main = ./main

OBJ_C = $(wildcard ${DIR_OBJ}/*.c ${DIR_Main}/*.c ${DIR_Config}/*.c ${DIR_MotorDriver}/*.c ${DIR_PCA9685}/*.c )
OBJ_O = $(patsubst %.c,${DIR_BIN}/%.o,$(notdir ${OBJ_C}))

TARGET = assignment3
#BIN_TARGET = ${DIR_BIN}/${TARGET}

CC = gcc

DEBUG = -g -O0 -Wall
CFLAGS += $(DEBUG)

# USELIB = USE_BCM2835_LIB
# USELIB = USE_WIRINGPI_LIB
USELIB = USE_DEV_LIB
DEBUG = -D $(USELIB) 
ifeq ($(USELIB), USE_BCM2835_LIB)
    LIB = -lbcm2835 -lm 
else ifeq ($(USELIB), USE_WIRINGPI_LIB)
    LIB = -lwiringPi -lm 

endif

${TARGET}:${OBJ_O}
	$(CC) $(CFLAGS) $(OBJ_O) -o $@ $(LIB) -lm

${DIR_BIN}/%.o : $(DIR_Main)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB) -I $(DIR_OBJ) -I $(DIR_Config) -I $(DIR_PCA9685)

${DIR_BIN}/%.o : $(DIR_OBJ)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB) -I $(DIR_Config)
    
${DIR_BIN}/%.o : $(DIR_Config)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB)

${DIR_BIN}/%.o : $(DIR_PCA9685)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB) -I $(DIR_Config)


clean :
	rm $(DIR_BIN)/*.* 
	rm $(TARGET) 
