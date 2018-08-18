#define DEFAULT_ALPHA_MAG   0x43FCCF4B
#define DEFAULT_ALPHA_PHASE 0x3F45EE7D


//DAQ identifier
//1 is full flagship DAQ
//0 is mini DAQ
#define IS_FLAGSHIP_DAQ 0


/*************************************************************************/
/*                                                                       */
/*                         FLAGSHIP DAQ SETTINGS                         */
/*                                                                       */
/*************************************************************************/
#if IS_FLAGSHIP_DAQ == 1
    /****************  GENERAL SETTINGS  ****************/

    //nRF power
    //0 disable nRF trigger
    //1 ***enable nRF trigger***
    #define DEFAULT_USE_NRF_TRIGGER 1

    //power on imp check setting
    //0 disable imp check startup
    //1 ***enable imp check on startup***
    #define DEFAULT_STARTUP_IMP_CHECK 1

    //flash storage setting
    //0 disables flash storage
    //1 ***allows the use of flash storage***
    #define USE_FLASH_STORAGE 1

    //USB multiport setting
    //0 hard-set port to BT wireless mode
    //1 hard-set port to AUX box mode
    //2 hard-set port to wired line mode
    //3 ***auto-detect mode***
    #define DEFAULT_MULTIPORT_MODE 3

    //wireless data transfer protocol
    //0 use manual update
    //1 ***use DMA protocol***
    #define USE_DMA_WIRELESS 1

    //wired data transfer protocol
    //0 use manual update
    //1 ***use DMA protocol***
    #define USE_DMA_WIRED 1

    //data format setting
    //0 use new compressed data transmission and trigger format
    //1 ***use legacy data transmission format***
    #define DEFAULT_USE_LEGACY_DATA_FORMAT 1


    /****************  CHANNEL SETTINGS  ****************/

    //ADS and components setup
    #define DEFAULT_ADS_COUNT       8
    #define DEFAULT_ADS_NCH         8
    #define DEFAULT_EXT_CHS         8
    #define DEFAULT_ACC_CHS         3

    //selective channel mode setting
    //only set DEFAULT_SELECTIVE_CHANNEL_MODE
    //if 8 ADS chips installed; we always use 1298s for if using 8 chips, so <DEFAULT_ADS_NCH> is assumed to be 8
    #if DEFAULT_ADS_COUNT == 8
        // 0 runs all 64 channels of the ADS
        // 1 is standard 32 channels down-select from 64 channels
        // 2 is standard 16 channels down-select from 64 channels
        //69 is custom one-time/debug down-select; this configuration may not be synchronized across the same code version
        #define DEFAULT_SELECTIVE_CHANNEL_MODE 0
    //if 4 ADS chips installed
    #elif DEFAULT_ADS_COUNT == 4
        //if 1298s installed
        #if DEFAULT_ADS_NCH == 8
            // 0 runs all 64 channels of the ADS for debugging; make sure to set <ADS_CHS_IN_USE_LIMIT> to at least 64
            // 1 runs all 32 channels of the ADS
            //69 is custom one-time/debug down-select; this configuration may not be synchronized across the same code version
            #define DEFAULT_SELECTIVE_CHANNEL_MODE 1
        //if 1294s installed
        #elif DEFAULT_ADS_NCH == 4
            // 0 runs all 64 channels of the ADS for debugging; make sure to set <ADS_CHS_IN_USE_LIMIT> to at least 64
            // 1 runs all 16 channels of the ADS
            //69 is custom one-time/debug down-select; this configuration may not be synchronized across the same code version
            #define DEFAULT_SELECTIVE_CHANNEL_MODE 1
        #endif /* DEFAULT_ADS_NCH */
    //if 1 ADS chip installed, mostly for debugging
    #elif DEFAULT_ADS_COUNT == 1
        // 0 runs all 64 channels of the ADS for debugging; make sure to set <ADS_CHS_IN_USE_LIMIT> to at least 64
        //69 is custom one-time/debug down-select; this configuration may not be synchronized across the same code version
        #define DEFAULT_SELECTIVE_CHANNEL_MODE 0
    #endif /* DEFAULT_ADS_COUNT */

    //software limit on the maximum number of usable channels
    #define ADS_CHS_IN_USE_LIMIT 64

    //device id setting
    // 0 is raw mapping
    // 1 is newflex
    // 2 is wetcap
    // 3 is childsize 16-CH DAQ64
    // 4 is new 16-CH DAQ64
    //69 is custom one-time/debug mapping; this configuration may not be synchronized across the same code version
    #define DEFAULT_DEVICE_ID_CONSTANT 1

    //channel mode settings according to the ADS datasheet
    //0 ***normal electrode input***
    //1 input shorted
    //2 used in conjunction with RLD_MEAS bit for RLD measurements
    //3 MVDD for supply measurement
    //4 temperature sensor
    //5 ***test signal***
    //6 RLD_DRP
    //7 RLD_DRN
    #define DEFAULT_ADS_CHANNEL_MODE 0

    //channel gain settings according to the ADS datasheet
    //0 gain of 6
    //1 gain of 1
    //2 gain of 2
    //3 ***gain of 3***
    //4 gain of 4
    //5 gain of 8
    //6 gain of 12
    #define DEFAULT_ADS_CHANNEL_GAIN 3

    //ADS high resolution enable according to the ADS datasheet
    //0 low-power mode
    //1 ***high-resolution mode***
    #define DEFAULT_ADS_HIGH_RESOLUTION 1

    //ADS sampling rate according to the ADS datasheet
    //0 f_MOD/16    (19.2k SPS low-power, 38.4 SPS high-resolution)
    //1 f_MOD/32    (9.6k SPS low-power, 19.2k SPS high-resolution)
    //2 f_MOD/64    (4.8k SPS low-power, 9.6k SPS high-resolution)
    //3 f_MOD/128   (2.4k SPS low-power, 4.8k SPS high-resolution)
    //4 f_MOD/256   (1.2k SPS low-power, 2.4k SPS high-resolution)
    //5 f_MOD/512   (600 SPS low-power, 1.2k SPS high-resolution)
    //6 ***f_MOD/1024  (300 SPS low-power, 600 SPS high-resolution)***
    #define DEFAULT_ADS_SAMPLING_RATE 6

    //impedance check setting
    //0 use the internal ADS impedance check function
    //1 ***use the proprietary external impedance check hardware***
    #define DEFAULT_EXTERNAL_IMPEDANCE_CHECK 1


