#include <iostream>
#include <ctime>
#include <iomanip>

#include "Device.h"
#include "DS3231.h"

// DS3231 Registers
#define SECONDS 	0x00	// seconds register
#define MINUTES 	0x01	// minutes register
#define HOURS   	0x02	// hours register
#define DAY			0x03	// day of week
#define DATE		0x04	// day of month
#define MONTH		0x05	// month
#define YEAR		0x06	// year
#define ALSEC		0x07	// alarm seconds
#define ALMIN		0x08	// alarm minutes
#define ALHR		0x09	// alarm hours
#define ALDAY		0x0A	// alarm day/date
#define AL2MIN		0x0B	// alarm 2 minutes
#define AL2HR		0x0C	// alarm 2 hours
#define AL2DAY		0x0D	// alarm 2 day/date
#define CTRLREG 	0x0E	// control register
#define CTRLSTAT	0x0F	// control/status
#define AGOFFSET	0x10	// aging offset
#define TEMPMSB		0x11	// msb of temperature
#define TEMPLSB		0x12	// lsb of temperature


/**
 * Construct the DS3231 and set the initial time/date
 * Uses sysfs to access i2c bus
 * 
 * @param bus : The bus number (e.g. i2c-1)
 * @param device : The address of the device in the bus (e.g. 0x68)
 */ 
DS3231::DS3231(unsigned int bus, unsigned int device) : Device(bus, device)
{
	// autoset the time and date
	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);
	int yr = (now->tm_year+1900) - 2000;
	this->writeToReg(DAY, now->tm_wday);
	this->setDate(yr, now->tm_mon+1, now->tm_mday);
	this->setTime(now->tm_hour, now->tm_min, now->tm_sec);
}


/**
 * Convert from bcd to decimal format
 * 
 * @param b : The bcd number to be converted
 * @return : Decimal representation
 */
int DS3231::bcd2dec(char b) { return (b/16)*10 + (b%16); }


/**
 * Convert from decimal to bcd
 * 
 * @param b : The decimal to be converted
 * @return : BCD representation
 */
char DS3231::dec2bcd(int b) { return ( ((b/10)*16) + (b%10) ); }


/**
 * Read the date and time registers and
 * print them to cmdline
 */ 
void DS3231::readDateAndTime()
{
	unsigned char sec = this->readFromReg(SECONDS);
	unsigned char min = this->readFromReg(MINUTES);
	unsigned char hr  = this->readFromReg(HOURS);
	unsigned char day = this->readFromReg(DATE);
	unsigned char mon = this->readFromReg(MONTH);
	unsigned char yr  = this->readFromReg(YEAR);
	
	printf("The RTC time is %02d:%02d:%02d", bcd2dec(hr), bcd2dec(min), bcd2dec(sec));
	printf(" and the date is %02d-%02d-%02d\n", bcd2dec(day), bcd2dec(mon), bcd2dec(yr));
}


/**
 * Read the temperature registers and
 * print them to cmdline
 */ 
void DS3231::readTemperature()
{
	unsigned char msb = this->readFromReg(TEMPMSB);
	unsigned char lsb = (this->readFromReg(TEMPLSB)>>6) * 0.25;
	
	printf("RTC temperature is %02d.%02d\n", msb, lsb);
}


/**
 * Set the clock's time
 * 
 * @param hr : The time in hours (24 hour clock)
 * @param min : The minutes
 * @param sec : The seconds
 */ 
void DS3231::setTime(int hr, int min, int sec)
{
	this->writeToReg(HOURS, dec2bcd(hr));
	this->writeToReg(MINUTES, dec2bcd(min));
	this->writeToReg(SECONDS, dec2bcd(sec));
}


/**
 * Set the date of year
 * 
 * @param yr : The last 2 digits of the year (e.g. 97, 21, etc.)
 * @param month : The month represented in numbers (e.g. 3, 11, etc.)
 * @param day : The day of the month (1-31)
 */ 
void DS3231::setDate(int yr, int month, int day)
{
	this->writeToReg(YEAR, dec2bcd(yr));
	this->writeToReg(MONTH, dec2bcd(month));
	this->writeToReg(DATE, dec2bcd(day));
}


/**
 * Read the control register byte
 * 
 * @return ctrl : The control register state
 */ 
unsigned char DS3231::readControlReg()
{
	unsigned char ctrl = this->readFromReg(CTRLREG);
	// std::cout << "Control Register : " <<  +ctrl << std::endl; // +ctrl promotes the uchar so it can be seen
	return ctrl;
}


/**
 * Set the specified bit of the control register
 * 
 * @param bit : The number of the bit to be set (0-7)
 */ 
void DS3231::writeControlReg(unsigned char bit)
{
	unsigned char value = this->readControlReg();
	value = value | (1<<bit);
	this->writeToReg(CTRLREG, value);	
}


/**
 * Set the specified alarm to the specified time and date
 * 
 * @param which : 0 to represent alarm 1, 1 for alarm 2
 * @param hr : The hour to set the alarm
 * @param min : The minute to set
 * @param day : Either the day of week (1-7) or month (1-31)
 * 		depends on mode
 * @param mode : 0 (default) set the day of month, 1 sets day of week
 */ 
void DS3231::setAlarm(bool which, int hr, int min, int day, bool mode)
{
	// if which is 0, set alarm 1
	if(which==0){
		this->writeToReg(ALHR, dec2bcd(hr));
		this->writeToReg(ALMIN, dec2bcd(min));
		if(mode==1) day = (char) (day | (1<<6)); // set alarm 1 enable bit
		this->writeToReg(ALDAY, day);
		this->writeControlReg(DS3231::CTRLBits::AL1E);
	} 
	
	// else which is 1, set alarm 2
	else if(which==1) {
		this->writeToReg(AL2HR, dec2bcd(hr));
		this->writeToReg(AL2MIN, dec2bcd(min));
		if(mode==1) day = (char) (day | (1<<6)); // set alarm 2 enable bit
		this->writeToReg(ALDAY, day);
		this->writeControlReg(DS3231::CTRLBits::AL2E);
	} 
	
	else {
		std::cout << "Could not set selected alarm\n";
	}
	
	printf("Alarm set for %02d:%02d\n", hr, min);
}


/**
 * Check if the alarm flag has been set, alarm is triggered
 * 
 * @param which : 0 for alarm 1, 1 for alarm 2
 */
bool DS3231::isAlarmSet(bool which)
{
	bool alarmFlag = false;
	unsigned char status = this->readFromReg(CTRLSTAT);
	// alarm 1
	if(which==0)
		if(status & (0x01)){
			alarmFlag = true;
			status &= 0xfe;	// clear alarm 1 flag
		}
	
	// alarm 2		
	if(which==1)
		if(status & (0x02)){
			alarmFlag = true;
			status &= 0xfd;	// clear alarm 2 flag
		}
	
	this->writeToReg(CTRLSTAT, status);
	return alarmFlag;
}
		
