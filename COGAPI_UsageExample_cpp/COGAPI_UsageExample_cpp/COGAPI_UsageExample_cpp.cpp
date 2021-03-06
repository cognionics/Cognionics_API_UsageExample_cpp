//------------------------------------------------------------------------------
// COGAPI_UsageExample_cpp.cpp
// 
// Description:
// example code that shows how to use the Cognionics library
// to configure and stream data from Cognionics headsets
// 
// @authors: William Bei and Junle Zhang
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
// function that prints out the name of the error flag given a return value
// 
// @param flag_value: the value of the flag
// 
// prints UNRECOGNIZED_VALUE if the value doesn't correspond to
// any given flag values
//------------------------------------------------------------------------------
void printFlagName(int flag_value);
//------------------------------------------------------------------------------
// function that prints the number of packets lost while reading data
// 
// @param error_array: array that stores the number of errors per sample rate
// @param arr_length: the length of the array
//------------------------------------------------------------------------------
void printNumErrors(int error_array[], int arr_length);
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
// user can also enable/disable (connected) extension channels through
// the function int COG_SetExtensionChannelsNumber(COGDevice device)
//------------------------------------------------------------------------------
int main()
{
	int numDevices;				// stores number of devices returned by COG_DeviceQuery
	int devDescriptionLength;	// stores the number of chars each device takes up in array
								// cognionicsDeviceList

	COGDevice device;			// handle for the Cognionics device

	char* cognionicsDeviceList = COG_DeviceQuery(&numDevices, &devDescriptionLength);
	// retreive list of available devices for connection
	int connectnum;				// stores index of connected device
	int return_status;			// stores return flags from COG functions
	char printString[128];		// buffer used to store string outputs

	int i;						//
	int j;						// array/loop transversal variables

	struct COGConfiguration currentDeviceCFG;
	// stores device settings

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

	// list available devices, if any are available; else program exits
	if (numDevices > 0)
	{
		printf("FTDI Query Successful\n");

		for (i = 0; i < numDevices; i++)
		{
			printf("Device %d: %s\n", i, &cognionicsDeviceList[devDescriptionLength*i]);
		}
	}
	else
	{
		printf("FTDI Query Failed\n");

		system("pause");
		return -1;
	}

	// connect to an available device
	printf("Enter the number of the device to connect to: ");
	connectnum = getValidINTInput(0, (numDevices - 1));
	while (connectnum < 0 || connectnum >= numDevices)
	{
		printf("Error - option does not exist; try again: ");
		connectnum = getValidINTInput(0, (numDevices - 1));
	}
	COG_Connect(&device, connectnum);

	// grab device config; if error occurs, prints the name of the error flag
	return_status = COG_GetConfig(device);
	if (return_status != COG_OK)
	{
		printf("Error reading device configuration\n");
		printf("Return flag: %d", return_status);
		printf(" (");
		printFlagName(return_status);
		printf(")\n");
		system("pause");

		return -2;
	}
	printf("Device configuration received\n");

	// initialize local buffer values
	currentDeviceCFG.sampleRate = COG_GetSampleRate(device);
	currentDeviceCFG.gain = COG_GetGain(device);
	currentDeviceCFG.testModeEnable = COG_GetTestModeEnable(device);
	currentDeviceCFG.accelerometerEnable = COG_GetAccelerometerEnable(device);
	currentDeviceCFG.gyroscopeEnable = COG_GetGyroscopeEnable(device);
	currentDeviceCFG.extensionChannelsNumber = COG_GetExtensionChannelsNumber(device);
	currentDeviceCFG.powerOnImpedanceCheckEnable = COG_GetPowerOnImpedanceCheckEnable(device);
	currentDeviceCFG.impedanceCheckMode = COG_GetImpedanceCheckMode(device);
	currentDeviceCFG.wirelessTriggerEnable = COG_GetWirelessTriggerEnable(device);
	currentDeviceCFG.dataStreamMode = COG_GetDataStreamMode(device);

	// prompt for any settings changes
	for (i = 0; i < NUMSETTINGS; i++)
	{
		printf(PROMPTMSGS[i][0], (CFGPTR[i] == NULL) ? INTToString(*DEVPTR[i], printString) : *(CFGPTR[i] + *DEVPTR[i]));
	}

	for (i = 0; i < NUMSETTINGS; i++)
	{
		if (i == 5)
		{
			if (*DEVPTR[5] > 0)
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
			for (j = 0; j < CFGPTRSIZE[0][i] / CFGPTRSIZE[1][i]; j++)
			{
				printf(PROMPTMSGS[i][2], j, *(CFGPTR[i] + j));

				if (j == *DEVPTR[i])
					printf(" - current\n");
				else
					printf("\n");
			}
			printf(PROMPTMSGS[i][3]);
			*DEVPTR[i] = getValidINTInput(0, (CFGPTRSIZE[0][i] / CFGPTRSIZE[1][i] - 1));
		}
	}

	printf("\n");
	for (i = 0; i < NUMSETTINGS; i++)
	{
		if (i != 6 || *DEVPTR[5] == 1)
		{
			printf(PROMPTMSGS[i][0], (CFGPTR[i] == NULL) ? INTToString(*DEVPTR[i], printString) : *(CFGPTR[i] + *DEVPTR[i]));
		}
	}

	//update device settings
	COG_SetSampleRate(device, currentDeviceCFG.sampleRate);
	COG_SetGain(device, currentDeviceCFG.gain);
	COG_SetTestModeEnable(device, currentDeviceCFG.testModeEnable);
	COG_SetAccelerometerEnable(device, currentDeviceCFG.accelerometerEnable);
	COG_SetGyroscopeEnable(device, currentDeviceCFG.gyroscopeEnable);
	COG_SetExtensionChannelsNumber(device, currentDeviceCFG.extensionChannelsNumber);
	COG_SetPowerOnImpedanceCheckEnable(device, currentDeviceCFG.powerOnImpedanceCheckEnable);
	COG_SetImpedanceCheckMode(device, currentDeviceCFG.impedanceCheckMode);
	COG_SetWirelessTriggerEnable(device, currentDeviceCFG.wirelessTriggerEnable);
	COG_SetDataStreamMode(device, currentDeviceCFG.dataStreamMode);
	
	// set device config; if error occurs, prints the name of the error flag
	return_status = COG_SetConfig(device);
	if (return_status != COG_OK)
	{
		printf("Error setting device configuration\n");
		printf("Return flag: %d", return_status);
		printf(" (");
		printFlagName(return_status);
		printf(")\n");
		system("pause");

		return -2;
	}

	printf("Device configuration set.\n\n");

	system("pause");

	printf("\n\n");

	// retrieve and print channel settings
	printf("Printing channel statuses...\n\n");

	FILE * channelFile;
	channelFile = fopen("COG_Channel_Data.txt", "w");

	COGChannels device_channels = COG_GetChannelStatuses(device);
	int numEnabledChannels = COG_GetNumEnabledChannels(device);

	// printing channel data to console
	// channel data is also simultaneously written to file
	if (channelFile != NULL) 
		fprintf(channelFile, "Data from device %s\n", &cognionicsDeviceList[devDescriptionLength*connectnum]);
	printf("Device Channels: \n");
	if (channelFile != NULL) 
		fprintf(channelFile, "Device Channels: \n\n");
	printf("%-20s", "Channel Number:");
	if (channelFile != NULL) 
		fprintf(channelFile, "%-20s", "Channel Number:");
	printf("%-20s", "Channel Name:");
	if (channelFile != NULL) 
		fprintf(channelFile, "%-20s", "Channel Name:");
	printf("Channel Status:\n");
	if (channelFile != NULL) 
		fprintf(channelFile, "%-20s\n", "Channel Status:");
	printf("---------------------------------------------------------\n");
	if (channelFile != NULL) fprintf(channelFile, "---------------------------------------------------------\n");

	for (i = 0; i < ((device_channels.num_channels > (numEnabledChannels + 32)) ? (numEnabledChannels + 32) : device_channels.num_channels); i++)
	{
		printf("%-20d", (i + 1));
		if (channelFile != NULL) 
			fprintf(channelFile, "%-20d", (i + 1));

		for (j = 1; j < device_channels.labelSize; j++)
		{
			printf("%c", device_channels.channelLabels[i][j]);
			if (channelFile != NULL) 
				fprintf(channelFile, "%c", device_channels.channelLabels[i][j]);
		}
		printf("%-5s", " ");
		if (channelFile != NULL) 
			fprintf(channelFile, "%-5s", " ");

		if (device_channels.channelStatuses[i] == 1)
		{
			printf("%s\n", "on");
			if (channelFile != NULL) 
				fprintf(channelFile, "%s\n", "on");
		}
		else
		{
			printf("%s\n", "off");
			if (channelFile != NULL) 
				fprintf(channelFile, "%s\n", "off");
		}
	}

	fclose(channelFile);

	system("pause");
	printf("\n");

	// read device data
	int time_to_read = 1; 				// value is in terms of minutes

	int num_read_samples = time_to_read * NUMBEROFSAMPLESPERMINUTE;
										// total number of samples to read

	int num_read_samples_size = 1;		// number of samples to read at a time
	int num_channels;					// number of device channels

	int packetsdropped[5] = { 0, 0, 0, 0, 0 };
										// stores the number of lost packets for
										// each sample rate

	// Raw mode results in [NUMCHANNELS] + 4 values-long data per packet
	// High-speed mode results in [NUMCHANNELS] + 2 values-long data per packet
	if (currentDeviceCFG.dataStreamMode == 1)
		num_channels = (numEnabledChannels + 4);
	else
		num_channels = (numEnabledChannels + 2);

	// reading one packet at a time
	double* dataByOne = new double[num_read_samples*num_channels];
	printf("Reading %d minute(s) worth of data, %d packets at a time...", time_to_read, num_read_samples_size);

	for (i = 0; i < (num_read_samples / num_read_samples_size); i++)
	{
		packetsdropped[0] += COG_GetSample(device, num_read_samples_size, &dataByOne[num_read_samples_size*i*num_channels]);
		
		// conditional statement eliminates lag that occurs when output statments are spammed
		if ((i + 1) % 1000 == 0)
			printf("Progress: %f\n", (100.0 * ((double)(i + 1) / ((double)num_read_samples / (double)num_read_samples_size))));
	}
	printf("\n");

	// reading ten packets at a time
	num_read_samples_size = 10;

	double* dataByTen = new double[num_read_samples*num_channels];
	printf("Reading %d minute(s) worth of data, %d packets at a time...", time_to_read, num_read_samples_size);

	for (i = 0; i < (num_read_samples / num_read_samples_size); i++)
	{
		packetsdropped[1] = COG_GetSample(device, num_read_samples_size, &dataByTen[num_read_samples_size*i*num_channels]);

		if ((i + 1) % 100 == 0)
			printf("Progress: %f\n", (100.0 * (((double)i + 1) / ((double)num_read_samples / (double)num_read_samples_size))));
	}
	printf("\n");

	// reading one hundred packets at a time
	num_read_samples_size = 100;

	double* dataByHundred = new double[num_read_samples*num_channels];
	printf("Reading %d minute(s) worth of data, %d packets at a time...", time_to_read, num_read_samples_size);

	for (i = 0; i < (num_read_samples / num_read_samples_size); i++)
	{
		packetsdropped[2] += COG_GetSample(device, num_read_samples_size, &dataByHundred[num_read_samples_size*i*num_channels]);

		if ((i + 1) % 10 == 0)
			printf("Progress: %f\n", (100.0 * (((double)i + 1) / ((double)num_read_samples / (double)num_read_samples_size))));
	}
	printf("\n");

	// reading 1000 packets at a time
	num_read_samples_size = 1000;

	double* dataByThousand = new double[num_read_samples*num_channels];
	printf("Reading %d minute(s) worth of data, %d packets at a time...", time_to_read, num_read_samples_size);

	for (i = 0; i < (num_read_samples / num_read_samples_size); i++)
	{
		packetsdropped[3] += COG_GetSample(device, num_read_samples_size, &dataByThousand[num_read_samples_size*i*num_channels]);

		printf("Progress: %f\n", (100.0 * (((double)i + 1) / ((double)num_read_samples / (double)num_read_samples_size))));
	}
	printf("\n");

	// reading 10000 packets at a time
	num_read_samples_size = 10000;

	double* dataBy10000 = new double[num_read_samples*num_channels];
	printf("Reading %d minute(s) worth of data, %d packets at a time...", time_to_read, num_read_samples_size);

	for (i = 0; i < (num_read_samples / num_read_samples_size); i++)
	{
		packetsdropped[4] += COG_GetSample(device, num_read_samples_size, &dataBy10000[num_read_samples_size*i*num_channels]);

		printf("Progress: %f\n", (100.0 * (((double)i + 1) / ((double)num_read_samples / (double)num_read_samples_size))));
	}
	printf("\n");

	printNumErrors(packetsdropped, 5);

	printf("\n");

	printf("\nFinished read, writing output to file COG_Data.txt...\n");

	// write device data into text file
	FILE * outFile;
	outFile = fopen("COG_Data.txt", "w");
	if (outFile != NULL)
	{
		fprintf(outFile, "Data from device %s\n", &cognionicsDeviceList[devDescriptionLength*connectnum]);
		fprintf(outFile, "%s", "Channel Output\n");
		fprintf(outFile, "----------------------------------------\n");

		// write one-sample-at-a-time data
		for (i = 0; i < num_read_samples*num_channels; i++)
		{
			fprintf(outFile, "%g\n", dataByOne[i]);
		}
		// write ten-samples-at-a-time data
		for (i = 0; i < num_read_samples*num_channels; i++)
		{
			fprintf(outFile, "%g\n", dataByTen[i]);
		}
		// write one-hundred-samples-at-a-time data
		for (i = 0; i < num_read_samples*num_channels; i++)
		{
			fprintf(outFile, "%g\n", dataByHundred[i]);
		}
		// write one-thousand-samples-at-a-time data
		for (i = 0; i < num_read_samples*num_channels; i++)
		{
			fprintf(outFile, "%g\n", dataByThousand[i]);
		}
		// write 10000-samples-at-a-time data
		for (i = 0; i < num_read_samples*num_channels; i++)
		{
			fprintf(outFile, "%g\n", dataBy10000[i]);
		}
	}

	// close output file
	fclose(outFile);

	printf("File written.\n");

	system("pause");
	
	return 0;
}

