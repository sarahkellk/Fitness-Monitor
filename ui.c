/*
 * File: ui.c
 *
 * Authors: Kenneth Huang, Sarah Kellock
 *
 * Date: April 2022
 *
 * Module for the user interface functionalities including
 * the handling and operations of program states.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "driverlib/sysctl.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "driverlib/adc.h"
#include "inc/hw_memmap.h"

#include "ui.h"
#include "display.h"
#include "potentiometer.h"
#include "accelerometer.h"

/* Test mode flag */
static bool test_mode;

/* The state of the UI which determines what data to show and
 * how to program can be interacted with. */
static ui_state state;

/* State of units to display for distance and steps. */
static distance_units dist_state;
static step_units step_state;

/* Steps counted which can be set in test mode. */
static uint16_t steps_counted;

/* Distance recorded in meters.
 * Although values will be output as kilometers and miles, we want to avoid floats.
 * This value can be adjusted in test mode. */
static uint16_t distance_traveled;

/* User set step goal. */
static uint16_t step_goal;

/* A flag to keep track of whether the current goal has been reach yet.
 * This is to ensure that the user is only notified once when they reach their goal. */
static bool goal_reached_flag;

/* A value to keep track of how long the magnitude has been above the threshold.
 * A step will be registered if the magnitude is above the threshold for this duration. */
static uint16_t above_threshold_duration;

/* Initializes UI data */
void init_ui(void) {
	initDisplay();
	test_mode = false;
	OLEDStringDraw("Steps Counted", 0, 0);
	state = STEPS_COUNTED;
	dist_state = KMS;
	step_state = STEPS;
	step_goal = 1000;
	goal_reached_flag = false;
}

/* Load the state which is called during initialization or state change. */
static void load_state(ui_state state) {
	switch (state) {
	case STEPS_COUNTED:
		state = STEPS_COUNTED;
		OLEDStringDraw("Steps Counted", 0, 0);
		break;
	case DISTANCE_TRAVELED:
		state = DISTANCE_TRAVELED;
		OLEDStringDraw("Dist. Traveled", 0, 0);
		break;
	case SET_GOAL:
		state = SET_GOAL;
		OLEDStringDraw("Set Step Goal", 0, 0);
		display_val("Current", step_goal, 3);
		break;
	}
}

/* Load test mode display and toggles the flag to allow for test mode functionality. */
void toggle_test_mode(void) {
	if (!test_mode) {
		test_mode = true;
		clear_display();
		OLEDStringDraw("TEST MODE", 0, 0);
	} else {
		test_mode = false;
		clear_display();
		load_state(state);
	}
}

/* Cycle next UI state */
void next_ui_state(void) {
	clear_display();
	state = (ui_state) ((state + 1) % 3);
	load_state(state);
}

/* Cycle previous UI state */
void prev_ui_state(void) {
	clear_display();
	state = (ui_state) (state - 1 >= 0 ? state - 1 : 2);
	load_state(state);
}

/* Increments steps by 100 and distance traveled by 900 (0.09 kms).
 * This is used for testing while test mode is enabled. */
void test_increment(void) {
	if (steps_counted + 100 > 10000) {
		steps_counted = 10000;
	} else {
		steps_counted += 100;
	}
	if (distance_traveled + 90 > 9000) {
		distance_traveled = 9000;
	} else {
		distance_traveled += 90;
	}
}

/* Decrements steps by 500 and distance traveled by 450 (0.45 kms).
 * This is used for testing while test mode is enabled. */
void test_decrement(void) {
	if (steps_counted - 500 < 0) {
		steps_counted = 0;
	} else {
		steps_counted -= 500;
	}
	if (distance_traveled - 450 < 0) {
		distance_traveled = 0;
	} else {
		distance_traveled -= 450;
	}
	/* This lets us repeatedly test the functionality that notifies the user when goal reached. */
	if (steps_counted < step_goal) {
		goal_reached_flag = false;
	}
}

/* Changes the unit to display steps/distance.
 * Step and distance have separate states for which unit to output. */
