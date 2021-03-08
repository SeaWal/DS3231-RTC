#include <iostream>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <string>

#include "Device.h"
#include "DS3231.h"

using std::string;

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
DS3231::DS3231(unsigned int bus, unsigned int device, bool hasBattery) : Device(bus, device)
{
	if(hasBattery){
		// autoset the time and date
		std::time_t t = std::time(0);
		std::tm* now = std::localtime(&t);
		int yr = (now->tm_year+1900) - 2000;
		this->writeToReg(DAY, now->tm_wday);
		this->setDate(yr, now->tm_mon+1, now->tm_mday);
		this->setTime(now->tm_hour, now->tm_min, now->tm_sec);
	}
}


/**
 * Write the value to the file in sysfs
 * 
 * @param path : The path to the sysfs file
 * @param fname : The name of the file to write to
 * @param value: The value to be written
 */
 void DS3231::writeSysfs(string path, string fname, string value)
 {
	 std::ofstream fs; // open output stream to file
	 fs.open( (path+fname).c_str() );
	 fs << value;
	 fs.close();
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
* Read and display the time the alarm is set for
*
* @param which : 0 for alarm 1, 1 for alarm 2
*/
void DS3231::readAlarm(bool which)
{
	unsigned char hr = 0;
	unsigned char min = 0;
	if(which==0){
		hr = this->readFromReg(ALHR);
		min = this->readFromReg(ALMIN);
	}
	else {
		hr = this->readFromReg(AL2HR);
		min = this->readFromReg(AL2MIN);
	}
	
	printf("Alarm set for %02d:%02d\n", bcd2dec(hr), bcd2dec(min));
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


/**
 * Flash the LED at the given gpio number for
 * n_iters (default 5 iterations)
 * 
 * @param gpioNumber : The gpio pin number to drive
 * @param n_iters : The number of iterations to perform
 */ 
void DS3231::flashLED(int gpioNumber, int n_iters)
{
	using std::to_string;
	
	string path = "/sys/class/gpio/";
	string ledPath = path + "gpio" + to_string(gpioNumber) + "/";
	this->writeSysfs(path, "export", to_string(gpioNumber));
	usleep(10000);
	this->writeSysfs(ledPath, "direction", "out");
	usleep(10000);	
	
	// flash the LED on/off for n_iters
	for(int i=0; i<=n_iters; i++) {
		this->writeSysfs(ledPath, "value", "1");
		usleep(500000);
		this->writeSysfs(ledPath, "value", "0");
		usleep(500000);
	}
	
	this->writeSysfs(path, "unexport", to_string(gpioNumber));
}


/**
 * Toggle the INTCN bit to enable/disable SQW
 * 
 * @param enable : 1 to enable INTCN, 0 to disable
 */ 
void DS3231::toggleSQWInt(bool enable)
{
	unsigned char ctrl = this->readControlReg();
	if(enable)
		ctrl |= (1<<2);
	else
		ctrl &= (0xfb);
	
	this->writeToReg(CTRLREG, ctrl);
}


/**
 * Set the frequency of the squarewave output
 * 
 * @param Freq f : Select from the enumerated frequency
 * 				   options (LOW, MLOW, MHIGH, HIGH)
 */  
void DS3231::setSQWFreq(DS3231::Freq f)
{
	unsigned char ctrl = this->readControlReg();
	switch(f)
	{
		case DS3231::Freq::LOW:
			std::cout << "Setting low SQW Freq = 1 Hz" << std::endl;
			ctrl &= (0xe7);
			break;
			
		case DS3231::Freq::MLOW:
			std::cout << "Setting mid-low SQW Freq = 1.024 kHz" << std::endl;
			ctrl |= (0x08);
			break;
			
		case DS3231::Freq::MHIGH:
			std::cout << "Setting mid-high SQW Freq = 4.096 kHz" << std::endl;
			ctrl |= (0x10);
			ctrl &= (0xf7);
			break;
			
		case DS3231::Freq::HIGH:
			std::cout << "Setting high SQW Freq = 8.192 kHz" << std::endl;
			ctrl |= (0x18);
			break;
			
		default:
			std::cout << "Setting default SQW Freq = 1Hz" << std::endl;
			ctrl &= (0xe7);
			break;
	}
	
	this->writeToReg(CTRLREG, ctrl);
}
	
