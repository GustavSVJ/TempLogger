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
#include "Temperature.h"

using namespace std;

void UARTSetup(int uart0_filestream);
void *UARTReceive(void *UARTReference);

queue<Packages> packageQueue;

int main(int argc, char** argv) {

	int UART;

	// Open the Port. We want read/write, no "controlling tty" status, and open it no matter what state DCD is in
	UART = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);  //serialOpen ("/dev/ttyAMA0", 9600) ;used when serial uart
	if (UART == -1) {
		printf("Failed to open UART! Exiting program...\n");
		exit(-1);
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
	int status = Xbees::TransmitAndCheckResponse(UART, packageAssembly, 50, &packageQueue);
	if (status == -1) {
		printf("The setup could not be completed! Exiting...\n", status);
		exit(-1);
	}

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
				Temperature::PrintTemperature(adcValues[0], adcValues[1]);
			}
		}

		packageAssembly.AssemblePackage(0, D1, new (char) { 0x04 }, 1);
		status = Xbees::TransmitAndCheckResponse(UART, packageAssembly, 10, &packageQueue);

		if (status > -1) {
			printf("D0 turned off! It took %d tries\n", status);
		}
		else {
			printf("An error occurred while transmitting command...\n");
		}
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