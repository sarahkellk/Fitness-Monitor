/*
 * File: ui.h
 *
 * Authors: Kenneth Huang, Sarah Kellock
 *
 * Date: April 2022
 *
 * Header file for the user interface functionalities including
 * the handling and operations of program states.
 */

#ifndef UI_H
#define UI_H

typedef enum {
	STEPS_COUNTED, SET_GOAL, DISTANCE_TRAVELED
} ui_state;

typedef enum {
	KMS, MILES
} distance_units;

typedef enum {
	STEPS, GOAL_PERCENTAGE
} step_units;

/* Initializes UI data */
void init_ui(void);

/* Changes the unit to display steps/distance.
 * Step and distance have separate states for which unit to output. */
void change_step_units(void);

/* Load test mode display and toggles the flag to allow for test mode functionality. */
void toggle_test_mode(void);

/* Cycle next UI state */
void next_ui_state(void);

/* Cycle previous UI state */
void prev_ui_state(void);

/* Increments steps by 100 and distance traveled by 900 (0.09 kms).
 * This is used for testing while test mode is enabled. */
void test_increment(void);

/* Decrements steps by 500 and distance traveled by 450 (0.45 kms).
 * This is used for testing while test mode is enabled. */
void test_decrement(void);

/* Update the goal set by the user from the potentiometer. */
void set_goal_potentiometer(void);

/* Reset the steps traveled and distance traveled to zero*/
void reset_distance(void);

/*Handle the display of test mode*/
void handle_test_mode_display(void);

/*Handle the display of normal mode (not test mode)*/
void handle_normal_mode_display(void);

/* Update display to show relevant UI content. */
void display_ui(void);

/* Returns the current UI state */
ui_state get_ui_state(void);

/* Returns true if test mode is on */
bool is_test_mode(void);

/* Runs the UI task corresponding to the state. */
void ui_task(void);

/* Checks if step goal has been reached.
 * Notify user if it has. */
void check_step_goal(void);

/* A function to determine whether a step should be counted.
 * The function takes a parameter min_step_duration that dictates how long a step should
 * be relative to the number of times this function is called. */
void handle_step_event(uint8_t min_step_duration);

#endif /* UI_H */
