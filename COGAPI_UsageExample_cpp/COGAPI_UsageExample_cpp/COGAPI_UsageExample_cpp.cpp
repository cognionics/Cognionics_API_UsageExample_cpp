// COGAPI_UsageExample_cpp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "COGConstants.h"


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

char* INTToString(int inputINT, char* outputString);
int getValidINTInput(int lowerLimit, int upperLimit);

int main()
{
    int numDevices;
	int devDescriptionLength;
	// char* cognionicsDeviceList = COG_DeviceQuery(&numDevices, &devDescriptionLength);
	int i;
	int j;
	char tempChar;
	int deviceSelection = -1;
	char temp;
	char printString[128];

	struct COGConfiguration currentDeviceCFG;

	int* const DEVPTR[NUMSETTINGS] = {
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

	for(i = 0; i < NUMSETTINGS; i++)
	{
		if(i != 6 || *DEVPTR[5] == 1)
		{
			printf(PROMPTMSGS[i][0], (CFGPTR[i] == NULL) ? INTToString(*DEVPTR[i], printString) : *(CFGPTR[i] + *DEVPTR[i]));
		}
	}

	//print channels name, channels status (on/off)

	for(i = 0; i < NUMSETTINGS; i++)
	{
		if(i != 5)
		{
			if(i == 6)
			{
				if(*DEVPTR[5] == 1)
				{
					printf(PROMPTMSGS[i][1]);
					printf(PROMPTMSGS[i][2], INTToString(*DEVPTR[6], printString));
					printf(PROMPTMSGS[i][3]);
					*DEVPTR[i] = getValidINTInput(1, 16);
				}
			}
			else
			{
				printf(PROMPTMSGS[i][1]);
				for(j = 0; j < CFGPTRSIZE[0][i]/CFGPTRSIZE[1][i]; j++)
				{
					printf(PROMPTMSGS[i][2], j, *(CFGPTR[i] + j));

					if(j == *DEVPTR[i])
						printf(" - current\n");
					else
						printf("\n");
				}
				printf(PROMPTMSGS[i][3]);
				*DEVPTR[i] = getValidINTInput(0, (CFGPTRSIZE[0][i]-1));
			}
		}
	}

	printf("\n");
	for(i = 0; i < NUMSETTINGS; i++)
	{
		if(i != 6 || *DEVPTR[5] == 1)
		{
			printf(PROMPTMSGS[i][0], (CFGPTR[i] == NULL) ? INTToString(*DEVPTR[i], printString) : *(CFGPTR[i] + *DEVPTR[i]));
		}
	}


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

char* INTToString(int inputINT, char* outputString)
{
	sprintf_s(outputString, sizeof(outputString), "%d", inputINT);

	return outputString;
}
