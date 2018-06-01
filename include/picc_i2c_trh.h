#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#ifndef PICC_I2C_TRH_H
#define PICC_I2C_TRH_H

#ifndef I2C_DEV_FILE
#define I2C_DEV_FILE "/dev/i2c-1" //default I2C device for Raspberry Pi 3
#endif /* I2C_DEV_FILE */

#ifndef I2C_ADDR
#define I2C_ADDR 0x40 //base I2C slave address
#endif //I2C_ADDR

/* Device Specific Things */

typedef enum { //registers available
	TEMPERATURE = 0x00,
	HUMIDITY = 0x01,
	CONFIGURATION = 0x02,
	MANUFACTURER_ID = 0xFE,
	DEVICE_ID = 0xFF,
	SERIAL_ID_FIRST = 0xFB,
	SERIAL_ID_MID = 0xFC,
	SERIAL_ID_LAST = 0xFD,
} hdc1010_ptrs ;

typedef union {
	uint8_t rawData ;
	struct {
		//register bits
		uint8_t HumidityMeasurementResolution : 2; //8,9 [00 = 14bit res, 01 = 11bit res, 10 = 8bit res]
		uint8_t TemperatureMeasurementResolution : 1; //10 [0 for 14 bit, 1 for 11 bit]
		uint8_t BatteryStatus : 1; //11 [power indicator]
		uint8_t ModeOfAcquisition : 1; //12 [0 for stand-alone, 1 for simultaneous]
		uint8_t Heater : 1; //13 [0: Heater off, 1: Heater on.]
		uint8_t ReservedAgain : 1; //14 [Reserved bit]
		uint8_t SoftwareReset : 1; //15 [Reset device at 1]
	} ;	
} hdc1010_regs ;

typedef struct {
	uint8_t address ; //device address
	int i2cdevbus ; //i2c bus opened for the device
	uint8_t mode : 2 ; //acquisition mode flag
	/*
	mode = 00 : Non-simultaneous measurement
	mode = 01 : Simultaneous measurement
	*/
	uint8_t trh : 2 ; //2 bit read flag
	/*
	_trh = 11 : T and RH data available
	_trh = 10 : T data available, RH has been read
	_trh = 01 : RH data available, T has been read
	_trh = 00 : Make measurement again
	*/

	/*
	_trh flag and _TRH_BUF are used only when simultaneous measurements are being made.
	*/

	uint32_t trh_buf ;

} hdc1010_dev ;

#define HDC1010_POW16 65536.0

#ifndef PICC_TIME_USEC
#define PICC_TIME_USEC 30000 //30ms (14bit temp: ~7ms, 14bit RH: ~7ms), with lots of head room.
#endif

/* Methods for HDC1010 */

#define PICC_CONVERT_T(x,y) ((((uint16_t)x<<8|y)*1.0/65536.0)*165-40) //higher byte, lower byte
#define PICC_CONVERT_H(x,y) ((((uint16_t)x<<8|y)*1.0/65536.0)*100)

#define PICC_WORD_T(x) ((x*1.0/65536.0)*165-40) //word
#define PICC_WORD_RH(x) (x*100./65536.)

#define PICC_INT_T(x)(PICC_WORD_T((uint16_t)(x>>16))) //dword
#define PICC_INT_RH(x)(PICC_WORD_RH((uint16_t)x)

hdc1010_dev * hdc1010_begin( uint8_t address ) ; //open and return the pointer to the device
uint16_t hdc1010_readMfId( hdc1010_dev * d ) ; //return manufacturer ID

uint16_t hdc1010_readDevId( hdc1010_dev * ) ; //return device ID

hdc1010_regs hdc1010_readReg( hdc1010_dev * ) ; // read configuration register

void hdc1010_writeReg ( hdc1010_dev * , hdc1010_regs reg ) ; //write register to device

void hdc1010_acquisition_mode ( hdc1010_dev * , uint8_t ) ; // other input is boolean

void hdc1010_heatUp ( hdc1010_dev * , uint8_t ) ; //number of seconds to heat up

float hdc1010_readT ( hdc1010_dev * ) ; //read temperature from device

float hdc1010_readRH ( hdc1010_dev * ) ; //read humidity from device

void hdc1010_sleep ( uint32_t ) ; //number of ms to sleep, wrapped around usleep
void hdc1010_free ( hdc1010_dev * ) ;

/* Internal functions */

uint16_t hdc1010_readData ( hdc1010_dev * , uint8_t ) ;

void hdc1010_writeData ( hdc1010_dev * , uint8_t ) ;

void hdc1010_readBytes ( hdc1010_dev * , uint8_t * , uint8_t ) ;

uint32_t hdc1010_getTRH ( hdc1010_dev * ) ;

/**************************************/

#endif //PICC_I2C_TRH_H