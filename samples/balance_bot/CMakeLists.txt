cmake_minimum_required(VERSION 3.22.1)

# Add gazebo bridge snippet to enable the bridge
list(APPEND SNIPPET gz-bridge)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(balance_bot C)

target_sources(app PRIVATE
    src/main.c
    src/motors_gazebo.c
)
