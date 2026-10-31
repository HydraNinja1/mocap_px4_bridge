# MoCap PX4 Bridge

`mocap_px4_bridge` is a ROS 2 package that converts motion-capture (MoCap) pose data from a VRPN/OptiTrack rigid body into the `px4_msgs/msg/VehicleOdometry` message format used by PX4.

The package contains a single C++ ROS 2 node, `tracker_bridge`. It subscribes to a `geometry_msgs/msg/PoseStamped` topic, converts the position and orientation from the **FUR (Forward–Up–Right)** axes to a **local FRD (Forward–Right–Down)** reference frame, and republishes the converted data. Forwarding to PX4 is optional and **disabled by default**.

## Requirements

- ROS 2 (e.g. Humble)
- `rclcpp`
- `geometry_msgs`
- `px4_msgs`
- `colcon` and an existing ROS 2 workspace for building

## Build

Place this package in the `src` directory of a ROS 2 workspace, then build and source it:

```bash
cd ~/ros2_ws
colcon build --packages-select mocap_px4_bridge
source install/setup.bash
```

Make sure the ROS 2 installation and the workspace containing `px4_msgs` have been sourced before building, as applicable.

## Run

### Debug output only (default)

```bash
ros2 run mocap_px4_bridge tracker_bridge
```

This subscribes to `/vrpn_mocap/Octane/pose` and publishes the converted message to `/vrpn_mocap/out/debug`. It does **not** forward data to PX4.

### Select a different MoCap rigid body

```bash
ros2 run mocap_px4_bridge tracker_bridge --ros-args -p asset_name:=Octopus
```

This changes the default input topic to `/vrpn_mocap/Octopus/pose`.

### Enable forwarding to PX4

```bash
ros2 run mocap_px4_bridge tracker_bridge --ros-args \
  -p asset_name:=Octane \
  -p mocap_fusion:=true
```

When enabled, the node publishes both the debug topic and `/fmu/in/vehicle_visual_odometry`. The PX4 ROS 2/DDS connection must already be running and configured for the matching `px4_msgs` version.

### Verify the output

```bash
ros2 topic echo /vrpn_mocap/out/debug
ros2 topic hz /vrpn_mocap/out/debug
```

To inspect the incoming pose:

```bash
ros2 topic echo /vrpn_mocap/Octane/pose
```
## Parameters

| Parameter | Type | Default | Description |
| --- | --- | --- | --- |
| `mocap_fusion` | `bool` | `false` | Also publish the converted message to PX4 |
| `asset_name` | `string` | `Octane` | Tracked rigid-body name used to form the default input topic |
| `mocap_topic` | `string` | `/vrpn_mocap/Octane/pose` | Input MoCap pose topic (derived from `asset_name` unless overridden) |
| `mocap_bridged_topic` | `string` | `/vrpn_mocap/out/debug` | Always-on converted odometry output |
| `fmu_in_topic` | `string` | `/fmu/in/vehicle_visual_odometry` | Optional PX4 external-odometry input |

## Topics

| Direction | Topic | Message type | Description |
| --- | --- | --- | --- |
| Subscribed | `/vrpn_mocap/Octane/pose` | `geometry_msgs/msg/PoseStamped` | Raw rigid-body pose (default topic) |
| Published | `/vrpn_mocap/out/debug` | `px4_msgs/msg/VehicleOdometry` | Converted odometry, always published |
| Published (optional) | `/fmu/in/vehicle_visual_odometry` | `px4_msgs/msg/VehicleOdometry` | Converted odometry forwarded to PX4 when `mocap_fusion=true` |

The node uses ROS 2 `SensorDataQoS` for its subscriber and publishers. The input topic changes with the `asset_name` parameter unless `mocap_topic` is explicitly overridden.

For PX4 message fields and external-vision configuration, see:

- [PX4 `VehicleOdometry` message](https://docs.px4.io/main/en/msg_docs/VehicleOdometry)
- [PX4 external position estimation (MoCap/VIO)](https://docs.px4.io/main/en/ros/external_position_estimation)

## Coordinate-frame conversion

The bridge **assumes** the incoming MoCap axes use FUR. Check the coordinate-system settings of your OptiTrack/VRPN installation: the code cannot determine the source frame automatically.

| Input MoCap axis (FUR) | Output local FRD axis |
| --- | --- |
| X: Forward | X: Forward |
| Y: Up | Z: Down (negated) |
| Z: Right | Y: Right |

**Position:**

```cpp
x_frd = x_mocap;
y_frd = z_mocap;
z_frd = -y_mocap;
```

**Orientation (quaternion):**

```cpp
qx_frd = qx_mocap;
qy_frd = qz_mocap;
qz_frd = -qy_mocap;
qw_frd = qw_mocap;
```

The output quaternion is stored in PX4's `(w, x, y, z)` order. The node sets `pose_frame = 2`, corresponding to PX4's `POSE_FRAME_FRD`: a **fixed local reference frame with an arbitrary heading**, not a body-fixed frame rotating with the vehicle. The transformation assumes the source pose and tracked rigid-body axis conventions are compatible with the stated mapping.




