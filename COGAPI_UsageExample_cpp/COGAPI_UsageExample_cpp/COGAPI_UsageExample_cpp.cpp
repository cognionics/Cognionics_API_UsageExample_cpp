// COGAPI_UsageExample_cpp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "COGConstants.h"

#define NUMSETTINGS	11

// #include "COGLib.h"
struct COGConfiguration
{
	int sampleRate;
	int gain;
	int testModeEnable;
	int accelerometerEnable;
	int gyroscopeEnable;
	int extensionChannelsDetect;
	int extensionChannelsNumber;
	int powerOnImpedanceCheckEnable;
	int impedanceCheckMode;
	int wirelessTriggerEnable;
	int dataStreamMode;
};

int getValidINTInput(int lowerLimit, int upperLimit);

int main()
{
    int numDevices;
	int devDescriptionLength;
	// char* cognionicsDeviceList = COG_DeviceQuery(&numDevices, &devDescriptionLength);
	int i;
	char tempChar;
	int deviceSelection = -1;
	char temp;

	struct COGConfiguration currentDeviceCFG;

	const void* CFGPTRMAP[2][NUMSETTINGS] = {
		{
			SAMPLE_RATE_MAP,
			GAIN_MAP,
			BOOLTOSTR,
			BOOLTOSTR,
			BOOLTOSTR,
			BOOLTOSTR,
			NULL,
			BOOLTOSTR,
			IMPEDANCEMODE,
			BOOLTOSTR,
			DATAMODE
		},
		{
			&currentDeviceCFG.sampleRate,
			&currentDeviceCFG.gain,
			&currentDeviceCFG.testModeEnable,
			&currentDeviceCFG.accelerometerEnable,
			&currentDeviceCFG.gyroscopeEnable,
			&currentDeviceCFG.extensionChannelsDetect,
			&currentDeviceCFG.extensionChannelsNumber,
			&currentDeviceCFG.powerOnImpedanceCheckEnable,
			&currentDeviceCFG.impedanceCheckMode,
			&currentDeviceCFG.wirelessTriggerEnable,
			&currentDeviceCFG.dataStreamMode
		}
	};


	// if(numDevices > 0)
	// {
	// 	printf("FTDI Query Successful\n");

	// 	for (i = 0; i < numDevices; i++)
	// 	{
	// 		printf("Device %d: %s\n", i, &cognionicsDeviceList[devDescriptionLength*i]);
	// 	}
	// }
	// else
	// {
	// 	printf("FTDI Query Failed\n");

	// 	system("pause");
	// 	return -1;
	// }


	// printf("Enter device number to connect to: ");
	// COG_DeviceConnect(getValidINTInput(0, (numDevices-1)));


	// if(COG_GetDeviceInformation())
	// {
	// 	printf("Error reading device configuration\n");

	// 	system("pause");
	// 	return -2;
	// }

	// printf("Device configuration received\n");

	// currentDeviceCFG.sampleRate = COG_GetSampleRate();
	// currentDeviceCFG.gain = COG_GetGain();
	// currentDeviceCFG.testModeEnable = COG_GetTestModeEnable();
	// currentDeviceCFG.accelerometerEnable = COG_GetAccelerometerEnable();
	// currentDeviceCFG.gyroscopeEnable = COG_GetGyroscopeEnable();
	// currentDeviceCFG.extensionChannelsDetect = COG_GetExtensionChannelsDetect();
	// currentDeviceCFG.extensionChannelsNumber = COG_GetExtensionChannelsNumber();
	// currentDeviceCFG.powerOnImpedanceCheckEnable = COG_GetPowerOnImpedanceCheckEnable();
	// currentDeviceCFG.impedanceCheckMode = COG_GetImpedanceCheckMode();
	// currentDeviceCFG.wirelessTriggerEnable = COG_GetWirelessTriggerEnable();
	// currentDeviceCFG.dataStreamMode = COG_GetDataStreamMode();

	// int* currentDeviceCFG[11] = 
	// {
	// 	&currentDeviceCFG.sampleRate,
	// 	&currentDeviceCFG.gain,
	// 	&currentDeviceCFG.testModeEnable,
	// 	&currentDeviceCFG.accelerometerEnable,
	// 	&currentDeviceCFG.gyroscopeEnable,
	// 	&currentDeviceCFG.extensionChannelsDetect,
	// 	&currentDeviceCFG.extensionChannelsNumber,
	// 	&currentDeviceCFG.powerOnImpedanceCheckEnable,
	// 	&currentDeviceCFG.impedanceCheckMode,
	// 	&currentDeviceCFG.wirelessTriggerEnable,
	// 	&currentDeviceCFG.dataStreamMode
	// };

	

	currentDeviceCFG.sampleRate = 1;
	currentDeviceCFG.gain = 2;
	currentDeviceCFG.testModeEnable = 0;
	currentDeviceCFG.accelerometerEnable = 1;
	currentDeviceCFG.gyroscopeEnable = 0;
	currentDeviceCFG.extensionChannelsDetect = 0;
	currentDeviceCFG.extensionChannelsNumber = 8;
	currentDeviceCFG.powerOnImpedanceCheckEnable = 1;
	currentDeviceCFG.impedanceCheckMode = 1;
	currentDeviceCFG.wirelessTriggerEnable = 1;
	currentDeviceCFG.dataStreamMode = 1;

	printf("Sample rate: %d Hz\n", SAMPLE_RATE_MAP[currentDeviceCFG.sampleRate]);
	printf("Gain: %d\n", GAIN_MAP[currentDeviceCFG.gain]);
	printf("Test mode: %s\n", BOOLTOSTR[currentDeviceCFG.testModeEnable]);
	printf("Accelerometer enabled: %s\n", BOOLTOSTR[currentDeviceCFG.accelerometerEnable]);
	printf("Gyroscope enabled: %s\n", BOOLTOSTR[currentDeviceCFG.gyroscopeEnable]);
	printf("Extension channels detected: %s\n", BOOLTOSTR[currentDeviceCFG.extensionChannelsDetect]);
	printf("Number of extension channels: %d\n", currentDeviceCFG.extensionChannelsNumber);
	printf("Enable impedance check when device powered on: %s\n", BOOLTOSTR[currentDeviceCFG.powerOnImpedanceCheckEnable]);
	printf("Impedance check mode: %s\n", IMPEDANCEMODE[currentDeviceCFG.accelerometerEnable]);
	printf("Wireless trigger enabled: %s\n", BOOLTOSTR[currentDeviceCFG.accelerometerEnable]);
	printf("Data stream mode: %s\n", DATAMODE[currentDeviceCFG.accelerometerEnable]);
	


	//print channels name, channels status (on/off)

	printf("\nSample rate options:\n");
	for(i = 0; i < sizeof(SAMPLE_RATE_MAP)/sizeof(SAMPLE_RATE_MAP[0]); i++)
	{
		printf("%d: %d", i, SAMPLE_RATE_MAP[i]);

		if(i == currentDeviceCFG.sampleRate)
			printf(" - current\n");
		else
			printf("\n");
	}
	printf("Enter new sample rate setting: ");
	currentDeviceCFG.sampleRate = getValidINTInput(0, (sizeof(SAMPLE_RATE_MAP)-1));

	printf("\nGain options:\n");
	for(i = 0; i < sizeof(GAIN_MAP)/sizeof(GAIN_MAP[0]); i++)
	{
		printf("%d: %d", i, GAIN_MAP[i]);

		if(i == currentDeviceCFG.gain)
			printf(" - current\n");
		else
			printf("\n");
	}
	printf("Enter new gain setting: ");
	currentDeviceCFG.gain = getValidINTInput(0, (sizeof(GAIN_MAP)-1));

	printf("\nEnable test mode:\n");
	for(i = 0; i < sizeof(BOOLTOSTR)/sizeof(BOOLTOSTR[0]); i++)
	{
		printf("%d: %s", i, BOOLTOSTR[i]);

		if(i == currentDeviceCFG.testModeEnable)
			printf(" - current\n");
		else
			printf("\n");
	}
	printf("Enter new test mode setting: ");
	currentDeviceCFG.testModeEnable = getValidINTInput(0, (sizeof(BOOLTOSTR)-1));

	printf("\nEnable accelerometer:\n");
	for(i = 0; i < sizeof(BOOLTOSTR)/sizeof(BOOLTOSTR[0]); i++)
	{
		printf("%d: %s", i, BOOLTOSTR[i]);

		if(i == currentDeviceCFG.accelerometerEnable)
			printf(" - current\n");
		else
			printf("\n");
	}
	printf("Enter new accelerometer setting: ");
	currentDeviceCFG.accelerometerEnable = getValidINTInput(0, (sizeof(BOOLTOSTR)-1));

	printf("\nEnable gyroscope:\n");
	for(i = 0; i < sizeof(BOOLTOSTR)/sizeof(BOOLTOSTR[0]); i++)
	{
		printf("%d: %s", i, BOOLTOSTR[i]);

		if(i == currentDeviceCFG.gyroscopeEnable)
			printf(" - current\n");
		else
			printf("\n");
	}
	printf("Enter new gyroscope setting: ");
	currentDeviceCFG.gyroscopeEnable = getValidINTInput(0, (sizeof(BOOLTOSTR)-1));



	printf("\nEnable impedance check when device powered on:\n");
	for(i = 0; i < sizeof(BOOLTOSTR)/sizeof(BOOLTOSTR[0]); i++)
	{
		printf("%d: %s", i, BOOLTOSTR[i]);

		if(i == currentDeviceCFG.powerOnImpedanceCheckEnable)
			printf(" - current\n");
		else
			printf("\n");
	}
	printf("Enter new power on impedance check setting: ");
	currentDeviceCFG.powerOnImpedanceCheckEnable = getValidINTInput(0, (sizeof(BOOLTOSTR)-1));

	printf("\nImpedance check mode:\n");
	for(i = 0; i < sizeof(IMPEDANCEMODE)/sizeof(IMPEDANCEMODE[0]); i++)
	{
		printf("%d: %s", i, IMPEDANCEMODE[i]);

		if(i == currentDeviceCFG.impedanceCheckMode)
			printf(" - current\n");
		else
			printf("\n");
	}
	printf("Enter new impedance check mode setting: ");
	currentDeviceCFG.impedanceCheckMode = getValidINTInput(0, (sizeof(IMPEDANCEMODE)-1));

	printf("\nEnable wireless trigger:\n");
	for(i = 0; i < sizeof(BOOLTOSTR)/sizeof(BOOLTOSTR[0]); i++)
	{
		printf("%d: %s", i, BOOLTOSTR[i]);

		if(i == currentDeviceCFG.wirelessTriggerEnable)
			printf(" - current\n");
		else
			printf("\n");
	}
	printf("Enter new wireless trigger setting: ");
	currentDeviceCFG.wirelessTriggerEnable = getValidINTInput(0, (sizeof(BOOLTOSTR)-1));

	printf("\nData stream mode:\n");
	for(i = 0; i < sizeof(DATAMODE)/sizeof(DATAMODE[0]); i++)
	{
		printf("%d: %s", i, DATAMODE[i]);

		if(i == currentDeviceCFG.dataStreamMode)
			printf(" - current\n");
		else
			printf("\n");
	}
	printf("Enter new data stream mode setting: ");
	currentDeviceCFG.dataStreamMode = getValidINTInput(0, (sizeof(DATAMODE)-1));







	printf("\n");

	printf("Sample rate: %d Hz\n", SAMPLE_RATE_MAP[currentDeviceCFG.sampleRate]);
	printf("Gain: %d\n", GAIN_MAP[currentDeviceCFG.gain]);
	printf("Test mode: %s\n", BOOLTOSTR[currentDeviceCFG.testModeEnable]);
	printf("Accelerometer enabled: %s\n", BOOLTOSTR[currentDeviceCFG.accelerometerEnable]);
	printf("Gyroscope enabled: %s\n", BOOLTOSTR[currentDeviceCFG.gyroscopeEnable]);
	printf("Extension channels detected: %s\n", BOOLTOSTR[currentDeviceCFG.extensionChannelsDetect]);
	printf("Number of extension channels: %d\n", currentDeviceCFG.extensionChannelsNumber);
	printf("Enable impedance check when powered on: %s\n", BOOLTOSTR[currentDeviceCFG.powerOnImpedanceCheckEnable]);
	printf("Impedance check mode: %s\n", IMPEDANCEMODE[currentDeviceCFG.accelerometerEnable]);
	printf("Wireless trigger enabled: %s\n", BOOLTOSTR[currentDeviceCFG.accelerometerEnable]);
	printf("Data stream mode: %s\n", DATAMODE[currentDeviceCFG.accelerometerEnable]);

	//set all settings
	//write to device
	//verify new settings

	//stream1 and record a minute of data and output to file
	
	//live start dual SD and wireless

	//stream500 and record a minute of data and output to file














	system("pause");
    return 0;
}

int getValidINTInput(int lowerLimit, int upperLimit)
{
	int inputValue;
	
	while(true)
	{
		while(scanf_s("%d", &inputValue) != 1)
		{
			printf("Not an integer, try again: ");
		}
		if(inputValue >= lowerLimit && inputValue <= upperLimit)
		{
			return inputValue;
		}
		else
		{
			printf("Invalid integer, try again: ");
		}
	}
}
