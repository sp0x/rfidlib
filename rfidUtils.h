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
enum rfid_cmd{
	ReadCardType = 0x01,
	SearchCards_ReadSerial,
	Read, Write,
	Wallet_Init, Wallet_Recharge, Wallet_Deduct, Wallet_Read,
	Read_EEPROM, Write_EEPROM, Erase_EEPROM,
	RESET_CFG, RESET
};





#include <SoftwareSerial.h> 
#include "Arduino.h"


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
	
#pragma endregion

#pragma region IO
	size_t write(const char * chr);
	size_t write(uint8_t chr);
	int read();
	int* readAll(size_t & len);
	int* readBlock(int num, size_t & sz, rfid_key_type key_type);
	int available();


	int parseInput(byte input);
	int* parseInput(byte * cmdbytes, size_t & size);
#pragma endregion

#pragma region Commands
	void appendCmdHex(int cmdByte);
	int * commitCommand(size_t &rlen);
	bool writeCommand(int * cmdBytes, size_t cmd_size);
	int* cmd(rfid_cmd cmdFlag, prog_uchar * cmdData, size_t datalen, size_t & respLen);
	int* executeInput(byte * input, size_t inputsz, size_t & response_len);
	static size_t getCmdLen(rfid_cmd cmd);
#pragma endregion

#pragma region Command implementations
	rfid_card_type getCardType();
#pragma endregion


	void setMode(rfid_mode mode);
  private:
	uint8_t _pin_tx;
	uint8_t _pin_rx;
	


};


#endif