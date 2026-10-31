#include <array>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <filesystem>
#include <fstream>
#include <iomanip>

#include "rclcpp/rclcpp.hpp"
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <px4_msgs/msg/vehicle_odometry.hpp>

using namespace std;
using std::placeholders::_1;
using geometry_msgs::msg::PoseStamped;
using px4_msgs::msg::VehicleOdometry;

class TrackerBridge : public rclcpp::Node {
public:
    TrackerBridge() : Node("tracker_bridge") {
        enable_mocap_fusion_ = this->declare_parameter<bool>("mocap_fusion", false);
        asset_name_ = this->declare_parameter<std::string>("asset_name", "Octane");

        mocap_topic_ = this->declare_parameter<std::string>("mocap_topic", "/vrpn_mocap/" + asset_name_ + "/pose");
        mocap_bridged_topic_ = this->declare_parameter<std::string>("mocap_bridged_topic", "/vrpn_mocap/out/debug");
        fmu_in_topic_ = this->declare_parameter<std::string>("fmu_in_topic", "/fmu/in/vehicle_visual_odometry");

        rclcpp::QoS qos = rclcpp::SensorDataQoS();
        mocap_sub_ = this->create_subscription<PoseStamped>(mocap_topic_, qos, std::bind(&TrackerBridge::repost_callback, this, _1));

        mocap_bridged_pub_ = this->create_publisher<VehicleOdometry>(mocap_bridged_topic_, qos);

        if (enable_mocap_fusion_) {
            fmu_in_pub_ = this->create_publisher<VehicleOdometry>(fmu_in_topic_, qos);
        }
        
        RCLCPP_INFO(get_logger(), "Tracker Bridge Initialised");
    }

private:
    void repost_callback(const PoseStamped::SharedPtr in_msg) {
      // Reading mocap message 
      const auto& stamp = in_msg->header.stamp;

      // produce position 
      float x = (float)in_msg->pose.position.x;
      float y = (float)in_msg->pose.position.z;
      float z = -1 * (float)in_msg->pose.position.y; 

      array<float, 3> position = {x,y,z};

      // produce rotation 
      float qx = (float)in_msg->pose.orientation.x;
      float qy = (float)in_msg->pose.orientation.z;
      float qz = -1* (float)in_msg->pose.orientation.y;
      float qw = (float)in_msg->pose.orientation.w;

      array<float, 4> q = {qw,qx,qy,qz};

      RCLCPP_INFO(this->get_logger(), "stamp: '%i'", stamp.sec);

      VehicleOdometry out{};
      uint64_t timestamp_us = static_cast<uint64_t>(stamp.sec) * 1000000ULL  + static_cast<uint64_t>(stamp.nanosec) / 1000ULL;
      out.timestamp = timestamp_us;
      out.timestamp_sample = 0;
      out.pose_frame = 2;
      out.position = position;
      out.q = q;

      mocap_bridged_pub_->publish(out);

      if (enable_mocap_fusion_) {    
          fmu_in_pub_->publish(out);    
      }
    }

    rclcpp::Subscription<PoseStamped>::SharedPtr mocap_sub_;
    rclcpp::Publisher<VehicleOdometry>::SharedPtr mocap_bridged_pub_;
    rclcpp::Publisher<VehicleOdometry>::SharedPtr fmu_in_pub_;

    bool enable_mocap_fusion_{false};
    std::string asset_name_, mocap_topic_, mocap_bridged_topic_, fmu_in_topic_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TrackerBridge>());
    rclcpp::shutdown();
    return 0;
}
