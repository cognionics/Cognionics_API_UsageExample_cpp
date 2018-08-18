// COGLibva1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ftd2xx.h"
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "COGLib.h"
#include "channel_labels.h"
#include "config_default.h"

#define COG_NUMDEVICES_MAX		256
#define COG_DEVDESCRIPTION_MAX	64
#define READ_TIMEOUT			500
#define NUM_CHANNELS			23
#define CHANNEL_MULT_FACTOR		1.0
#define TAIL_LENGTH				4
#define HEADER_LENGTH			2
#define MAX_FILENAME_SIZE		32
#define MAX_SUBJECT_SIZE		64
#define MAX_DESCRIP_SIZE		256
#define MAX_BYTES_PREFLUSH		64*512
#define MAX_DELTA_ARR_SIZE		1024
#define INT_FIELDS				20 //number of int fields in configInfo

//some constants for initialling imp check filters
const double ACCOFF = 65536.0 / 2.0;
const double ACC2G = 3.0 / (420e-3 * 65536.0);

const double ISTIM = 0.000000024 * 1.4;

const double fs4NotchB1 = 0;
const double fs4NotchB2 = (1 + 0.7) / 2.0;
const double fs4NotchA1 = 0;
const double fs4NotchA2 = 0.7;
const double fs4NotchB0 = (1 + 0.7) / 2.0;
const double fs2NotchB0 = (1 + 0.6) / 2.0;
const double fs2NotchB1 = (1 + 0.6) / 2.0;
const double fs2NotchB2 = 0;
const double fs2NotchA1 = 0.6;
const double fs2NotchA2 = 0;

const double offsetFilterCutoff = 0.2;

const int PHASE_OFFSET = 1;

const double impFilterTime = 1.0;

//structs for Impedance Check
//FIR
const int A0 = 1; //const?

struct FIRinfo {
	double A1;
	double A2;
	double B0;
	double B1;
	double B2;

	//numEEG + numEXT + numACC + 1 + 1;
	int channels;

	double xp1[MAX_CHANNEL_NUM];
	double xp2[MAX_CHANNEL_NUM];
	double yp1[MAX_CHANNEL_NUM];
	double yp2[MAX_CHANNEL_NUM];

	//double hpf_comp[MAX_CHANNEL_NUM];
};

//long FIR
struct longFIRinfo {
	double xinvectors[MAX_CHANNEL_NUM][MAX_CHANNEL_NUM];
	double *fircoeff;

	//numEEG
	int numEEG;

	//length of fircoeff
	int ch_length;

	int xinIndex;

	bool initialized;
};

//impGenTwo
const int PHASEOFFSET = 1;
const double ZR = 10000.0;
const double OMEGA = 2 * PI * 125.0;

struct impGenTwo {
	double prevFourSample[4][MAX_CHANNEL_NUM];

	double ALPHA_MAG;
	double ALPHA_PHASE;

	double ZxMAG[MAX_CHANNEL_NUM];
	double ZxPhase[MAX_CHANNEL_NUM];
	double ZxR[MAX_CHANNEL_NUM];
	double ZxC[MAX_CHANNEL_NUM];

	int numEEG;
};

//settings struct for COG Devices
struct configInfo {
	//device index
	int index;

	//tests whether or not device config has been read
	bool config;

	//ftHandle and ftStatus, for using the ftFunctions
	FT_HANDLE* ftHandle;
	FT_STATUS* ftStatus;

	//Reserved, read only
	//int	CONFIG_DATA_CHECK1;
	//int	SETTINGS_ARRAYS_COUNT;
	//int	SETTINGS_ARRAYS_SIZE;
	//int	CHANNELS_ARRAYS_COUNT;
	//int	CHANNELS_ARRAYS_SIZE;
	//int	MAX_DL_SIZE;
	//int	ADS_CHS_IN_USE_LIMIT;
	//int	CONFIG_VERSION;
	//int RESERVED;
	//int CONFIG_DATA_CHECK2;
	int RESERVED[100];

	//Channel Settings, modifiable - 9 fields
	//int	ADS_COUNT;
	//int	ADS_NCH;
	//int	EXT_CHS;
	//int	ACC_CHS;
	//int	ADS_CHANNEL_MODE; //write set
	//int	ADS_CHANNEL_GAIN; //write set
	//int	ADS_HIGH_RESOLUTION;
	//int	ADS_SAMPLING_RATE;
	//int	EXTERNAL_IMPEDANCE_CHECK; //write set
	int CHANNEL_SETTINGS[100];

	//Peripheral settings - 2 fields
	//int	USE_LEGACY_DATA_FORMAT;
	//int	USE_NRF_TRIGGER;
	int PERIPHERAL_SETTINGS[100];
	//PERIPHERAL_SETTINGS[0] = USE_LEGACY_DATA_FORMAT;
	//PERIPHERAL_SETTINGS[1] = USE_NRF_TRIGGER;

	//gain values
	//RESERVED
	//RESERVED
	//RESERVED
	//RESERVED
	//RESERVED
	//RESERVED
	//RESERVED
	//RESERVED
	//RESERVED
	//RESERVED
	int GAIN_VALUES[100];

	//read-only settings
	/*eeg_enabled_channels_number
	eeg_disabled_channels_number
	auxiliary_enabled_channels_number
	auxiliary_disabled_channels_number
	accelerometer_enabled_channels_number
	accelerometer_disabled_channels_number
	RESERVED
	RESERVED
	RESERVED
	RESERVED*/
	int READ_ONLY_SETTINGS[100];

	//Device name
	char DEVICE_NAME[100];

	//Hardware and Software Vnumbers
	/*Mechanical Hardware Version Character 1
	Mechanical Hardware Version Character 2
	Mechanical Hardware Version Character 3
	Electrical Hardware Version Character 1
	Electrical Hardware Version Character 2
	Electrical Hardware Version Character 3
	Software Version Character 1
	Software Version Character 2
	Software Version Character 3
	RESERVED*/
	int HARDWARE_AND_SOFTWARE_VERSION_NUMBERS[100];

	//serial number
	/*Character 1
	Character 2
	Character 3
	Character 4
	Character 5
	Character 6
	Character 7
	Character 8
	Character 9
	Character 10*/
	int SERIAL_NUMBER[100];

	double BatteryVoltage;

	//channel mapping array
	int ch_map[MAX_CHANNEL_NUM];

	//channel mask array
	int ch_mask[MAX_CHANNEL_NUM];

	//std pos ID
	int ch_id[MAX_CHANNEL_NUM];

	//std positions map array
	char std_pos_map[MAX_CHANNEL_NUM][MAX_CHANNEL_NAME_SIZE];

	//tests whether device config has been initialized locally
	bool config_initialized;

	//whether or not compressed read is initialized for current device
	bool compressedread_initialized;

	//imp check structs and vars
	impGenTwo igt;

	FIRinfo impMAGLPF;
	FIRinfo impPHALPF;
	FIRinfo offFilter;
	FIRinfo fs4Filter;
	FIRinfo fs2Filter;

	longFIRinfo impSine;
	longFIRinfo impCosine;

	bool IMPGENTwo;
};

// 3 bytes per channel, plus: 1 packet counter, 1 trigger, 1 imp_status, 
// and 1 battery voltage
const int MAX_SAMPLE_SIZE = 3 * MAX_CHANNEL_NUM + 4; 

const char stopArray[] = { 
	'8',
	'8',
	'8',
	'8',
	'8',
	'8',
	'0',
	'0',
	'0',
	'0',
	'0',
	'0',
	'0',
	'0',
	'0',
	'0'
};

const double EEGConvFactor = ((5.0f) / (3.0f)) * ((1.0f) / (4294967296.0f)); 
	// = 3.880510727564494e-10
const double AccConvFactor = 1;//(2.5f) * ((1.0f) / (4294967296.0f)); 
	// = 5.82076609134674072265625e-10
const double BatteryGain = (2.0 * 2.5) / 128.0;

//EEG Channel and ACC constants for Quick-20
const double channelGain[NUM_CHANNELS + 4] = {
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	EEGConvFactor,
	AccConvFactor,
	AccConvFactor,
	AccConvFactor,
	1.0,
	1.0,
	1.0,
	1.0
};

using namespace std;

// FTDI and DLL function variables
char COGDeviceName[COG_NUMDEVICES_MAX][COG_DEVDESCRIPTION_MAX];
char RxBuffer[256];
char TxBuffer[256];

DWORD EventDWord;
DWORD TxBytes;
DWORD RxBytes;
DWORD BytesReceived;
DWORD BytesWritten;


//Device variables
DWORD configData[4096];

configInfo *device[COG_NUMDEVICES_MAX];

int devicesConnected;
static FT_DEVICE_LIST_INFO_NODE COGDeviceList[COG_NUMDEVICES_MAX];


//temporary variables for read/write functions
BYTE temp;
BYTE temp1;
BYTE temp2;

BYTE msb;
BYTE lsb1;
BYTE lsb2;
BYTE lsb3;

/**
returns the current software library version as a string.

@return a string that contains the version number and date last edited
*/
const char* COG_GetVersionNumber()
{
	return "Version e_4, last revised 8/15/2018\0";
}

