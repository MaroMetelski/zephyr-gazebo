# Gazebo message bridge

This small script subscribes to Gazebo topics and sends the messages using
a simple socket. The messages are received in the Zephyr application by the gz_bridge
library and dispatched to subscribing drivers and modules.
