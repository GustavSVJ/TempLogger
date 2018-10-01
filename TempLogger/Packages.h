#pragma once

///<summary>Class for storing and getting data from a package</summary>
class Packages {
public:

	Packages();
	~Packages();


	///<summary>Adds a byte to the package string.</summary>
	///<param name="byte">The value to add.</param>
	///<param name="location">The position in the package array where the value should be added.</param>
	void AddByte(char byte, int location);

	///<summary>Gets the entire package string.</summary>
	///<param name="package">Used to return the package. Must be at least 50 bytes long.</param>
	///<returns>Returns the number of bytes in the package.</returns>
	int GetEntirePackage(char package[]);

	///<summary>Gets the MAC adress from the package.</summary>
	///<param name="MAC">Used to return the MAC. Must be at least 8 bytes long.</param>
	void GetMAC(char MAC[]);

	///<summary>Gets the length of the package.</summary>
	///<returns>Returns the number of bytes in the package.</returns>
	int GetLength(void);

	///<summary>Gets the data from the package.</summary>
	///<param name="data">Used to return the data.</param>
	///<returns>Returns the number of data bytes.</returns>
	int GetData(char data[]);

	///<summary>Gets the command from the package.</summary>
	///<param name="cmd">Used to return the command. Must be at least 2 bytes long.</param>
	void GetCmd(char cmd[]);

	///<summary>Gets the checksum from the package.</summary>
	///<returns>Returns the checksum byte.</returns>
	char GetChecksum();

private:

	char package[50];

};