#if !defined(_ISA_MOBILE_H_)
#define _ISA_MOBILE_H_

#include "QMC5883.h" // kompas

/*
 * Silniki, sterowanie PWM
 * 
 */
 
#define A_ENABLE		2	//PWM
#define A_PHASE			46  //0 - przod, 1 - tyl DIGITAL
	
#define B_ENABLE		3	//PWM
#define B_PHASE			48	//0 - przod, 1 - tyl DIGITAL
#define MODE			44	//sterowanie 0 - ciężkie, 1 - uproszczone DIGITAL
							// Pin MODE sterownika jest na stałe podpięty do +5V (HIGH)

/*
 * Silniki, wejście enkoderowe
 *
 */
#define ENCODER_LEFT	50	// Enkoder lewej strony
#define ENCODER_RIGHT	51	// Enkoder prawej strony	
	
enum class PowerSideEnum
{
	Left,
	Right
};



 
/*
 * Czujniki odległości
 * Moduł HC-SR04
 */

#define US_FRONT				0
#define US_FRONT_TRIGGER_PIN	8
#define US_FRONT_ECHO_PIN		9

#define US_BACK					1
#define US_BACK_TRIGGER_PIN		10
#define US_BACK_ECHO_PIN		11

#define US_LEFT					2
#define US_LEFT_TRIGGER_PIN		6
#define US_LEFT_ECHO_PIN		7

#define US_RIGHT				3
#define US_RIGHT_TRIGGER_PIN	4
#define US_RIGHT_ECHO_PIN		5
 
enum class UltraSoundSensor {
	Front = 0,
	Back = 1,
	Left = 2,
	Right = 3,
	
	__first = Front,
	__last = Right,
	
	All,
};
 
int ultrasound_trigger_pin[] = {
	[UltraSoundSensor::Front]	= US_FRONT_TRIGGER_PIN,
	[UltraSoundSensor::Back]	= US_BACK_TRIGGER_PIN,
	[UltraSoundSensor::Left]	= US_LEFT_TRIGGER_PIN,
	[UltraSoundSensor::Right]	= US_RIGHT_TRIGGER_PIN,
};

 
int ultrasound_echo_pin[] = {
	[UltraSoundSensor::Front]	= US_FRONT_ECHO_PIN,
	[UltraSoundSensor::Back]	= US_BACK_ECHO_PIN,
	[UltraSoundSensor::Left]	= US_LEFT_ECHO_PIN,
	[UltraSoundSensor::Right]	= US_RIGHT_ECHO_PIN,
};



 
#endif // _ISA_MOBILE_H_
