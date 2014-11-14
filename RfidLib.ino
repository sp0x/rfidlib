#include <dht.h>
#include <SoftwareSerial.h> 
#include "std.h"
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

	Serial.begin(9600);
	rfid = rfidUtils(2, 3);
	Serial.println("Serial number will be displayed here if a card is detected by the module:\n");
	// set the data rate for the SoftwareSerial port 
	delay(10);
	rfid.setMode(READ_SERIALS);
	//rfid.write(0x02); //Send the command to RFID, please refer to RFID manual 



	return; //Closed because of previous project
}

byte*cmdbuff = new byte[64];
void loop()
{
	size_t rbCount = 0;
	int * rbuff;
	
	size_t cmdsz = 0;
	while (Serial.available())
	{
		byte a = (byte)Serial.read();
		if (a == 10) break;
		cmdbuff[cmdsz++] = a;
		//rfid.appendCmdHex(a);
		delay(10);
	}
	//rbuff=rfid.commitCommand(rbCount);
	if (cmdsz > 0){
		rbuff = rfid.executeInput(cmdbuff, cmdsz, rbCount);
	}
	else
	{
		rbuff = rfid.readAll(rbCount);
	}
	
	/*getDistance();
	checkDHT();*/
	//////parseInput(); 
	//////paiag();
	/* add main program code here */
}




