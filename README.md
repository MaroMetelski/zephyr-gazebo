# Zephyr RTOS + Gazebo simulator

This project provides integration between Zephyr RTOS and Gazebo for the purpose
of testing robotic application running Zephyr RTOS in a simulated environment
(SITL testing). It consists of the following parts:

- Gazebo communication bridge, allowing to exchange messages between Zephyr apps
  and Gazebo. The bridge consists of Zephyr-side driver and a PC-side Python script
  that communicate using a socket.

- Collection of Zephyr drivers that provide simulated implementation of common
  Zephyr APIs. The drivers use the bridge to communicate with Gazebo.

- Example applications, that show how to use the builtin drivers and how to create
  custom logic using the Gazebo bridge.

The project is a [Zephyr module](https://docs.zephyrproject.org/latest/develop/modules.html),
which means that it can be easily integrated into and used by other Zephyr projects.

To learn more about the ideas behind this project visit the [About](doc/about.md) page. \
If you want to get the project up and running, start from the [Getting Started guide](doc/getting_started.md). \
Visit [documentation](doc/index.md) for more information.
