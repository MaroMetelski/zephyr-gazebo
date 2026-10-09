zephyr_library_named(gz_bridge)

zephyr_library_sources(gz_bridge_impl.c gz_link_sockets.c)

list(APPEND CMAKE_MODULE_PATH ${ZEPHYR_BASE}/modules/nanopb)
include(nanopb)

set(NANOPB_GENERATE_CPP_STANDALONE OFF)

# Path to proto files of local installation of libgz-msgs
set(GZ_MSGS_PATH /usr/share/gz/gz-msgs/protos)

# Add these to Zephyr so that they can be used in all drivers?

nanopb_generate_cpp(internal_proto_srcs internal_proto_hdrs RELPATH ../../proto
    ../../proto/any.proto
    ../../proto/gz_message.proto
)

nanopb_generate_cpp(gz_proto_srcs gz_proto_hdrs RELPATH ${GZ_MSGS_PATH}
    ${GZ_MSGS_PATH}/gz/msgs/time.proto
    ${GZ_MSGS_PATH}/gz/msgs/header.proto
    ${GZ_MSGS_PATH}/gz/msgs/vector3d.proto
    ${GZ_MSGS_PATH}/gz/msgs/quaternion.proto
    ${GZ_MSGS_PATH}/gz/msgs/float_v.proto
    ${GZ_MSGS_PATH}/gz/msgs/imu.proto
    ${GZ_MSGS_PATH}/gz/msgs/double.proto
)

zephyr_include_directories(${CMAKE_CURRENT_BINARY_DIR})
zephyr_sources(${gz_proto_srcs} ${internal_proto_srcs})

string(MD5 unique_chars "${internal_proto_hdrs}")

set(gen_target_name proto_${unique_chars})

add_custom_target(${gen_target_name} DEPENDS ${internal_proto_hdrs} ${gz_proto_hdrs})

add_dependencies(nanopb_generated_headers ${gen_target_name})
add_dependencies(zephyr nanopb_generated_headers)
