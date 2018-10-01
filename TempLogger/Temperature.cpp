#include "Temperature.h"
#include "Packages.h"
#include <math.h>
#include <stdio.h>


#define ADC_REF 1.2f // Volt
#define R1 47000 //Ohm
#define Rt 10000 //Ohm
#define A 3.354016E-03
#define B 2.569850E-04
#define C 2.620131E-06


Temperature::Temperature()
{
}


Temperature::~Temperature()
{
}


float Temperature::Sample2Volt(char ADC_data[]) {
	int Sample = (ADC_data[0] << 8) + ADC_data[1];
	float Volt = (float) Sample * (ADC_REF / 1024);
	return Volt;
}

float Temperature::Volt2Celsius(float VD1, float VD2) {
	double I = (VD1 - VD2) / R1;
	double RNTC = VD2 / I;

	double temp = log(RNTC / Rt);
	double T = (1 / (A + B * temp + C * temp * temp)) - 273;
	
	return (float) T; 
}

void Temperature::PrintTemperature(char Sample[], char VCC[]) {
	float VD2 = Sample2Volt(Sample);
	float VD1 = Sample2Volt(VCC);
	float T = Volt2Celsius(VD1, VD2);
	printf("Temperature is: %f\n", T);
}