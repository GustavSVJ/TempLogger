#pragma once
class Temperature
{
public:
	Temperature();
	~Temperature();

	///<summary>Converts a 10-bit sample (or two bytes) into Volt</summary>
	///<param name="ADC_data">The sample contained in a char array with most significant byte as the first element.</param>
	///<returns>Returns the sampled value in Volt</returns>
	static float Sample2Volt(char ADC_data[]);

	///<summary>Converts the sampled value in Volt to a temperature in Celsius using the Steinhart-Hart equation</summary>
	///<param name="VD1">The value of the voltage supplied at the positive end of R1 in measurement circuit for the termistor.</param>
	///<param name="VD2">Sampled voltage over the termistor.</param>
	///<returns>Returns the temperature in Celsius.</returns>
	static float Volt2Celsius(float VD1, float VD2);

	///<summary>Converts a sample into a temperature and prints the value in Celsius.</summary>
	///<param name="Sample">The sample contained in a char array with most significant byte as the first element.</param>
	///<param name="VCC">VCC value contained in a char array with most significant byte as the first element.</param>
	static void PrintTemperature(char Sample[], char VCC[]);
};

