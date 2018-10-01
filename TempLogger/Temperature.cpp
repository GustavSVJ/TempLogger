#include "Temperature.h"
#include "Packages.h"
#include <math.h>
#include <stdio.h>
#define ADC_REF 1.2 // Volt
#define R1 47500 //Ohm

Temperature::Temperature()
{
}


Temperature::~Temperature()
{
}


float Temperature::Sample2Volt(char ADC_data[]) {
	int Sample = (ADC_data[0] << 8) + ADC_data[1];
	float Volt = Sample * (ADC_REF / 1024);
	return Volt;
}

float Temperature::Volt2Celsius(float VD1, float VD2) {
	double I = (VD1 - VD2) / R1;
	double RNTC = VD2 / I;
	double A = 3.354016E-03;
	double B = 2.569850E-04;
	double C = 2.620131E-06;
	double T = (1 / (A + B * log(RNTC) + C * (log(RNTC)*log(RNTC)*log(RNTC)))) + 272;
	return (float) T; 
}

void Temperature::PrintTemperature(float T) {
	printf("Temperature is: %f\n", T);
}