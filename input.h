/*
 * File: input.h
 *
 * Authors: Kenneth Huang, Sarah Kellock
 *
 * Date: March 2022
 *
 * Module for supporting buttons, switches, and other input peripherals on the
 * TIVA/Orbit board.
 */

#ifndef INPUT_H
#define INPUT_H

/* Initializes all the needed inputs needed for the fitness monitor.
 * Also sets up their interrupts.
 * Wraps initButtons from buttons4 to initialize the four buttons on the TIVA/Orbit. */
void init_inputs(void);

/* Polls the buttons and handles the each button press. */
void buttons_handler(void);

#endif /* INPUT_H */
