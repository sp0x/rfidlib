#include "Arduino.h"
#include "rfidUtils.h"

//fidUtils::mySerial = new SoftwareSerial(PIN_RX, PIN_TX);
int toInt(uchar * b, size_t offset){
	return (b[offset + 0] << 24) | (b[offset + 1] << 16) | (b[offset + 2] << 8) | (b[offset + 3]);
}
int toShort(int * b, size_t offset){
	return (int)((int)b[offset + 0] << 8) | (int)((int)b[offset + 1]);
}
void printall(int *arr, int len){
	for (int i = 0; i < len; i++){
		Serial.print(arr[i], HEX); Serial.print(" ");
	}
	Serial.println();
}

#pragma region Construct
rfidUtils::rfidUtils(uint8_t rx, uint8_t tx)
{
	this->serial = new SoftwareSerial(rx, tx);
	this->serial->begin(9600);
	this->comlen=0;
	this->out_flag = 0;
	unlock();
	printResponse = true;
}
rfidUtils::rfidUtils(){
	unlock();
	printResponse = true;
}
#pragma endregion



bool rfidUtils::setMode(rfid_mode mode){
	if (mode<DETECT || mode>STORE) return false; // unsupported mode
	this->MODE = mode;
	size_t res =this->write((uint8_t)mode);
	if (res == 0){
		Serial.print("Could not write!");
	}
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
	if (this->locked) {
		Serial.println("RFID IS LOCKED!");
		return RFID_READ_LOCKED;
	}
	return this->readAll(outputBuff);
}
/*
Reads all output from the module and returns the lenght of the read data.
*/
int rfidUtils::readAll(int*& outputBuff){
	bool out_flag = false;
	static int* buffer;
	if (outputBuff == NULL){
		outputBuff = (int*)calloc(MAX_RESPONSE_LEN, sizeof(int));
	}
	buffer = outputBuff;
	int tmpLen=0;
	while (this->available()) {
		int c = this->read();
		if (c != 0xff && this->printResponse){
			if (out_flag == 0) Serial.print("response: ");
			if (c < 16) Serial.print("0");
			Serial.print(c, HEX); Serial.print(" "); 		
		}
		out_flag = true; buffer[tmpLen++] = c;
	}
	if (out_flag && this->printResponse) Serial.println();
	if (tmpLen == 1){
		if (buffer[0] == 0xFF && this->MODE != DETECT) Serial.print(buffer[0]);  Serial.println("INVALID COMMAND!");
	}
	//if (tmpLen > 0)//there was output
	//{	
	//	if (outputBuff == NULL){
	//		outputBuff = buffer;
	//	}
	//	else{
	//		memcpy(outputBuff, buffer, sizeof(int) * tmpLen);
	//		free(buffer);
	//	}
	//}
	return tmpLen;
}

int rfidUtils::available(){
	int cnt =this->serial->available();
	return cnt;
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

/*
Blocks the process untill there's new data available.
*/
void rfidUtils::waitForResponse(){
	while (!this->available()){
		delay(1);
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

int rfidUtils::cmd(rfid_cmd cmdFlag, int &rlen){
	return *this->cmd(cmdFlag, 0, 0, rlen);
}

/*
Executes a given cmd
*/
int* rfidUtils::cmd(rfid_cmd cmdFlag, prog_uchar * cmdData, size_t datalen, int & respLen){
	prog_uchar command_buf[30];
	int *rbuff;
	int cmdLen = this->getCmdLen(cmdFlag);
	bool valid = false;

	prog_uchar cmdBytes[3] = { 0xAB, cmdLen, cmdFlag };
	memset(command_buf, 0, MEMSZ(cmdLen));			// Null the command buffer
	memcpy(&command_buf[0], cmdBytes,  OPSZ(3) ) ;	// Write the command headers
	if(datalen>0) memcpy(&command_buf[3], cmdData, datalen); // Copy DATA block

	this->write(command_buf, cmdLen+1);
	this->waitForResponse();
	respLen = this->readAll(rbuff);

	valid = (rbuff[2] == (int)cmdFlag);
	if (!valid){
		Serial.println("Last command failed!"); return NULL;
	}
	respLen -= 3;												//Move to response
	static int * rdata = (int*)calloc(respLen ,sizeof(int));
	memcpy(rdata, &rbuff[3], respLen*sizeof(int));
	switch (cmdFlag){
		case ReadCardType:
			rdata[0] = toShort(rdata, 0); respLen = 2;
			break;
	}
	return rdata;
}

size_t rfidUtils::getCmdLen(rfid_cmd cmd){
	switch (cmd){
	case 0x01:
		return 0x02;
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
	int rlen;
	return (rfid_card_type)this->cmd(ReadCardType, rlen);
}
int * rfidUtils::getCardSerial(){
	int len;
	static int * serial = this->cmd(SearchCards_ReadSerial, 0, 0, len);
	return serial;
}

/*
@param blockNum The block number to read. For S50 it's 0-65 and for S70 it's 0-255
@param cardType The type of the card 
@param keyType Most times A
@param key Authentication key for given block
*/
int * rfidUtils::readBlock(int blockNum, rfid_card_type cardType, rfid_key_type keyType, byte key[5]){
	int len;
	bool valid = false;
	byte rqdata[8] = { 0,0,0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	switch (cardType){
		case OneS50: valid = blockNum >= 0 && blockNum <= 64; break;//0-64
		case OneS70: valid = blockNum >= 0 && blockNum <= 255; break; //0-255
		default: Serial.println("INVALID CARD!"); return NULL;
	}
	if (!valid){
		Serial.println("Invalid block number!"); return NULL;
	}
	rqdata[0] = blockNum; rqdata[1] = keyType; 
	if(key!=NULL) memcpy(rqdata + 2, key, 6);
	static int * data = this->cmd(Read, rqdata, 0x8, len);// data is 16 blocks
	Serial.println(len);
	for (int i = 0; i < len; i++){
		Serial.print(String(data[i], HEX)); Serial.print(" ");
	}


}


int * rfidUtils::readEEPROM(byte addrHigh, byte addrLow, byte dtLen){
	static int * resp;
	int rlen;
	byte data[3] = { addrHigh, addrLow, dtLen };
	resp = this->cmd(Read_EEPROM, data, 3, rlen);
	return resp;
	//header = ab + dl + 2 + cmdflag + -- - data
	//	data - 4 bytes
}


#pragma endregion