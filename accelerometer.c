/*
 * File: accelerometer.c
 *
 * Authors: Kenneth Huang, Sarah Kellock
 *
 * Date: March 2022
 *
 * Module for using the ADXL345 accelerometer functionality including
 * reading and handling data.
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "acc.h"
#include "i2c_driver.h"
#include "circBufT.h"

#include "accelerometer.h"

#define BUF_SIZE 12

/* Circular buffer for each axis.
 * Each of size BUF_SIZE to store accelerometer data */
static circBuf_t x_buffer;
static circBuf_t y_buffer;
static circBuf_t z_buffer;

/* Circular buffer that stores the average of the x,y,z accelerometer values */
static circBuf_t mag_buffer;

/* Initializes accelerometer.
 * Acknowledgments: Based off C. P. Moore*/
void initAccl(void) {

	initCircBuf(&x_buffer, BUF_SIZE);
	initCircBuf(&y_buffer, BUF_SIZE);
	initCircBuf(&z_buffer, BUF_SIZE);
	initCircBuf(&mag_buffer, BUF_SIZE);

	/* This prevents a step being accounted as the magnitude buffer gets filled initially.
	 * This assignment forces magnitude value stays below the threshold until values stabilize. */
	uint8_t i;
	for (i = 0; i < BUF_SIZE; i++) {
		writeCircBuf(&mag_buffer, 768);
		writeCircBuf(&x_buffer, 256);
		writeCircBuf(&y_buffer, 256);
		writeCircBuf(&z_buffer, 256);
	}

	char toAccl[] = { 0, 0 };  // parameter, value

	/*
	 * Enable I2C Peripheral
	 */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
	SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

	/*
	 * Set I2C GPIO pins
	 */
	GPIOPinTypeI2C(I2CSDAPort, I2CSDA_PIN);
	GPIOPinTypeI2CSCL(I2CSCLPort, I2CSCL_PIN);
	GPIOPinConfigure(I2CSCL);
	GPIOPinConfigure(I2CSDA);

	/*
	 * Setup I2C
	 */
	I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), true);

	GPIOPinTypeGPIOInput(ACCL_INT2Port, ACCL_INT2);

	//Initialize ADXL345 Accelerometer

	// set +-2g, 13 bit resolution, active low interrupts
	toAccl[0] = ACCL_DATA_FORMAT;
	toAccl[1] = (ACCL_RANGE_16G | ACCL_FULL_RES);
	I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

	toAccl[0] = ACCL_PWR_CTL;
	toAccl[1] = ACCL_MEASURE;
	I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

	toAccl[0] = ACCL_BW_RATE;
	toAccl[1] = ACCL_RATE_100HZ;
	I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

	toAccl[0] = ACCL_OFFSET_X;
	toAccl[1] = 0x00;
	I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

	toAccl[0] = ACCL_OFFSET_Y;
	toAccl[1] = 0x00;
	I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

	toAccl[0] = ACCL_OFFSET_Z;
	toAccl[1] = 0x00;
	I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);
}

/* Function to read raw accelerometer data into a vector with x, y, z. */
static vector3_t get_raw_accl_data(void) {
	char fromAccl[] = { 0, 0, 0, 0, 0, 0, 0 }; // starting address, placeholders for data to be read.
	vector3_t acceleration;
	uint8_t bytesToRead = 6;

	fromAccl[0] = ACCL_DATA_X0;
	I2CGenTransmit(fromAccl, bytesToRead, READ, ACCL_ADDR);

	acceleration.x = (fromAccl[2] << 8) | fromAccl[1];
	acceleration.y = (fromAccl[4] << 8) | fromAccl[3];
	acceleration.z = (fromAccl[6] << 8) | fromAccl[5];

	return acceleration;
}

/* Returns mean content in a circular buffer for the accelerometer.
 * Similar to adc_average_buffer but the difference is the int types returned. */
static int16_t acc_average_buffer(circBuf_t buffer) {
	int32_t sum = 0;
	uint8_t i;
	for (i = 0; i < BUF_SIZE; i++) {
		sum = sum + readCircBuf(&buffer);
	}
	/* This method of determining the average allows us to forego using floats.
	 * To get around floats, the sum is doubled then halved later.
	 * It guarantees that the average calculated will be >0.5 (BUFF_SIZE/(2*BUFF_SIZE)).
	 * Then if sum is not 0, this method will round upwards if needed. */
	return ((2 * sum + BUF_SIZE) / 2 / BUF_SIZE);
}

/* Function to read raw accelerometer data into a vector with x, y, z and then
 * convert it to the appropriate units based on unit_state. */
vector3_t get_accl_data(void) {
	vector3_t raw_vector = get_raw_accl_data();
	writeCircBuf(&x_buffer, raw_vector.x);
	writeCircBuf(&y_buffer, raw_vector.y);
	writeCircBuf(&z_buffer, raw_vector.z);

	vector3_t raw_vector_avg;
	raw_vector_avg.x = acc_average_buffer(x_buffer);
	raw_vector_avg.y = acc_average_buffer(y_buffer);
	raw_vector_avg.z = acc_average_buffer(z_buffer);

	return raw_vector_avg;
}

/* Detects whether a step was taken based on comparing the magnitude to the last magnitude.
 * Returns true if the threshold for a step has been surpassed. */
bool detect_step(vector3_t acceleration) {
	int16_t mag_acc = sqrt(
			(acceleration.y * acceleration.y)
					+ (acceleration.z * acceleration.z)
					+ (acceleration.x * acceleration.x));

	/* Gets rid of the effect of gravity from the magnitude. */
	int16_t mag_acc_final = mag_acc - acc_average_buffer(mag_buffer);
	writeCircBuf(&mag_buffer, mag_acc);
	/* TODO: Make the threshold dynamic.
	 * The threshold 21 has been calculated based on the lower bound average walking speed (0.8m per second).
	 * The accelerometer can detect up to 1g on each axis and the raw units are in a range of [0, ~256].
	 * 0.8 / 9.807 = ~0.081
	 * 0.081 * 256 = ~21 */
	if (mag_acc_final > 21) {
		return true;
	}
	return false;
}