/**
debugging method that prints deviceID settings into the file filename.txt

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param filename the name of the file to write the deviceID settings into
*/
void debug_file_output(COGDevice *deviceID, const char* filename)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	ofstream myfile;
	myfile.open(filename);
	myfile << "Config Data\n" << endl;
	cout << "Writing output to file... " << endl;

	myfile << "*ftHandle: 0x" << hex << devInUse->ftHandle << endl;
	myfile << "ftHandle: 0x" << hex << *(devInUse->ftHandle) << endl;

	myfile << "ch_map: " << endl;
	for (int i = 0; i < 10; i++)
		myfile << dec << devInUse->ch_map[i] << "  ";
	myfile << endl;

	myfile << "RESERVED: " << endl;
	for (int i = 0; i < 10; i++)
		myfile << dec << devInUse->RESERVED[i] << "  ";
	myfile << endl;

	myfile << "CHANNEL_SETTINGS: " << endl;
	for (int i = 0; i < 10; i++)
		myfile << dec << devInUse->CHANNEL_SETTINGS[i] << "  ";
	myfile << endl;

	myfile << "GAIN_VALUES: " << endl;
	for (int i = 0; i < 10; i++)
		myfile << dec << devInUse->GAIN_VALUES[i] << "  ";
	myfile << endl;

	myfile << "READ-ONLY VALUES: " << endl;
	for (int i = 0; i < 10; i++)
		myfile << dec << devInUse->READ_ONLY_SETTINGS[i] << "  ";
	myfile <<endl;

	myfile << "PERIPHERAL_SETTINGS: " << endl;
	for (int i = 0; i < 10; i++)
		myfile << dec << devInUse->PERIPHERAL_SETTINGS[i] << "  ";
	myfile << endl;

	myfile.close();
}

#pragma region impcheck_functions

// STUFF FOR GETTING THE IMP CHECK STUFF

//impgentwo helper function
void CogComputeIMP(impGenTwo* imp_struct, double newData[], int PacketAtIndex)
{
   int i;
   double samplei;
   double sampleq;
   double sampleMAG;
   double samplePhase;

   for (i = 0; i < imp_struct->numEEG; i++)
   {
       imp_struct->prevFourSample[((int)newData[PacketAtIndex] + PHASE_OFFSET) % 4][i] = 1000000.0 * newData[i];
               
       samplei = (imp_struct->prevFourSample[0][i] - imp_struct->prevFourSample[2][i]);
       sampleq = (imp_struct->prevFourSample[3][i] - imp_struct->prevFourSample[1][i]);    //-1.0 * (imp_struct->prevFourSample[1][i] - imp_struct->prevFourSample[3][i]);

       sampleMAG = sqrt(pow(samplei, 2) + pow(sampleq, 2));
       samplePhase = atan2(sampleq, samplei);

       sampleMAG = imp_struct->ALPHA_MAG * sampleMAG;
       samplePhase = imp_struct->ALPHA_PHASE + samplePhase + PI;    //invert the vector since next action uses subtraction

       samplei = ZR + sampleMAG * cos(samplePhase);
       sampleq = sampleMAG * sin(samplePhase);

       imp_struct->ZxMAG[i] = sqrt(pow(samplei, 2) + pow(sampleq, 2));
       imp_struct->ZxPhase[i] = atan2(sampleq, samplei);

       samplePhase = tan(imp_struct->ZxPhase[i]);

       imp_struct->ZxR[i] = imp_struct->ZxMAG[i] * sqrt(1 + pow(samplePhase, 2));
       imp_struct->ZxC[i] = abs(samplePhase / (OMEGA * imp_struct->ZxR[i])); //-1.0 * (samplePhase / (OMEGA * ZxR[i])); since measurement error might result negative capacitance, this is used so capacitance amplitude can still be calculated since despite phase error, vector energy should still result the right capacitance magnitude
   }
}

//FIR info helper functions

// void UpdateCoefficents(FIRinfo* fir_struct, double b0, double b1, double b2, double a1, double a2)
// {
//     fir_struct->B0 = b0;
//     fir_struct->B1 = b1;
//     fir_struct->B2 = b2;
//     fir_struct->A1 = a1;
//     fir_struct->A2 = a2;
// }

// void SetHPFreq(FIRinfo* fir_struct, double HPFreq, double SR)
// {
//     double deltaT = 1.0 / SR; //sample interval is 1/current smaple rate
//     double rcHP = 1.0 / (6.28 * HPFreq);
//     double alpha = rcHP / (rcHP + deltaT);

//     fir_struct->B0 = alpha;
//     fir_struct->B1 = -alpha;
//     fir_struct->B2 = 0;
//     fir_struct->A1 = -alpha;
//     fir_struct->A2 = 0;
// }

// void SetLPFreq(FIRinfo* fir_struct, double LPFreq, double SR)
// {
//     double deltaT = 1.0 / SR; //sample interval is 1/current smaple rate
//     double rcLP = 1.0 / (6.28 * LPFreq);
//     double alpha = deltaT / (rcLP + deltaT);

//     fir_struct->B0 = alpha;
//     fir_struct->B1 = 0;
//     fir_struct->B2 = 0;
//     fir_struct->A1 = alpha - 1.0;
//     fir_struct->A2 = 0;
// }

void UpdateFilter(FIRinfo* fir_struct, double xin[], double yout[])
{
    double xin_temp; //placeholder for an in-place filter operation (xin is same as yout)
    for (int c = 0; c < fir_struct->channels; c++)
    {
        xin_temp = xin[c];

        //perform filtering
        yout[c] = fir_struct->B0 * xin_temp + fir_struct->B1 * fir_struct->xp1[c] + fir_struct->B2 * fir_struct->xp2[c] - fir_struct->A1 * fir_struct->yp1[c] - fir_struct->A2 * fir_struct->yp2[c];

        //update state variables
        fir_struct->xp2[c] = fir_struct->xp1[c];
        fir_struct->xp1[c] = xin_temp;

        fir_struct->yp2[c] = fir_struct->yp1[c];
        fir_struct->yp1[c] = yout[c];
    }
}

// void seedHPFState(FIRinfo* fir_struct, double xin_inital[])
// {
//     for(int c = 0; c < fir_struct->channels; c++)
//     {
//         fir_struct->hpf_comp[c] = xin_inital[c];
//     }
// }

//longFIR helper functions
void Update(longFIRinfo* long_fir_struct, double xin[], double yout[])
{
	//shift new samples in
	for (int c = 0; c < long_fir_struct->numEEG; c++)
	{
		double temp = xin[c];
		long_fir_struct->xinvectors[c][long_fir_struct->xinIndex] = temp;

		double t_yout = 0;
		int t_xindex = long_fir_struct->xinIndex;

		//perform dot product
		for (int j = (long_fir_struct->ch_length - 1); j >= 0; j--)
		{
			t_yout += long_fir_struct->xinvectors[c][t_xindex] * long_fir_struct->fircoeff[j];
			t_xindex--;
			if (t_xindex < 0)
				t_xindex = long_fir_struct->ch_length - 1;
		}

		yout[c] = t_yout;
	}

	long_fir_struct->xinIndex++;
	if (long_fir_struct->xinIndex == long_fir_struct->ch_length)
	{
		long_fir_struct->xinIndex = 0;
	}
}

//buffers for impedancecheck
double incomingWithImp[MAX_CHANNEL_NUM];
double incomingWithOutImp[MAX_CHANNEL_NUM];

double t_impedanceCarrier_I[MAX_CHANNEL_NUM];
double t_impedanceCarrier_Q[MAX_CHANNEL_NUM];

double ChannelImpedance[MAX_CHANNEL_NUM];
double ChannelPhase[MAX_CHANNEL_NUM];
double ChannelOffset[MAX_CHANNEL_NUM];

double* impedancecheck(COGDevice *deviceID, double channelData[], int channelData_length, bool ImpedanceCheckOn, int debug_counter)
{
	int i;
	int dc = debug_counter;
	configInfo* devInUse = (configInfo*)*deviceID;

	int NumEEG = devInUse->READ_ONLY_SETTINGS[0];
	int NumEXT = devInUse->READ_ONLY_SETTINGS[2];
	int NumACC = devInUse->READ_ONLY_SETTINGS[4];
	int PacketAtIndex = NumEEG + NumACC + NumEXT;

	for(i = 0; i < channelData_length; i++)
	{
		incomingWithImp[i] = channelData[i];
		incomingWithOutImp[i] = channelData[i];

	}
	
	UpdateFilter(&(devInUse->fs4Filter), incomingWithImp, incomingWithOutImp);
	UpdateFilter(&(devInUse->fs2Filter), incomingWithOutImp, incomingWithOutImp);

	//compute channel impedance values using DFT and 1/4 sample rate
	Update(&(devInUse->impSine), incomingWithImp, t_impedanceCarrier_I);
	Update(&(devInUse->impCosine), incomingWithImp, t_impedanceCarrier_Q);

	if ((devInUse->IMPGENTwo) == true)
	{
		CogComputeIMP(&(devInUse->igt), incomingWithImp, PacketAtIndex);

		for (int c = 0; c < NumEEG; c++)
		{
			ChannelImpedance[c] = devInUse->igt.ZxMAG[c];
			ChannelPhase[c] = devInUse->igt.ZxPhase[c];
			UpdateFilter(&(devInUse->impMAGLPF), ChannelImpedance, ChannelImpedance);
			UpdateFilter(&(devInUse->impPHALPF), ChannelPhase, ChannelPhase);
		}
	}
	else
	{
		for (int c = 0; c < NumEEG; c++)
		{
			ChannelImpedance[c] = sqrt(t_impedanceCarrier_I[c] * t_impedanceCarrier_I[c] + t_impedanceCarrier_Q[c] * t_impedanceCarrier_Q[c]) / ISTIM;
		}
	}

	//low pass offset data
	UpdateFilter(&(devInUse->offFilter), incomingWithOutImp, ChannelOffset);

	//determine which buffer to use for outout depending on status of impedance
	//if impedance check is on use the filtered
	if (ImpedanceCheckOn == true)
	{
		return incomingWithOutImp;
	}
	else
	{
		return incomingWithImp;
	}
}

// END STUFF FOR GETTING IMP CHECK STUFF

#pragma endregion impcheck_functions

#pragma region dev_status_functions

