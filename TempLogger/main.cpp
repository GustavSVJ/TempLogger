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
#include "devices.h"
#include <pthread.h>

using namespace std;

void UARTSetup(int uart0_filestream);
int XbeeTransmit(int ref, char *package, int packageLength);
int XbeeReceive(int ref, char *package, int packageLength);
unsigned char XbeeCheckSum(char *package, int packageLength);
int XbeeAssembleCommand(char *output, const unsigned char *mac, const unsigned char *command, int commandLength);
int XbeeParseResponse(char *package, int packageLength);
int GetData(char MAC_adress[], char data[], int dataLength);
char CompareALLMAC(char MAC_adress[]);
int CompareMAC(char MAC[], char k);


#define PackageReceiveBufferSize 50
#define PackageMaxLength 50
int packageReceiveBufferHead = 0;
int packageReceiveBufferTail = 0;
char packageReceiveLength[PackageReceiveBufferSize];
char packageReceiveBuffer[PackageReceiveBufferSize][PackageMaxLength];

void *UARTReceive(void *UARTReference) {
	while (1) {
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
		char receiveBuffer[PackageMaxLength];
		char byteBuffer[5];
		memset(byteBuffer, 0, 5);
		memset(receiveBuffer, 0, PackageMaxLength);
		int UARTFilestream = (int)UARTReference;
		int receiveCounter = 0;

		while (byteBuffer[0] != 0x7E) {
			read(UARTFilestream, (void*)byteBuffer, 1);
		}
		receiveBuffer[receiveCounter] = byteBuffer[0];
		receiveCounter++;

		while (receiveCounter < 3) {
			if (read(UARTFilestream, (void*)byteBuffer, 1) == 1) {
				receiveBuffer[receiveCounter] = byteBuffer[0];
				receiveCounter++;
			}
		}

		char packageLength = (receiveBuffer[1] << 4) + receiveBuffer[2] + 4;
		packageReceiveLength[packageReceiveBufferHead] = packageLength;

		for (; receiveCounter < packageLength;) {
			if (read(UARTFilestream, (void*)byteBuffer, 1) == 1) {
				receiveBuffer[receiveCounter] = byteBuffer[0];
				receiveCounter++;
			}
		}

		if (XbeeCheckSum(receiveBuffer, packageLength) == receiveBuffer[packageLength - 1]) {
			for (int i = 0; i < packageLength; i++) {
				packageReceiveBuffer[packageReceiveBufferHead][i] = receiveBuffer[i];
			}

			if ((packageReceiveBufferHead < PackageReceiveBufferSize - 1)) {
				packageReceiveBufferHead++;
			}
			else {
				packageReceiveBufferHead = 0;
			}
		}
	}
}

int GetData(char MAC_adress[], char data[], int *dataLength) {
	if (packageReceiveBufferHead != packageReceiveBufferTail) {
		int type;
		char xbeeID;
		for (int k = 0; k < 8; k++) {
			MAC_adress[k] = packageReceiveBuffer[packageReceiveBufferTail][5 + k]; //skip to mac adress 	
		}


		if (packageReceiveBuffer[packageReceiveBufferTail][15] == 0x44 && packageReceiveBuffer[packageReceiveBufferTail][16] == 0x30) {
			type = 1; // LED digital output command
		}
		else if (packageReceiveBuffer[packageReceiveBufferTail][15] == 0x44 && packageReceiveBuffer[packageReceiveBufferTail][16] == 0x31) {
			type = 2; // ADC setup response
		}
		else if (packageReceiveBuffer[packageReceiveBufferTail][15] == 0x49 && packageReceiveBuffer[packageReceiveBufferTail][16] == 0x53) {
			type = 3; // ADC readings package
		}
		else {
			type = 0;
		}

		switch (type) {
		case 1:
			*dataLength = 1;
			break;
		case 2:
			*dataLength = 1;
			break;
		case 3:
			*dataLength = packageReceiveLength[packageReceiveBufferTail] - 18; //this is a package type that contains more than one byte of data
			break;
		default:
			*dataLength = 0;
		}

		for (int k = 0; k < *dataLength; k++) {
			data[k] = packageReceiveBuffer[packageReceiveBufferTail][17 + k];
		}



		if (packageReceiveBufferTail < PackageReceiveBufferSize - 1) {
			packageReceiveBufferTail++;
		}
		else {
			packageReceiveBufferTail = 0;
		}
		return type;
	}
	return 0;
}

