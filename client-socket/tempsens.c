/**
*   @file : tempsens.c
*   @function : interface temperature sensor (TMP102) and send data every 3 secs.
*
*   @author : Ayush Dhoot
*   @references : https://github.com/jbdatko/tmp102/blob/master/tmp102.c
**/

#include "client_socket.h"
#include "tempsens.h"

/**
 * 
 * @function: temp_sensor_data
 * @return : temperature in raw format
 * 
 **/
int temp_sensor_data(){

    int addr = TMP102_ADDR;          /* The I2C address of TMP102 */
    char buf[2] = {0};
    int temperature;

    //float F,C;


    if ((fd = open(I2C_BUS, O_RDWR)) < 0) {
    /* ERROR HANDLING: you can check errno to see what went wrong */
    perror("Failed to open the i2c bus");
    exit(EXIT_FAIL);
    }

    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
    perror("Failed to acquire i2c bus access and/or talk to slave.\n");
    /* ERROR HANDLING; you can check errno to see what went wrong */
    exit(EXIT_FAIL);
    }

    // Using I2C Read
    if (read(fd,buf,2) != 2) {
        /* ERROR HANDLING: i2c transaction failed */
        perror("Failed to read from the i2c bus.\n");

    }  else {

        /* Convert 12bit int using two's compliment */
        /* Credit: http://bildr.org/2011/01/tmp102-arduino/ */
        temperature = ((buf[0] << 8) | buf[1]) >> 4;

        //C = temperature*0.0625;
        //F = (1.8 * C) + 32;

        //printf("Temperature in Fahrenheit: %f Celsius: %f\n", F, C);
    }

    return temperature;
}