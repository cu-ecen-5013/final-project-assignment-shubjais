/**
*   @file : tempsens.h
*   @function : function declaration and defines for interfacing temperature sensor (TMP102).
*
*   @author : Ayush Dhoot
*   @references : https://github.com/jbdatko/tmp102/blob/master/tmp102.c
**/

#ifndef TEMPSENS_H_
#define TEMPSENS_H_

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#define I2C_BUS       "/dev/i2c-2"
#define TMP102_ADDR   0x48

#define EXIT_FAIL     -1

int fd;
int temp_sensor_data();

#endif
