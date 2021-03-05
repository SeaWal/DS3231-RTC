#ifndef _DS3231_H
#define _DS3231_H

#include "Device.h"
#include <string>

using std::string;


class DS3231 : public Device
{
	private:
		int bcd2dec(char b);
		char dec2bcd(int b);
		enum CTRLBits {AL1E, AL2E, INTCN, RS1, RS2, CONV, BBSQW, EOSC};
		
		void writeSysfs(string path, string fname, string value);
	
	public:
		DS3231(unsigned int bus, unsigned int device, bool hasBattery=false);
		
		enum Freq { LOW, MLOW, MHIGH, HIGH };
		
		virtual void readDateAndTime();
		virtual void readTemperature();
		virtual void setTime(int hr, int min, int sec);
		virtual void setDate(int yr, int month, int day);
		virtual unsigned char readControlReg();
		virtual void writeControlReg(unsigned char bit);
		virtual void setAlarm(bool which, int hr, int min, int day, bool mode=0);
		virtual bool isAlarmSet(bool which);
		virtual void flashLED(int gpioNumber, int n_iters=5);
		virtual void toggleSQWInt(bool enable);
		virtual void setSQWFreq(DS3231::Freq f);
		
};

#endif
