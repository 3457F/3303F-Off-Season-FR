/* comments for CODER's reference */

// ---------- SIZE OF COMMENTS  -----------
// ------- (FOR VARIABLE DOCSTRINGS) ------

/**
 * IMPORTS:
*/
#include <cmath>
#include <algorithm>
#include <string>

#include "main.h"

#include "lemlib/api.hpp"

/**
 * CONSTS:
*/

// enum pid_consts {
// 	P = 0
// 	, I = 1
// 	, D = 2
// };

/**
 * CONFIG VARS:
*/
// whether PID is being tuned, or normal
// driver control should run
bool tuningPID = true;
// whether the physical PID tuner is being 
// used, or P, I, and D values are being set
// manually, through the C++ structs
// `lateralController` and
// `angularController`
bool usingPhysicalPIDTuner = true;
// if `tuningPID` is set to `true`, whether
// LINEAR PID is being tuned, or ANGULAR
// PID is being tuned
bool runningLinearPIDTest = false;


/**
 * RUNTIME VARS: (**DO NOT MODIFY**)
*/
// if `tuningPID` is set to `true`, whether
// the robot is CURRENTLY running a test
// auton with target kP and kD values!
bool runningPIDTest = false;
// represents the TARGET (NOT actual) kP,
// that you want to be set
int kp_target = 0;
// represents the TARGET (NOT actual) kD,
// that you want to be set
int kd_target = 0;

lemlib::MoveToPointParams linearPIDTestMoveToPointParams = {
	forwards: false
	, maxSpeed: 127
	, minSpeed: 0
	, earlyExitRange: 0
};

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

void screenTaskFunc(void* chassis) {
	lemlib::Chassis* myChassis = (lemlib::Chassis *)(chassis);

	while (true) {
		char runningPIDTestBufferString[20];

		// the space in the format string is important... don't delete it!
		std::snprintf(
			runningPIDTestBufferString
			, sizeof(runningPIDTestBufferString)
			, " PID Test? %s"
			, runningPIDTest ? "YES" : "NO"
		);

		// first value: whether tuning PID / normal driver control is enabled
		// second value: if tuning PID, whether test PID auton running or not
		pros::lcd::print(
			0
			, "Tuning? %s%s"
			, tuningPID ? "YES" : "NO"
			, tuningPID ? runningPIDTestBufferString : ""
		);

		pros::lcd::print(1, "Pos X (Relative): %f", myChassis->getPose().x);
		pros::lcd::print(2, "Pos Y (Relative): %f", myChassis->getPose().y);
		pros::lcd::print(3, "Bot Heading (Relative): %f", myChassis->getPose().theta);

		// printing target and actual kP, kD values
		pros::lcd::print(
			4
			, "TARGET %s kP: %i"
			, runningLinearPIDTest ? "LINEAR" : "ANGULAR"
			, kp_target
		);

		pros::lcd::print(
			5
			, "TARGET %s kD: %i"
			, runningLinearPIDTest ? "LINEAR" : "ANGULAR"
			, kd_target
		);

		pros::lcd::print(
			6
			, "CURRENT %s kP: %f"
			, runningLinearPIDTest ? "LINEAR" : "ANGULAR"
			, runningLinearPIDTest ? myChassis->lateralPID.kP : myChassis->angularPID.kP 
		);

		pros::lcd::print(
			7
			, "CURRENT %s kD: %f"
			, runningLinearPIDTest ? "LINEAR" : "ANGULAR"
			, runningLinearPIDTest ? myChassis->lateralPID.kD : myChassis->angularPID.kD
		);
		
		pros::delay(20);
	}
}

std::vector<std::string> split(const std::string& _input,
                               const std::string& delimiter) {
	std::vector<std::string> tokens;

	std::string source = _input;

	size_t pos = 0;

	while ((pos = source.find(delimiter)) != std::string::npos) {
		tokens.push_back(source.substr(0, pos));

		source.erase(0, pos + delimiter.length());
	}

	// returns last entry AFTER delimiter
	tokens.push_back(source);

	return tokens;
}

void makeLowerCase(std::string& str) {
	std::transform(
		str.begin()		// passes in the
		, str.end()		// full string to be transformed

		, str.begin()	// section of string to start inserting
						// transformed string into
		
		// basically lambda function that returns lowercase version of
		// each character in the string
		, [](unsigned char c) { return std::tolower(c); }
	);
}