/*************************************************************************/
/*                                                                       */
/*                           MINI DAQ SETTINGS                           */
/*                                                                       */
/*************************************************************************/
#elif IS_FLAGSHIP_DAQ == 0
    /****************  GENERAL SETTINGS  ****************/

    //Quick Plus LED convention
    #define QUICK_PLUS_LED 1

    //BT transceiver module selection
    #define USE_AMPEDRF 0

    //Amped RF BT device name for mini DAQ
    #define AMPEDRF_BT_NAME "Cog Sleep 1611SBC"

    //nRF power
    //0 disable nRF trigger
    //1 ***enable nRF trigger***
    #define DEFAULT_USE_NRF_TRIGGER 1

    //power on imp check setting
    //0 disable imp check startup
    //1 ***enable imp check on startup***
    #define DEFAULT_STARTUP_IMP_CHECK 1

    //flash storage setting
    //0 disables flash storage
    //1 ***allows the use of flash storage***
    #define USE_FLASH_STORAGE 1

    //USB multiport setting
    //0 hard-set port to BT wireless mode
    //1 hard-set port to AUX box mode
    //2 hard-set port to wired line mode
    //3 ***auto-detect mode***
    #define DEFAULT_MULTIPORT_MODE 3

    //wireless data transfer protocol
    //0 use manual update
    //1 ***use DMA protocol***
    #define USE_DMA_WIRELESS 1

    //wired data transfer protocol
    //0 use manual update
    //1 ***use DMA protocol***
    #define USE_DMA_WIRED 1

    //data format setting
    //0 use new compressed data transmission and trigger format
    //1 ***use legacy data transmission format***
    #define DEFAULT_USE_LEGACY_DATA_FORMAT 1


    /****************  CHANNEL SETTINGS  ****************/

    //ADS and components setup
    #define DEFAULT_ADS_COUNT       4
    #define DEFAULT_ADS_NCH         8
    #define DEFAULT_EXT_CHS         8
    #define DEFAULT_ACC_CHS         3

    //selective channel mode setting
    //only set DEFAULT_SELECTIVE_CHANNEL_MODE
    // 0 runs all 64 channels of the ADS for debugging; make sure to set <ADS_CHS_IN_USE_LIMIT> to at least 64
    // 1 is 20 channels down-select from 24 channels, standard Q20 production
    // 2 is 21 channels down-select from 24 channels, demo Q20 with DRL channel
    // 3 is 32 channels, standard Q32 production
    // 4 is 32 channels, demo Q32 with DRL channel
    //69 is custom one-time/debug down-select; this configuration may not be synchronized across the same code version
    #define DEFAULT_SELECTIVE_CHANNEL_MODE 1

    //software limit on the maximum number of usable channels
    #define ADS_CHS_IN_USE_LIMIT 20

    //digitize DRL
    #define DRL_READ 0

    //device id setting
    // 0 is raw map and OG Q20 map
    // 1 is alan Q20 map to match raw OG Q20 map for neurofeedback
    // 2 is rayray Q32 map
    //69 is custom one-time/debug mapping; this configuration may not be synchronized across the same code version
    #define DEFAULT_DEVICE_ID_CONSTANT 1

    //channel mode settings according to the ADS datasheet
    //0 ***normal electrode input***
    //1 input shorted
    //2 used in conjunction with RLD_MEAS bit for RLD measurements
    //3 MVDD for supply measurement
    //4 temperature sensor
    //5 ***test signal***
    //6 RLD_DRP
    //7 RLD_DRN
    #define DEFAULT_ADS_CHANNEL_MODE 0

    //channel gain settings according to the ADS datasheet
    //0 gain of 6
    //1 gain of 1
    //2 gain of 2
    //3 ***gain of 3***
    //4 gain of 4
    //5 gain of 8
    //6 gain of 12
    #define DEFAULT_ADS_CHANNEL_GAIN 3

    //ADS high resolution enable according to the ADS datasheet
    //0 low-power mode
    //1 ***high-resolution mode***
    #define DEFAULT_ADS_HIGH_RESOLUTION 1

    //ADS sampling rate according to the ADS datasheet
    //0 f_MOD/16    (19.2k SPS low-power, 38.4 SPS high-resolution)
    //1 f_MOD/32    (9.6k SPS low-power, 19.2k SPS high-resolution)
    //2 f_MOD/64    (4.8k SPS low-power, 9.6k SPS high-resolution)
    //3 f_MOD/128   (2.4k SPS low-power, 4.8k SPS high-resolution)
    //4 f_MOD/256   (1.2k SPS low-power, 2.4k SPS high-resolution)
    //5 f_MOD/512   (600 SPS low-power, 1.2k SPS high-resolution)
    //6 ***f_MOD/1024  (300 SPS low-power, 600 SPS high-resolution)***
    #define DEFAULT_ADS_SAMPLING_RATE 6

    //impedance check setting
    //0 use the internal ADS impedance check function
    //1 ***use the proprietary external impedance check hardware***
    #define DEFAULT_EXTERNAL_IMPEDANCE_CHECK 1
#endif /* IS_FLAGSHIP_DAQ */
