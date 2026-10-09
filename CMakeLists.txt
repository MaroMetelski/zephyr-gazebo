# Module implements gazebo bridge subsystem, add syscalls to Zephyr
list(APPEND SYSCALL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(SYSCALL_INCLUDE_DIRS ${SYSCALL_INCLUDE_DIRS} PARENT_SCOPE)

# Compile and link protobuf messages
include(${ZEPHYR_ZEPHYR_GAZEBO_MODULE_DIR}/cmake/gz_proto.cmake)

add_subdirectory(drivers/sensor/)
add_subdirectory(drivers/gz_bridge/)

zephyr_include_directories(include/)
