#ifndef _QMC5883_HPP_
#define _QMC5883_HPP_

#include <Wire.h>

// brak konfigurowalnych bitów w adresie
#define QMC_ADDRESS		((uint8_t)0x0D)

#define OSR_512         ((uint8_t)0b00000000)
#define OSR_256         ((uint8_t)0b01000000)
#define OSR_128         ((uint8_t)0b10000000)
#define OSR_64          ((uint8_t)0b11000000)

#define RNG_2G          ((uint8_t)0b00000000)
#define RNG_8G          ((uint8_t)0b00010000)

#define ODR_10HZ        ((uint8_t)0b00000000)
#define ODR_50HZ        ((uint8_t)0b00000100)
#define ODR_100HZ       ((uint8_t)0b00001000)
#define ODR_200HZ       ((uint8_t)0b00001100)

#define MODE_STANDBY    ((uint8_t)0b00000000)
#define MODE_CONTINOUS  ((uint8_t)0b00000001)
#define MODE_RESERVE1   ((uint8_t)0b00000010)
#define MODE_RESERVE2   ((uint8_t)0b00000011)


class QMC5883
{
	private:
		int16_t x, y, z;

		void writeRegister(uint8_t reg, uint8_t value);
	public:
		void init(void);
		void reset(void);

		void measure(void);
	
	public:
		int16_t getX(void) const { return this->x; }
		int16_t getY(void) const { return this->y; }
		int16_t getZ(void) const { return this->z; }
};

#endif // _QMC5883_HPP_