// Compare MAC_adress to the list of Xbees. Return Xbee number if there is a match, if no match is found returns -1
char CompareALLMAC(char MAC_adress[]) {
	char k;
	int match = 0;
	for (k = 0; k < NUMBER_OF_XBEES; k++) { //compare
		if (CompareMAC(MAC_adress, k) == 0) {
			match = 1;
			break;
		}
	}
	if (match == 1) {
		return k;
	}
	else {
		return -1;
	}
}

// Compare MAC adress to xbee number k in xbee list. Returns 0 if MAC adresses match
int CompareMAC(char MAC[], char k) {
	int i;
	int no = 0;
	for (i = 0; i < 8; i++) {
		if (MAC[i] != xbees[k].mac[i])
			no++;
	}
	return no;
}

void PrintMacInfo(char MAC_adress[]) {
	int xbeeID = CompareALLMAC(MAC_adress);
	if (xbeeID == -1) {
		printf("Unknown MAC adress\n");
	}
	else {
		printf("Package received from ");
		printf(xbees[xbeeID].name);
		printf("\n");
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

	char buf[30];
	char MAC_adress[8];
	char data[25];
	int dataLength;
	int type;


	sleep(1);

	printf("Starting program!\n");

	int rc;
	pthread_t thread;

	rc = pthread_create(&thread, NULL, UARTReceive, (void *)UART);

	if (rc) {
		cout << "Error:unable to create thread," << rc << endl;
		exit(-1);
	}


	while (1) {
		char commandBuffer[50];
		int commandLenght = XbeeAssembleCommand(commandBuffer, xbees[0].mac, D0_ON, 3);
		int status = XbeeTransmit(UART, commandBuffer, commandLenght);



		if (status > -1) {
			printf("D0 turned on!\n");
		}
		else {
			printf("An error occurred while transmitting command...\n");
		}
		/*
		if (status < 0) {
			printf("An error occurred while reading response\n");
		}
		else if (status == 0) {
			printf("No response received\n");
		}
		else {
			XbeeParseResponse(buf, status);
		}
		*/

		type = GetData(MAC_adress, data, &dataLength);
		PrintMacInfo(MAC_adress);

		sleep(1);

		commandLenght = XbeeAssembleCommand(commandBuffer, xbees[0].mac, D0_OFF, 3);
		status = XbeeTransmit(UART, commandBuffer, commandLenght);



		if (status > -1) {
			printf("D0 turned off!\n");
		}
		else {
			printf("An error occurred while transmitting command...\n");
		}
		/*
				if (status < 0) {
					printf("An error occurred while reading response\n");
				}
				else if (status == 0) {
					printf("No response received\n");
				}
				else {
					XbeeParseResponse(buf, status);
				}
				*/

		type = GetData(MAC_adress, data, &dataLength);
		PrintMacInfo(MAC_adress);


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

int XbeeTransmit(int ref, char *package, int packageLength) {
	// Write to the port
	int n = write(ref, package, packageLength);

	return n;
}

int XbeeReceive(int ref, char *package, int packageLength) {

	int n = read(ref, (void*)package, packageLength);

	return n;
}

unsigned char XbeeCheckSum(char *package, int packageLength) {
	unsigned char checksum = 0;
	long sum = 0;

	for (int i = 3; i < packageLength - 1; i++) {

		sum += package[i];
		checksum = (unsigned char)0xFF - sum;
	}

	return checksum;
}

int XbeeAssembleCommand(char *output, const unsigned char *mac, const unsigned char *command, int commandLength) {

	int n = 13 + commandLength;

	output[0] = 0x7E; //Start delimiter

	//Length of the package
	output[1] = n >> 8;
	output[2] = n & 0x00FF;

	output[3] = 0x17; //Frame type
	output[4] = 0x01; //Frame ID

	//MAC address for receiver
	output[5] = mac[0];
	output[6] = mac[1];
	output[7] = mac[2];
	output[8] = mac[3];
	output[9] = mac[4];
	output[10] = mac[5];
	output[11] = mac[6];
	output[12] = mac[7];

	//16-bit destination address
	output[13] = 0xFF;
	output[14] = 0xFE;

	output[15] = 0x02; //Remote command options

	for (int i = 0; i < commandLength; i++) {
		output[16 + i] = command[i];
	}

	output[16 + commandLength] = XbeeCheckSum(output, 17 + commandLength);

	return 17 + commandLength;
}

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
