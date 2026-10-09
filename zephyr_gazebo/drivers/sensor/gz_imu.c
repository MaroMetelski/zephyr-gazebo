#include <zephyr/device.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include "gz/msgs/imu.pb.h"
#include "zephyr/kernel.h"

#include "gz/msgs/imu.pb.h"
#include "gz_message.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

#include <drivers/gz_bridge.h>

#define DT_DRV_COMPAT gz_imu

/* IMU data topic */
#define TOPIC_IMU DT_PROP(DT_DRV_INST(0), topic)
#define BRIDGE DT_BUS(DT_DRV_INST(0))

LOG_MODULE_REGISTER(gz_imu_sensor, CONFIG_SENSOR_LOG_LEVEL);

static struct gz_imu_data {
    double accel[3];
    double angv[3];
} inst_data;

K_MUTEX_DEFINE(imu_data_mtx);

static void imu_chan_callback(const char *topic, GazeboMessage *msg)
{
    gz_msgs_IMU imu_msg = gz_msgs_IMU_init_default;

    /* Decode the internal IMU message. */
    pb_istream_t istream = pb_istream_from_buffer(msg->payload.value.bytes,
        msg->payload.value.size);

    bool status = pb_decode(&istream, gz_msgs_IMU_fields, &imu_msg);
    if (!status) {
        LOG_ERR("Failed to decode nested message from Gazebo. %s", PB_GET_ERROR(&istream));
        return;
    }

    k_mutex_lock(&imu_data_mtx, K_FOREVER);
    inst_data.accel[0] = imu_msg.linear_acceleration.x;
    inst_data.accel[1] = imu_msg.linear_acceleration.y;
    inst_data.accel[2] = imu_msg.linear_acceleration.z;
    inst_data.angv[0] = imu_msg.angular_velocity.x;
    inst_data.angv[1] = imu_msg.angular_velocity.y;
    inst_data.angv[2] = imu_msg.angular_velocity.z;
    k_mutex_unlock(&imu_data_mtx);
}

int gz_imu_init(const struct device *dev)
{
    const struct device *bridge = DEVICE_DT_GET(BRIDGE);
    /* TODO: Handle return code. */
    (void)gz_bridge_register_channel(bridge, TOPIC_IMU, GAZEBO_TO_ZEPHYR, imu_chan_callback);

    LOG_INF("Gazebo IMU driver started");
    return 0;
}

static int gz_imu_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    return 0;
}

static int gz_imu_channel_get(const struct device *dev,
			       enum sensor_channel chan,
			       struct sensor_value *val)
{

    struct gz_imu_data *data = dev->data;

    int ret = 0;
    k_mutex_lock(&imu_data_mtx, K_FOREVER);

    switch (chan) {
    case SENSOR_CHAN_ACCEL_X:
        sensor_value_from_double(val, data->accel[0]);
        break;
    case SENSOR_CHAN_ACCEL_Y:
        sensor_value_from_double(val, data->accel[1]);
        break;
    case SENSOR_CHAN_ACCEL_Z:
        sensor_value_from_double(val, data->accel[2]);
        break;
    case SENSOR_CHAN_ACCEL_XYZ:
        for (int i = 0; i < 3; i++) {
            sensor_value_from_double(val++, data->accel[i]);
        }
        break;
    case SENSOR_CHAN_GYRO_X:
        sensor_value_from_double(val, data->angv[0]);
        break;
    case SENSOR_CHAN_GYRO_Y:
        sensor_value_from_double(val, data->angv[1]);
        break;
    case SENSOR_CHAN_GYRO_Z:
        sensor_value_from_double(val, data->angv[2]);
        break;
    case SENSOR_CHAN_GYRO_XYZ:
        for (int i = 0; i < 3; i++) {
            sensor_value_from_double(val++, data->angv[i]);
        }
        break;
    default:
        ret = -ENOTSUP;
    }
    k_mutex_unlock(&imu_data_mtx);

    return ret;
}

static DEVICE_API(sensor, gz_imu_api) = {
    .sample_fetch = gz_imu_sample_fetch,
    .channel_get = gz_imu_channel_get,
};

/* TODO: Add support for multiple instances with data / config per instance. */
SENSOR_DEVICE_DT_INST_DEFINE(0, gz_imu_init, NULL, &inst_data, NULL, POST_KERNEL,
    99, &gz_imu_api);
