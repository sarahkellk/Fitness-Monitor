/*
 * File: main.c
 *
 * Authors: Kenneth Huang, Sarah Kellock
 *
 * Date: March 2022
 *
 * Main file for the ENCE360 fitness monitor project. This file calls
 * initialization routines and contains the main program loop.
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "driverlib/interrupt.h"
#include "utils/ustdlib.h"

#include "potentiometer.h"
#include "accelerometer.h"
#include "input.h"
#include "display.h"
#include "ui.h"

/* SAMPLE_RATE_HZ = 2 * BUF_SIZE (12) * max frequency (50Hz)
 * Nyquist theorem */
#define SAMPLE_RATE_HZ 1200

/* Can be used to schedule events. */
static uint32_t sample_count;
static uint8_t display_tick;
static uint16_t step_check_tick;
static uint8_t step_count_tick;
static uint8_t button_tick;
static uint8_t ui_task_tick;

/* Used to increment sample count and other task counts to keep track of tasks. */
static void sys_tick_int_handler(void) {
	sample_count++;
	/* Check for overflow. */
	if (1 > 0xFFFFFFFF - sample_count) {
		sample_count = 0;
	}
	step_check_tick++;
	display_tick++;
	step_count_tick++;
	button_tick++;
	ui_task_tick++;
}

/* Initialize clock and interrupts and set the clock rate to 20 MHz. */
static void initClock(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	SysTickPeriodSet(SysCtlClockGet() / SAMPLE_RATE_HZ);
	SysTickIntRegister(sys_tick_int_handler);
	SysTickIntEnable();
	SysTickEnable();
}

/* Main program loop. */
int main(void) {
	initClock();
	init_potentiometer();
	initDisplay();
	initAccl();
	init_inputs();
	init_ui();

	/* Enable interrupts to the processor. */
	IntMasterEnable();

	while (1) {
		/* Poll buttons at 50Hz */
		if (button_tick > 24) {
			buttons_handler();
			button_tick = 0;
		}

		/* Handle UI events at 40Hz */
		if (ui_task_tick > 30) {
			ui_task();
			ui_task_tick = 0;
		}

		/* Update display at 12Hz */
		if (display_tick >= 100) {
			display_ui();
			display_tick = 0;
		}

		/* Check if the step goal has been reached at 4Hz */
		if (step_check_tick >= 300) {
			check_step_goal();
			step_check_tick = 0;
		}

		/* Sample for steps at 50Hz */
		if (step_count_tick >= 24 && !is_test_mode()) {
		    /* Duration threshold is ~0.2 seconds. */
			handle_step_event(5);
			step_count_tick = 0;
		}
	}
}
