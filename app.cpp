#include "DS3231.h"
#include <iostream>
#include <unistd.h>

int main()
{
	DS3231 rtc = DS3231(1, 0x68);	// i2c-1 and address 0x68

	// display the initial date and time
	// and temperature
	rtc.readDateAndTime();
	rtc.readTemperature();

	// (alarm number, hour, minute, day/date, alarm mode)
	rtc.setAlarm(0, 19, 02, 5);
	
	
	// demonstrating alarm trigger which flashes LED
	for(int i=0; i<100; i++)
	{
		if(rtc.isAlarmSet(0))
		{
			std::cout << "Alarm triggered" << std::endl;
			rtc.flashLED(17);	// gpio17
			break;
		}
		rtc.readDateAndTime();
		sleep(1); // wait 1 second
	}
	
	// set the frequency of the SQW output
	rtc.setSQWFreq(rtc.Freq::MLOW);
		
	return 0;
}
