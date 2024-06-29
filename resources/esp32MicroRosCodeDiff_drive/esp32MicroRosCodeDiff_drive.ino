// to publish mot speed on to the ros topic enter below command on the CLI
// $ros2 topic pub /PWM_motor_speed std_msgs/msg/Int32 "{data: 40}"

//in place of 40 you can put any value between 0-255

// TO RUN THE MOTORS ENTER BELOW COMMAND and press i,j,k, for control, kinldy NOTE all
// commands from teleop_twist wont work as this code is tailored for diff_drive 
// of ball tracker. This code works due to the parameters from 
//src/ESP32bot_one/config/ball_tracker_purpleball_robot_params.yaml
//Command $ros2 run teleop_twist_keyboard teleop_twist_keyboard

#include <micro_ros_arduino.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h>
#include <geometry_msgs/msg/twist.h>

rcl_subscription_t subscriber1;
rcl_subscription_t subscriber2;
std_msgs__msg__Int32 msg1;
geometry_msgs__msg__Twist msg2;
rclc_executor_t executor;
rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;


#define LED_PIN 2

//right motor
#define ENA 19
#define IN1 18
#define IN2 5
//left motor
#define ENB 23
#define IN3 16
#define IN4 4

int speed;
  //  int val;
void setMotorSpeed(int speedLeft, int speedRight);
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}

void error_loop(){
  while(1){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
}

  int limitToMaxValue(int value, int maxLimit) {
  if (value > maxLimit) {
    return maxLimit;
  } else {
    return value;
  }
}

void subscription_callback1(const void * msgin)
{  
  const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
  speed = msg->data;
  limitToMaxValue(speed,230);
}

//twist message cb
void subscription_callback2(const void *msgin) {
  const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;
  // if velocity in x direction is 0 turn off LED, if 1 turn on LED
  if(msg->linear.x == 0.0)
  {
    digitalWrite(LED_PIN,LOW);
  }
  else
  {
     digitalWrite(LED_PIN,HIGH);
  }
  float linear = msg->linear.x;
  float angular = msg->angular.z;
   analogWrite(ENA,speed);
   analogWrite(ENB,speed);
if (linear > 0 && abs(angular) < 4) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
} else if (linear < 0)  {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}
  // Set right motor direction and speed
  if (angular > 4) {  //turn left
   analogWrite(ENA,speed-40);
   analogWrite(ENB,speed-40);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  } else if (angular < -4 ) {  // turn right
    analogWrite(ENA,speed-40);
   analogWrite(ENB,speed-40);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  }

  if (linear == 0 && angular == 0  ) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN1, LOW);
  }
  
}



void setup() {
  //set_microros_transports();
  set_microros_wifi_transports("TP-Link_922A_EXT", "28757641", "192.168.0.144", 8888);
  pinMode(LED_PIN, OUTPUT);
   pinMode(ENA, OUTPUT);
   pinMode(ENB, OUTPUT);
   pinMode(IN1, OUTPUT);
   pinMode(IN2, OUTPUT);
   pinMode(IN3, OUTPUT);
   pinMode(IN4, OUTPUT);

   digitalWrite(IN1, LOW);
   digitalWrite(IN2, LOW);
   digitalWrite(IN3, LOW);
   digitalWrite(IN4, LOW);
  digitalWrite(LED_PIN, HIGH);

  speed = 230;
  delay(2000);

  allocator = rcl_get_default_allocator();

   //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "micro_ros_arduino_node", "", &support));

    // create subscriber for getting motor PWM speed
  RCCHECK(rclc_subscription_init_default(
    &subscriber1,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "PWM_motor_speed"));

  // create subscriber
  RCCHECK(rclc_subscription_init_default(
    &subscriber2,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
    "cmd_vel"));

  // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber1, &msg1, &subscription_callback1, ON_NEW_DATA));
   RCCHECK(rclc_executor_add_subscription(&executor, &subscriber2, &msg2, &subscription_callback2, ON_NEW_DATA));

}

void loop() {
  delay(10);
  RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10)));
}
