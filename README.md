# General Information
- The **wheelbase** (or horizontal "height") of the robot, is **44.5cm / 17.5in**
- The **track width** (or horizontal "width") of the robot, is **12 + (13/16) inches**

# IMPORTANT CONFIGURATION INFORMATION
- every time LemLib is reinstalled, **please go to** `include/lemlib/pid.hpp` and make the `PID.kP`, `PID.kI` and `PID.kD` constants *public class variables!*

# Specific Information
- PLEASE NOTE that when you are tuning PID values with the physical tuner, if you try to set negative PID constants, *only their absolute value will be taken*
