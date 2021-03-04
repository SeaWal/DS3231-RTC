#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <iostream>
#include <sstream>

#include "Device.h"

// I2C devices sysfs
#define I2C_0 "/dev/i2c-0"
#define I2C_1 "/dev/i2c-1"


Device::Device(unsigned int bus, unsigned int device) : bus(bus), device(device)
{
	this->file = -1;
	this->open();
}

int Device::open()
{
	std::string name;
	if(this->bus == 0) name=I2C_0;
	else name=I2C_1;
	
	if ((this->file=::open(name.c_str(), O_RDWR)) < 0){
		std::cout << "Failed to open the bus.\n";
		return 1;
	}
	
	if(ioctl(this->file, I2C_SLAVE, this->device) < 0){
		std::cout << "Failed to connect to the device.\n";
		return 1;
	}
	
	return 0;
}

int Device::writeToReg(unsigned int addr, unsigned char value)
{
	unsigned char buf[2];
	buf[0] = addr;
	buf[1] = value;
	
	if(write(this->file, buf, 2) != 2){
		std::cout << "Failed to write to register\n";
		return 1;
	}
	
	return 0;
}

unsigned char Device::readFromReg(unsigned int addr)
{
	unsigned char buf[1];
	buf[0] = addr;
	if(write(this->file, buf, 1) != 1){
		std::cout << "Failed to reset register\n";
		return 1;
	}
	
	if(read(this->file, buf, 1) != 1){
		std::cout << "Failed to read from register\n";
		return 1;
	}
	
	return buf[0];
}

void Device::close()
{
	::close(this->file);
	this->file = -1;
}

Device::~Device()
{
	if( this->file != -1)
		this->close();
}
