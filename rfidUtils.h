#ifndef RFID_UTILS
#define RFID_UTILS
#include "Arduino.h"
#include <SoftwareSerial.h>
#define MAX_RESPONSE_LEN 64
#define RFID_READ_LOCKED -2
#define OPSIZE (sizeof(unsigned char))
#define OPSZ(ops) ( OPSIZE * (ops) )
#define MEMSZ(var) ( sizeof(var) * var)



typedef unsigned char uchar;

enum rfid_mode {
	DETECT = 1, READ_SERIALS, STORE
};
enum rfid_card_type{
	UltraLight = 0x4040, OneS50 = 0x400, OneS70 = 0x200, ProX = 0x80000, DESFire = 0x4040003
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


/*Converts a byte array to 4byte WORD */
int toInt(uchar * arr, size_t offset);
int toShort(int * b, size_t offset);

class rfidUtils
{
  public:
	rfidUtils(uint8_t rx, uint8_t tx);
	rfidUtils();
#pragma region Variables
	bool locked;
	SoftwareSerial* serial;
	int CMD[64];
	int comlen;
	int out_flag;
	rfid_mode MODE;
#pragma endregion
#pragma region Locking
	void lock();
	void unlock();
#pragma endregion
#pragma region IO
	size_t write(const char * chr);
	size_t write(uint8_t chr);
	size_t write(uint8_t * buff, size_t len);
	int read();
	int GetInput(int*& outputBuff);
	int* readBlock(int num, size_t & sz, rfid_key_type key_type);
	int available();

	int parseInput(byte input);
	int* parseInput(byte * cmdbytes, int & size);
	void waitForResponse();
#pragma endregion

#pragma region Commands
	void appendCmdHex(int cmdByte);
	int * commitCommand(int &rlen);
	bool writeCommand(int * cmdBytes, size_t cmd_size);
	int* cmd(rfid_cmd cmdFlag, prog_uchar * cmdData, size_t datalen, int & respLen);
	int* executeInput(byte * input, int inputsz, int & response_len);
	static size_t getCmdLen(rfid_cmd cmd);
#pragma endregion

#pragma region Command implementations
	rfid_card_type getCardType();
#pragma endregion


	bool setMode(rfid_mode mode);
  private:
	int readAll(int*& buffer);
	uint8_t _pin_tx;
	uint8_t _pin_rx;



};


#endif