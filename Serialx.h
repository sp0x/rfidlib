#ifndef SERIAL_x
#define SERIAL_X
#include "Arduino.h"
#define S_SOFTWARE 1
#define S_HARDWARE 2


class Serialx
	: public SoftwareSerial {
private:
	void * hwSerial;
	unsigned char serialType;

public:
	Serialx(const SoftwareSerial& ser) : SoftwareSerial(ser) {
		this->serialType = S_SOFTWARE;
	}
	Serialx(const HardwareSerial& hser) : SoftwareSerial(254,255) {
		this->serialType = S_HARDWARE;
		this->hwSerial = (void*)&hser;
	}
	Serialx(unsigned char rx, unsigned char tx, unsigned char stype, bool inverse = false )  : SoftwareSerial(rx,tx, inverse) {
		switch (stype){
		case 1:	this->hwSerial = new SoftwareSerial(rx, tx, inverse); break;
		case 2:
			break;
			//not implemented
		}
		this->serialType = stype;
	}



	Serialx& operator << (based& val){
		switch (serialType){
		case 1: print(val.value);  break;
		case 2: ((HardwareSerial*)hwSerial)->print(val.value); break;
		}
	}
};


#endif