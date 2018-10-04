#include "Xbees.h"
#include "devices.h"
#include "Packages.h"
#include <stdio.h>
#include <unistd.h>			//Used for UART
#include <queue>
#include <string.h>

using namespace std;

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

int Xbees::TransmitAndCheckResponse(int UART, Packages package, int maxTries, queue<Packages> *packageQueue) {
	char buff[50], data[50];
	package.GetEntirePackage(buff);

	for (int counter = 0; counter < maxTries; counter++) {
		int n = write(UART, buff, package.GetLength());

		if (n == package.GetLength()) {
			sleep(1);
			if (!packageQueue->empty()) {
				Packages packageReceived = packageQueue->back();
				
				char cmdA[2], cmdB[2];
				packageReceived.GetCmd(cmdA);
				packageReceived.GetData(data);
				package.GetCmd(cmdB);

				if (cmdA[0] == cmdB[0] && cmdA[1] == cmdB[1] && data[0] == 0x00) {
					return counter + 1;
				}
				else {
					sleep(4);
				}
			}
		}
		else {
			printf("Error:The package failed to send! The error code returned was %x\n", data[0]);
			return -1;
		}
	}

	printf("Error:The package failed to send! The error code returned was %x\n", data[0]);
	return -1;
}
