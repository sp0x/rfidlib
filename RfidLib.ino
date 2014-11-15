#include <SoftwareSerial.h> 
#include "std.h"
#include "MemoryFree.h"
#include "rfidUtils.h"



int rLow = 13 - 7;
int rHigh = 13;
int iDelay = 50;
int special = 9;
int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

#pragma region "RFID for RFID Module v1.2"
int CMD[64];
int comlen = 0;
int out_flag = 0;
#define RFID_COMMAND_READ_SERIAL_AUTO 0x02;
//SoftwareSerial mySerial(2, 3); //pin2 Rx, pin3 Tx 
rfidUtils rfid;
#pragma endregion

const char nl = '\n';
int gotData = NULL;


void setup()
{
	// Open serial communications and wait for port to open:
	Serial.begin(115200);
	rfid = rfidUtils(2,3);
	Serial.println("Serial number will be displayed here if a card is detected by the module:\n");
	delay(10);
	rfid.setMode(READ_SERIALS);
	return; //Closed because of previous project
}

byte*cmdbuff = new byte[32];
void loop()
{
	int rbCount = 0;
	int * rbuff;
	size_t cmdsz = 0;
	while (Serial.available())
	{
		byte a = (byte)Serial.read();
		if (a == 10) break;
		cmdbuff[cmdsz++] = a;
		delay(10);
	}

	if (rfid.locked) return;
	if (cmdsz > 0){
		byte md;
		switch (cmdsz)
		{
		case 2:
			md = cmdbuff[1]-48;
			Serial.print("Setting mode to: "); Serial.println(md);
			rfid.setMode((rfid_mode)md);
			break;
		default:
			rbuff= processCommand(cmdbuff, cmdsz, rbCount);
			Serial.println("processed command!");
		}		
	}
	else
	{
		rbCount = rfid.GetInput(rbuff);
		if (rbCount > 0) { 
			Serial.print("Main read: "); Serial.println(rbCount);
		}
	}
}


int * processCommand(byte*cmd, size_t cmdSz, int & resSz){
	void * cmdPtr;
	int * rbuff;
	if (memmem(cmdbuff, cmdSz, &"type", 4) != NULL){
		rfid_card_type type = rfid.getCardType();
		switch (type){
		case OneS70:	Serial.println("OneS70"); break;
		case OneS50:	Serial.println("OneS50"); break;
		case DESFire:	Serial.println("DESFire"); break;
		case ProX:		Serial.println("ProX"); break;
		case UltraLight: Serial.println("UltraLight"); break;
		default:
			Serial.print("Unknown card type "); Serial.print((int)type);  Serial.println(" !");
		}
	}
	else{
		rbuff = rfid.executeInput(cmdbuff, cmdSz, resSz);
	}
	return rbuff;
}



