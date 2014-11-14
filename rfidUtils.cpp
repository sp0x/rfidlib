#include "Arduino.h"
#include "rfidUtils.h"

//fidUtils::mySerial = new SoftwareSerial(PIN_RX, PIN_TX);
#define MAX_RESPONSE_LEN 64
#define OPSIZE (sizeof(unsigned char))
#define MEMSZ(ops) ( OPSIZE * (ops) )



#pragma region Construct
rfidUtils::rfidUtils(uint8_t rx, uint8_t tx)
{
	this->serial = new SoftwareSerial(rx, tx);
	this->serial->listen();
	this->serial->begin(9600);
	this->comlen=0;
	this->out_flag = 0;
}
rfidUtils::rfidUtils(){

}
#pragma endregion



void rfidUtils::setMode(rfid_mode mode){
	this->write((uint8_t)mode);
}

#pragma region IO
size_t rfidUtils::write(const char *chr){
	return this->serial->write(chr);
}
size_t rfidUtils::write(uint8_t uint){
	return this->serial->write(uint);
}

int rfidUtils::read(){
	return this->serial->read();
}
int * rfidUtils::readAll(size_t & len){
	int out_flag = 0;
	int * redNfo = new int[MAX_RESPONSE_LEN];
	len = 0;
	while (this->available()) {
		if (out_flag == 0) Serial.println("response: ");
		int c = this->read();
		if (c<16) Serial.print("0");
		Serial.print(c, HEX); //Display the Serial Number in HEX
		Serial.print(" ");
		out_flag = 1;
		redNfo[len++] = c;
	}
	if (out_flag >0) {
		Serial.println();
		out_flag = 0;
	}
	return redNfo;
}

int rfidUtils::available(){
	return this->serial->available();
}

/*
Parses input by Serial.read()
*/
int* rfidUtils::parseInput(byte * cmdbytes, size_t & size){
	int* out = new int[size];
	int ix2 = 0;
	size_t outputSize = size;
	for (int i = 0; i < size; i++){
		int vl = this->parseInput(cmdbytes[i]);		
		if (vl==-1){
			outputSize--;
		}
		else {
			out[ix2++] = vl;
		}		
	}
	size = outputSize;
	return out;
}

int rfidUtils::parseInput(byte c){
		if (c >= '0' && c <= '9') {
			return c - '0';
		}
		else if (c >= 'a' && c <= 'f') {
			return c - 'a' + 10;
		}
		else if (c >= 'A' && c <= 'F') {
			return c - 'A' + 10;
		}
		else {
			return -1;   // getting here is bad: it means the character was invalid
		}
}

#pragma endregion


#pragma region Commands
void rfidUtils::appendCmdHex(int cmdByte){
	if (cmdByte<0) return;
	this->CMD[this->comlen++] = cmdByte;
}

/*
Writes the current command in the cache to the device serial.
*/
int * rfidUtils::commitCommand(size_t &rlen){
	for (int i = 0; i<comlen; i += 2){
		int c = this->serial->write(CMD[i] * 16 + CMD[i + 1]);
	}
	this->comlen = 0; // reset comlen because all bytes have been written
	return this->readAll(rlen);
}
bool rfidUtils::writeCommand(int * cmd, size_t cmd_size){
	for (int i = 0; i<cmd_size; i += 2){
		int c = this->serial->write(cmd[i] * 16 + cmd[i + 1]);
	}
}
int * rfidUtils::executeInput(byte * input, size_t inputsz, size_t & response_len){
	if (input != NULL && inputsz>0){
		int *opcodes=this->parseInput(input, inputsz);
		for (int i = 0; i < inputsz; i++){
			Serial.print(opcodes[i]);  Serial.print(" ");
		}
		this->writeCommand(opcodes, inputsz);
	}
	return this->readAll(response_len);
}

/*
Executes a given cmd
*/
int* rfidUtils::cmd(rfid_cmd cmdFlag, prog_uchar * cmdData, size_t datalen, size_t & respLen){
	prog_uchar command_buf[30];
	size_t cmdLen = this->getCmdLen(cmdFlag);
	prog_uchar cmdBytes[3] = { 0xAB, cmdLen, cmdFlag };
	memcpy(&command_buf[0], cmdBytes,  MEMSZ(3) ) ;
	memcpy(&command_buf[3], cmdData, MEMSZ(datalen) ); // Copy DATA block
	int * cmdOps =this->parseInput( command_buf, cmdLen);
	for (int i = 0; i < cmdLen; i += 2){
		int c = this->serial->write(cmdOps[i] * 16 + cmdOps[i + 1]);
	}

	return this->readAll(respLen);

	//this->comlen = 0; // reset comlen because all bytes have been written
	Serial.println("Written to card");
}

size_t rfidUtils::getCmdLen(rfid_cmd cmd){
	switch (cmd){
	case 0x01:
		return 0x01;
	case 0x02:
		return 0x02;
	case 0x03:
		return 0x0a;

	default:
		return 0x01;
	}
}
#pragma endregion


#pragma region Command utils 

rfid_card_type rfidUtils::getCardType(){
	/*int cmdBytes = new int[6]{ 0xA, 0xB, 0x0, 0x2, 0x0, 0x1 };
	int *cmd = this->parseInput(cmdBytes, 6);
	this->writeCommand(cmd, 6);
	*/
	
	//TODO: readAll();

}


/*
@param num 0-63
*/
int * rfidUtils::readBlock(int num, size_t & sz, rfid_key_type key_type){
	prog_uchar data[10] = { (prog_uchar)key_type, 1, 2, 3, 4 };
	size_t rlen;
	int *resp = this->cmd(Read, data, 10, rlen);
	return resp;
}

#pragma endregion