/**
returns a list of all available Cognionics devices.

@param numDevices a pointer that holds the number of devices found
@param stringLength the max number of characters of for a device name
@return a list of all available Cognionics devices
*/
char* COG_DeviceQuery(int* numDevices, int* stringLength)
{
	FT_STATUS ftStatus;
	DWORD numDevs;

	devicesConnected = 0;

	// create the device information list
	ftStatus = FT_CreateDeviceInfoList(&numDevs);

	//no devices connected
	if (ftStatus != FT_OK)
	{
		*numDevices = COG_NODEVICESCONNECTED;
		*stringLength = COG_DEVDESCRIPTION_MAX;
		return COGDeviceName[0];
	}

	//too many devices
	if (numDevs > COG_NUMDEVICES_MAX)
	{
		*numDevices = COG_MAXDEVEXCEEDED;
		*stringLength = COG_DEVDESCRIPTION_MAX;
		return COGDeviceName[0];
	}

	ftStatus = FT_GetDeviceInfoList(COGDeviceList, &numDevs);
	//cannot get info list
	if (ftStatus != FT_OK)
	{
		*numDevices = COG_CANNOTGETDEVINFO;
		*stringLength = COG_DEVDESCRIPTION_MAX;
		return COGDeviceName[0];
	}

	//this code seems redundant, but I'm just keeping it here just in case... who knows
	if (numDevs > COG_NUMDEVICES_MAX)
	{
		*numDevices = COG_MAXDEVEXCEEDED;
		*stringLength = COG_DEVDESCRIPTION_MAX;
		return COGDeviceName[0];
	}

	//all good - safe string copy COG device name into COGDeviceList descriptions
	//static memory for easy accessibility
	for (int i = 0; i < (signed)numDevs; i++)
	{
		if (strlen(COGDeviceName[i]) < COG_DEVDESCRIPTION_MAX)
			strcpy(COGDeviceName[i], COGDeviceList[i].Description);
	}

	*numDevices = numDevs;
	*stringLength = COG_DEVDESCRIPTION_MAX;

	return COGDeviceName[0];
}

/**
connects to a Cognionics device.

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param i the index of the device to connect to
@return 0 if successfully connected, -1 if otherwise
*/
int COG_Connect(COGDevice *deviceID, int i)
{
	int j;

	FT_STATUS ftStatus;
	FT_HANDLE ftHandle;

	//open a connection to the device
	ftStatus = FT_Open(i, &ftHandle);
	if (ftStatus != FT_OK)
	{
		printf("Connection failure. \n");
		// FT_Open failed
		return COG_CONNECTIONFAIL;
	}
	printf("Connected. \n");

	//Honestly this part is just copy-pasted from Mike's program
	FT_SetBaudRate(ftHandle, 3000000);
	FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0x11, 0x13);
	FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
	FT_SetLatencyTimer(ftHandle, 2);

	device[devicesConnected] = (configInfo*)malloc(sizeof(configInfo));
	device[devicesConnected]->ftHandle = (FT_HANDLE*)malloc(sizeof(FT_HANDLE));
	device[devicesConnected]->ftStatus = (FT_STATUS*)malloc(sizeof(FT_STATUS));

	*(device[devicesConnected]->ftHandle) = ftHandle;
	*(device[devicesConnected]->ftStatus) = ftStatus;
	device[devicesConnected]->index = devicesConnected;
	device[devicesConnected]->BatteryVoltage = 0.0;
	device[devicesConnected]->compressedread_initialized = false;
	device[devicesConnected]->config_initialized = false;

	//initialize impGenTwo struct
	device[devicesConnected]->igt.ALPHA_MAG = 0.0;
	device[devicesConnected]->igt.ALPHA_PHASE = 0.0;

	//initialize FIRinfo structs
	device[devicesConnected]->fs2Filter.A1 = fs2NotchA1;
	device[devicesConnected]->fs2Filter.A2 = fs2NotchA2;
	device[devicesConnected]->fs2Filter.B0 = fs2NotchB0;
	device[devicesConnected]->fs2Filter.B1 = fs2NotchB1;
	device[devicesConnected]->fs2Filter.B2 = fs2NotchB2;
	for(j = 0; j < MAX_CHANNEL_NUM; j++)
	{
		device[devicesConnected]->fs2Filter.xp1[j] = 0;
		device[devicesConnected]->fs2Filter.xp2[j] = 0;
		device[devicesConnected]->fs2Filter.yp1[j] = 0;
		device[devicesConnected]->fs2Filter.yp2[j] = 0;
	}

	device[devicesConnected]->fs4Filter.A1 = fs4NotchA1;
	device[devicesConnected]->fs4Filter.A2 = fs4NotchA2;
	device[devicesConnected]->fs4Filter.B0 = fs4NotchB0;
	device[devicesConnected]->fs4Filter.B1 = fs4NotchB1;
	device[devicesConnected]->fs4Filter.B2 = fs4NotchB2;
	for(j = 0; j < MAX_CHANNEL_NUM; j++)
	{
		device[devicesConnected]->fs4Filter.xp1[j] = 0;
		device[devicesConnected]->fs4Filter.xp2[j] = 0;
		device[devicesConnected]->fs4Filter.yp1[j] = 0;
		device[devicesConnected]->fs4Filter.yp2[j] = 0;
	}

	device[devicesConnected]->impPHALPF.A1 = 0;
	device[devicesConnected]->impPHALPF.A2 = 0;
	device[devicesConnected]->impPHALPF.B0 = 0;
	device[devicesConnected]->impPHALPF.B1 = 0;
	device[devicesConnected]->impPHALPF.B2 = 0;
	for(j = 0; j < MAX_CHANNEL_NUM; j++)
	{
		device[devicesConnected]->impPHALPF.xp1[j] = 0;
		device[devicesConnected]->impPHALPF.xp2[j] = 0;
		device[devicesConnected]->impPHALPF.yp1[j] = 0;
		device[devicesConnected]->impPHALPF.yp2[j] = 0;
	}

	device[devicesConnected]->impMAGLPF.A1 = 0;
	device[devicesConnected]->impMAGLPF.A2 = 0;
	device[devicesConnected]->impMAGLPF.B0 = 0;
	device[devicesConnected]->impMAGLPF.B1 = 0;
	device[devicesConnected]->impMAGLPF.B2 = 0;
	for(j = 0; j < MAX_CHANNEL_NUM; j++)
	{
		device[devicesConnected]->impMAGLPF.xp1[j] = 0;
		device[devicesConnected]->impMAGLPF.xp2[j] = 0;
		device[devicesConnected]->impMAGLPF.yp1[j] = 0;
		device[devicesConnected]->impMAGLPF.yp2[j] = 0;
	}

	device[devicesConnected]->impCosine.xinIndex = 0;
	device[devicesConnected]->impSine.xinIndex = 0;

	device[devicesConnected]->impCosine.initialized = false;
	device[devicesConnected]->impSine.initialized = false;

	device[devicesConnected]->IMPGENTwo = false;

	*deviceID = device[devicesConnected];
	devicesConnected++;

	return 0;
}

/**
disconnects from a Cognionics device.

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if successfully disconnected, -1 if otherwise
*/
int COG_Disconnect(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	*(devInUse->ftStatus) = FT_Close(*(devInUse->ftHandle));

	if (*(devInUse->ftStatus) != FT_OK)
	{
		return COG_DISCONNECTFAIL;
	}

	if(devInUse->impCosine.initialized)
		free(devInUse->impCosine.fircoeff);
	if(devInUse->impSine.initialized)
		free(devInUse->impCosine.fircoeff);

	free(devInUse->ftHandle);
	free(devInUse->ftStatus);
		
	free(deviceID);

	return 0;
}

/**
Sets the impedance check of a Cognionics device to on

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
*/
int COG_SetImpedanceOn(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	TxBuffer[0] = 0x11;
	*(devInUse->ftStatus) = FT_Write(*(devInUse->ftHandle), TxBuffer, 1, &BytesWritten);
	if (*(devInUse->ftStatus) != FT_OK) return COG_WRITEFAIL;
	return 0;
}

/**
Sets the impedance check of a Cognionics device to off

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
*/
int COG_SetImpedanceOff(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	TxBuffer[0] = 0x12;
	*(devInUse->ftStatus) = FT_Write(*(devInUse->ftHandle), TxBuffer, 1, &BytesWritten);
	if (*(devInUse->ftStatus) != FT_OK) return COG_WRITEFAIL;
	return 0;
}

#pragma endregion dev_status_functions

#pragma region readwrite_functions

/**
writes a byte to a Cognionics device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param val the byte to send to the device
@return 0 if write successful, -1 if otherwise
*/
int writebyte(COGDevice *deviceID, BYTE val)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	TxBuffer[0] = val;
	*(devInUse->ftStatus) = FT_Write(*(devInUse->ftHandle), TxBuffer, 1, &BytesWritten);
	if (*(devInUse->ftStatus) != FT_OK) return COG_WRITEFAIL;
	return 0;
}

/**
writes a word to a Cognionics device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param val the word to send to the device
@return 0 if write successful, -1 to -4 if otherwise depending on the number of write failures
*/
int writeword(COGDevice *deviceID, DWORD data)
{
	int sum_res = 0;
	sum_res += writebyte(deviceID, (data >> 24) & 0xFF);
	sum_res += writebyte(deviceID, (data >> 16) & 0xFF);
	sum_res += writebyte(deviceID, (data >> 8) & 0xFF);
	sum_res += writebyte(deviceID, (data >> 0) & 0xFF);
	
	return sum_res;
}

/**
reads a byte from a Cognionics device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param temp a pointer that stores the byte that was read from the device
@return true if byte read is 0xFF, flase if otherwise
*/
bool readbyte(COGDevice *deviceID, BYTE* temp)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	*(devInUse->ftStatus) = FT_GetStatus(*(devInUse->ftHandle), &RxBytes, &TxBytes, &EventDWord);
	*(devInUse->ftStatus) = FT_Read(*(devInUse->ftHandle), temp, 1, &BytesReceived);

	if (*(devInUse->ftStatus) != FT_OK)
	{
		printf("Read error\n");
		return false;
	}
	if (*temp == 0xFF) return true;
	return false;
}

