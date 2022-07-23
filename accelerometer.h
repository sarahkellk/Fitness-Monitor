/*
 * File: accelerometer.h
 *
 * Authors: Kenneth Huang, Sarah Kellock
 *
 * Date: March 2022
 *
 * Module for using the ADXL345 accelerometer functionality including
 * reading and handling data.
 *
 */

#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} vector3_t;

typedef enum {
	DISPLAY_RAW, DISPLAY_G, DISPLAY_MS2
} display_unit;

/* Initializes accelerometer. */
void initAccl(void);

/* Function to read raw accelerometer data into a vector with x, y, z and then
 * convert it to the appropriate units based on unit_state. */
vector3_t get_accl_data(void);

bool detect_step(vector3_t acceleration);

#endif /* ACCELEROMETER_H */
