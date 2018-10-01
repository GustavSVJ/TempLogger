#include "Xbees.h"
#include "devices.h"
#include "Packages.h"
#include <stdio.h>
#include <unistd.h>			//Used for UART

Xbees::Xbees()
{
}

Xbees::~Xbees()
{
}

int Xbees::getDeviceID(char MAC[]) {
	int match;

	for (int i = 0; i < NUMBER_OF_XBEES; i++) { //compare
		match = 1;
		for (int j = 0; j < 8; j++) {
			if (MAC[j] != xbees[i].mac[j]) {
				match = 0;
				break;
			}
		}
		if (match == 1) {
			return i;
		}
	}

	return -1;
}


void Xbees::PrintMacInfo(char MAC_adress[]) {
	int xbeeID = getDeviceID(MAC_adress);
	if (xbeeID == 0 && xbeeID < 3) {
		printf("Package received from ");
		printf(xbees[xbeeID].name);
		printf("\n");
	}
	else {
		printf("Unknown MAC adress\n");
	}
}

int Xbees::Transmit(int UART, Packages package) {

	char buff[50];
	package.GetEntirePackage(buff);

	int n = write(UART, buff, package.GetLength());

	return n;
}
