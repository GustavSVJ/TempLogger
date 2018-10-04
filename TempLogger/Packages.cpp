#include "Packages.h"
#include <string.h>
#include <stdio.h>
#include "devices.h"

Packages::Packages()
{
	memset(package, 0, 50);
}


Packages::~Packages()
{
}

int Packages::GetEntirePackage(char fullPackage[]) {
	int length = GetLength();
	for (int i = 0; i < length; i++) {
		fullPackage[i] = package[i];
	}
	return length;
}

void Packages::AddByte(char byte, int location) {
	package[location] = byte;
}

int Packages::AssemblePackage(int deviceID, const char command[], char data[], char dataLength) {
	int length = 15 + dataLength;

	package[0] = 0x7E; //Start delimiter

	//Length of the package
	package[1] = (char)(length >> 8);
	package[2] = (char)(length & 0x00FF);

	package[3] = 0x17; //Frame type
	package[4] = 0x01; //Frame ID

	//MAC address for receiver
	package[5] = xbees[deviceID].mac[0];
	package[6] = xbees[deviceID].mac[1];
	package[7] = xbees[deviceID].mac[2];
	package[8] = xbees[deviceID].mac[3];
	package[9] = xbees[deviceID].mac[4];
	package[10] = xbees[deviceID].mac[5];
	package[11] = xbees[deviceID].mac[6];
	package[12] = xbees[deviceID].mac[7];

	//16-bit destination address
	package[13] = 0xFF;
	package[14] = 0xFE;

	package[15] = 0x02; //Remote command options

	package[16] = command[0];
	package[17] = command[1];

	for (int i = 0; i < dataLength; i++) {
		package[18 + i] = data[i];
	}

	package[18 + dataLength] = CalculateChecksum();

	return 19 + dataLength;
}

char Packages::CalculateChecksum() {
	unsigned char checksum = 0;
	long sum = 0;

	for (int i = 3; i < GetLength() - 1; i++) {

		sum += package[i];
		checksum = (unsigned char)((unsigned char)0xFF - sum);
	}

	return checksum;
}

void Packages::GetMAC(char MAC[]) {
	for (int i = 0; i < 8; i++) {
		MAC[i] = package[5 + i]; //skip to mac adress 	
	}
}

int Packages::GetLength(void) {
	return (package[1] << 4) + package[2] + 4;
}

void Packages::GetCmd(char cmd[]) {
	if (GetFrametype() == 0x17) {
		for (int i = 0; i < 2; i++) {
			cmd[i] = package[16 + i];
		}
	}
	else if (GetFrametype() == 0x97){
		for (int i = 0; i < 2; i++) {
			cmd[i] = package[15 + i];
		}
	}
	else {
		printf("Error:The Frametype is unknown\n");
		for (int i = 0; i < 2; i++) {
			cmd[i] = 0x00;
		}
	}

}

int Packages::GetData(char data[]){
	memset(data, 0, 50);
	int lenght = 0;
	for (int i = 17; i < GetLength() - 1; i++) {
		data[i-17] = package[i];
		lenght++;
	}
	return lenght;
}

char Packages::GetChecksum() {
	return package[GetLength() - 1];
}

char Packages::GetFrametype() {
	return package[3];
}

int Packages::ParseISRespons(char *ADCBitmask, char ADCReadings[][2]) {
	char cmd[2];
	GetCmd(cmd);
	if (cmd[0] == 'I' && cmd[1] == 'S') {
		char data[50];
		int length = GetData(data);

		*ADCBitmask = data[4];
		char n = data[4];

		char count = 0;
		
		while (n) {
			count += n & 1;
			n >>= 1;
		}

		if (length == count * 2 + 7) {
			int locOuter = 0;
			int locInner = 0;
			for (int i = 7; i < count * 2 + 7 ;i++) {
				if (locInner == 2) {
					locOuter++;
					locInner = 0;
				}
				ADCReadings[locOuter][locInner] = data[i];
				locInner++;
			}
			return count;
		}
		else {
			printf("Error:The bitmask doesn't match the number of returned bytes!");
			return -1;
		}

	}
	else {
		printf("Error:The received package was not a IS package! The package type was %x %x\n", cmd[0], cmd[1]);
		return -1;
	}
}

