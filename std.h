#ifndef STD_ARDUINO
#define STD_ARDUINO
#include <Arduino.h>
#include <stdlib.h>


struct based{
	String value;
	based(int var, unsigned char base = 0){
		if (base == 0) base = 10;
		this->value = String(var, base);
	}
	based(const char * val, unsigned char base = 0){
		this->value = val;
	}
	based(float var, unsigned char base = 0){
		char * ptr = (char*)calloc(6, sizeof(char));
		dtostrf(var, 4, 1, ptr);
		this->value = String(ptr);
	}
};
#endif