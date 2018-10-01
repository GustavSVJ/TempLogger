#ifndef XBEES_H_
#define XBEES_H_
#include "Packages.h"
#include <queue>

using namespace std;

///<summary>Class for functions concerning the Xbee's</summary>
class Xbees
{
public:
	Xbees();
	~Xbees();

	///<summary>Checks if the MAC is known</summary>
	///<param name="MAC">The MAC adress to check.</param>
	///<returns>Returns the DeviceID.</returns>
	static int getDeviceID(char MAC[]);

	///<summary>Checks if the MAC is known and prints the corresponding name if it is.</summary>
	///<param name="MAC">The MAC adress to check.</param>
	static void PrintMacInfo(char MAC[]);

	///<summary>Send a package to the Xbee via UART</summary>
	///<param name="UART">File reference to the UART.</param>
	///<param name="package">The package to send.</param>
	///<returns>Returns either the number of bytes send or -1 if an error occured.</returns>
	static int Transmit(int UART, Packages package);

	///<summary>Send a package to the Xbee via UART and retry every 5 seconds until succes or max number of tries</summary>
	///<param name="UART">File reference to the UART.</param>
	///<param name="package">The package to send.</param>
	///<param name="maxTries">The number of maximum number of tries before returning.</param>
	///<returns>Returns the number of tries or -1 if an error occured.</returns>
	static int TransmitAndCheckResponse(int UART, Packages package, int maxTries, queue<Packages> *packageQueue);

};

//commands

const char READ_PINS[] = { 0x49, 0x53 };
const char D0[] = { 0x44, 0x30};
const char D1[] = { 0x44, 0x31};
const char D2[] = { 0x44, 0x32};
const char D3[] = { 0x44, 0x33};
const char D4[] = { 0x44, 0x34};
const char D5[] = { 0x44, 0x35};

#endif /* XBEES_H_ */