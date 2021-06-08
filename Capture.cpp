
#include "main.h"
#include "Capture.h"

#include "UVCManager.h"
#include "FrameConverter.h"

//#include "UserFilter.h"


// When frame is captured call this function
float CCapture::CalcFPS(void){
	static struct timeval lastTime;
	static int frameCnt;
	static float lastFPS = -1;
	const int frameDiv = 32;
	if(lastTime.tv_usec == 0){
		// first time called
		gettimeofday(&lastTime, NULL);
	}else if(++frameCnt >= frameDiv){
		frameCnt = 0;
		struct timeval curTime;
		gettimeofday(&curTime, NULL);

		double curSec = curTime.tv_sec + 0.000001 * curTime.tv_usec;
		double lastSec = lastTime.tv_sec + 0.000001 * lastTime.tv_usec;
		// Calculate FPS
		if(curSec - lastSec > 0){
			lastFPS = frameDiv / (curSec - lastSec);
		}else{
//			lastFPS = -1;
		}

		lastTime = curTime;
	}
	return lastFPS;
}

int CCapture::GetCameraName(int deviceID, char* name, int nameSize)
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

FILE* CCapture::RecStart(CV4LCapture::sVideoFormat* pFormat, int camCnt, const char* fileNameBase)
{
	// Create filename (filename + date + time)
	time_t curTime = time(NULL);
	struct tm* pCurTime = localtime(&curTime);
	char fileName[512];
	sprintf(fileName, "%s_%02d%02d%02d_%02d%02d%02d.rec",
		fileNameBase,
		pCurTime->tm_year % 100, pCurTime->tm_mon+1, pCurTime->tm_mday,
		pCurTime->tm_hour, pCurTime->tm_min, pCurTime->tm_sec);
	printf(" Filename: %s\n", fileName);
	
	FILE* fp = fopen(fileName, "wb");
	if(fp){
		setvbuf(fp, NULL, _IOFBF, 512*1024);
		
		char idText[4] = {'v', '4', 'l', 'c'};
		// write header
		fwrite(idText, sizeof(char)*4, 1, fp); // identifier
		fwrite(&camCnt, sizeof(int), 1, fp); // camera count
		
		int i;
		for(i=0; i<camCnt; i++)
		{
			int width = pFormat[i].width;
			int height = pFormat[i].height;
			int bits = pFormat[i].bitPerPixel;
			float fps = 30.0f;
			int padding = 0;
			
			fwrite(pFormat[i].deviceName, 128, 1, fp);
			fwrite(&width, sizeof(int), 1, fp);
			fwrite(&height, sizeof(int), 1, fp);
			fwrite(&bits, sizeof(int), 1, fp);
			fwrite(&fps, sizeof(float), 1, fp);
			fwrite(pFormat[i].pixelFormat, 4, 1, fp); // frame format (fourcc)
			fwrite(&padding, sizeof(int), 1, fp);
		}

	}
	return fp;
}
int CCapture::RecWrite(FILE* fp, const uint8_t* pData, uint32_t dataSize)
{
	if(fp == NULL) return -1;
	uint32_t cnt = 0;
	do{
		cnt += fwrite(pData, 1, dataSize-cnt, fp);
	}while(cnt < dataSize);
	return 0;
}
int CCapture::RecEnd(FILE* fp)
{
	if(fp) fclose(fp);
	return 0;
}

