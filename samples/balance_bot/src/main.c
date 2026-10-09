#include <stdbool.h>
#include <math.h>

#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <drivers/gz_bridge.h>
#include <zephyr/drivers/sensor.h>
#include <dsp/controller_functions.h>

#include "motors.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define IMU_NODE DT_ALIAS(imu)

const struct device *imu = DEVICE_DT_GET(IMU_NODE);

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Define whether wheels turn in the same direction as positive pitch or reverse. */
#define REVERSE 1

static double get_pitch(const double *accel)
{
	return -(atan2(accel[0], sqrt(accel[1] * accel[1] + accel[2] * accel[2])) * 180.0) / M_PI;
}

/* PID values for the simulated balance bot. */
static arm_pid_instance_f32 pid = {
	.Kd = 11.2,
	.Ki = 0.005,
	.Kp = 9.8,
};

static double pitch_setpoint = 0.0;

int main(void)
{
	bool reverse = (bool)REVERSE;
	arm_pid_init_f32(&pid, 1);
	motors_init();

	while (true) {
		struct sensor_value val[3] = {};
		double accel[3] = {};

		/* Get IMU data. */
		sensor_sample_fetch(imu);
		sensor_channel_get(imu, SENSOR_CHAN_ACCEL_XYZ, val);

		for (int i = 0; i < 3; i++) {
			accel[i] = sensor_value_to_double(&val[i]);
		}

		/* Calculate pitch. */
		double pitch = get_pitch(accel);

		/* Get PID output. */
		double err = pitch_setpoint - pitch;
		double force = (double)arm_pid_f32(&pid, err);

		if (reverse) {
			force = -force;
		}

		/* This should make the bot go in circles. */
		// double const_right = 200.0;
		// double const_left = -150.0;

		/* Apply force to motors. */
		motor_force_set(MOTOR_LEFT, force);
		motor_force_set(MOTOR_RIGHT, force);

		/* Loop delay. */
		k_sleep(K_MSEC(10));
	}

	return 0;
}
