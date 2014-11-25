#ifndef STD_ARDUINO
#define STD_ARDUINO
#include <Arduino.h>
#include <stdlib.h>


struct based{
	String value;
	based(int var, unsigned char base = 0, bool endln=false){
		if (base == 0) base = 10;
		this->value = String(var, base);
		if (endln) this->value = this->value + "\n\r"; 
	}
	based(const char * val, unsigned char base = 0, bool endln = false){
		this->value = val;
		if (endln) this->value = this->value + "\n\r";
	}
	based(float var, unsigned char base = 0, bool endln = false){
		char * ptr = (char*)calloc(6, sizeof(char));
		dtostrf(var, 4, 2, ptr);
		this->value = String(ptr);
		if (endln) this->value = this->value + "\n\r"; 
	}
};
#endif