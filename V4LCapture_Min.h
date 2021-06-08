// Video for Linux Wrapper Class
// To see detail of Video for Linux API, see
// https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/v4l2.html
// [18/03/03] Updated for adjusting time-out and buffer size

#ifndef _V4L_CAPTURE_H_
#define _V4L_CAPTURE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>
#include <inttypes.h>

#include <linux/videodev2.h>


class CV4LCapture
{
public:
	struct sVideoFormat{
		unsigned char pixelFormat[5];
		int width;
		int height;
		char deviceName[128];
		int bitPerPixel;
	};
public:
	CV4LCapture();
	virtual ~CV4LCapture();

	int Init(int id, int width = 640, int height = 480);
	// timeout: time out duration, in second
	// queueSize: size of frame buffer
	// width, height: device resotuion to be opened, ignored in SVM-03U
	int Init(int id, 
		int width, int height,
		unsigned int timeout, unsigned int queueSize);

	// Capture 1 frame
	int CaptureFrame(unsigned char* buf, uint32_t bufSize, uint32_t* pSequence = NULL);
	int GetVideoFormat(sVideoFormat* pFmt);
	int GetCameraID(){
		return m_id;
	}
private:
	unsigned int m_timeout;
	int m_fd, m_id;
	int m_initialized;
	char m_deviceName[128];

	struct sBuffer{
		void* start;
		size_t length;
	};
	sBuffer* m_pBuffer;
	int m_bufferCnt;

	// local function
	void InitMmap(char* dev_name, int reqCount);
	void CloseDevice(void);
	void StartCapture(void);
	void StopCapture(void);
	int  ReadFrame(uint32_t count, uint8_t* outputBuf, uint32_t bufSize, uint32_t* pSequence);


	static void errno_exit(const char *s)
	{
		fprintf(stderr, "%s error %d, %s\n",s, errno, strerror(errno));  
		exit(EXIT_FAILURE);
	};

	static int xioctl(int fd, int request, void *arg)
	{
		int r;
		do{
			r = ioctl(fd, request, arg);
		}while((r == -1) && (errno == EINTR));
		return r;
	};
	int GetCameraName(int deviceID, char* name, int nameSize)
	{
		char buf[128];
		sprintf(buf, "/sys/class/video4linux/video%d/name", deviceID);
		FILE* fp = fopen(buf, "r");
		if(fp == NULL) return -1;

		char* temp = fgets(name, nameSize-1, fp);
		int l = strlen(name);
		name[l-1] = '\0';
		fclose(fp);
		return 0;
	}
};

// debug
//extern struct timeval consTime[16];

#endif

