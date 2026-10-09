/* motors_gazebo.c Implementation of motors.h API for simulation purposes.
 *
 */

#include "gz_message.pb.h"
#include "gz/msgs/double.pb.h"
#include "motors.h"
#include "pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include <zephyr/device.h>
#include <drivers/gz_bridge.h>

#define WHEEL_RIGHT_TOPIC "/wheel_right"
#define WHEEL_LEFT_TOPIC "/wheel_left"

#define DOUBLE_URL "/gz.msgs.Double"

/* Find the bridge node */
#define BRIDGE DT_COMPAT_GET_ANY_STATUS_OKAY(gz_bridge)

const struct device *bridge = DEVICE_DT_GET(BRIDGE);

static const char *get_topic_name(enum motor_instance motor)
{
    switch(motor) {
    case MOTOR_LEFT:
        return WHEEL_LEFT_TOPIC;
    case MOTOR_RIGHT:
        return WHEEL_RIGHT_TOPIC;
    default:
        return NULL;
    }
}

bool motors_init(void)
{
    /* Register Zephyr->Gazebo channels */
    int ret = gz_bridge_register_channel(bridge, "/wheel_right", ZEPHYR_TO_GAZEBO, NULL);
    ret = gz_bridge_register_channel(bridge, "/wheel_left", ZEPHYR_TO_GAZEBO, NULL);
    return 0;
}

bool motor_force_set(enum motor_instance motor, double force)
{
    GazeboMessage tx_msg = GazeboMessage_init_default;
    gz_msgs_Double dbl = gz_msgs_Double_init_zero;

    const char *topic_name = get_topic_name(motor);
    if (!topic_name) {
        return false;
    }

    dbl.data = force;

    strncpy(tx_msg.topic, topic_name, sizeof(tx_msg.topic));

    gz_bridge_encode_nested(&tx_msg, gz_msgs_Double_fields, DOUBLE_URL, &dbl);

    gz_bridge_send_message(bridge, topic_name, &tx_msg);

    return true;
}
