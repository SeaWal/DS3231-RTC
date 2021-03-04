#ifndef _DEVICE_H
#define _DEVICE_H

class Device
{
	
	private:
		unsigned int bus, device;
		int file;
		
	public:
		Device(unsigned int bus, unsigned int device);
		virtual int open();
		virtual int writeToReg(unsigned int addr, unsigned char value);
		virtual unsigned char readFromReg(unsigned int addr);
		virtual void close();
		virtual ~Device();
		
};

#endif
