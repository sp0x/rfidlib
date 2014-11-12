#ifndef RFID_UTILS
#define RFID_UTILS
static uint8_t PIN_RX = 3;
static uint8_t PIN_TX = 2;

enum rfid_mode {
	DETECT = 1, READ_SERIALS, STORE
};
enum rfid_card_type{
	Mifare_onek_S50 = 4, Mifare_onek_S70 = 2
};
enum rfid_key_type{
	typeA=0x00	, typeB=0x01
};


#include "Arduino.h"

#include <SoftwareSerial.h> 

class rfidUtils
{
  public:
	rfidUtils(uint8_t rx, uint8_t tx);
	rfidUtils();
#pragma region Variables
	SoftwareSerial* serial;
	int CMD[64];
	int comlen;
	int out_flag;
	int parseInput(byte input);
	int* parseInput(int* cs, int size);
	void rfidUtils::executeInput(int * input, size_t inputsz);
#pragma endregion

#pragma region IO
	size_t write(const char * chr);
	size_t write(uint8_t chr);
	int read();
	int readAll();
	int* readBlock(int num, size_t & sz, rfid_key_type key_type);
	int available();
#pragma endregion

#pragma region Commands
	void appendCmdHex(int cmdByte);
	bool writeCommand();
	bool writeCommand(int * cmd, size_t cmd_size);
	int * rfidUtils::cmd(prog_uchar cmd, prog_uchar * cmdData, size_t datalen, size_t respLen);
	rfid_card_type getCardType();
	static prog_uchar getCmdLen(int cmd);
#pragma endregion

	void setMode(rfid_mode mode);
  private:
	uint8_t _pin_tx;
	uint8_t _pin_rx;
	


};


#endif