/**
 * inspired by code from: https://github.com/meisZWFLZ/OverUnder781X
*/
void tuningCLI() {
	lemlib::PID* pid = &(runningLinearPIDTest ? chassis.lateralPID
											  : chassis.angularPID);

	while (tuningPID) {
		try {
			// informs user they can start typing command
			std::cout << "enter command> ";

			// fetches command (WAITS UNTIL NEWLINE)
			// and formats it to lowercase
			std::string input;
			getline(std::cin, input);
			makeLowerCase(input);

			auto params = split(input, " ");

			std::string command = params.at(0);

			if (command == "s" || command == "set") {
				if (params.size() < 3) {
					std::cout << "not enough arguments to process request (need 3!)..." << std::endl;
					continue;
				}

				std::string whichGain = params.at(1);
				std::string valueToSetStr = params.at(2);

				float *constToSet = nullptr;


				if (whichGain == "p") {
					constToSet = &(pid->kP);
				
				} else if (whichGain == "i") {
					constToSet = &(pid->kI);
				
				} else if (whichGain == "d") {
					constToSet = &(pid->kD);
				
				} else {
					std::cout << " | INVALID gain to set!" << std::endl;
					continue;
				}

				// stores old value of the PID constant, to inform the user later
				float oldValue = (*constToSet);

				// initializes `valueToSet`
				float valueToSet = -1;

				/**
				 * if the user types in a certain string as the second parameter (that is, NOT a float),
				 * fetches the PID constant they want to change, and adds or subtracts 1 or 2, accordingly
				 * to get its new value
				*/
				if (valueToSetStr == "+1") {
					valueToSet = (*constToSet) + 1;
				} else if (valueToSetStr == "-1") {
					valueToSet = (*constToSet) - 1;
				
				} else if (valueToSetStr == "+2") {
					valueToSet = (*constToSet) + 2;
				} else if (valueToSetStr == "-2") {
					valueToSet = (*constToSet) - 2;
				
				} else {
					try {
						// tries to convert user's desired gain value to a float
						valueToSet = std::stof(valueToSetStr);
					} catch (const std::invalid_argument &e) {
						// we were scammed! the user didn't pass in a float!
						std::cout << " | Gain value not a valid float!" << std::endl;
						continue;
					}
				}

				// sets the new value of the PID constant!
				(*constToSet) = valueToSet;

				printf(" | successfully changed gain value! old value: %f, new value %f!\n", oldValue, *constToSet);

			} else if (command == "g" || command == "get") {
				if (params.size() < 2) {
					std::cout << "not enough arguments to process request (need 2!)..." << std::endl;
					continue;
				}

				std::string whatInfo = params.at(2);

				if (whatInfo == "mode") {
					std::cout << " | currently tuning " << (runningLinearPIDTest ? "LINEAR" : "ANGULAR") << " PID" << std::endl;
				
				} else if (whatInfo == "p") {
					std::cout << " | kP: " << pid->kP << std::endl;
				
				} else if (whatInfo == "i") {
					std::cout << " | kI: " << pid->kI << std::endl;
				
				} else if (whatInfo == "d") {
					std::cout << " | kD: " << pid->kD << std::endl;
				
				} else {
					std::cout << " | INVALID gain to fetch info for!" << std::endl;
				}
			
			} else if (command == "turn-left") { // returns robot to original position IF tuning angular PID
				if (!runningLinearPIDTest) {
					chassis.setPose(0, 0, 0);

					chassis.turnToHeading(-90, 1500, {}, false);
				}

			} else if (command == "run" || command == "r") {
				runningPIDTest = true;

				// resets position before runs, in case test auton is being run multiple times
				chassis.setPose(0, 0, 0);

				switch (runningLinearPIDTest) {
					// running linear PID test
					case true:
						chassis.moveToPoint(0, -24, 3000, linearPIDTestMoveToPointParams, false);
						
						break;
					default:
						chassis.turnToHeading(90, 1500, {}, false);

						pros::delay(1000);
				}
			
			} else if (command == "stop") {
				chassis.cancelAllMotions();
			
			} else if (command == "exit") {
				std::cout << " | switching to driver control..." << std::endl << std::endl << "---" << std::endl << std::endl;

				chassis.cancelAllMotions();
				tuningPID = false;

				break;
			
			} else {
				std::cout << "| not a valid command..." << std::endl;
			}
		} catch (std::exception e) {
			std::cout << " | something went wrong: " << e.what() << std::endl;
		}
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
	// opcontrol runs forever! (while in driver control; it's its own task so we gucci)
	while (true) {
		if (!tuningPID) {
			/* normal driver control */

			bool X_pressed = controller.get_digital(DIGITAL_X);
			bool A_pressed = controller.get_digital(DIGITAL_A);

			// if both the X and A buttons are pressed
			if (X_pressed && A_pressed) {
				printf("X and A were both pressed, transferring to tuning PID mode...\n\n---\n\n");
				tuningPID = true;
			}

			// replace with tank() if u really don't like tank that much
			arcade();
		} else {
			/* tuning PID! wee! */

			tuningCLI();
		}

		// delay to save system resources
		pros::delay(20);
	}
}
