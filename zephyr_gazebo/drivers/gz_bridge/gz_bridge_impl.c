#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>

#include <drivers/gz_bridge.h>
#include "gz_link.h"
#include "gz/msgs/imu.pb.h"
#include "gz_message.pb.h"

#define DT_DRV_COMPAT gz_bridge

LOG_MODULE_REGISTER(gz_bridge, LOG_LEVEL_DBG);

/* Maximum amount of supported sub and pub bridge channels. Needs to be same or
larger than number of channels required by all drivers. */
#define GZ_BRIDGE_MAX_CHANNELS 10

struct bridge_channel {
    struct zbus_runtime_channel chan;
    struct zbus_channel_data chan_data;
    /* Storage for the message. */
    GazeboMessage message;
    /* Subscribe channel callback. NULL for publisher channel. */
    gz_bridge_cb sub_cb;
    /* Direction. */
    enum gz_bridge_dir dir;
};

static struct bridge_channel channels[GZ_BRIDGE_MAX_CHANNELS];
static uint8_t registered_channels = 0;

/* Common listener callback for all subscribers. */
static void gz_bridge_common_cb(const struct zbus_channel *chan, const void *msg);

static int gz_bridge_impl_register_channel(const struct device *dev, const char *topic,
    enum gz_bridge_dir dir, gz_bridge_cb cb);

ZBUS_ASYNC_LISTENER_DEFINE(gz_bridge_listener, gz_bridge_common_cb);

static struct bridge_channel *find_channel(const struct zbus_channel *chan)
{
    for (int i = 0; i < registered_channels; i++) {
        /* Compare IDs. */
        if (channels[i].chan.channel.id == chan->id) {
            return &channels[i];
        }
    }
    return NULL;
}

static void on_rx_msg(GazeboMessage *msg)
{
    /* Simply publish to one of the topic channels. */
    const struct zbus_channel *ch = zbus_chan_from_name(msg->topic);
    if (ch) {
        zbus_chan_pub(ch, msg, K_NO_WAIT);
    } else {
        LOG_ERR("Failed to publish to a channel %s", ch->name);
    }
}

static void gz_bridge_common_cb(const struct zbus_channel *chan, const void *msg)
{
    struct bridge_channel *bridge_ch = find_channel(chan);
    if (bridge_ch && bridge_ch->sub_cb && (bridge_ch->dir == GAZEBO_TO_ZEPHYR)) {
        bridge_ch->sub_cb(bridge_ch->chan.channel.name, (GazeboMessage*)msg);
    } else if (bridge_ch->dir == ZEPHYR_TO_GAZEBO) {
        gz_link_write(msg);
    }
}

static int gz_bridge_impl_register_channel(const struct device *dev, const char *topic,
    enum gz_bridge_dir dir, gz_bridge_cb cb)
{
    struct bridge_channel *ch = &channels[registered_channels];

    /* Set ID starting from 1. */
    zbus_runtime_channel_init(&ch->chan, &ch->chan_data, topic, registered_channels + 1, NULL,
        &ch->message, sizeof(ch->message), NULL);

    int ret = zbus_runtime_channel_register(&ch->chan);
    if (ret != 0) {
        LOG_ERR("Failed to register runtime channel!");
        return false;
    }
    ret = zbus_chan_add_obs(&ch->chan.channel, &gz_bridge_listener, K_NO_WAIT);
    if (ret != 0) {
        LOG_ERR("Failed to add channel observer");
        return false;
    }

    /* Set callback. */
    if (cb && (dir == GAZEBO_TO_ZEPHYR)) {
        ch->sub_cb = cb;
    }
    ch->dir = dir;
    registered_channels++;

    LOG_INF("Created Gazebo bridge channel '%d' for topic '%s'", registered_channels, topic);

    return 0;
}

static int gz_bridge_impl_send_message(const struct device *dev, const char *topic,
    const GazeboMessage *msg)
{
    /* Simply publish to one of the topic channels. Works either way. */
    const struct zbus_channel *ch = zbus_chan_from_name(topic);
    if (ch) {
        zbus_chan_pub(ch, msg, K_NO_WAIT);
    } else {
        LOG_ERR("Failed to publish to a channel %s", ch->name);
    }
    return 0;
}

static int gz_bridge_impl_init(const struct device *dev)
{
    gz_link_start(on_rx_msg);
    LOG_INF("Gazebo bridge initialized");
    return 0;
}

DEVICE_API(gz_bridge, gz_bridge_impl_api) = {
    .register_channel = gz_bridge_impl_register_channel,
    .send_message = gz_bridge_impl_send_message,
};

/* Only single bridge instance should be possible. */
DEVICE_DT_INST_DEFINE(0, gz_bridge_impl_init, NULL, NULL, NULL, POST_KERNEL, 90,
    &gz_bridge_impl_api);
