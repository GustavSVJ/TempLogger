#pragma once
class Packages {
public:

	Packages();
	~Packages();
	//Setters
	void AddByte(char byte, int location);

	//Getters
	int GetEntirePackage(char package[]);
	void GetMAC(char MAC[]);
	int GetLength(void);
	int GetData(char data[]);
	int GetCmd(char cmd[]);
	char GetChecksum();

private:

	char package[50];

};