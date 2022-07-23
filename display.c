/*
 * File: display.c
 *
 * Authors: Kenneth Huang, Sarah Kellock
 *
 * Date: March 2022
 *
 * Module for updating the information shown on the Orbit OLED display.
 * This has functionality to change the unit of the output (e.g. g, m/s^2,
 * or raw units)
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "utils/ustdlib.h"

#include "display.h"

/* Initializes the Orbit OLED display */
void initDisplay(void) {
	OLEDInitialise();
}

/* Update the display on the Orbit OLED display in form of "prefix: value". */
void display_val(char *prefix, uint32_t value, uint8_t row) {
	char text_buffer[17]; /* Display fits 16 characters wide. */
	/* "Undraw" the previous contents of the line to be updated. */
	OLEDStringDraw("                ", 0, row);
	usnprintf(text_buffer, sizeof(text_buffer), "%s: %d", prefix, value);
	/* Update line on display. */
	OLEDStringDraw(text_buffer, 0, row);
}

/* Update the display on the Orbit OLED display to show step related data. */
void display_steps(uint32_t value, uint8_t row, char *units) {
	char text_buffer[17]; /* Display fits 16 characters wide. */
	/* "Undraw" the previous contents of the line to be updated. */
	OLEDStringDraw("                ", 0, row);
	usnprintf(text_buffer, sizeof(text_buffer), "%d %s", value, units);
	/* Update line on display. */
	OLEDStringDraw(text_buffer, 0, row);
}

/* Update the display on the Orbit OLED display in form of "prefix: value units". */
void display_val_units(char *prefix, uint32_t value, uint8_t row, char *units) {
	char text_buffer[17]; /* Display fits 16 characters wide. */
	/* "Undraw" the previous contents of the line to be updated. */
	OLEDStringDraw("                ", 0, row);
	usnprintf(text_buffer, sizeof(text_buffer), "%s%d.%03d %s", prefix,
			value / 1000, abs(value % 1000), units);
	/* Update line on display. */
	OLEDStringDraw(text_buffer, 0, row);
}

/* Clears the entire display to be blank. */
void clear_display(void) {
	uint8_t i;
	for (i = 0; i < 4; i++) {
		OLEDStringDraw("                ", 0, i);
	}
}

/* Displays when step goal has been reached.
 * Show step and distance data. */
void display_goal_reached(uint16_t steps, uint16_t distance, uint16_t goal) {
	clear_display();
	char steps_text_buffer[17];
	char dist_text_buffer[17];
	char goal_text_buffer[17];
	usnprintf(goal_text_buffer, sizeof(goal_text_buffer), "Step Goal: %d", goal);
	usnprintf(steps_text_buffer, sizeof(steps_text_buffer), "Steps: %d", steps);
	usnprintf(dist_text_buffer, sizeof(dist_text_buffer), "Km: %1d.%02d",
			abs(distance / 1000), abs(distance % 1000));
	OLEDStringDraw("*GOAL COMPLETE*", 0, 0);
	OLEDStringDraw(steps_text_buffer, 0, 2);
	OLEDStringDraw(dist_text_buffer, 0, 3);
	OLEDStringDraw(goal_text_buffer, 0, 1);
}
