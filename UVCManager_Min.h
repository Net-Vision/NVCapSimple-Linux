// Linux SVM UVC Manager
// To read/write i2c register or fpga register use this library
// 2015.06.01 / H.Yamada / NetVision Corp.
// 2017.06.14 Updated

#ifndef _UVCMANAGER_H_
#define _UVCMANAGER_H_

#include <unistd.h>

class CUVCManager{
public:
	CUVCManager();
	CUVCManager(int deviceID);
	~CUVCManager();

	// Get device name using deviceID
	static int GetDeviceName(int deviceID, char* name, int nameSize);
	// Get current device if device opened
	int GetDeviceName(char* name, int nameSize);

	// Open device using deviceID
	int OpenDevice(int deviceID);
	// Close device
	void CloseDevice();
	// Is device open?
	int IsOpen(){
		return (m_deviceID >= 0)? 1: 0;
	}

protected:
	// Primitive functions, usually used by inherited class,
	int QueryControl(struct v4l2_queryctrl* pQueryCtrl);
	int SetControl(unsigned int id, int value);
	int GetControl(unsigned int id, int* pValue);
	
private:
	int m_fd;
	int m_deviceID;
	
};


#endif

