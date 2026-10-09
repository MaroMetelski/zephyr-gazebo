/* motors.h Control motors' velocity for a balance bot.
 *
 * This is a simple API for the balance bot application that allows to control
 * the velocity of the motors in the system.
 *
 * The purpose of this module is to show how to implement a simulated variant
 * of some custom hadware control logic using the gazebo bridge. In that case,
 * the implementation is provided by the 'motors_gazebo.c' file.
 */
#ifndef MOTORS_H_
#define MOTORS_H_

#include <stdbool.h>

enum motor_instance { MOTOR_LEFT,
	MOTOR_RIGHT,
};

bool motors_init(void);

/* Set force for a motor instance. */
bool motor_force_set(enum motor_instance motor, double force);

#endif /* MOTORS_H_ */
