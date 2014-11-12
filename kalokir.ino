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

#pragma region "DHT"
dht DHT;
int pinDHT = 4;
int dhtOld[2] = {0,0};
int dhtMinDiffH = 3;
int dhtMinDiffT = 2;
#pragma endregion

#pragma region "Distance ultrasonic"
int pinUdis_echo = 12;
int pinUdis_trig = 13;
#pragma endregion



const char nl = '\n';
int gotData = NULL;

String readln()
{ return Serial.readStringUntil(nl); }
int readlnInt()
{ Serial.readStringUntil(nl).toInt(); }
void initDHT()
{
	Serial.println(DHT_LIB_VERSION);
	Serial.println();
	Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");
}
bool checkDHT()
{
	int chk = DHT.read11(pinDHT);
	switch (chk)
	{
	case DHTLIB_OK:
		//Serial.print("OK,\t");
		break;
	case DHTLIB_ERROR_CHECKSUM:
		Serial.print("Checksum error,\t");
		break;
	case DHTLIB_ERROR_TIMEOUT:
		//Serial.print("Time out error,\t");
		break;
	default:
		Serial.print("Unknown error,\t"); 
		break;
	}
	if (chk != 0) return chk;

	int dhtCur[2] = { DHT.humidity, DHT.temperature };

	if (dhtOld[0] == 0 || (abs(dhtOld[0] - dhtCur[0]) >= dhtMinDiffH)) // check humidity
	{
		Serial.print("humidity changed to "); Serial.println(dhtCur[0]);
		dhtOld[0] = dhtCur[0];
	}
	if (dhtOld[1] == 0 || (abs(dhtOld[1] - dhtCur[1]) >= dhtMinDiffT)) // check temperature
	{
		Serial.print("temperature changed to "); Serial.println(dhtCur[1]);
		dhtOld[1] = dhtCur[1];
	}
	
}
void parseInput()
{
	if (Serial.available() > 0)
		gotData = readlnInt();

	if (gotData != NULL)
	{
		special = gotData;
		gotData = NULL;
		Serial.println("Changed special pin to " + special);
	}
}

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
	for (int i = rLow; i <= rHigh; i++)
	{
		pinMode(i, OUTPUT);
	}

	pinMode(pinUdis_trig, OUTPUT);
	pinMode(pinUdis_echo, INPUT);
	initDHT();
	Serial.println("Kalokir awaiting your commands!");
}


void loop()
{
	while (Serial.available())
	{
		int a = rfid.parseInput(Serial.read());
		rfid.appendCmdHex(a);
		delay(10);
	}
	rfid.writeCommand();	
	rfid.readAll();
	/*getDistance();
	checkDHT();*/
	//////parseInput(); 
	//////paiag();
	/* add main program code here */
}

void getDistance()
{
	long duration, inches, cm;

	// The sensor is triggered by a HIGH pulse of 10 or more microseconds.
	// Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
	digitalWrite(pinUdis_trig, LOW);
	delayMicroseconds(2);
	digitalWrite(pinUdis_trig, HIGH);
	delayMicroseconds(10);
	digitalWrite(pinUdis_trig, LOW);

	// Read the signal from the sensor: a HIGH pulse whose
	// duration is the time (in microseconds) from the sending
	// of the ping to the reception of its echo off of an object.

	duration = pulseIn(pinUdis_echo, HIGH);

	// convert the time into a distance
	cm = duration / 29 / 2;// speed of light is 390m/s = 29 microseconds per cm, from and to,  microsecondsToCentimeters(duration);
	if (cm > 0){
		Serial.print(cm);
		Serial.print("cm");
		Serial.println();
	}
}
void paiag()
{
	for (int i = rLow; i <= rHigh; i++)
	{
		if (i == special)
		{
			analogWrite(i, brightness);
			brightness = brightness + fadeAmount;
			if (brightness == 0 || brightness == 200) {
				fadeAmount = -fadeAmount;
			}
		}
		else
		{
			digitalWrite(i, HIGH);			delay(iDelay);
			digitalWrite(i, LOW);			delay(iDelay);
		}

	}
}



