cmake_minimum_required(VERSION 3.8)
project(mocap_px4_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
  add_compile_options(-Wno-class-memaccess)
endif()

# find dependencies
find_package(rclcpp REQUIRED)
find_package(px4_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(tracker_bridge src/tracker_bridge.cpp)
ament_target_dependencies(tracker_bridge
  rclcpp
  geometry_msgs
  px4_msgs
)
install(TARGETS tracker_bridge DESTINATION lib/${PROJECT_NAME})

ament_package()