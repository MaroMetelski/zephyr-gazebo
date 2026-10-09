#ifndef GZ_BRIDGE_H_
#define GZ_BRIDGE_H_

#include <stdint.h>

#include <zephyr/device.h>

#include <pb.h>
#include <pb_encode.h>
#include <gz_message.pb.h>

enum gz_msg_type {
    GZ_MSG_IMU,
    GZ_MSG_ALTIMETER,
};

enum gz_bridge_dir {
    GAZEBO_TO_ZEPHYR = 0,
    ZEPHYR_TO_GAZEBO,
};

/** Gazebo bridge message callback. */
typedef void (*gz_bridge_cb)(const char *topic, GazeboMessage *msg);

struct gz_bridge_driver_api {
    /* Register a topic channel. */
    int (*register_channel)(const struct device *dev, const char *topic,
        enum gz_bridge_dir dir, gz_bridge_cb cb);
    /* Publish a message to the bridge. Should be used for sending messages from driver to gazebo. */
    int (*send_message)(const struct device *dev, const char *topic, const GazeboMessage *msg);
};

__syscall int gz_bridge_register_channel(const struct device *dev, const char *topic,
    enum gz_bridge_dir dir, gz_bridge_cb cb);

static inline int z_impl_gz_bridge_register_channel(const struct device *dev, const char *topic,
    enum gz_bridge_dir dir, gz_bridge_cb cb)
{
    const struct gz_bridge_driver_api *api = DEVICE_API_GET(gz_bridge, dev);
    if (api->register_channel == NULL) {
        return -ENOSYS;
    }

    return api->register_channel(dev, topic, dir, cb);
}

__syscall int gz_bridge_send_message(const struct device *dev, const char *topic,
    const GazeboMessage *msg);

static inline int z_impl_gz_bridge_send_message(const struct device *dev, const char *topic,
    const GazeboMessage *msg)
{
    const struct gz_bridge_driver_api *api = DEVICE_API_GET(gz_bridge, dev);
    if (api->send_message == NULL) {
        return -ENOSYS;
    }

    return api->send_message(dev, topic, msg);
}

/* Helper function to encode a nested protobuf messages inside GazeboMessage wrapper. */
static inline bool gz_bridge_encode_nested(GazeboMessage *msg, const pb_msgdesc_t *nested_desc,
    const char *url, const void *nested_msg)
{
    msg->has_payload = true;
    strncpy(msg->payload.type_url, url, sizeof(msg->payload.type_url));

    pb_ostream_t stream = pb_ostream_from_buffer(msg->payload.value.bytes,
        sizeof(msg->payload.value.bytes));

    if (!pb_encode(&stream, nested_desc, nested_msg)) {
        return false;
    }

    msg->payload.value.size = stream.bytes_written;
    return true;
}

#include <zephyr/syscalls/gz_bridge.h>

#endif /* GZ_BRIDGE_H_ */
