# About the project

## Introduction

The goal of this project is to enable simulation of Zephyr-based applications
for quadcopters, robots and other mechanical appliances on a local PC, with use
of a robotics simulator, without the requirement for hardware
(*Software-In-The-Loop testing*, *SIL* or *SITL* in short).

The inspiration came from [Betaflight](https://betaflight.com/),
[Ardupilot](https://ardupilot.org/) and [ROS](https://www.ros.org/), that all
provide a SITL environment by integrating with [Gazebo Simulator](https://gazebosim.org/home)
to test their various features, that normally require a working hardware
set-up - autopilots, AHRS systems, control algorithms and many others.

## Software-in-the-loop

SITL approach provides many benefits for embedded robotic application development:
- firmware can be developed regardless of project's hardware status,
- Improves safety by allowing simulation of interactions with mechanical components,
- Test iteration will be faster,
- Enables rapid changes and parametrisation of environment conditions,
  locations, materials and physical components.

This comes with a few requirements and limiations:
- Code must be designed and developed with good hardware abstraction, so that
  it's possible to plug-in drivers for simulated hardware without changing
  application logic.
- Requires development of extra drivers for simulated components.
- Development of complex models will take time and requires knowledge of the
  target simulator platform.

## Gazebo Simulator

Gazebo is primarly a robotics simulator - allows to create and control physical
models of robots, attach sensors and simulate various environmental conditions.
One advantage of Gazebo is the ability to interact with the simulation from other
programs and scripts through it's messaging infractructure, based on a
subscribe-publish communication protocol and protobuf messages.

## Zephyr native

