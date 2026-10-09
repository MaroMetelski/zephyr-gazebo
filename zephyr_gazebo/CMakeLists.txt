# Module implements gazebo bridge subsystem, add syscalls to Zephyr
list(APPEND SYSCALL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(SYSCALL_INCLUDE_DIRS ${SYSCALL_INCLUDE_DIRS} PARENT_SCOPE)

add_subdirectory(drivers/sensor/)
add_subdirectory(drivers/gz_bridge/)

zephyr_include_directories(include/)