/**
reads a word from a Cognionics device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param temp a pointer that stores the byte that was read from the device
@return 0 if read successful, -1 if otherwise
*/
DWORD readword(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	DWORD result;

	readbyte(deviceID, &temp);
	msb = temp;
	readbyte(deviceID, &temp);
	lsb1 = temp;
	readbyte(deviceID, &temp);
	lsb2 = temp;
	readbyte(deviceID, &temp);
	lsb3 = temp;

	cout << (int)msb << " " << (int)lsb1 << " " << (int)lsb2 << " " << (int)lsb3 << " ";

	result = ((int)msb << 24 | (int)lsb1 << 16 | (int)lsb2 << 8 | (int)lsb3);

	return result;
}

#pragma endregion readwrite_functions

#pragma region COG_GetSample_functions

//variable used for keeping track of packet loss
int lastPacket = -1;
/**
reads uncompressed data from a Cognionics device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored
@param numSample the number of samples to read from the device
@param channelData a pointer that stores the data read from the device
@return 0 <- should add more return values
*/
int read_uncompressed(COGDevice *deviceID, int numSample, double* channelData)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	int num_EEG = devInUse->READ_ONLY_SETTINGS[0];
	int num_EXT = devInUse->READ_ONLY_SETTINGS[2];
	int num_ACC = devInUse->READ_ONLY_SETTINGS[4];

	int num_Channels = num_EEG + num_EXT + num_ACC;
	int packetSize = num_Channels + 4;
	
	int i = 0;
	int j;

	int packet_byte;
	int result;
	int trig_result;

	int packetErrorCounter = 0;
	int t_packetDiff;

	BYTE trig1;
	BYTE trig2;
	BYTE battery;
	BYTE imp_status;

	bool ImpedanceCheckOn;
	bool needToFindSync = true;
	int syncErrorFlag;
	double* currentSampleData;
	double* temp;

	while(i < numSample)
	{
		syncErrorFlag = 0;
		currentSampleData = channelData + packetSize * i;

		//ensures that every packet will start with 0xFF and therefore read/parse all data correctly
		while (needToFindSync)
		{
			if (readbyte(deviceID, &msb))
			{
				needToFindSync = false;
			}
			else
			{
				if (lastPacket >= 0)
				{
					syncErrorFlag = 1;
				}
			}
		}

		if (readbyte(deviceID, &msb))
		{
			packetErrorCounter++;
			i++;
			continue;
		}
		packet_byte = msb;

		//parse data
		for (j = 0; j < num_Channels; j++)
		{
			//read msb
			if (readbyte(deviceID, &msb))
			{
				packetErrorCounter++;
				i++;
				j = -1;
				break;
			}
			//read lsb2
			if (readbyte(deviceID, &lsb2))
			{
				packetErrorCounter++;
				i++;
				j = -1;
				break;
			}
			//read lsb1
			if (readbyte(deviceID, &lsb1))
			{
				packetErrorCounter++;
				i++;
				j = -1;
				break;
			}

			result = ((int)msb << 24) | ((int)lsb2 << 17) | ((int)lsb1 << 10);

			currentSampleData[j] = (double)result;
			currentSampleData[j] *= channelGain[j];
		}

		if (j != -1)
		{
			if (lastPacket >= 0)
			{
				t_packetDiff = packet_byte - lastPacket;
				if (t_packetDiff != 1 && t_packetDiff != -127)//if (t_packetDiff != 1 && t_packetDiff != -32767)
				{
					if (t_packetDiff < 0)
					{
						packetErrorCounter += 128 + t_packetDiff;//packetErrorCounter += 32768 + t_packetDiff;
					}
					else
					{
						packetErrorCounter++;
					}
				}
			}
			lastPacket = packet_byte;

			//store packet counter
			currentSampleData[j] = (double)packet_byte;
			currentSampleData[j] *= channelGain[j];
			j++;

			//parse impedance
			//note: impedance will be stored after the trigger
			if (readbyte(deviceID, &imp_status))
			{
				packetErrorCounter++;
				i++;
				continue;
			}

			//parse battery
			//note: battery will be stored last
			if (readbyte(deviceID, &battery))
			{
				packetErrorCounter++;
				i++;
				continue;
			}

			//parse trigger1
			if (readbyte(deviceID, &trig1))
			{
				packetErrorCounter++;
				i++;
				continue;
			}

			//parse trigger2
			if (readbyte(deviceID, &trig2))
			{
				packetErrorCounter++;
				i++;
				continue;
			}

			trig_result = ((int)trig1 << 8) | ((int)trig2);
			currentSampleData[j] = (double)trig_result;
			currentSampleData[j] *= channelGain[j];
			j++;

			//store battery
			currentSampleData[j] = (double)battery;
			currentSampleData[j] *= channelGain[j];
			j++;

			//store impedance
			currentSampleData[j] = (double)imp_status;
			currentSampleData[j] *= channelGain[j];
			if(imp_status == 0x11)
				ImpedanceCheckOn = true;
			else ImpedanceCheckOn = false;

			if (syncErrorFlag == 1)
			{
				packetErrorCounter++;
			}

			temp = impedancecheck(deviceID, currentSampleData, (num_Channels + 2), ImpedanceCheckOn, i*packetSize + j);

			for(j = 0; j < packetSize; j++)
				channelData[packetSize*i + j] = temp[j];

			needToFindSync = true;
		}

		i++;
	}

	return packetErrorCounter;
}

//buffers used by read_compressed

//buffer for incoming data
int incomingRawInt[MAX_CHANNEL_NUM];

//buffer for previously received data
int prevSampleBuffer[MAX_SAMPLE_SIZE][MAX_CHANNEL_NUM];

/**
reads encoded data - Function basically copy-pasted off the Mike's code
*/
int readdeltadata(COGDevice *deviceID, int numPrevSamples, int numChannels, int pastSampleIndex)
{
	int i;
	int j;

	int t_dataByteCount = 0;
	

	//read byte to see if it's a keyframe
	readbyte(deviceID, &temp);
	if (temp == 1)
    {
        for (i = 0; i < numPrevSamples; i++)
        {
            for(j = 0; j < numChannels; j++)
                prevSampleBuffer[i][j] = 0;
        }
    }
    t_dataByteCount++;


	for (i = 0; i < numChannels; i++)
	{
		//read byte and determine which type of update
		BYTE t_newByte;
		readbyte(deviceID, &t_newByte);

		if ((t_newByte & 0x01) == 0) // 7-bit update
		{
			t_dataByteCount += 1;

			int t_lsb = t_newByte;

			t_lsb = t_lsb >> 1;

			t_lsb = t_lsb << 25;
			t_lsb = t_lsb >> 25;
			t_lsb = t_lsb << 3;

			incomingRawInt[i] = prevSampleBuffer[pastSampleIndex][i] + t_lsb;
		}
		else if (((t_newByte >> 1) & 0x01) == 0) //14-bit update
		{
			t_dataByteCount += 2;

			int t_lsb = t_newByte;
			readbyte(deviceID, &temp);
			int t_msb = (int)temp;

			int t_difference = (t_lsb & 0xFC) | (t_msb << 8);

			t_difference = t_difference << 16;
			t_difference = t_difference >> 15;

            incomingRawInt[i] = prevSampleBuffer[pastSampleIndex][i] + t_difference;
		}

		else //21-bit update
		{
			t_dataByteCount += 3;

			int t_lsb1 = t_newByte; 
			readbyte(deviceID, &temp);
			int t_lsb2 = (int)temp;
			readbyte(deviceID, &temp);
			int t_msb = (int)temp;

			incomingRawInt[i] = (t_msb << 24) | (t_lsb2 << 16) | ((t_lsb1&0xF8) << 8);
            incomingRawInt[i] = incomingRawInt[i] >> 8;
		}

		//update past sample buffer (24-bit signed int with sign bits padded to MSB)
		prevSampleBuffer[pastSampleIndex][i] = incomingRawInt[i];
	}

	//increment past sample bufer index
	pastSampleIndex++;
	if (pastSampleIndex == numPrevSamples)
		pastSampleIndex = 0;

	return t_dataByteCount;
}


