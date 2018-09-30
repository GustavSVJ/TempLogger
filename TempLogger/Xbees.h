#pragma once

class Xbees
{
public:
	Xbees();
	~Xbees();

	static int getDeviceID(char MAC[]);
	static void PrintMacInfo(char MAC[]);
};

//commands
const unsigned char D0_ON[] = { 0x44, 0x30, 0x05 };
const unsigned char D0_OFF[] = { 0x44, 0x30, 0x04 };
const unsigned char READ_PINS[] = { 0x49, 0x53 };
const unsigned char D1_ADC[] = { 0x44, 0x31, 0x02 };
const unsigned char D2_DIS[] = { 0x44, 0x32, 0x00 };
const unsigned char D3_DIS[] = { 0x44, 0x33, 0x00 };
const unsigned char D4_DIS[] = { 0x44, 0x34, 0x00 };
const unsigned char D5_DIS[] = { 0x44, 0x35, 0x00 };
