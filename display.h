/*
 * File: display.h
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

#ifndef DISPLAY_H
#define DISPLAY_H

/* Initializes the Orbit OLED display */
void initDisplay(void);

/* Update the display on the Orbit OLED display to show step related data. */
void display_steps(uint32_t value, uint8_t row, char *units);

/* Update the display on the Orbit OLED display in form of "prefix: value". */
void display_val(char *prefix, uint32_t value, uint8_t row);

/* Update the display on the Orbit OLED display in form of "prefix: value units". */
void display_val_units(char *prefix, uint32_t value, uint8_t row, char *units);

/* Clears the entire display to be blank. */
void clear_display(void);

/* Displays when step goal has been reached.
 * Show step and distance data. */
void display_goal_reached(uint16_t steps, uint16_t distance, uint16_t goal);

#endif /* DISPLAY_H */