void change_step_units(void) {
	//toggle units depending on current state - probably meant to be a switch
	if (get_ui_state() == DISTANCE_TRAVELED) {
		//change between kms and miles
		switch (dist_state) {
		case KMS:
			dist_state = MILES;
			break;
		case MILES:
			dist_state = KMS;
			break;
		}
	} else if (get_ui_state() == STEPS_COUNTED) {
		//switch between between raw steps and steps as a percentage of the goal
		switch (step_state) {
		case STEPS:
			step_state = GOAL_PERCENTAGE;
			break;
		case GOAL_PERCENTAGE:
			step_state = STEPS;
			break;
		}
	}
}

/* Update the goal set by the user from the potentiometer. */
void set_goal_potentiometer(void) {
	uint16_t new_goal = get_potentiometer_data();
	step_goal = new_goal;
	state = STEPS_COUNTED;
	OLEDStringDraw("                ", 0, 4);
	OLEDStringDraw("Steps Counted", 0, 0);
	goal_reached_flag = false;
}

/* Sets the step distance to 0. */
void reset_distance(void) {
	distance_traveled = 0;
	steps_counted = 0;
	goal_reached_flag = false;
}

/*Handle the display of test mode*/
void handle_test_mode_display() {
	display_val("Steps", steps_counted, 2);
	display_val_units("Dist: ", distance_traveled, 3, "km");
}

/*Handle the display of normal mode (not test mode)*/
void handle_normal_mode_display(void) {
	switch (get_ui_state()) {
	case STEPS_COUNTED:
		if (step_state == STEPS) {
			//add the units on the end of steps_counted
			display_steps(steps_counted, 2, "steps");
		} else {
			uint16_t goal_percent = (steps_counted * 100 / step_goal); /* Gets around floats */
			display_val("Goal %", goal_percent, 2);
		}
		break;
	case DISTANCE_TRAVELED:
		if (dist_state == KMS) {
			//add the units on the end of steps_counted
			display_val_units("", distance_traveled, 2, "km");
		} else {
			display_val_units("", distance_traveled / 1.609, 2, "miles");
		}
		break;
	case SET_GOAL:
		display_val("New Goal", get_potentiometer_data(), 2);
		break;
	}
}

/* Update display to show relevant UI content. */
void display_ui(void) {
	if (test_mode) {
		handle_test_mode_display();
	} else {
		handle_normal_mode_display();
	}
}

/* Returns the current UI state. */
ui_state get_ui_state(void) {
	return state;
}

/* Returns true if test mode is on */
bool is_test_mode(void) {
	return test_mode;
}

static uint16_t convert_to_dist(uint16_t steps) {
	uint16_t distance = (steps * 9) / 10;
	return distance;
}

/* Runs the UI task corresponding to the state. */
void ui_task(void) {
	switch (get_ui_state()) {
	case STEPS_COUNTED:
		break;
	case DISTANCE_TRAVELED:
		distance_traveled = convert_to_dist(steps_counted);
		break;
	case SET_GOAL:
		ADCProcessorTrigger(ADC0_BASE, 3);
		break;
	}
}

/* Checks if step goal has been reached.
 * Notify user if it has. */
void check_step_goal(void) {
	if (goal_reached_flag == false && steps_counted >= step_goal) {
		/* Notify user has reached goal */
		display_goal_reached(steps_counted, distance_traveled, step_goal);
		SysCtlDelay(SysCtlClockGet());
		goal_reached_flag = true;
		/* Return to regular display */
		clear_display();
		if (is_test_mode()) {
			OLEDStringDraw("TEST MODE", 0, 0);
		} else {
			load_state(state);
		}
	}
}

/* A function to determine whether a step should be counted.
 * The function takes a parameter min_step_duration that dictates how long a step should
 * be relative to the number of times this function is called.
 * To quantify a step...
 * The magnitude must cross the threshold.
 * The magnitude must be above threshold for at least min_step_duration times.
 * The magnitude must return below the threshold.
 * If all these conditions are met then a step is registered. */
void handle_step_event(uint8_t min_step_duration) {
	vector3_t acceleration_data = get_accl_data();
	bool step_detected = detect_step(acceleration_data);
	if (step_detected) {
		above_threshold_duration++;
	} else {
		if (above_threshold_duration >= min_step_duration) {
			steps_counted++;
		}
		above_threshold_duration = 0;
	}
}