int prevPacketCount = 0;
int newPacketCount = 0;
int LostPackets = 0;
/**
reads compressed data from a Cognionics device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param numSample the number of samples to read from the device
@param channelData a pointer that stores the data read from the device
@param num_Channel the number of channels, including one for battery and one for impedance
@return 0 if read successful, else returns the number of packets lost
*/
int read_compressed(COGDevice *deviceID, int numSample, double* channelData)
{
	configInfo *devInUse = (configInfo*)*deviceID;

	int NumEEG = devInUse->READ_ONLY_SETTINGS[0];
	int NumEXT = devInUse->READ_ONLY_SETTINGS[2];
	int NumACC = devInUse->READ_ONLY_SETTINGS[4];

	int num_Channel = NumEEG + NumEXT + NumACC + 2;

	int i;
	int j;

	int PacketAtIndex = num_Channel;
	int TriggerAtIndex = num_Channel + 1;

	bool ImpedanceCheckOn = true;
	bool needToFindSync = true;

	int bytesRcved = 0;

	bool TriggerLinked = true;

	int numPrevSamples;

	int pastSampleIndex;

	numPrevSamples = 4;

	pastSampleIndex = 0;

	int t_packetDiff;
	int t_deltaLen;
	int t_status1;
	double t_batt1;
	double t_batt2;

	COGDevice *ref = deviceID;

	double* temp_Arr;

	if (!devInUse->compressedread_initialized)
	{
		for (i = 0; i < numPrevSamples; i++)
		{
			for (j = 0; j < num_Channel; j++)
			{
				prevSampleBuffer[i][j] = 0;
			}
		}
		devInUse->compressedread_initialized = true;
	}
	int counter = 0;

	while (counter < numSample)
	{
		while(needToFindSync)
		{
			while(!readbyte(deviceID, &temp)){}
			if(readbyte(deviceID, &temp))
			{
				if(readbyte(deviceID, &temp))
				{
					needToFindSync = false;
					break;
				}
			}
		}

		prevPacketCount = newPacketCount;
		readbyte(deviceID, &temp1);
		readbyte(deviceID, &temp2);
		newPacketCount = ((((int)temp1 << 8)) | (int)temp2);
		t_packetDiff = newPacketCount - prevPacketCount;
		if (t_packetDiff != 1 && t_packetDiff != -32767)
		{
			if (t_packetDiff<0)
			{
				LostPackets += 32768 + t_packetDiff;
			}
			else
			{
				LostPackets += 1;
			}
		}

		incomingRawInt[PacketAtIndex] = newPacketCount;
		
		t_deltaLen = readdeltadata(deviceID, numPrevSamples, (NumEEG + NumEXT + NumACC), pastSampleIndex);

		bytesRcved += t_deltaLen;
		readbyte(deviceID, &temp1);
		t_status1 = (int)temp1;
		bytesRcved += 1;

		for (i = NumEEG; i < (NumEEG + NumEXT); i++)
		{
			incomingRawInt[i] = incomingRawInt[i] << 5;
		}
		for (i = (NumEEG + NumEXT); i < num_Channel; i++)
		{
			incomingRawInt[i] = incomingRawInt[i] << 3;
		}
		if ((t_status1 & 0x01) != 0)
		{
			ImpedanceCheckOn = true;
		}
		else
		{
			ImpedanceCheckOn = false;
		}
		
		//if (counter >= 13767)
		//	cout << "Imp check: " << counter << " " << ref << ", " << *ref << endl;

		if ((t_status1 & 0x04) != 0)
		{
			readbyte(deviceID, &msb);
			readbyte(deviceID, &lsb1);
			readbyte(deviceID, &lsb2);
			readbyte(deviceID, &lsb3);
			incomingRawInt[TriggerAtIndex] = ((DWORD)msb << 24) | ((DWORD)lsb1 << 16) | ((DWORD)lsb2 << 8) | ((DWORD)lsb3 << 0);
			bytesRcved += 4;
		}
		if ((t_status1 & 0x20) != 0)
		{
			readbyte(deviceID, &temp1);
			readbyte(deviceID, &temp2);
			t_batt1 = BatteryGain * temp1;
			t_batt2 = BatteryGain * temp2;
			devInUse->BatteryVoltage = t_batt1;
			bytesRcved += 2;
		}

		if ((t_status1 & 0x02) != 0)
		{
			TriggerLinked = true;
		}
		else
		{
			TriggerLinked = false;
		}
		
		if(ImpedanceCheckOn) 
			incomingRawInt[num_Channel - 1] = 0x11;
		else incomingRawInt[num_Channel - 1] = 0x12;
	
		//if (counter >= 13767)
		//	cout << "Line 1454: " << counter << " " << ref << ", " << *ref << endl;

		for (i = 0; i < num_Channel; i++)
		{
			int pos = counter * num_Channel + i;
			channelData[pos] = channelGain[i] * (double)incomingRawInt[i];
		}

		channelData[counter * num_Channel + num_Channel - 2] = devInUse->BatteryVoltage;

		temp_Arr = impedancecheck(deviceID, &channelData[counter * num_Channel], num_Channel, ImpedanceCheckOn, counter);

		for (j = 0; j < num_Channel; j++)
			channelData[counter * num_Channel + j] = temp_Arr[j];

		needToFindSync = true;
		counter++;
	}	

	return LostPackets;
}

/**
retrieves a specified number of data samples from a Cognionics device; this method is compatible 
with all Cognionics devices

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param numSample the number of samples to read from the device
@param channelData a pointer that stores the data read from the device
@return 0 if successful, else returns an error flag
*/
int COG_GetSample(COGDevice *deviceID, int numSample, double* channelData)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	int use_legacy_data_format = devInUse->PERIPHERAL_SETTINGS[0];
	
	if (numSample < 0) return COG_INVALIDINPUT;

	//uncompressed data
	if (use_legacy_data_format == 1) return read_uncompressed(deviceID, numSample, channelData);
	//compressed data
	//copied from Mike's code
	else if (use_legacy_data_format == 0) return read_compressed(deviceID, numSample, channelData);

	//use_legacy_data_format has a bad value - shouldn't happen
	return COG_INVALIDVALUE;
}

#pragma endregion COG_GetSample_functions

#pragma region Config_functions

/**
finds the stop sequence after reading a configuration data sequence

@param end_test a buffer that stores the four bytes to test
@return true if buffer stores the stop sequence, false if otherwise
*/
bool testend(BYTE* end_test)
{
	bool first = (end_test[3] == 0x49);
	bool sec = (end_test[2] == 0x96);
	bool third = (end_test[1] == 0x02);
	bool fourth = (end_test[0] == 0xd2);

	return first && sec && third && fourth;
}

/**
finds the start sequence after sending the a command byte

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param config_cmd the command byte to send to the device
@return true if buffer stores the start sequence, false if otherwise
*/
bool findStart(COGDevice *deviceID, BYTE config_cmd)
{
	int true_counter;
	int read_counter;
	int tries;

	true_counter = 0;
	read_counter = 0;
	tries = 0;
	
	while (tries < 1) {
		printf("Sending query...");
		writebyte(deviceID, config_cmd);
		
		read_counter = 0;
		while (read_counter < 60000)
		{
			read_counter++;

			if (readbyte(deviceID, &temp))
			{
				true_counter++;
			}
			else
				true_counter = 0;

			if (true_counter == 4)
			{
				printf("Start sequence found\n");
				return true;
			}
		}
		tries++;
		//Sleep(1000);
	}

	return false;
}

/**
flushes the device read buffer

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param return 0 is successfully flushed, COG_BADFLUSH if otherwise
*/
int preFlush(COGDevice *deviceID)
{
	int i;
	int q;
	int r;

	configInfo* devInUse = (configInfo*)*deviceID;

	FT_GetStatus(*(devInUse->ftHandle), &RxBytes, &TxBytes, &EventDWord);

	q = RxBytes/256;
	r = RxBytes - q*256;

	if(RxBytes > 0)
	{
		for(i = 0; i < q; i++)
		{
			*(devInUse->ftStatus) = FT_Read(*(devInUse->ftHandle), RxBuffer, 256, &BytesReceived);
			if(*(devInUse->ftStatus) != FT_OK)
				return COG_BADFLUSH;
		}
		*(devInUse->ftStatus) = FT_Read(*(devInUse->ftHandle), RxBuffer, r, &BytesReceived);
			if(*(devInUse->ftStatus) != FT_OK)
				return COG_BADFLUSH;
	}

	return 0;
	// for (i = 0; i < MAX_BYTES_PREFLUSH; i++)
	// {
	// 	readbyte(deviceID, &temp);
	// }
}

