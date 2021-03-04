#include "DS3231.h"
#include <iostream>
#include <unistd.h>

int main()
{
	DS3231 rtc = DS3231(1, 0x68);	// i2c-1 and address 0x68
	rtc.readDateAndTime();
	rtc.readTemperature();
	sleep(0.5);

	// (alarm, hour, minute, day/date, alarm mode)
	rtc.setAlarm(0, 19, 24, 4);
	
	if(rtc.isAlarmSet(0))
		std::cout << "Alarm 1 flag is set" << std::endl;
	
	if(rtc.isAlarmSet(1))
		std::cout << "Alarm 2 flag is set" << std::endl;
	
	return 0;
}
