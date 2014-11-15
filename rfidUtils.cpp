#include "Arduino.h"
#include "rfidUtils.h"



//fidUtils::mySerial = new SoftwareSerial(PIN_RX, PIN_TX);
int toInt(uchar * b, size_t offset){
	return (b[offset + 0] << 24) | (b[offset + 1] << 16) | (b[offset + 2] << 8) | (b[offset + 3]);
}
short toShort(uchar * b, size_t offset){
	return (b[offset +  1] << 8) | (b[offset + 3]);
}

#pragma region Construct
rfidUtils::rfidUtils(uint8_t rx, uint8_t tx)
{
	this->serial = new SoftwareSerial(rx, tx);
	this->serial->listen();
	this->serial->begin(9600);
	this->comlen=0;
	this->out_flag = 0;
	unlock();
}
rfidUtils::rfidUtils(){
	unlock();
}
#pragma endregion



bool rfidUtils::setMode(rfid_mode mode){
	if (mode<DETECT || mode>STORE) return false; // unsupported mode
	this->MODE = mode;
	this->write((uint8_t)mode);
	return true;
}

#pragma region IO
size_t rfidUtils::write(const char *chr){
	return this->serial->write(chr);
}
size_t rfidUtils::write(uint8_t uint){
	return this->serial->write(uint);
}
size_t rfidUtils::write(uint8_t * buff, size_t len){
	return this->serial->write(buff, len);
}

int rfidUtils::read(){
	if (!this->available()){
		Serial.println("Can't read from unavailable stream!");
		return -1;
	} else 
		return this->serial->read();
}


int rfidUtils::GetInput(int*& outputBuff){
	if (this->locked) return RFID_READ_LOCKED;
	return this->readAll(outputBuff);
}
/*
Reads all output from the module and returns the lenght of the read data.
*/
int rfidUtils::readAll(int*& outputBuff){
	bool out_flag = false;
	static int* buffer=(int*)calloc(MAX_RESPONSE_LEN, sizeof(int));
	int tmpLen=0;
	while (this->available()) {
		int c = this->read();
		if (c != 0xff){
			if (out_flag == 0) Serial.println("response: ");
			if (c < 16) Serial.print("0");
			Serial.print(c, HEX); Serial.print(" "); 		
		}
		out_flag = true; buffer[tmpLen++] = c;
	}
	if (out_flag) Serial.println();
	if (tmpLen == 1){
		if (buffer[0] == 0xFF && this->MODE != DETECT) Serial.print(buffer[0]);  Serial.println("INVALID COMMAND!");
	}
	if (tmpLen > 0)//there was output
	{	
		if (outputBuff == NULL){
			outputBuff = buffer;
		}
		else{
			memcpy(outputBuff, buffer, sizeof(int) * tmpLen);
			Serial.print("Ptr2: ");  Serial.println((int)outputBuff);
			free(buffer);
		}
	}
	return tmpLen;
}

int rfidUtils::available(){
	return this->serial->available();
}

/*
Parses input by Serial.read()
*/
int* rfidUtils::parseInput(byte * cmdbytes, int & size){
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
/*
Appends a new hex value to the command buffer.
*/
void rfidUtils::appendCmdHex(int cmdByte){
	if (cmdByte<0) return;
	this->CMD[this->comlen++] = cmdByte;
}

/*
Writes the current command in the cache to the device serial.
*/
int * rfidUtils::commitCommand(int &rlen){
	int *buff;
	for (int i = 0; i<comlen; i += 2){
		int c = this->serial->write(CMD[i] * 16 + CMD[i + 1]);
	}
	this->comlen = 0; // reset comlen because all bytes have been written
	rlen=this->readAll(buff);
	return buff;
}

/*Executes the command and returns true or false, depending if it failed. */
bool rfidUtils::writeCommand(int * cmd, size_t cmd_size){
	for (int i = 0; i<cmd_size; i += 2){
		int c = this->serial->write(cmd[i] * 16 + cmd[i + 1]);
	}
	return true;
}
/*Executes an input command, from a raw byte input array. Returns the response.*/
int * rfidUtils::executeInput(byte * input, int inputsz, int & response_len){
	int *buff;
	if (input != NULL && inputsz>0){
		int *opcodes=this->parseInput(input, inputsz);
		Serial.print("Writing total "); Serial.print(inputsz); Serial.println(" bytes");
		this->writeCommand(opcodes, inputsz);
	}
	response_len=this->readAll(buff);
	return buff;
}

/*
Executes a given cmd
*/
int* rfidUtils::cmd(rfid_cmd cmdFlag, prog_uchar * cmdData, size_t datalen, int & respLen){
	prog_uchar command_buf[30];
	int *rbuff;
	int cmdLen = this->getCmdLen(cmdFlag);
	prog_uchar cmdBytes[3] = { 0xAB, cmdLen, cmdFlag };
	memcpy(&command_buf[0], cmdBytes,  OPSZ(3) ) ;
	memcpy(&command_buf[3], cmdData, OPSZ(datalen)); // Copy DATA block
	int * cmdOps =this->parseInput( command_buf, cmdLen);
	for (int i = 0; i < cmdLen; i += 2){
		int c = this->serial->write(cmdOps[i] * 16 + cmdOps[i + 1]);
	}

	respLen= this->readAll(rbuff);
	return rbuff;
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
void rfidUtils::lock(){
	this->locked = true;
}
void rfidUtils::unlock(){
	this->locked = !true;
}


rfid_card_type rfidUtils::getCardType(){
	uint8_t cmdBytes[3] = { 0xAB, 0x02, 0x01 };
	// SHOULD SYNCLOCK READ, BECAUSE CURRENT READ NEEDS TO BE BLOCKED!
	this->lock();
	this->write(cmdBytes,3);
	int* buff=(int*)calloc(32, sizeof(int));
	memset(buff, 0, MEMSZ((int)32));
	Serial.print("Ptr1: "); Serial.println((int)buff);
	int len= this->readAll(buff);
	this->unlock();
	// RELEASE SYNCLOCK
	Serial.print("Ptr3: "); Serial.println((int)buff);
	Serial.print("Len: "); Serial.println(len);
	for (int i = 0; i < len+5; i++){
		Serial.println((int)buff[i], HEX);
	}
	
	return (rfid_card_type)0;
	rfid_card_type kt= (rfid_card_type)toShort((prog_uchar*)buff[3], 0);
	free(buff);
	return kt;
}


/*
@param num 0-63
*/
int * rfidUtils::readBlock(int num, size_t & sz, rfid_key_type key_type){
	prog_uchar data[10] = { (prog_uchar)key_type, 1, 2, 3, 4 };
	int rlen;
	int *resp = this->cmd(Read, data, 10, rlen);
	return resp;
}

#pragma endregion