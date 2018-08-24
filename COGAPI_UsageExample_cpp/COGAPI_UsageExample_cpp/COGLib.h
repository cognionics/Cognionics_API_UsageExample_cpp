//------------------------------------------------------------------------------
// COGLib.h
// 
// Description:
// Header file for COGLib functions
// 
// @authors: William Bei and Junle Zhang
// 
// www.cognionics.net
// info@cognionics.com
//------------------------------------------------------------------------------

#pragma once  

#ifdef COGLIB_EXPORTS  
#define COGLIB_API __declspec(dllexport)   
#else  
#define COGLIB_API __declspec(dllimport)   
#endif  

#pragma once
//#include <string.h>
#ifndef BYTE
typedef unsigned char BYTE;
#endif // !BYTE
#ifndef DWORD
typedef unsigned long DWORD;
#endif // !DWORD

//device constants
#define MAX_CHANNEL_NUM			512
#define MAX_CHANNEL_NAME_SIZE	16

//SD Star stop constants
#define COG_SD_ONLY				0X31
#define COG_DUAL_SDBT			0X32

//Error flags
//general flags
#define COG_OK 0
#define COG_INVALIDINPUT -1
#define COG_INVALIDCHANNELVAL -2
#define COG_INVALIDVALUE -3
#define COG_TIMEOUTERROR -4
#define COG_WRITEFAIL -5
#define COG_BADFLUSH -6

//device query and connection error flags
#define COG_NODEVICESCONNECTED -25
#define COG_MAXDEVEXCEEDED -26
#define COG_CANNOTGETDEVINFO -27
#define COG_CONNECTIONFAIL -28
#define COG_DISCONNECTFAIL -29

//get config error flags
#define COG_CONFIGFINDSTARTFAIL -50
#define COG_CONFIGDATAERROR -51

//set config error flags
#define COG_BADINIT -75
#define COG_NONSTARTRECEIVED -76
#define COG_SHORTDATAERROR -77
#define COG_LONGDATAERROR -78
#define COG_CRCCHECKFAILURE -79
#define COG_CONFIGSETERROR -80

//SD start stop flags
#define COG_NOCARDDETECTED -100
#define COG_INVALIDMEM -101
#define COG_SDMODEFAIL -102
#define COG_SDSTARTFAIL -103

//general constants
const double PI =  3.1415926535897932384626433832795;

//COG device handle
typedef void *COGDevice;

//COG_SDStartStop modes
typedef int MODE;

//holds Channel Statuses
struct COGChannels
{
	//num channels
	int num_channels;

	//Max size of a channel label
	int labelSize = MAX_CHANNEL_NAME_SIZE;

	//std positions map array
	char **channelLabels; //[MAX_CHANNEL_NUM][MAX_CHANNEL_NAME_SIZE];

	//channel mask array
	int channelStatuses[MAX_CHANNEL_NUM];
};

/**
returns the current software library version as a string.

@return a string that contains the version number and date last edited
*/
extern "C" COGLIB_API const char* COG_GetVersionNumber();

/**
returns a list of all available Cognionics devices.

@param numDevices a pointer that holds the number of devices found
@param stringLength the max number of characters of for a device name
@return a list of all available Cognionics devices if successful, else returns an error flag
*/
extern "C" COGLIB_API char* COG_DeviceQuery(int *numDevices, int *stringLength);

/**
connects to a Cognionics device.

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param i the index of the device to connect to
@return 0 if successfully connected, COG_CONNECTIONFAIL if otherwise
*/
extern "C" COGLIB_API int COG_Connect(COGDevice *deviceID, int i);

/**
disconnects from a Cognionics device.

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if successfully disconnected, COG_DISCONNECTFAIL if otherwise
*/
extern "C" COGLIB_API int COG_Disconnect(COGDevice *deviceID);

/**
Sets the impedance check of a Cognionics device to on

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
*/
extern "C" COGLIB_API int COG_SetImpedanceOn(COGDevice *deviceID);

/**
Sets the impedance check of a Cognionics device to off

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
*/
extern "C" COGLIB_API int COG_SetImpedanceOff(COGDevice *deviceID);

/**
reads a byte from a Cognionics device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param temp a pointer that stores the byte that was read from the device
@return true if byte read is 0xFF, flase if otherwise
*/
extern "C" COGLIB_API bool readbyte(COGDevice *deviceID, BYTE* temp);

/**
retrieves a specified number of data samples from a Cognionics device; this method is compatible with all devices and can read compressed data

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param numSample the number of samples to read from the device
@param channelData a pointer that stores the data read from the device
@return 0 if successful, else returns an error flag
*/
extern "C" COGLIB_API int COG_GetSample(COGDevice *deviceID, int numSample, double *channelData);
/**
gets the configuration settings of a connected Cognionics device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if configuration successfully retrieved, else returns an error flag
*/
extern "C" COGLIB_API int COG_GetConfig(COGDevice *deviceID);

/**
updates a connected Cognionics device with local settings

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if update successful, else returns an error flag
*/
extern "C" COGLIB_API int COG_SetConfig(COGDevice *deviceID);

