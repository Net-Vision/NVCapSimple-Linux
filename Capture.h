// Recording and misc. function
// 2018.02.20 created

#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include "V4LCapture.h"
#include "FrameConverter.h"

// 
class CCapture
{
public:
	CCapture(){
	};
	~CCapture(){
	};

	float CalcFPS(void);
	int GetCameraName(int deviceID, char* name, int nameSize);
	
	FILE* RecStart(CV4LCapture::sVideoFormat* pFormat, int camCnt, const char* fileNameBase);
	int RecWrite(FILE* fp, const uint8_t* pData, uint32_t dataSize);
	int RecEnd(FILE* fp);
	// [18/03/06]
	int GetPixelFormat(CV4LCapture::sVideoFormat* pFormat){
		int pixelFormat = FC_FORMAT_UYVY; // default
		const char* pf = (const char*)pFormat->pixelFormat;
		if(strncasecmp(pf, "yuyv", 4) == 0) pixelFormat = FC_FORMAT_YUYV;
		else if(strncasecmp(pf, "grey", 4) == 0) pixelFormat = FC_FORMAT_Y8;
		else if(pf[0] == '\0') pixelFormat = FC_FORMAT_RGB24;
		return pixelFormat;
	}
};

#endif