/**
gets the configuration settings of a connected Cognionics device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if configuration successfully retrieved, else returns an error flag
*/
int COG_GetConfig(COGDevice *deviceID)
{
	//read_counter is used for both during finding 0xFFFFFFFF and finding 0x1234567890
	int read_counter;
	int wordPart_counter;

	bool config_error;
	bool end;

	int configSize;

	int i;
	int j;
	int location; 

	BYTE config_cmd = 0x14;

	BYTE end_test[4];

	BYTE temp = 0x00;

	BYTE wordParts[4];

	int tempcounter = 0;
	int channelNameLength = 0;

	if (!findStart(deviceID, config_cmd))
	{

		if(!findStart(deviceID, config_cmd))
		{
			if(!findStart(deviceID, config_cmd))
			{
				printf(" COG_GetConfig failure\n");
				return COG_CONFIGFINDSTARTFAIL;
			}
		}
	}

	printf("Reading config data...");
	//retrieve actual config data
	//even though configData is stored in words, data will be read a byte at a time
	wordPart_counter = 0;
	read_counter = 0;
	configSize = 0;
	end = false;
	while (!end)
	{
		read_counter++;
		if (read_counter > 15000)
			return COG_TIMEOUTERROR;
		
		readbyte(deviceID, &temp);
		wordParts[wordPart_counter++] = temp;
		
		if (wordPart_counter == 4)
		{
			configData[configSize++] = ((DWORD)wordParts[0] << 24 | (DWORD)wordParts[1] << 16
				| (DWORD)wordParts[2] << 8 | (DWORD)wordParts[3]);
			wordPart_counter = 0;
		}

		//end_test holds the last four bytes
		//if array holds 0x499602d2, end
		end_test[3] = end_test[2];
		end_test[2] = end_test[1];
		end_test[1] = end_test[0];
		end_test[0] = temp;
		end = testend(end_test);
	}
	printf("Data read.\n");

	configInfo** prelim = (configInfo**)deviceID;
	configInfo* devInUse = *prelim;

	int connectionIndex = devInUse->index;

	config_error = false;
	//store data into the configInfo struct
	//storing RESERVED settings
	for (i = 0; i < 100; i++)
	{
		devInUse->RESERVED[i] = configData[i];
		if (configData[i] < 0) config_error = true;
	}

	//storing channel settings
	for (i = 0; i < 100; i++)
	{
		devInUse->CHANNEL_SETTINGS[i] = configData[100 + i];
		if (configData[100 + i] < 0) config_error = true;
	}

	//storing peripheral settings
	for (i = 0; i < 100; i++)
	{
		devInUse->PERIPHERAL_SETTINGS[i] = configData[200 + i];
		if (configData[200 + i] < 0) config_error = true;
	}

	//storing gain values
	for (i = 0; i < 100; i++)
	{
		devInUse->GAIN_VALUES[i] = configData[500 + i];
		if (configData[500 + i] < 0) config_error = true;
	}

	//storing read-only settings
	for (i = 0; i < 100; i++)
	{
		devInUse->READ_ONLY_SETTINGS[i] = configData[600 + i];
		if (configData[600 + i] < 0) config_error = true;
	}

	//storing device name
	for (i = 0; i < 100; i++)
	{
		devInUse->DEVICE_NAME[i] = (char)configData[700 + i];
		if (configData[700 + i] < 0) config_error = true;
	}

	//storing hardware and software version numbers
	for (i = 0; i < 100; i++)
	{
		devInUse->HARDWARE_AND_SOFTWARE_VERSION_NUMBERS[i] = configData[800 + i];
		if (configData[800 + i] < 0) config_error = true;
	}

	//storing serial numbers
	for (i = 0; i < 100; i++)
	{
		devInUse->SERIAL_NUMBER[i] = configData[900 + i];
		if (configData[900 + i] < 0) config_error = true;
	}
	
	//storing channel mapping
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		devInUse->ch_map[i] = configData[1000 + i];
		if (configData[1000 + i] < 0) config_error = true;
	}

	//storing channel mask
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		devInUse->ch_mask[i] = configData[1512 + i];
		if (configData[1512 + i] < 0) config_error = true;
	}

	//storing std positions
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		location = configData[2024 + i];
		if (configData[2024 + i] < 0) config_error = true;	
		devInUse->ch_id[i] = location;
		//detect stop flag
		if (location == 888888)
		{
			strcpy(devInUse->std_pos_map[i], stopArray);
			break;
		}
		channelNameLength = strlen(ChannelLabels[location]);
		strcpy(devInUse->std_pos_map[i], ChannelLabels[location]);
		for (j = channelNameLength; j < MAX_CHANNEL_NAME_SIZE; j++)
			devInUse->std_pos_map[i][j] = ' ';
		
	}

	devInUse->fs2Filter.channels = devInUse->READ_ONLY_SETTINGS[0];
	devInUse->fs4Filter.channels = devInUse->READ_ONLY_SETTINGS[0];

	int numChannels = devInUse->READ_ONLY_SETTINGS[0] + devInUse->READ_ONLY_SETTINGS[2] + devInUse->READ_ONLY_SETTINGS[4] + 2;
	devInUse->impMAGLPF.channels = numChannels;
	devInUse->impPHALPF.channels = numChannels;
	devInUse->offFilter.channels = numChannels;

	//init longFIR structs
	double SampleRate;
	int SampleRateOptionSetting = 6 + devInUse->CHANNEL_SETTINGS[6] - devInUse->CHANNEL_SETTINGS[7];
	switch(SampleRateOptionSetting)
	{
		case 0:
			SampleRate = 250.0;
			break;
		case 1:
			SampleRate = 500.0;
			break;
		case 2:
			SampleRate = 1000.0;
			break;	
		case 3:
			SampleRate = 2000.0;
			break;
		case 4:
			SampleRate = 4000.0;
			break;
		case 5:
			SampleRate = 8000.0;
			break;
		case 6:
			SampleRate = 16000.0;
			break;
		case 7:
			SampleRate = 32000.0;
			break;
	}
    int t_impFilLen = (int)(SampleRate * impFilterTime);
    double *t_sine = (double*)malloc(sizeof(double)*t_impFilLen);
    double *t_cosine = (double*)malloc(sizeof(double)*t_impFilLen);

    double t_sineSign = 1.0;
    double t_cosineSign = 1.0;
	for (int c = 0; c < t_impFilLen; c++)
	{
		if (c % 2 == 1)
		{
			t_sine[c] = t_sineSign / (t_impFilLen / 2);
			t_sineSign = t_sineSign * -1.0;

			t_cosine[c] = 0;
		}
		else
		{
			t_cosine[c] = t_cosineSign / (t_impFilLen / 2);
			t_cosineSign = t_cosineSign * -1.0;

			t_sine[c] = 0;
		}
	}
	devInUse->impCosine.fircoeff = t_cosine;
	devInUse->impSine.fircoeff = t_sine;

	devInUse->impCosine.numEEG = devInUse->READ_ONLY_SETTINGS[0];
	devInUse->impSine.numEEG = devInUse->READ_ONLY_SETTINGS[0];

	devInUse->impCosine.initialized = true;
	devInUse->impSine.initialized = true;

	device[connectionIndex] = devInUse;
	*deviceID = device[connectionIndex];

	if(config_error) return COG_CONFIGDATAERROR;

	return 0;
}

/**
private ConfigGet, not meant to be used in the full API; used in COGConfigSet() after config settings are sent

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if configuration successfully retrieved, -1 if otherwise
*/
int getconfig(COGDevice *deviceID)
{
	//read_counter is used for both during finding 0xFFFFFFFF and finding 0x1234567890
	int read_counter;
	int wordPart_counter;

	bool config_error;
	bool end;

	int configSize;
	int i;
	int j;
	int location;
	

	BYTE config_cmd = 0x14;

	BYTE end_test[4];

	BYTE temp = 0x00;

	BYTE wordParts[4];

	int tempcounter = 0;

	int channelNameLength = 0;

	//retrieve actual config data
	//even though configData is stored in words, data will be read a byte at a time
	wordPart_counter = 0;
	configSize = 0;
	read_counter = 0;
	end = false;

	configInfo** prelim = (configInfo**)deviceID;
	configInfo* devInUse = *prelim;

	int connectionIndex = devInUse->index;

	while (!end)
	{
		read_counter++;

		readbyte(deviceID, &temp);
		wordParts[wordPart_counter++] = temp;
		if (wordPart_counter == 4)
		{
			configData[configSize++] = ((DWORD)wordParts[0] << 24 | (DWORD)wordParts[1] << 16
				| (DWORD)wordParts[2] << 8 | (DWORD)wordParts[3]);
			wordPart_counter = 0;
		}

		//end_test holds the last four bytes
		//if array holds 0x499602d2, end
		end_test[3] = end_test[2];
		end_test[2] = end_test[1];
		end_test[1] = end_test[0];
		end_test[0] = temp;
		end = testend(end_test);
	}

	config_error = false;
	//store data into the configInfo struct
	//storing RESERVED settings
	for (i = 0; i < 100; i++)
	{
		devInUse->RESERVED[i] = configData[i];
		if (configData[i] < 0) config_error = true;
	}

	//storing channel settings
	for (i = 0; i < 100; i++)
	{
		devInUse->CHANNEL_SETTINGS[i] = configData[100 + i];
		if (configData[100 + i] < 0) config_error = true;
	}

	//storing peripheral settings
	for (i = 0; i < 100; i++)
	{
		devInUse->PERIPHERAL_SETTINGS[i] = configData[200 + i];
		if (configData[200 + i] < 0) config_error = true;
	}

	//storing gain values
	for (i = 0; i < 100; i++)
	{
		devInUse->GAIN_VALUES[i] = configData[500 + i];
		if (configData[500 + i] < 0) config_error = true;
	}

	//storing read-only settings
	for (i = 0; i < 100; i++)
	{
		devInUse->READ_ONLY_SETTINGS[i] = configData[600 + i];
		if (configData[600 + i] < 0) config_error = true;
	}

	//storing device name
	for (i = 0; i < 100; i++)
	{
		devInUse->DEVICE_NAME[i] = (char)configData[700 + i];
		if (configData[700 + i] < 0) config_error = true;
	}

	//storing hardware and software version numbers
	for (i = 0; i < 100; i++)
	{
		devInUse->HARDWARE_AND_SOFTWARE_VERSION_NUMBERS[i] = configData[800 + i];
		if (configData[800 + i] < 0) config_error = true;
	}

	//storing serial numbers
	for (i = 0; i < 100; i++)
	{
		devInUse->SERIAL_NUMBER[i] = configData[900 + i];
		if (configData[900 + i] < 0) config_error = true;
	}
	
	//storing channel mapping
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		devInUse->ch_map[i] = configData[1000 + i];
		if (configData[1000 + i] < 0) config_error = true;
	}

	//storing channel mask
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		devInUse->ch_mask[i] = configData[1512 + i];
		if (configData[1512 + i] < 0) config_error = true;
	}

	//storing std positions
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		location = configData[2024 + i];
		if (configData[2024 + i] < 0) config_error = true;	
		devInUse->ch_id[i] = location;
		//detect stop flag
		if (location == 888888)
		{
			strcpy(devInUse->std_pos_map[i], stopArray);
			break;
		}
		channelNameLength = strlen(ChannelLabels[location]);
		strcpy(devInUse->std_pos_map[i], ChannelLabels[location]);
		for (j = channelNameLength; j < MAX_CHANNEL_NAME_SIZE; j++)
			devInUse->std_pos_map[i][j] = ' ';
		
	}

	devInUse->fs2Filter.channels = devInUse->READ_ONLY_SETTINGS[0];
	devInUse->fs4Filter.channels = devInUse->READ_ONLY_SETTINGS[0];

	int numChannels = devInUse->READ_ONLY_SETTINGS[0] + devInUse->READ_ONLY_SETTINGS[2] + devInUse->READ_ONLY_SETTINGS[4] + 2;
	devInUse->impMAGLPF.channels = numChannels;
	devInUse->impPHALPF.channels = numChannels;
	devInUse->offFilter.channels = numChannels;

	//init longFIR structs
	double SampleRate;
	int SampleRateOptionSetting = 6 + devInUse->CHANNEL_SETTINGS[6] - devInUse->CHANNEL_SETTINGS[7];
	switch(SampleRateOptionSetting)
	{
		case 0:
			SampleRate = 250.0;
			break;
		case 1:
			SampleRate = 500.0;
			break;
		case 2:
			SampleRate = 1000.0;
			break;	
		case 3:
			SampleRate = 2000.0;
			break;
		case 4:
			SampleRate = 4000.0;
			break;
		case 5:
			SampleRate = 8000.0;
			break;
		case 6:
			SampleRate = 16000.0;
			break;
		case 7:
			SampleRate = 32000.0;
			break;
	}
    int t_impFilLen = (int)(SampleRate * impFilterTime);
    double *t_sine = (double*)malloc(sizeof(double)*t_impFilLen);
    double *t_cosine = (double*)malloc(sizeof(double)*t_impFilLen);

    double t_sineSign = 1.0;
    double t_cosineSign = 1.0;
	for (int c = 0; c < t_impFilLen; c++)
	{
		if (c % 2 == 1)
		{
			t_sine[c] = t_sineSign / (t_impFilLen / 2);
			t_sineSign = t_sineSign * -1.0;

			t_cosine[c] = 0;
		}
		else
		{
			t_cosine[c] = t_cosineSign / (t_impFilLen / 2);
			t_cosineSign = t_cosineSign * -1.0;

			t_sine[c] = 0;
		}
	}
	devInUse->impCosine.fircoeff = t_cosine;
	devInUse->impSine.fircoeff = t_sine;

	devInUse->impCosine.numEEG = devInUse->READ_ONLY_SETTINGS[0];
	devInUse->impSine.numEEG = devInUse->READ_ONLY_SETTINGS[0];

	devInUse->impCosine.initialized = true;
	devInUse->impSine.initialized = true;

	device[connectionIndex] = devInUse;
	*deviceID = device[connectionIndex];

	if(config_error) return COG_CONFIGDATAERROR;

	return 0;
}

