/*
 * File: potentiometer.c
 *
 * Authors: Kenneth Huang, Sarah Kellock
 *
 * Date: April 2022
 *
 * Module for potentiometer functionality.
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "circBufT.h"
#include "driverlib/sysctl.h"

#include "potentiometer.h"

#define BUF_SIZE 12

/* Circular buffer to store potentiometer data */
static circBuf_t adc_buffer;

/* Returns the mean value of the circular buffer used to store potentiometer data.
 * The value is between the range of 0 to 10000 inclusive and rounded to the 100th.
 * Similar to acc_average_buffer but the difference is the int types returned.*/
uint16_t get_potentiometer_data(void) {
	uint32_t sum = 0;
	uint8_t i;
	for (i = 0; i < BUF_SIZE; i++) {
		sum = sum + readCircBuf(&adc_buffer);
	}
	/* This method of determining the average allows us to forego using floats.
	 * To get around floats, the sum is doubled then halved later.
	 * The ADC resolution is 12 bits so to get the output to be in the range 0 to 10000...
	 * 10000 / 4096 = ~2.45
	 * Changing 2.45 to 245 then diving by 100 means we do not need to use floats and
	 * rounding is done to the 100th. */
	return (((245 * 2 * sum + BUF_SIZE) / 2 / 100 / BUF_SIZE) / 100) * 100;
}

/* Routine for ADC interrupt which writes value that is read into the circular buffer. */
void ADCIntHandler(void) {
	uint32_t ulValue;

	// Get the single sample from ADC0.  ADC_BASE is defined in
	// inc/hw_memmap.h
	ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);

	// Place it in the circular buffer (advancing write index)
	writeCircBuf(&adc_buffer, ulValue);

	// Clean up, clearing the interrupt
	ADCIntClear(ADC0_BASE, 3);
}

/* Initialize potentiometer and the required peripherals needed. */
void init_potentiometer(void) {
	initCircBuf(&adc_buffer, BUF_SIZE);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

	// Enable sample sequence 3 with a processor signal trigger.  Sequence 3
	// will do a single sample when the processor sends a signal to start the
	// conversion.
	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

	// Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
	// single-ended mode (default) and configure the interrupt flag
	// (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
	// that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
	// 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
	// sequence 0 has 8 programmable steps.  Since we are only doing a single
	// conversion using sequence 3 we will only configure step 0.  For more
	// on the ADC sequences and steps, refer to the LM3S1968 datasheet.
	ADCSequenceStepConfigure(ADC0_BASE, 3, 0,
	ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

	// Since sample sequence 3 is now configured, it must be enabled.
	ADCSequenceEnable(ADC0_BASE, 3);

	// Register the interrupt handler
	ADCIntRegister(ADC0_BASE, 3, ADCIntHandler);

	// Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
	ADCIntEnable(ADC0_BASE, 3);
}
