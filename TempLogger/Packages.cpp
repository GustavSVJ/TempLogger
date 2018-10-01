#include "Packages.h"
#include <string.h>

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
}

void Packages::AddByte(char byte, int location) {
	package[location] = byte;
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
	for (int i = 0; i < 2; i++) {
		cmd[i] = package[15 + i];
	}
}

int Packages::GetData(char data[]){
	int lenght = 0;
	for (int i = 0; i < GetLength() - 1; i++) {
		data[i] = package[17 + i];
		lenght++;
	}
	return lenght;
}

char Packages::GetChecksum() {
	return package[GetLength() - 1];
}


