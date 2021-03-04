#ifndef _DS3231_H
#define _DS3231_H

#include "Device.h"


class DS3231 : public Device
{
	private:
		int bcd2dec(char b);
		char dec2bcd(int b);
		enum CTRLBits {AL1E, AL2E, INTCN, RS1, RS2, CONV, BBSQW, EOSC};
	
	public:
		DS3231(unsigned int bus, unsigned int device);
		virtual void readDateAndTime();
		virtual void readTemperature();
		virtual void setTime(int hr, int min, int sec);
		virtual void setDate(int yr, int month, int day);
		virtual unsigned char readControlReg();
		virtual void writeControlReg(unsigned char bit);
		virtual void setAlarm(bool which, int hr, int min, int day, bool mode=0);
		virtual bool isAlarmSet(bool which);
		
};

#endif
