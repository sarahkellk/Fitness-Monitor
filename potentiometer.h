/*
 * File: potentiometer.h
 *
 * Authors: Kenneth Huang, Sarah Kellock
 *
 * Date: April 2022
 *
 * Header file for potentiometer functionality.
 */

#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

/* Returns the mean value of the circular buffer used to store potentiometer data.
 * Similar to acc_average_buffer but the difference is the int types returned.*/
uint16_t get_potentiometer_data(void);

/* Routine for ADC interrupt which writes value that is read into the circular buffer. */
void ADCIntHandler(void);

/* Initialize potentiometer and the required peripherals needed. */
void init_potentiometer(void);

#endif /* POTENTIOMETER_H */
