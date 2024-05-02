#include "main.h"
#include "lemlib/api.hpp"
#include <map>

/**
 * TODO: move ALL of this to separate files!
*/

// constants
const int DRIVE_SPEED = 127;

// controller definition
pros::Controller controller(pros::E_CONTROLLER_MASTER);

// motor definitions
pros::Motor left_front(19);
pros::Motor left_middle(20);
pros::Motor left_back(9);

pros::Motor right_front(-15);
pros::Motor right_middle(-14);
pros::Motor right_back(-5);

// motor GROUP definitions
pros::Motor_Group left_motors({
	left_front
	, left_middle
	, left_back
});

pros::Motor_Group right_motors({
	right_front
	, right_middle
	, right_back
});

// "tracking wheel" (rotation sensor) encoder definitions
/** TODO: find port of rotation sensor! */
pros::Rotation h_track_wheel_rot(18);

// inertial sensor definitions
/** TODO: find port of inertial sensor! */
pros::Imu inertial_sensor(17);

// LemLib structs
lemlib::Drivetrain_t drivetrain {
	&left_motors
	, &right_motors
	, 12.5				// width in inches!
	, 4.125				// 4" omni-wheels
	, 600				// blue cartridge (600 rpm) - direct drive
};

lemlib::TrackingWheel h_track_wheel_1(
	&h_track_wheel_rot
	, 3.25
	, 0 /** TODO: find the vertical component of the distance from the geometric center! */	
);

lemlib::OdomSensors_t sensors { 
	nullptr							// vertical tracking wheel 1
	, nullptr						// vertical tracking wheel 2
	, &h_track_wheel_1				// horiz tracking wheel 1
	, nullptr						// horiz tracking wheel 2
	, &inertial_sensor				// inertial sensor
};

// i love PID!

// forward/backward PID
lemlib::ChassisController_t lateralController {
    8, // kP
    30, // kD
    1, // smallErrorRange
    100, // smallErrorTimeout
    3, // largeErrorRange
    500, // largeErrorTimeout
    5 // slew rate
};
 
// turning PID
lemlib::ChassisController_t angularController {
    4, // kP
    40, // kD
    1, // smallErrorRange
    100, // smallErrorTimeout
    3, // largeErrorRange
    500, // largeErrorTimeout
    0 // slew rate
};

// actual lemlib chassis object
lemlib::Chassis chassis(
	drivetrain
	, lateralController
	, angularController
	, sensors
);

struct Params {
	lemlib::Chassis* myChassis;
	pros::Rotation* myTrackWheel;
};

void tank() {
	left_motors.move((controller.get_analog(ANALOG_LEFT_Y) / 127.0) * DRIVE_SPEED);
	right_motors.move((controller.get_analog(ANALOG_RIGHT_Y) / 127.0) * DRIVE_SPEED);
}

void arcade() {
	int move = controller.get_analog(ANALOG_LEFT_Y);
	int turn = controller.get_analog(ANALOG_RIGHT_X);

	// idk if this will break?
	left_motors.move(((move + turn) / 127.0) * DRIVE_SPEED);
	right_motors.move(((move - turn) / 127.0) * DRIVE_SPEED);
}

void screenTaskFunc(void* params) {
	Params* myParams = (Params *)params;
	lemlib::Chassis* myChassis = (lemlib::Chassis *)(myParams->myChassis);
	pros::Rotation* myTrackWheel = (pros::Rotation *)(myParams->myTrackWheel);

	while (true) {
		pros::lcd::print(1, "IMU Pos X: %f", myChassis->getPose().x);
		pros::lcd::print(2, "IMU Pos Y: %f", myChassis->getPose().y);
		pros::lcd::print(3, "IMU Theta: %f", myChassis->getPose().theta);

		pros::delay(20);
	}
}


// ---------------------------------------------
// --------- actual pros functions -------------
// ---------------------------------------------


/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	// LEAVE THIS HERE
	chassis.calibrate();

	pros::lcd::initialize();
	
	// resets tracking wheel rotation sensor
	h_track_wheel_rot.reset_position();

	Params params = {&chassis, &h_track_wheel_rot};

	pros::Task screenTask(screenTaskFunc, &params);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	while (true) {
		// driver control -- tank bc codemygame is coding!
		
		arcade();

		// printing stuff that i can't pass to screenTask for some dumb reason
		pros::lcd::print(4, "Tracking Wheel Angle: %d", h_track_wheel_rot.get_angle() / 100);
		pros::lcd::print(5, "Tracking Wheel Position: %d", h_track_wheel_rot.get_position() / 100);
		pros::lcd::print(6, "Tracking Wheel Velocity: %d", h_track_wheel_rot.get_velocity());
		pros::lcd::print(7, "IMU Heading: %f", inertial_sensor.get_heading());
		
		// delay so system resources don't go nyooom
		pros::delay(20);
	}
}
