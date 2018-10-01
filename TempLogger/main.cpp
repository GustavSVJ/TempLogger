//============================================================================
// Name        : Xbee_Commander.cpp
// Author      : GustavSVJ
// Version     :
// Copyright   : This doesn't belong to you!
//============================================================================

#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <queue>

#include "devices.h"
#include "Packages.h"
#include "Xbees.h"

using namespace std;

void UARTSetup(int uart0_filestream);

queue<Packages> packageQueue;

void *UARTReceive(void *UARTReference) {
	while (1) {

		Packages packageAssembly;

		char byteBuffer[5];
		memset(byteBuffer, 0, 5);
		int UARTFilestream = (int)UARTReference;
		int receiveCounter = 0;

		while (byteBuffer[0] != 0x7E) {
			read(UARTFilestream, (void*)byteBuffer, 1);
		}

		packageAssembly.AddByte(byteBuffer[0], receiveCounter);
		receiveCounter++;

		while (receiveCounter < 3) {
			if (read(UARTFilestream, (void*)byteBuffer, 1) == 1) {
				packageAssembly.AddByte(byteBuffer[0], receiveCounter);
				receiveCounter++;
			}
		}

		while (receiveCounter < packageAssembly.GetLength()) {
			if (read(UARTFilestream, (void*)byteBuffer, 1) == 1) {
				packageAssembly.AddByte(byteBuffer[0], receiveCounter);
				receiveCounter++;
			}
		}

		if (packageAssembly.CalculateChecksum() == packageAssembly.GetChecksum()) {
				packageQueue.push(packageAssembly);
		}
	}
}

int main(int argc, char** argv) {

	int UART;

	// Open the Port. We want read/write, no "controlling tty" status, and open it no matter what state DCD is in
	UART = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);  //serialOpen ("/dev/ttyAMA0", 9600) ;used when serial uart
	if (UART == -1) {
		printf("Failed to open UART! Exiting program...\n");
		return(-1);
	}
	else {
		UARTSetup(UART);
	}

	fcntl(UART, F_SETFL, 0);

	sleep(1);

	printf("Starting program!\n");

	pthread_t thread;

	if (pthread_create(&thread, NULL, UARTReceive, (void *)UART)) {
		cout << "Error:unable to create thread" << endl;
		exit(-1);
	}

	Packages packageAssembly;

	packageAssembly.AssemblePackage(0, D0, new (char){ 0x02 }, 1);
	int status = Xbees::TransmitAndCheckResponse(UART, packageAssembly, 10, &packageQueue);

	while (1) {

		packageAssembly.AssemblePackage(0, D1, new (char){ 0x05 }, 1);
		status = Xbees::TransmitAndCheckResponse(UART, packageAssembly, 10, &packageQueue);


		if (status > -1) {
			printf("D0 turned on! It took %d tries\n", status);
		}
		else {
			printf("An error occurred while transmitting command...\n");
		}
		sleep(1);

		packageAssembly.AssemblePackage(0, READ_PINS, new (char){ 0x00 }, 0);
		status = Xbees::TransmitAndCheckResponse(UART, packageAssembly, 10, &packageQueue);

		if (status > -1) {
			printf("Send IS request! It took %d tries\n", status);
		}
		else {
			printf("An error occurred while transmitting command...\n");
		}

		while (!packageQueue.empty()) {
			Packages package = packageQueue.front();
			packageQueue.pop();

			char cmd[2];
			package.GetCmd(cmd);

			if (cmd[0] == 'I' && cmd[1] == 'S') {
				char adcBitmask;
				char adcValues[10][2];
				for (int i = 0; i < 10; i++) {
					adcValues[i][0] = 0x00;
					adcValues[i][1] = 0x00;
				}

				package.ParseISRespons(&adcBitmask, adcValues);
				printf("The ADC bitmask is: %x\n", adcBitmask);
				printf("Measurement one is %x%x\n", adcValues[0][0], adcValues[0][1]);
				printf("Measurement two is %x%x\n", adcValues[1][0], adcValues[1][1]);
			}
		}

		sleep(1);

		packageAssembly.AssemblePackage(0, D1, new (char) { 0x04 }, 1);
		status = Xbees::TransmitAndCheckResponse(UART, packageAssembly, 10, &packageQueue);

		if (status > -1) {
			printf("D0 turned off! It took %d tries\n", status);
		}
		else {
			printf("An error occurred while transmitting command...\n");
		}
		sleep(1);
	}

}

void UARTSetup(int uart0_filestream) {
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	options.c_cc[VMIN] = 0;        //wait blocking
	options.c_cc[VTIME] = 1;         //timeout 10 ms
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);
}

/*
int XbeeParseResponse(char *package, int packageLength) {
	int returnValue = -1;

	if (XbeeCheckSum(package, packageLength) == package[packageLength - 1]) {
		//checksum okay
		switch (package[packageLength - 2]) {
		case 0:
			printf("Command was correctly received!\n");
			returnValue = 0;
			break;
		case 4:
			printf("No response received!\n");
			break;
		default:
			printf("An unknown response was received: %i", package[packageLength - 2]);
			break;
		}
	}
	else {
		printf("The checksum was incorrect!\n");
	}

	printf("Response received: ");
	for (int i = 0; i < packageLength; i++) {
		printf("%x ", package[i]);
	}
	printf("\n");

	return returnValue;

}
*/

