/*
 * File: input.c
 *
 * Authors: Kenneth Huang, Sarah Kellock
 *
 * Date: March 2022
 *
 * Module for supporting buttons, switches, and other input peripherals on the
 * TIVA/Orbit board.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "buttons4.h"
#include "driverlib/sysctl.h"

#include "ui.h"

/* Flag to keep track of the most recent switch state to check
 * whether the switch has been flip. */
static int32_t last_switch_state;

/* Returns true if the button is a long press (2 seconds). */
static bool is_long_press(uint8_t button) {
	uint16_t tick = 0;
	updateButtons();
	while (checkButton(button) == NO_CHANGE) {
		tick++;
		updateButtons();
		/* Delay for ~1ms 3000/3 = 1000HZ */
		SysCtlDelay(SysCtlClockGet() / 3000);
		if (tick >= 2000) {
			return true;
		}
	}
	return false;
}

/* When the up button is pushed, cycle through the units to display. */
static void handle_button_up(void) {
	switch (checkButton(UP)) {
	case PUSHED:
		change_step_units();
		break;
	case RELEASED:
		break;
	}
}

/* When the down button is pushed handle behaviour depending on mode.
 * Used to set goal while in set goal state and not in test state. */
static void handle_button_down(void) {
	switch (checkButton(DOWN)) {
	case PUSHED:
		if (get_ui_state() == SET_GOAL) {
			set_goal_potentiometer();
		} else if (get_ui_state() != SET_GOAL && is_long_press(DOWN)) {
			reset_distance();
		}
		break;
	case RELEASED:
		break;
	}
}

/* When the left button is pushed, cycle previous through UI states. */
static void handle_button_left(void) {
	switch (checkButton(LEFT)) {
	case PUSHED:
		prev_ui_state();
		break;
	case RELEASED:
		break;
	}
}

/* When the right button is pushed, cycle next through UI states. */
static void handle_button_right(void) {
	switch (checkButton(RIGHT)) {
	case PUSHED:
		next_ui_state();
		break;
	case RELEASED:
		break;
	}
}

/* Checks whether the switch has been flipped and what state it is in.
 * Toggles test_mode accordingly. */
static void handle_switch_1(void) {
	int32_t current_state = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_7);
	if (current_state != last_switch_state) {
		last_switch_state = current_state;
		toggle_test_mode();
	}
}

/* When the up button is pushed then increment the steps.
 * This should only be called in test mode. */
static void handle_button_up_test(void) {
	switch (checkButton(UP)) {
	case PUSHED:
		test_increment();
		break;
	case RELEASED:
		break;
	}
}

/* When the down button is pushed then decrement the steps.
 * This should only be called in test mode. */
static void handle_button_down_test(void) {
	switch (checkButton(DOWN)) {
	case PUSHED:
		test_decrement();
		break;
	case RELEASED:
		break;
	}
}

/* Polls the buttons and handles the each button press. */
void buttons_handler(void) {
	updateButtons();
	if (is_test_mode()) {
		handle_button_up_test();
		handle_button_down_test();
	} else {
		handle_button_up();
		handle_button_down();
		handle_button_right();
		handle_button_left();
	}
	handle_switch_1();
}

/* Initializes all the needed inputs needed for the fitness monitor.
 * Also sets up their interrupts.
 * Wraps initButtons from buttons4 to initialize the four buttons on the TIVA/Orbit. */
void init_inputs(void) {
	initButtons();

	/* Initialize the switch SW1 */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_7);
	GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA,
	GPIO_PIN_TYPE_STD_WPD);

	/* Need to make sure that last switch state is initialized to 0 to start in test mode if
	 * the switch is up on start.*/
	last_switch_state = 0;
}