/**
updates a connected Cognionics device with local settings

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if update successful, else returns an error flag
*/
int COG_SetConfig(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	BYTE config_cmd = 0x13;

	BYTE temp = 0x00;

	int tempcounter = 0;
	int i;

	//preFlush(deviceID);

	 *(devInUse->ftStatus) = FT_Purge(*(devInUse->ftHandle), FT_PURGE_RX | FT_PURGE_TX);
	 if(*(devInUse->ftStatus) != FT_OK)
	 	return COG_BADFLUSH;

	if(!findStart(deviceID, config_cmd)) 
		if(!findStart(deviceID, config_cmd)) 
			return -1;

	cout << ">>> sending new config <<<" << endl;

	//update Channel Settings
	for(i = 0; i < 9; i++)
	{
		writeword(deviceID, 1);
		writeword(deviceID, i);
		writeword(deviceID, devInUse->CHANNEL_SETTINGS[i]);
	}

	//update Peripheral Settings
	for(i = 0; i < 4; i++)
	{
		writeword(deviceID, 2);
		writeword(deviceID, i);
		writeword(deviceID, devInUse->PERIPHERAL_SETTINGS[i]);
	}

	//chMask
	for (i = 0; i < devInUse->RESERVED[4]; i++)
	{
		writeword(deviceID, 11);
		writeword(deviceID, i);
		writeword(deviceID, devInUse->ch_mask[i]);
	}

	FT_GetStatus(*(devInUse->ftHandle), &RxBytes, &TxBytes, &EventDWord);
	cout << "Rx: " << RxBytes << endl;

	 *(devInUse->ftStatus) = FT_Purge(*(devInUse->ftHandle), FT_PURGE_RX);
	 if(*(devInUse->ftStatus) != FT_OK)
	 	return COG_BADFLUSH;

	writeword(deviceID, 0);
	
	writeword(deviceID, 0x499602D3);

	readbyte(deviceID, &temp);

	BYTE returnval = temp;

	cout << "Returnval: " << (unsigned int)returnval << endl;

	if (returnval == 0x7E)
	{
		return getconfig(deviceID);
	}
	else
	{
		configData[0] = returnval;
		readbyte(deviceID, &temp);
		configData[1] = temp;

		if(temp == 0x45) return COG_NONSTARTRECEIVED;
		if(temp == 0x54) return COG_TIMEOUTERROR;
		if(temp == 0x53) return COG_SHORTDATAERROR;
		if(temp == 0x4C) return COG_LONGDATAERROR;
		if(temp == 0x43) return COG_CRCCHECKFAILURE;
		else return temp;//COG_CONFIGSETERROR;
	}
}

/**
retrieves the statuses of the available channels Cognionics device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return a struct containing the number of available channels, the maximum size of a channel label,
the available channels' labels, and their respective statuses
*/
COGChannels COG_GetChannelStatuses(COGDevice *deviceID)
{
	int current;
	int i;
	int j;
	int currentStatus;

	COGChannels dev_channels;
	configInfo* devInUse = (configInfo*)*deviceID;
	current = 0;

	dev_channels.channelLabels;

	for (i = 0; i < MAX_CHANNEL_NUM; i++)
	{
		if (strcmp(devInUse->std_pos_map[i], stopArray) == 0)
		{
			strcpy(dev_channels.channelLabels[i], stopArray);
			break;
		}
		currentStatus = devInUse->ch_mask[i];
		if(currentStatus != 0)
		{
			dev_channels.channelStatuses[current] = (currentStatus == 1) ? 1:0;
			for (j = 0; j < MAX_CHANNEL_NAME_SIZE; j++)
			{
				dev_channels.channelLabels[current][j] = devInUse->std_pos_map[i][j];
			}
			current++;
		}
		else
		{
			for (j = 0; j < MAX_CHANNEL_NAME_SIZE; j++)
			{
				dev_channels.channelLabels[current][j] = ' ';
			}
		}
	}

	dev_channels.num_channels = current;
	return dev_channels;
}

#pragma endregion Config_functions

#pragma region SD_functions

/**
converts a char buffer to a word buffer

@param charBuffer the array of characters to be converted
@param wordBuffer the array of words to store the results
*/
int chartoword(char* charBuffer, DWORD* wordBuffer, int limit)
{
	int i;
	int charBuff_length;

	for (i = 0; i < limit; i++)
	{
		if (charBuffer[i] < 32 || charBuffer[i] > 255)
		{
			break;
		}
	}
	charBuff_length = i;

	for (i = 0; i < charBuff_length; i++)
	{
		wordBuffer[i] = (unsigned int)charBuffer[i];
	}
	return charBuff_length;
}

/**
controls writing to SD card on DAQ headset

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param SD_MODE the SD card mode - see documentation for modes
@param filename the name of the file that the data will be stored in
@param day, hour, min, sec the 
@return 0 if successful, else returns an error flag
*/
int COG_SDStartStop(COGDevice *deviceID, MODE SD_MODE, char* filename, int day, int month, int year, int hour, int min, int sec, char* subject, char* description)
{
	int i;

	configInfo* devInUse = (configInfo*)*deviceID;
	bool exists;
	bool sd_start;
	DWORD tot_sect;
	DWORD fre_sect;

	//buffer to store filename for sending
	DWORD temp_wordbuffer_filename[512];
	//buffer to store subject for sending
	DWORD temp_wordbuffer_subject[512];
	//buffer to store description for sending
	DWORD temp_wordbuffer_description[512];
	
	//the size of the filename
	int size_filename;
	//the size of the subject
	int size_subject;
	//the size of the description
	int size_description;

	BYTE config_cmd = 0x30;

	BYTE end_status;

	size_filename = chartoword(filename, temp_wordbuffer_filename, MAX_FILENAME_SIZE);
	size_subject = chartoword(subject, temp_wordbuffer_subject, MAX_SUBJECT_SIZE);
	size_description = chartoword(description, temp_wordbuffer_description, MAX_DESCRIP_SIZE);

	//sd query
	
	if (SD_MODE != COG_DUAL_SDBT && SD_MODE != COG_SD_ONLY)
	{
		return COG_SDMODEFAIL;
	}
	exists = findStart(deviceID, config_cmd);

	if (!exists)
	{
		return COG_NOCARDDETECTED;
	}
	tot_sect = readword(deviceID);

	fre_sect = readword(deviceID);

	if (tot_sect == 0x00000000 || fre_sect == 0xFFFFFFFF)
	{
		return COG_NOCARDDETECTED;
	}
	if (fre_sect > tot_sect)
		return COG_INVALIDMEM;

	//sd_start
	sd_start = findStart(deviceID, SD_MODE);
	if (!sd_start) return COG_SDMODEFAIL;

	//write filename
	for (i = 0; i < size_filename; i++)
	{
		writeword(deviceID, temp_wordbuffer_filename[i]);
	}
	writeword(deviceID, 0);

	//write time
	writeword(deviceID, year);
	writeword(deviceID, month);
	writeword(deviceID, day);
	writeword(deviceID, hour);
	writeword(deviceID, min);
	writeword(deviceID, sec);

	//write subject
	for (i = 0; i < size_subject; i++)
	{
		writeword(deviceID, temp_wordbuffer_subject[i]);
	}
	writeword(deviceID, 0);

	//write description
	for (i = 0; i < size_description; i++)
	{
		writeword(deviceID, temp_wordbuffer_description[i]);
	}
	writeword(deviceID, 0);
	writeword(deviceID, 1234567890);
	//end flag
	readbyte(deviceID, &end_status);
	if (end_status == 0x57) //0x57 is all good
		return 0;
	else if (end_status == 0x4E) //0x4E is SD failed to start
		return COG_SDSTARTFAIL; //0xFFFFFFFE,
	else if (end_status == 0x54)
		return COG_TIMEOUTERROR; 
	else
		return end_status;
}

#pragma endregion SD_functions

#pragma region Helper_functions

/**
returns the number of enabled channels

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the number of enabled channels on the device
*/
int COG_GetNumEnabledChannels(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	int numEEG = devInUse->READ_ONLY_SETTINGS[0];
	int numEXT = devInUse->READ_ONLY_SETTINGS[2];
	int numACC = devInUse->READ_ONLY_SETTINGS[4];

	return numEEG + numEXT + numACC;
}

/**
retrieves the sample rate of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the sampling rate of the Cognionics device
*/
int COG_GetSampleRate(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	return (6 + devInUse->CHANNEL_SETTINGS[6] - devInUse->CHANNEL_SETTINGS[7]);
}

/**
retrieves the channel gain of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the channel gain of the Cognionics device
*/
int COG_GetGain(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	int gainGetRemap[7] = 
	{
		4,
		0,
		1,
		2,
		3,
		5,
		6
	};

	return gainGetRemap[devInUse->CHANNEL_SETTINGS[5]];
}

/**
retrieves the impedance check mode of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the impedance check mode of the Cognionics device
*/
int COG_GetImpedanceCheckMode(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	return devInUse->CHANNEL_SETTINGS[8];
}

