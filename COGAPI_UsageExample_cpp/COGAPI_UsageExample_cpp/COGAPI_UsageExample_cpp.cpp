//------------------------------------------------------------------------------
// COGAPI_UsageExample_cpp.cpp
// 
// Description:
// example code that shows how to use the Cognionics library
// to configure and stream data from Cognionics headsets
// 
// @author: Junle Zhang
// 
// COGLib @authors: William Bei and Junle Zhang
// 
// www.cognionics.net
// info@cognionics.com
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "COGConstants.h"
#include "COGLib.h"

//------------------------------------------------------------------------------
// simple structure to help organize device settings
//------------------------------------------------------------------------------
struct COGConfiguration
{
	int sampleRate;
	int gain;
	int testModeEnable;
	int accelerometerEnable;
	int gyroscopeEnable;
	int extensionChannelsNumber;
	int powerOnImpedanceCheckEnable;
	int impedanceCheckMode;
	int wirelessTriggerEnable;
	int dataStreamMode;
};

//------------------------------------------------------------------------------
// simple function that convert an integer to a string
// 
// @param inputINT: input integer
// @param outputString: location where output is stored
// 
// function is protected by using sprintf_s
//------------------------------------------------------------------------------
char* INTToString(int inputINT, char* outputString);

//------------------------------------------------------------------------------
// function that gets a valid integer input
// 
// @param lowerLimit: >= lower limit for integer input
// @param upperLimit: <= upper limit for integer input
// 
// will keep prompting user for input until valid input is found
//------------------------------------------------------------------------------
int getValidINTInput(int lowerLimit, int upperLimit);

//------------------------------------------------------------------------------
// main function that uses the COGLib to:
// -lists available devices
// -prompt the user for and connect to selected device
// -retrieve and print device settings and channel list/status
// -verbosely prompt the user for new device settings
// -write new device settings and channel configuration to device
// -print updated device settings and channel list/status
// -read 60 seconds of data with read size of 1 sample
// -read 60 seconds of data with read size of 10 samples
// -read 60 seconds of data with read size of 100 samples
// -read 60 seconds of data with read size of 1000 samples
// -read 60 seconds of data with read size of 10000 samples
// -output all read data to external file
// 
// user can also enable/disable channels through ...
//------------------------------------------------------------------------------
int main()
{
    int numDevices;				//stores number of devices returned by COG_DeviceQuery
	int devDescriptionLength;	//store s

	
	char* cognionicsDeviceList = COG_DeviceQuery(&numDevices, &devDescriptionLength);
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
	currentDeviceCFG.extensionChannelsNumber = 8;
	currentDeviceCFG.powerOnImpedanceCheckEnable = 1;
	currentDeviceCFG.impedanceCheckMode = 1;
	currentDeviceCFG.wirelessTriggerEnable = 1;
	currentDeviceCFG.dataStreamMode = 1;

	for(i = 0; i < NUMSETTINGS; i++)
	{
		printf(PROMPTMSGS[i][0], (CFGPTR[i] == NULL) ? INTToString(*DEVPTR[i], printString) : *(CFGPTR[i] + *DEVPTR[i]));
	}

	//print channels name, channels status (on/off)

	for(i = 0; i < NUMSETTINGS; i++)
	{
		if(i == 5)
		{
			if(*DEVPTR[5] > 0)
			{
				printf(PROMPTMSGS[i][1]);
				printf(PROMPTMSGS[i][2], INTToString(*DEVPTR[5], printString));
				printf(PROMPTMSGS[i][3]);
				*DEVPTR[i] = getValidINTInput(1, 32);
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
			*DEVPTR[i] = getValidINTInput(0, (CFGPTRSIZE[0][i]/CFGPTRSIZE[1][i]-1));
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

	////initialize and connect to device
	//COGDevice device;

	////read data
	//int num_read_samples = 1 * NUMBEROFSAMPLESPERMINUTE;
	//int num_read_samples_size = 10;
	//double* channelDataBuffer = new double[NUMBEROFSAMPLESPERMINUTE*NUM_CHANNELS];

	//printf("Device connected, starting read\n");
	//for (i = 0; i < (num_read_samples / num_read_samples_size); i++)
	//{

	//	COG_GetSample(&device, NUM_READ_SAMPLES_SIZE, &channelDataBuffer[NUM_READ_SAMPLES_SIZE*i*NUM_CHANNELS]);

	//	printf("Progress: %f\n", (100.0 * (((double)i + 1) / ((double)(NUM_READ_SAMPLES / NUM_READ_SAMPLES_SIZE)))));
	//}


	//ofstream myfile;
	//myfile.open("data.txt");
	//myfile << "COGNextStepa1 Data#2\n" << endl;
	//cout << "Writing output to file... " << endl;
	//for (i = 0; i < NUM_READ_SAMPLES*NUM_CHANNELS; i++)
	//{
	//	myfile << channelDataBuffer[i] << endl;
	//}
	//cout << "Data written. " << endl;
	//myfile.close();
	//cout << "File closed. " << endl;

	system("pause");
    return 0;
}

int getValidINTInput(int lowerLimit, int upperLimit)
{
	int inputValue;
	int i;
	char inputString[64];

	while(1)
	{
		for(i = 0; i < sizeof(inputString); i++)
		{
			inputString[i] = getchar();
			if(inputString[i] == '\n')
			{
				inputString[i] = '\0';
				break;
			}
		}
		
		for(i = 0; i < sizeof(inputString); i++)
		{
			if(inputString[i] == '\0')
			{
				if(i == 0)
				{
					printf("Nothing entered, try again: ");
					break;
				}
				else
				{
					inputValue = atoi(inputString);

					if(inputValue >= lowerLimit && inputValue <= upperLimit)
					{
						printf("Entered integer: %d\n", inputValue);
						return inputValue;
					}
					else
					{
						printf("Invalid integer, try again: ");
						break;
					}
				}
			}
			else
			{
				if(inputString[i] < 48 || inputString[i] > 57)
				{
					printf("Not an integer, try again: ");
					break;
				}
			}
		}
	}
}

char* INTToString(int inputINT, char* outputString)
{
	sprintf_s(outputString, sizeof(outputString), "%d", inputINT);

	return outputString;
}
