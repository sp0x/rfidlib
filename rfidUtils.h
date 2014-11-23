#ifndef RFID_UTILS
#define RFID_UTILS
#include "Arduino.h"
#include <SoftwareSerial.h>
#include "MemoryFree.h"

#define PIN_ALT_RX 12
#define PIN_ALT_TX 13
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
	UltraLight = 0x4400, OneS50 = 0x400, OneS70 = 0x200, ProX = 0x800, DESFire = 0x4403
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
void printall(int *arr, int len);

class rfidUtils
{
  public:
	rfidUtils(uint8_t rx, uint8_t tx, bool useAlt = false);
	rfidUtils();
#pragma region Variables
	bool printResponse;
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
	void print(const char *arg, int base =0 , bool endl	= false );
	void print(float arg, int base = 0 , bool endl = false);
	int GetInput(int*& outputBuff);
	int available();

	int parseInput(byte input);
	int* parseInput(byte * cmdbytes, int & size);
	void waitForResponse();
#pragma endregion

#pragma region Commands
	void appendCmdHex(int cmdByte);
	int *commitCommand(int &rlen);
	bool writeCommand(int * cmdBytes, size_t cmd_size);
	int* cmd(rfid_cmd cmdFlag, prog_uchar * cmdData, size_t datalen, int & respLen);
	int  cmd(rfid_cmd cmdFlag, int &rlen);
	int* executeInput(byte * input, int inputsz, int & response_len);
	static size_t getCmdLen(rfid_cmd cmd);
#pragma endregion

#pragma region Command implementations
	rfid_card_type getCardType();
	int * getCardSerial();
	int * readBlock(int blockNum, rfid_card_type cardType, rfid_key_type keyType, byte key[5]);
	int * readSector(int sector, rfid_card_type cType, rfid_key_type kType, byte key[5]);
	int * readEEPROM(byte addrHigh, byte addrLow, byte dtLen);
#pragma endregion


	bool setMode(rfid_mode mode);
  private:
	int readAll(int*& outputBuff, int sz = 0);
	uint8_t _pin_tx;
	uint8_t _pin_rx;
	SoftwareSerial * altSerial;



};


#endif