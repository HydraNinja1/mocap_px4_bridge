# ROS2 SLAM PX4 Bridge

A collection of ROS 2 bridge nodes for integrating external localisation systems with PX4. The package provides utilities for converting SLAM, motion capture, tracker and IMU data into formats compatible with PX4's visual odometry interface, enabling indoor and outdoor autonomous navigation.

The package includes automatic calibration tools for transforming SLAM coordinates into the PX4 NED (North-East-Down) frame using GPS and IMU measurements.

---

## Features

* **DLIO FRD Bridge**

  * Converts DLIO odometry into a Forward-Right-Down (FRD) frame.
  * Publishes PX4-compatible `VehicleOdometry`.

* **DLIO NED Bridge**

  * Automatically estimates the rotation between the SLAM frame and the PX4 NED frame.
  * Uses IMU orientation to initialise the transformation.
  * Supports optional visual odometry fusion directly into PX4.
  * Publishes:

    * SLAM in NED
    * SLAM in a user-defined map frame
    * GPS Cartesian position for debugging

* **Motion Capture Bridge**

  * Converts motion capture poses into PX4 `VehicleOdometry`.

* **Tracker Bridge**

  * Bridges external tracking systems into PX4-compatible messages.

* **IMU Bridge**

  * Converts external IMU messages into the coordinate conventions expected by PX4.

* **Map Calibration Tool**

  * Interactive utility for calibrating an outdoor map.
  * Averages GPS measurements at two reference locations.
  * Computes stable map origin parameters.
  * Saves calibration directly to a ROS 2 parameter YAML file.

---

## Package Structure

```
slam_px4_bridge/
├── config/
│   └── dlio_ned_bridge.yaml
├── src/
│   ├── dlio_frd_bridge.cpp
│   ├── dlio_mocap_bridge.cpp
│   ├── dlio_ned_bridge.cpp
│   ├── imu_bridge.cpp
│   ├── map_calibration.cpp
│   └── tracker_bridge.cpp
├── CMakeLists.txt
└── package.xml
```

---

## Dependencies

* ROS 2 Humble
* PX4 ROS 2 Interface (`px4_msgs`)
* Eigen3
* nav_msgs
* geometry_msgs
* sensor_msgs
* rclcpp

---

## Building

Clone the repository into your ROS 2 workspace:

```bash
cd ~/ws/src
git clone <repository_url>
```

Build the package:

```bash
cd ~/ws
colcon build --packages-select slam_px4_bridge
source install/setup.bash
```

---

## Map Calibration

Before running the outdoor SLAM bridge, generate a map calibration file.

Launch the calibration node:

```bash
ros2 run slam_px4_bridge map_calibration
```

The calibration process:

1. Waits for a valid GPS fix.
2. Prompts the user to position the vehicle at the bottom-left map corner.
3. Averages GPS measurements for a configurable duration.
4. Prompts the user to move to the top-left corner.
5. Repeats the averaging process.
6. Saves the resulting calibration to a ROS 2 YAML parameter file.

Example output:

```yaml
/**:
  ros__parameters:
    map_bl_lat: 51.xxxxxxxxx
    map_bl_lon: 0.xxxxxxxxx
    map_bl_alt: xxx.xxx

    map_tl_lat: 51.xxxxxxxxx
    map_tl_lon: 0.xxxxxxxxx
    map_tl_alt: xxx.xxx
```

---

## Running the NED Bridge

Launch using the generated calibration file:

```bash
ros2 run slam_px4_bridge dlio_ned_bridge \
    --ros-args \
    --params-file config/dlio_ned_bridge.yaml
```

During initialisation the bridge:

* Collects several seconds of SLAM and IMU data.
* Estimates the SLAM → NED rotation.
* Computes the map frame transformation.
* Begins publishing transformed odometry.

---

## Published Topics

Depending on configuration, the package publishes:

* `/dlio/odom_node/odom_bridged`
* `/dlio/odom_node/odom_bridged_ned`
* `/dlio/odom_node/odom_bridged_map`
* `/fmu/in/vehicle_visual_odometry`
* `/fmu/out/gps_cartesian`

---

## Applications

This package is intended for:

* Outdoor LiDAR SLAM with PX4
* Indoor motion capture integration
* External visual odometry fusion
* Autonomous UAV localisation
* GPS-assisted SLAM initialisation
* Research and development using PX4 and ROS 2

---

## License

Specify your chosen open-source license (MIT, BSD-3-Clause, Apache-2.0, etc.).
# mocap_px4_bridge