/**
retrieves the power on impedance check enable of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the power on impedance check enable of the Cognionics device
*/
int COG_GetPowerOnImpedanceCheckEnable(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	return devInUse->PERIPHERAL_SETTINGS[2];
}

/**
retrieves the test mode enable of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the test mode enable of the Cognionics device
*/
int COG_GetTestModeEnable(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	if(devInUse->CHANNEL_SETTINGS[4] == 0)
	{
		return 0;
	}
	else if(devInUse->CHANNEL_SETTINGS[4] == 5)
	{
		return 1;
	}
	else
	{
		return COG_INVALIDVALUE;
	}
}

/**
retrieves the number of extension channels connected to the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the number of extension channels connected
*/
int COG_GetExtensionChannelsNumber(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	return devInUse->READ_ONLY_SETTINGS[2];
}

/**
checks if device's accelerometer is enabled

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 1 if accelerometer is enabled, 0 otherwise
*/
int COG_GetAccelerometerEnable(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	int x = -1;
	int y = -1;
	int z = -1;
	int i = 0;

	while((x == -1 || y == -1 || z == -1) && (i < MAX_CHANNEL_NUM))
	{
		if(devInUse->ch_id[i] == 8001)
		{
			x = i;
		}
		else if(devInUse->ch_id[i] == 8002)
		{
			y = i;
		}
		else if(devInUse->ch_id[i] == 8003)
		{
			z = i;
		}

		i++;
	}

	if(devInUse->ch_mask[x] == 1 && devInUse->ch_mask[y] == 1 && devInUse->ch_mask[z] == 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
checks if device's gyroscope is enabled

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 1 if gyroscope is enabled, 0 otherwise
*/
int COG_GetGyroscopeEnable(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	int x = -1;
	int y = -1;
	int z = -1;
	int i = 0;

	while((x == -1 || y == -1 || z == -1) && (i < MAX_CHANNEL_NUM))
	{
		if(devInUse->ch_id[i] == 8101)
		{
			x = i;
		}
		else if(devInUse->ch_id[i] == 8102)
		{
			y = i;
		}
		else if(devInUse->ch_id[i] == 8103)
		{
			z = i;
		}

		i++;
	}

	if(devInUse->ch_mask[x] == 1 && devInUse->ch_mask[y] == 1 && devInUse->ch_mask[z] == 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
retrieves the status of the device's wireless trigger

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the status of the device's wireless trigger
*/
int COG_GetWirelessTriggerEnable(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	return devInUse->PERIPHERAL_SETTINGS[1];
}

/**
retrieves the data stream mode of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 for High Speed data, 1 for legacy data
*/
int COG_GetDataStreamMode(COGDevice *deviceID)
{
	configInfo* devInUse = (configInfo*)*deviceID;

	return devInUse->PERIPHERAL_SETTINGS[0];
}

/**
sets the sample rate of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if setting is changed, COG_INVALIDINPUT if input is invalid
*/
int COG_SetSampleRate(COGDevice *deviceID, int setting)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	
	if(setting < 0 || setting > 7)
	{
		return COG_INVALIDINPUT;
	}

	if(setting == 0)
	{
		devInUse->CHANNEL_SETTINGS[6] = 0;
		devInUse->CHANNEL_SETTINGS[7] = 6;
	}
	else
	{
		devInUse->CHANNEL_SETTINGS[6] = 1;
		devInUse->CHANNEL_SETTINGS[7] = 6 + 1 - setting;
	}

	return 0;
}

/**
sets the device's gain

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if setting is changed, COG_INVALIDINPUT if input is invalid
*/
int COG_SetGain(COGDevice* deviceID, int setting)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	
	int gainGetRemap[7] = 
	{
		1,
		2,
		3,
		4,
		0,
		5,
		6
	};

	if(setting < 0 || setting > 6)
	{
		return COG_INVALIDINPUT;
	}

	devInUse->CHANNEL_SETTINGS[5] = gainGetRemap[setting];

	return 0;
}

/**
sets the device's impedance check mode

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if setting is changed, COG_INVALIDINPUT-3 if input is invalid
*/
int COG_SetImpedanceCheckMode(COGDevice *deviceID, int setting)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	
	if(setting < 0 || setting > 1)
	{
		return COG_INVALIDINPUT;
	}

	devInUse->CHANNEL_SETTINGS[8] = setting;

	return 0;
}

/**
sets the device's Power On Impedance Check Enable

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if setting is changed, COG_INVALIDINPUT if input is invalid
*/
int COG_SetPowerOnImpedanceCheckEnable(COGDevice *deviceID, int setting)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	
	if(setting < 0 || setting > 1)
	{
		return COG_INVALIDINPUT;
	}

	devInUse->PERIPHERAL_SETTINGS[2] = setting;

	return 0;
}

/**
sets the device's test mode enable

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if setting is changed, COG_INVALIDINPUT if input is invalid
*/
int COG_SetTestModeEnable(COGDevice *deviceID, int setting)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	
	if(setting < 0 || setting > 1)
	{
		return COG_INVALIDINPUT;
	}

	if(setting == 1)
	{
		devInUse->CHANNEL_SETTINGS[4] = 5;
	}
	else
	{
		devInUse->CHANNEL_SETTINGS[4] = 0;
	}

	return 0;
}

/**
sets the number of enabled extension channels

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if setting is changed, COG_INVALIDINPUT if input is invalid
*/
int COG_SetExtensionChannelsNumber(COGDevice *deviceID, int setting)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	
	int i = 0;
	int extensionChannelLocation = -1;

	if(setting < 0 || setting > 32)
	{
		return COG_INVALIDINPUT;
	}

	while((extensionChannelLocation == -1) && (i < MAX_CHANNEL_NUM))
	{
		if(devInUse->ch_id[i] == 7001)
		{
			extensionChannelLocation = i;
		}

		i++;
	}

	for(i = 0; i < 32; i++)
	{
		if(devInUse->ch_mask[extensionChannelLocation + i] == 0)
		{
			return COG_INVALIDCHANNELVAL;
		}
	}

	for(i = 0; i < 32; i++)
	{
		if(i < setting)
		{
			devInUse->ch_mask[extensionChannelLocation + i] = 1;
		}
		else
		{
			devInUse->ch_mask[extensionChannelLocation + i] = 2;
		}
	}

	return 0;
}

/**
sets the device's accelerometer channel enable

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if setting is changed, COG_INVALIDINPUT if input is invalid, COG_INVALIDCHANNELVAL if array addreses are invalid
*/
int COG_SetAccelerometerEnable(COGDevice *deviceID, int setting)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	
	int x = -1;
	int y = -1;
	int z = -1;
	int i = 0;

	if(setting < 0 || setting > 1)
	{
		return COG_INVALIDINPUT;
	}

	while((x == -1 || y == -1 || z == -1) && (i < MAX_CHANNEL_NUM))
	{
		if(devInUse->ch_id[i] == 8001)
		{
			x = i;
		}
		else if(devInUse->ch_id[i] == 8002)
		{
			y = i;
		}
		else if(devInUse->ch_id[i] == 8003)
		{
			z = i;
		}

		i++;
	}

	if(devInUse->ch_mask[x] == 0 || devInUse->ch_mask[y] == 0 || devInUse->ch_mask[z] == 0)
	{
		return COG_INVALIDCHANNELVAL;
	}
	
	if(setting == 1)
	{
		devInUse->ch_mask[x] = 1;
		devInUse->ch_mask[y] = 1;
		devInUse->ch_mask[z] = 1;
	}
	else
	{
		devInUse->ch_mask[x] = 2;
		devInUse->ch_mask[y] = 2;
		devInUse->ch_mask[z] = 2;
	}

	return 0;
}

/**
sets the device's gyroscope enable

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if setting is changed, COG_INVALIDINPUT if input is invalid, COG_INVALIDCHANNELVAL if array addreses are invalid
*/
int COG_SetGyroscopeEnable(COGDevice *deviceID, int setting)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	
	int x = -1;
	int y = -1;
	int z = -1;
	int i = 0;

	if(setting < 0 || setting > 1)
	{
		return COG_INVALIDINPUT;
	}

	while((x == -1 || y == -1 || z == -1) && (i < MAX_CHANNEL_NUM))
	{
		if(devInUse->ch_id[i] == 8101)
		{
			x = i;
		}
		else if(devInUse->ch_id[i] == 8102)
		{
			y = i;
		}
		else if(devInUse->ch_id[i] == 8103)
		{
			z = i;
		}

		i++;
	}

	if(devInUse->ch_mask[x] == 0 || devInUse->ch_mask[y] == 0 || devInUse->ch_mask[z] == 0)
	{
		return COG_INVALIDCHANNELVAL;
	}
	
	if(setting == 1)
	{
		devInUse->ch_mask[x] = 1;
		devInUse->ch_mask[y] = 1;
		devInUse->ch_mask[z] = 1;
	}
	else
	{
		devInUse->ch_mask[x] = 2;
		devInUse->ch_mask[y] = 2;
		devInUse->ch_mask[z] = 2;
	}

	return 0;
}

/**
sets the device's wireless trigger enable

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if setting is changed, COG_INVALIDINPUT if input is invalid
*/
int COG_SetWirelessTriggerEnable(COGDevice *deviceID, int setting)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	
	if(setting < 0 || setting > 1)
	{
		return COG_INVALIDINPUT;
	}

	devInUse->PERIPHERAL_SETTINGS[1] = setting;

	return 0;
}

/**
sets the device's data stream mode

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if setting is changed, COG_INVALIDINPUT if input is invalid
*/
int COG_SetDataStreamMode(COGDevice *deviceID, int setting)
{
	configInfo* devInUse = (configInfo*)*deviceID;
	
	if(setting < 0 || setting > 1)
	{
		return COG_INVALIDINPUT;
	}

	devInUse->PERIPHERAL_SETTINGS[0] = setting;

	return 0;
}

#pragma endregion Helper_functions