// wheelbase 44.5cm / 17.5in
// track width 12 and 13/16 in (12.5 in)

#include "main.h"
#include "lemlib/api.hpp"

// debug
bool runningLinearPIDTest = false;

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
pros::Motor_Group left_motors({left_front, left_middle, left_back});

pros::Motor_Group right_motors({right_front, right_middle, right_back});

// "tracking wheel" (rotation sensor) encoder definitions
/** TODO: find port of rotation sensor! */
pros::Rotation h_track_wheel_rot(18);

pros::ADIEncoder v_track_wheel_adi('C', 'D');

// inertial sensor definitions
/** TODO: find port of inertial sensor! */
pros::Imu inertial_sensor(17);

// // LemLib structs
// lemlib::Drivetrain_t drivetrain {
// 	&left_motors
// 	, &right_motors
// 	, 12.5				// width in inches!
// 	, 4.125				// 4" omni-wheels
// 	, 600				// blue cartridge (600 rpm) - direct drive
// };

lemlib::Drivetrain drivetrain{
	&left_motors
	, &right_motors //
	, 12 + (13 / 16) // track width
	, 4.125 				// wheel size
	, 600 // blue cartridge (600 rpm) - direct drive
	, 2 // horizontal drift
};

// ~2.5cm away
lemlib::TrackingWheel h_track_wheel_1(
	&h_track_wheel_rot		// pointer to the rotation sensor
	, 3.25					// size of tracking wheel
	, -0.25 				// vertical offset—ESSENTIALLY 0!
);

lemlib::TrackingWheel v_track_wheel_1(
	&v_track_wheel_adi
	, lemlib::Omniwheel::NEW_275
	, -3.0625
);

lemlib::OdomSensors sensors(
	&v_track_wheel_1		// vertical tracking wheel 1
	, nullptr				// vertical tracking wheel 2
	, &h_track_wheel_1		// horizontal tracking wheel 1
	, nullptr				// horizontal tracking wheel 2
	, &inertial_sensor
);

// lateral PID controller
// kP 3, kD 0 -> gets there! bit drift but thas ok
lemlib::ControllerSettings lateralController(
	4 // proportional gain (kP)
	, 0 // integral gain (kI)
	, 0 // derivative gain (kD)
	, 0 // anti windup
	, 0 // small error range, in inches
	, 0 // small error range timeout, in milliseconds
	, 0 // large error range, in inches
	, 0 // large error range timeout, in milliseconds
	, 0 // maximum acceleration (slew)
);

// angular PID controller
// target is 90 deg, no oscillations!
// kP 1 kD 0 -> no oscillate; overshoot (~101)
// kP 2 kD 0 -> oscillate but good angle (~89.9)
//    - kD 1 - 3; oscillate AND off by ~4-6 degrees
//    - kD 5; oscillates LESS (~1 oscillation) (still off tho)
//    - kD 7; not off (~1 degree)! one oscillation :D
//	  - kD 9; not off (~2 degrees)! one oscillation + less :P
//	  - kD 10; bit more off (5 deg); " w/ regards to oscillation
//    - kD 12; "; NEGLIGIBLE OSCILLATION!
// kP 3 kD 12 -> 88 deg; 1 oscillation
//    - kD 14; not off (89 deg); 1 oscillation
//    - kD 16; NOT OFF (~90); 1 oscillation
//    - kD 18; "
//    - kD 20; no (93 degrees); NEGLIGIBLE OSCILLATION!
// kP 4 kD 20 -> 

lemlib::ControllerSettings angularController(
	4 			// proportional gain (kP)
	, 0 		// integral gain (kI)
	, 20 		// derivative gain (kD)
	, 0 		// anti windup
	, 0 		// small error range, in degrees
	, 0 		// small error range timeout, in milliseconds
	, 0 		// large error range, in degrees
	, 0 		// large error range timeout, in milliseconds
	, 0 		// maximum acceleration (slew)
);

// actual lemlib chassis object
lemlib::Chassis chassis(
	drivetrain
	, lateralController
	, angularController
	, sensors
);

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

void screenTaskFunc(void* chassis) {
	lemlib::Chassis* myChassis = (lemlib::Chassis *)(chassis);

	while (true)
	{
		pros::lcd::print(1, "Pos X (Relative): %f", myChassis->getPose().x);
		pros::lcd::print(2, "Pos Y (Relative): %f", myChassis->getPose().y);
		pros::lcd::print(3, "Bot Heading (Relative): %f", myChassis->getPose().theta);
		pros::lcd::print(4, "Current LINEAR kP: %f", lateralController.kP);
		pros::lcd::print(5, "Current LINEAR kD: %f", lateralController.kD);
		pros::lcd::print(6, "Current ANGULAR kP: %f", angularController.kP);
		pros::lcd::print(7, "Current ANGULAR kD: %f", angularController.kD);
		

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

	pros::Task screenTask(
		screenTaskFunc			// function that is the task
		, &chassis				// pointer to parameter to task
	);
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
void autonomous() {
	// lemlib::MoveToPointParams moveToParams = {
	// 	forwards: false
	// 	, maxSpeed: 127
	// 	, minSpeed: 0
	// 	, earlyExitRange: 0
	// };

	// config
	lemlib::TurnToHeadingParams turnToParams = {
		direction: AngularDirection::AUTO
		, maxSpeed: 127
		, minSpeed: 0
		, earlyExitRange: 0
	};

	
	// actual auton
	pros::delay(5000);

	// chassis.moveToPoint(0, -24, 3000, moveToParams, false);
	chassis.turnToHeading(90, 100000);
}

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

		// delay so system resources don't go nyooom
		pros::delay(20);
	}
}