/**
retrieves the statuses of the available channels Cognionics device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return a struct containing the number of available channels, the maximum size of a channel label,
the available channels' labels, and their respective statuses
*/
extern "C" COGLIB_API COGChannels COG_GetChannelStatuses(COGDevice *deviceID);

/**
controls writing to SD card on DAQ headset

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@param SD_MODE the SD card mode - see documentation for modes
@param filename the name of the file that the data will be stored in
@param day, hour, min, sec the
@return 0 if successful, else returns an error flag
*/
extern "C" COGLIB_API int COG_SDStartStop(COGDevice *deviceID, MODE SD_MODE, char *filename, int day, int month, int year, int hour, int min, int sec, char *subject, char *description);

/**
returns the number of enabled channels

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the number of enabled channels on the device
*/
extern "C" COGLIB_API int COG_GetNumEnabledChannels(COGDevice *deviceID);

/**
retrieves the sample rate of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the sampling rate of the Cognionics device
*/
extern "C" COGLIB_API int COG_GetSampleRate(COGDevice *deviceID);

/**
retrieves the channel gain of a Cognionics device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return device channel gain
*/
extern "C" COGLIB_API int COG_GetGain(COGDevice *deviceID);

/**
retrieves the impedance check mode of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the impedance check mode of the Cognionics device
*/
extern "C" COGLIB_API int COG_GetImpedanceCheckMode(COGDevice *deviceID);

/**
retrieves the power on impedance check enable of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the power on impedance check enable of the Cognionics device
*/
extern "C" COGLIB_API int COG_GetPowerOnImpedanceCheckEnable(COGDevice *deviceID);

/**
retrieves the test mode enable of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the test mode enable of the Cognionics device
*/
extern "C" COGLIB_API int COG_GetTestModeEnable(COGDevice *deviceID);

/**
retrieves the number of extension channels connected to the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the number of extension channels connected
*/
extern "C" COGLIB_API int COG_GetExtensionChannelsNumber(COGDevice *deviceID);

/**
checks if device's accelerometer is enabled

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 1 if accelerometer is enabled, 0 otherwise
*/
extern "C" COGLIB_API int COG_GetAccelerometerEnable(COGDevice *deviceID);

/**
checks if device's gyroscope is enabled

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 1 if gyroscope is enabled, 0 otherwise
*/
extern "C" COGLIB_API int COG_GetGyroscopeEnable(COGDevice *deviceID);

/**
retrieves the status of the device's wireless trigger

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return the status of the device's wireless trigger
*/
extern "C" COGLIB_API int COG_GetWirelessTriggerEnable(COGDevice *deviceID);

/**
retrieves the data stream mode of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 for High Speed data, 1 for legacy data
*/
extern "C" COGLIB_API int COG_GetDataStreamMode(COGDevice *deviceID);

/**
sets the sample rate of the device

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if local setting is changed, COG_INVALIDINPUT if input is invalid
*/
extern "C" COGLIB_API int COG_SetSampleRate(COGDevice *deviceID, int setting);

/**
sets the device's gain

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if local setting is changed, COG_INVALIDINPUT if input is invalid
*/
extern "C" COGLIB_API int COG_SetGain(COGDevice *deviceID, int setting);

/**
sets the device's impedance check mode

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if local setting is changed, COG_INVALIDINPUT-3 if input is invalid
*/
extern "C" COGLIB_API int COG_SetImpedanceCheckMode(COGDevice *deviceID, int setting);

/**
sets the device's Power On Impedance Check Enable

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if local setting is changed, COG_INVALIDINPUT if input is invalid
*/
extern "C" COGLIB_API int COG_SetPowerOnImpedanceCheckEnable(COGDevice *deviceID, int setting);

/**
sets the device's test mode enable

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if local setting is changed, COG_INVALIDINPUT if input is invalid
*/
extern "C" COGLIB_API int COG_SetTestModeEnable(COGDevice *deviceID, int setting);

/**
sets the number of enabled extension channels

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if local setting is changed, COG_INVALIDINPUT if input is invalid
*/
extern "C" COGLIB_API int COG_SetExtensionChannelsNumber(COGDevice *deviceID, int setting);

/**
sets the device's accelerometer channel enable

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if local setting is changed, COG_INVALIDINPUT if input is invalid, COG_INVALIDCHANNELVAL if channel addresses are invalid
*/
extern "C" COGLIB_API int COG_SetAccelerometerEnable(COGDevice *deviceID, int setting);

/**
sets the device's gyroscope enable

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if local setting is changed, COG_INVALIDINPUT if input is invalid, COG_INVALIDCHANNELVAL if channel addresses are invalid
*/
extern "C" COGLIB_API int COG_SetGyroscopeEnable(COGDevice *deviceID, int setting);

/**
sets the device's wireless trigger enable

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if local setting is changed, COG_INVALIDINPUT if input is invalid
*/
extern "C" COGLIB_API int COG_SetWirelessTriggerEnable(COGDevice *deviceID, int setting);

/**
sets the device's data stream mode

@param deviceID a pointer to a variable of type COGDevice where the device handle will be stored.
@return 0 if local setting is changed, COG_INVALIDINPUT if input is invalid
*/
extern "C" COGLIB_API int COG_SetDataStreamMode(COGDevice *deviceID, int setting);
