/* comments for CODER's reference */

// ---------- SIZE OF COMMENTS  -----------
// ------- (FOR VARIABLE DOCSTRINGS) ------

/**
 * IMPORTS:
*/
#include <cmath>

#include "main.h"

#include "lemlib/api.hpp"

#include "liblvgl/lvgl.h"

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
pros::MotorGroup left_motors({
	19		// left_front
	, 20	// left_middle
	, 9		// left_back
});

pros::MotorGroup right_motors({
	-15		// right_front
	, -14	// right_middle
	, -5	// right_back
});

// "tracking wheel" (rotation sensor) encoder definitions
/** TODO: find port of rotation sensor! */
pros::Rotation h_track_wheel_rot(18);

pros::ADIEncoder v_track_wheel_adi('C', 'D');

pros::ADIEncoder kp_tuner('A', 'B', true);

pros::ADIEncoder kd_tuner('G', 'H');

// inertial sensor definitions
/** TODO: find port of inertial sensor! */
pros::Imu inertial_sensor(17);

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
lemlib::ControllerSettings lateralController(
	1 // proportional gain (kP)
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
lemlib::ControllerSettings angularController(
	1 			// proportional gain (kP)
	, 0 		// integral gain (kI)
	, 0 		// derivative gain (kD)
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

// lv_obj_t* home_screen = lv_obj_create(NULL);
Screen home_screen = Screen();
lv_obj_t* robot_central_screen = lv_obj_create(NULL);
pros::Motor test_motor(10);

void temperature_btn_cb(lv_event_t* e) {
	lv_obj_t* robot_central_screen = (lv_obj_t*)lv_event_get_user_data(e);
	lv_scr_load(robot_central_screen);
}

void update_test_motor_1(lv_timer_t* timer) {
	lv_obj_t* test_motor_1_label = (lv_obj_t*)timer->user_data;

	const char* temp_string = "Value: %.2f";
	char value_str[32];
	snprintf(value_str, sizeof(value_str), temp_string, test_motor.get_temperature());

	lv_label_set_text(test_motor_1_label, value_str);
}

void brain_gui() {
	lv_obj_t* team_obj = home_screen.add_obj(
		nullptr

		, 10	// x
		, 10	// y
		
		, 200	// w
		, 50	// h
	);

	lv_obj_t* team_label = home_screen.add_label(
		team_obj
		, "3457F Fuse"
		, true
	);

	lv_obj_t* temperature_btn = home_screen.add_btn(
		nullptr

		, 10
		, 150

		, 150
		, 50

		, temperature_btn_cb
		, LV_EVENT_CLICKED
		, robot_central_screen
	);

	lv_obj_t* temperature_btn = lv_btn_create(home_screen);
	lv_obj_set_pos(temperature_btn, 10, 150);
	lv_obj_set_size(temperature_btn, 150, 50);
	lv_obj_add_event_cb(temperature_btn, temperature_btn_cb, LV_EVENT_CLICKED, robot_central_screen);

	lv_obj_t* temperature_label = lv_label_create(temperature_btn);
	lv_obj_center(temperature_label);

	lv_obj_t* test_motor_1_obj = lv_obj_create(robot_central_screen);
	lv_obj_set_pos(test_motor_1_obj, 10, 10);
	lv_obj_set_size(test_motor_1_obj, 130, 60);

	lv_obj_t* test_motor_1_label = lv_label_create(test_motor_1_obj);
	lv_obj_center(test_motor_1_label);

	lv_timer_t* test_motor_1_timer = lv_timer_create(update_test_motor_1, 20, test_motor_1_label);
	
	// while (1) {
	// 	lv_task_handler();
	// 	lv_label_set_text_fmt(test_motor_1_label, "Motor Temperature: %f", test_motor.get_temperature());
	// 	pros::delay(20);
	// }

	lv_scr_load(home_screen);	
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

	lv_init();
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
void competition_initialize() {
	
}

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
	// pros::Task brainGui(brain_gui);
	brain_gui();
}
