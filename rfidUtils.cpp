#include "Arduino.h"
#include "rfidUtils.h"

//fidUtils::mySerial = new SoftwareSerial(PIN_RX, PIN_TX);
#define MAX_RESPONSE_LEN 64;


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


void rfidUtils::setMode(rfid_mode mode){
	this->write((uint8_t)mode);
}

size_t rfidUtils::write(const char *chr){
	return this->serial->write(chr);
}
size_t rfidUtils::write(uint8_t uint){
	return this->serial->write(uint);
}
int rfidUtils::read(){
	return this->serial->read();
}
int rfidUtils::readAll(){
	int out_flag = 0;
	while (this->available()) {
		if (out_flag == 0) Serial.println("response: ");
		byte C = this->read();
		if (C<16) Serial.print("0");
		Serial.print(C, HEX); //Display the Serial Number in HEX
		Serial.print(" ");
		out_flag = 1;
	}
	if (out_flag >0) {
		Serial.println();
		out_flag = 0;
	}


}
int rfidUtils::available(){
	return this->serial->available();
}
int* rfidUtils::parseInput(int * cs, int size){
	int* out = new int[size];
	for (int i = 0; i < size; i++){
		out[i] = this->parseInput(cs[i]);
	}
	return out;
}
int rfidUtils::parseInput(byte c){
		int out = 0;
		if (c >= '0' && c <= '9') {
			out = c - '0';
			return out;
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

#pragma region Commands
void rfidUtils::appendCmdHex(int cmdByte){
	if (cmdByte<0) return;
	this->CMD[this->comlen++] = cmdByte;
}
//void rfidUtils::cmd()
bool rfidUtils::writeCommand(){
	for (int i = 0; i<comlen; i += 2){
		int c = this->serial->write(CMD[i] * 16 + CMD[i + 1]);
	}
	this->comlen = 0; // reset comlen because all bytes have been written
}
bool rfidUtils::writeCommand(int * cmd, size_t cmd_size){
	for (int i = 0; i<cmd_size; i += 2){
		int c = this->serial->write(cmd[i] * 16 + cmd[i + 1]);
	}
}
void rfidUtils::executeInput(int * input, size_t inputsz){

}


rfid_card_type rfidUtils::getCardType(){
	int *cmd= parseInput(new byte['A', 'B', '0', '2', '0', '1'], 6);
	this->writeCommand(cmd, 6);
	readAll();

}


/*
@param num 0-63
*/
int * rfidUtils::readBlock(int num, size_t & sz,  rfid_key_type key_type){
	prog_uchar data[10] = { (prog_uchar)key_type, 1, 2, 3, 4 };
	this->cmd((prog_uchar)0x03, &data, 10);
}


int* rfidUtils::cmd(prog_uchar cmd, prog_uchar * cmdData, size_t datalen, size_t respLen){
	unsigned char command_buf[30];
	unsigned char cmdLen = this->getCmdLen(cmd);
	memcpy(&command_buf[0], 	[0xAB, cmdLen, cmd]		, 3 * sizeof(unsigned char));
	memcpy(&command_buf[3] , cmdData, datalen * sizeof(unsigned char)); // Copy DATA block
	int * cmdOps =this->parseInput(command_buf,cmdLen);
	for (int i = 0; i < cmdLen; i += 2){
		int c = this->serial->write(cmdOps[i] * 16 + cmdOps[i + 1]);
	}
	//this->comlen = 0; // reset comlen because all bytes have been written
	Serial.println("Written to card");
}



prog_uchar rfidUtils::getCmdLen(int cmd){
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