/* OP_Devices.h     Open Panzer Devices 		*/ 


#ifndef OP_DEVICES_H
#define OP_DEVICES_H

// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// OPEN PANZER DEVICES
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
	typedef unsigned char DEVICE;
	#define DEVICE_UNKNOWN      0
	#define DEVICE_TCB_MKI      1
	#define DEVICE_TCB_MKII     2
	#define DEVICE_TCB_DIY      3           		// DIY version of the TCB firmware, moves a few pins around for compatibility with stock Arduino MEGA boards
	#define DEVICE_SCOUT        4           		// Scout board versions R11 and later
	#define DEVICE_SCOUT_R10    5           		// Scout board versions R10 and earlier
	#define DEVICE_TEENSYSOUND  6
	#define DEVICE_ATMEGA328    7           
	#define DEVICE_TEENSY32     8           
	#define DEVICE_AT_MKI       9
	#define DEVICE_HECLO_SHIELD 10
	#define DEVICE_ATMEGA2560   11


#endif