int getValidINTInput(int lowerLimit, int upperLimit)
{
	int inputValue;
	int i;
	char inputString[64];

	while (1)
	{
		for (i = 0; i < sizeof(inputString); i++)
		{
			inputString[i] = getchar();
			if (inputString[i] == '\n')
			{
				inputString[i] = '\0';
				break;
			}
		}

		for (i = 0; i < sizeof(inputString); i++)
		{
			if (inputString[i] == '\0')
			{
				if (i == 0)
				{
					printf("Nothing entered, try again: ");
					break;
				}
				else
				{
					inputValue = atoi(inputString);

					if (inputValue >= lowerLimit && inputValue <= upperLimit)
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
				if (inputString[i] < 48 || inputString[i] > 57)
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

void printFlagName(int flag_value)
{
	switch (flag_value)
	{
	// general flags
	case 0:
		printf("COG_OK");
		return;
	case -1:
		printf("COG_INVALIDINPUT");
		return;
	case -2:
		printf("COG_INVALIDCHANNELVAL");
		return;
	case -3:
		printf("COG_INVALIDVALUE");
		return;
	case -4:
		printf("COG_TIMEOUTERROR");
		return;
	case -5:
		printf("COG_WRITEFAIL");
		return;

	// device query and connection error flags
	case -25:
		printf("COG_NODEVICESCONNECTED");
		return;
	case -26:
		printf("COG_MAXDEVEXCEEDED");
		return;
	case -27:
		printf("COG_CANNOTGETDEVINFO");
		return;
	case -28:
		printf("COG_CONNECTIONFAIL");
		return;
	case -29:
		printf("COG_DISCONNECTFAIL");
		return;

	// get config error flags
	case -50:
		printf("COG_CONFIGFINDSTARTFAIL");
		return;
	case -51:
		printf("COG_CONFIGFINDSTARTFAIL");
		return;

	// set config error flags
	case -75:
		printf("COG_BADINIT");
		return;
	case -76:
		printf("COG_BADFLUSH");
		return;
	case -77:
		printf("COG_NONSTARTRECEIVED");
		return;
	case -78:
		printf("COG_SHORTDATAERROR");
		return;
	case -79:
		printf("COG_LONGDATAERROR");
		return;
	case -80:
		printf("COG_SHORTDATAERROR");
		return;
	case -81:
		printf("COG_LONGDATAERROR");
		return;

	// SD start stop flags
	case -100:
		printf("COG_NOCARDDETECTED");
		return;
	case -101:
		printf("COG_INVALIDMEM");
		return;
	case -102:
		printf("COG_SDMODEFAIL");
		return;
	case -103:
		printf("COG_SDSTARTFAIL");
		return;

	default:
		printf("UNRECOGNIZED_VALUE (%d)", flag_value);
		return;
	}
}

void printNumErrors(int error_array[], int arr_length)
{
	int i;
	int total_errors = 0;
	
	for (i = 0; i < arr_length; i++)
	{
		total_errors += error_array[i];
	}

	// no packets lost during transmission
	if (total_errors == 0)
	{
		printf("No packets lost during read.\n");
		return;
	}
	else
	{
		for (i = 0; i < arr_length; i++)
		{
			// ordinal number formatting
			printf("%d packets lost during ", error_array[i]);
			if (i % 10 > 2 || i % 100 == 10 || i % 100 == 11 || i % 100 == 12 || (i - 1) % 10 == 0)
			{
				printf("%dth", i + 1);
			}
			else if (i % 10 == 0)
			{
				printf("%dst", i + 1);
			}
			else if (i % 10 == 1)
			{
				printf("%dnd", i + 1);
			}
			else if (i % 10 == 2)
			{
				printf("%drd", i + 1);
			}
			printf(" sample rate\n");
		}

		printf("Total packets lost during read: %d", total_errors);
		printf("\n");
	}

	return;
}