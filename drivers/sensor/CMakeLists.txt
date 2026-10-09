zephyr_library()
zephyr_library_sources_ifdef(CONFIG_GZ_IMU_SENSOR gz_imu.c)
