// OpenCV + v4l2 Camera Sample
// For Ubuntu 14.x or later
// Will not work on Windows environment

#define _FILE_OFFSET_BITS 64

#include "main.h"
#include "Capture.h"

// XWindow (Used to get display size)
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "V4LCapture_Min.h"
#include "CommandParser.h"

#include "UVCManager_Min.h"
#include "FrameConverter.h"



#define RGB_USE_MT 1

int g_queueSize = 10; // frame queue size
int g_timeout = 1; // CaptureFrame() timeout width

int ParseOption(int argc, char** argv, int* pOption)
{
	*pOption = 0;
	CCommandParser cmd;
	int options = 0;
	int i;
	
	if(argc >= 1) {
		if(cmd.Parse(argv, argc)) {
			return 1;
		}
		for(i=0; i<cmd.GetArgumentSize(); i++){
			const CCommandParser::sParseData* p = cmd.GetArgument(i);
			if(strcasecmp(p->argument, "debug") == 0){
				options |= DEBUG_MODE;

				// get info
				//printf("OpenCV will use %d threads\n", getNumThreads());

			}
			else if(strcasecmp(p->argument, "simple_window") == 0){
				options |= SIMPLE_WINDOW;
			}
			else if(strcasecmp(p->argument, "simple_gui") == 0){
				options |= SIMPLE_GUI;
			}
			else if(strcasecmp(p->argument, "half_resolution") == 0 || 
					strcasecmp(p->argument, "half_window") == 0){
				options |= HALF_WINDOW;
			}
			else if(strcasecmp(p->argument, "queue_size") == 0){
				printf("Queue size = %d\n", atoi(p->value));
				g_queueSize = atoi(p->value);
			}
			else if(strcasecmp(p->argument, "timeout") == 0){
				printf("Timeout = %d [sec]\n", atoi(p->value));
				g_timeout = atoi(p->value);
			}
			else if(strcasecmp(p->argument, "timeout_ms") == 0){
				int value = atoi(p->value);
				if(value > 999999) value = 999999;
				printf("Timeout = %d [msec]\n", value);
				g_timeout = 0x80000000UL | (value*1000);
			}
			else{
				printf("Invalid command option: [%s]\n", p->argument);
				return 1;
			}
		}
	}
	*pOption = options;
	
	return 0;
}
void CreateWindow(int camCnt, int options){
	int i;
	
	for(i=0; i<((options & SIMPLE_WINDOW)? 1: camCnt); i++){
		char buf[128];
		sprintf(buf, "Capture %d", i);
		namedWindow(buf, WINDOW_NORMAL 
			| ((options & SIMPLE_GUI)? WINDOW_GUI_NORMAL: WINDOW_GUI_EXPANDED)
//			| WINDOW_AUTOSIZE
//			| WINDOW_KEEPRATIO
//			| WINDOW_FREERATIO
			);
		
		// Get display size
		Display* pDisplay = XOpenDisplay(NULL);
		int width = 1280, height = 720;
		if(pDisplay){
			int screen = DefaultScreen(pDisplay);
			width = DisplayWidth(pDisplay, screen);
			height = DisplayHeight(pDisplay, screen);
			XCloseDisplay(pDisplay);
		}

		// Try moving window (This wouldn't work correctly on OpenCV 2.4.x)
		resizeWindow(buf, 640, 480);
		moveWindow(buf, (int)(1+ 640*((4-i)/2)), (int)(1+ 480*(i%2)) );
		if(i==1){
			moveWindow(buf, 1+width/2,1);
		}else if(i==3){
			moveWindow(buf, 1+width/2,1+height/2);
		}else if(i==0){
			moveWindow(buf, 1,1);
		}else if(i==2){
			moveWindow(buf, 1,1+height/2);
		}
		if(camCnt == 1) resizeWindow(buf, width*0.5, height*0.5);
		else resizeWindow(buf, width/2*0.9, height/2*0.9);
	}
}

#if 1
struct sUserThreadData;
struct sMainLoopParam{
	CV4LCapture** capSub;
	CFrameConverter** pFrameConverter;
	CV4LCapture::sVideoFormat* videoFormat;
	uint8_t* captureBufSame;
	uint8_t** captureBufInd;
	CCapture* pCapture;
	sUserThreadData* threadData;
	int camIdx;
};
int Mainloop(sMainLoopParam* pParam, int camCnt, int options)
{
	uint8_t* captureBufSame = pParam->captureBufSame;
	uint8_t** captureBufInd = pParam->captureBufInd;
	CV4LCapture** capSub = pParam->capSub;
	CFrameConverter** pFrameConverter = pParam->pFrameConverter;
	CV4LCapture::sVideoFormat* videoFormat = pParam->videoFormat;
	CCapture* pCapture = pParam->pCapture;
	sUserThreadData* threadData = pParam->threadData;
	int camIdx = pParam->camIdx;

	int activeFlags = SHOW_INFO_MSG;
	int i;

	float fps = 0;
	int frameCnt = 0;

	// Get Camera Device Name
	char cameraName[MAX_CAMERA][128];
	pCapture->GetCameraName(camIdx, cameraName[0], 128);

	for(i=1; i<camCnt; i++){
		if(pCapture->GetCameraName(i, cameraName[i], 128)){
			camCnt = i;
			break;
		}
	}
	uint32_t devFrameNum = 0;

	FILE* fpRecording = NULL;
	// Main loop
	for(;;){
		int timedOut = 0;
		
		// At first capture all frames into single buffer
		for(i=0; i<camCnt; i++){

			// Capture one frame
			if(options & SIMPLE_WINDOW){
				int width = videoFormat[0].width;
				int height = videoFormat[0].height;
				int bytePerPixel = (videoFormat[0].bitPerPixel + 7)/8;
				int offs = width * height*bytePerPixel * (i);
				if(capSub[i]->CaptureFrame(
					&captureBufSame[offs], 
					width*height*bytePerPixel,
					&devFrameNum)){
						timedOut |= 1<<i;
				}
			}else{
				int bytePerPixel = (videoFormat[i].bitPerPixel + 7)/8;
				if(capSub[i]->CaptureFrame(captureBufInd[i], 
					videoFormat[i].width * videoFormat[i].height * bytePerPixel,
					&devFrameNum)){
						timedOut |= 1<<i;
				}
			}

		}
		
		/*
		if((timedOut == 0) && (activeFlags & CAPTURE_ON))
		{
			if(options & SIMPLE_WINDOW){
				int width = videoFormat[0].width;
				int height = videoFormat[0].height;
				pCapture->RecWrite(fpRecording, captureBufSame, width*height*2*camCnt);
			}else{
				for(i=0; i<camCnt; i++){
					pCapture->RecWrite(fpRecording, captureBufInd[i], videoFormat[i].width * videoFormat[i].height * 2);
				}
			}
		}*/

		for(i=0; i<((options & SIMPLE_WINDOW)? 1: camCnt); i++)
		{
			int width = videoFormat[0].width;
			int height = videoFormat[0].height;
			int offs = width*height*2*i;
			char buf[128];
			char buf2[128];
			sprintf(buf, "Capture %d", i);
			Mat frame;
			
			// RAW -> Gray Conversion
#if defined(FORMAT_RAW10) || defined(FORMAT_RAW12) 

#if defined(FORMAT_RAW10) 
			int bw = 10;
#else
			int bw = 12;
#endif
			if(options & SIMPLE_WINDOW){
				pFrameConverter[i]->ConvertRAW2Mono(
					&captureBufSame[offs], bw, 1.0, 1.0);
			}else{
				pFrameConverter[i]->ConvertRAW2Mono(
					captureBufInd[i], bw, 1.0, 1.0);
			}
			
#endif
			
			if(options & SIMPLE_WINDOW){
				pFrameConverter[i]->ConvertFrame(camCnt, 
					&captureBufSame[offs], frame, options);
			}else{
				pFrameConverter[i]->ConvertFrame(camCnt, 
					captureBufInd[i], frame, options);
			}
			//UserFilter(frame, activeFlags, &threadData[i]);
			
			if((i >= 0) && (activeFlags & SHOW_INFO_MSG)){
				double fontSize = 1.0 * videoFormat[i].height/1080;
				putText(frame, cameraName[i], Point(20,40*fontSize), 
					FONT_HERSHEY_DUPLEX, fontSize, Scalar(255,255,255));	
				if(activeFlags & CAPTURE_ON){
					putText(frame, "Recording", Point(20,120*fontSize), 
						FONT_HERSHEY_DUPLEX, fontSize, Scalar(255,255,255));
				}

				if(1){
					if(timedOut & (1<<i)){
						strcpy(buf2, "Error: timed out");
						// Open Device Again
						int ret = capSub[i]->ReOpen();
						if(ret) printf("ReInit failed./n");
						else printf("Reinit ok\n");
					}else{
						sprintf(buf2, "%dx%d, %s, frame = %d, FPS = %.1f",
							videoFormat[i].width, videoFormat[i].height,
							videoFormat[i].pixelFormat, 
							devFrameNum, fps);
					}
					putText(frame, buf2, Point(20,80*fontSize), 
						FONT_HERSHEY_DUPLEX, fontSize, Scalar(255,255,255));
				}
			}
	
			imshow(buf, frame);
		
		}
		fps = pCapture->CalcFPS();
		int key = waitKey(1);
		//if(UserOptionHandler(key, &activeFlags, capSub, camCnt)){
		//}
		//else 
		if(key == 'i'){
			activeFlags ^= SHOW_INFO_MSG;
		}
		/*
		else if(key == 'r'){
			activeFlags ^= CAPTURE_ON;
			if(activeFlags & CAPTURE_ON){
				printf("Recording started.\n");
				fpRecording = pCapture->RecStart(videoFormat, camCnt, "rec");
			}else{
				pCapture->RecEnd(fpRecording);
				fpRecording = NULL;
				printf("Recording stopped.\n");
			}
		}*/
		else if(key >= 0) break;
	}	
	return 0;
}
#endif

// gtk
//#include <gtk/gtk.h>
int main(int argc, char** argv)
{
	int i;
	// Parse command line
	int activeFlags = SHOW_INFO_MSG;
	int camIdx = 0, camCnt = MAX_CAMERA;
	int options = 0;
	FILE* fout = NULL;
	
	CCapture capture;
	
	int agcs = 1;
	if(argc >= 2 && argv[1][0] >= '0' && argv[1][0] <= '9'){ // Camera ID is specified
		camIdx = atoi(argv[1]);
		if(camIdx < 0) camIdx = 0;
		camCnt = 1;
		agcs ++;
	}
	
	int parseError = ParseOption(argc-agcs, &argv[agcs], &options);
	if(parseError){
		// usage
		printf("NVCapSimple Linux version 1.0 \n");
		printf("Usage: \n");
		printf("%s (Camera ID) (Options)\n", argv[0]);
		printf("Info:\n");
		printf("Using Opencv %d.%d.%d\n", CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION);
		//printf("Will use %d threads.\n", getNumThreads());
		return 0;
	}
	
	setpriority(PRIO_PROCESS, 0, CPU_PRIORITY);

	// Get Camera Device Name
	char cameraName[MAX_CAMERA][128];
	capture.GetCameraName(camIdx, cameraName[0], 128);

	for(i=1; i<camCnt; i++){
		if(capture.GetCameraName(i, cameraName[i], 128)){
			camCnt = i;
			break;
		}
	}
	// Specify 1 SVM Device
	if(camCnt > 1)
	{
		for(i=0; i<camCnt; i++)
		{
			if(strstr(cameraName[i], "SVM") != NULL){
				camIdx = i;
				camCnt = 1;
			}
		}
	}
	if(camCnt != 1){
		printf("Error: Please specify camera ID.\n");
		return -1;
	}

	CV4LCapture* capSub[MAX_CAMERA];
	CFrameConverter* pFrameConverter[MAX_CAMERA];
	CV4LCapture::sVideoFormat videoFormat[MAX_CAMERA];
	FILE* fpRecording = NULL;
	for(i=0; i<MAX_CAMERA; i++) {
		capSub[i] = NULL;
		pFrameConverter[i] = NULL;
	}
	if(camIdx){
		capSub[0] = new CV4LCapture();
		capSub[0]->Init(camIdx, WIDTH, HEIGHT, g_timeout, g_queueSize);
		
	}else{
		for(i=0; i<camCnt; i++){
			capSub[i] = new CV4LCapture();
			capSub[i]->Init(i, WIDTH, HEIGHT, g_timeout, g_queueSize);

			
		}
	}
	
	// Check camera name and swap in ascending order
	for(i=0; i<camCnt; i++){
		char temp[128];
		CV4LCapture* temp2;
		for(int j=i; j<camCnt; j++){
			if(strcmp(cameraName[i], cameraName[j]) > 0){
				strcpy(temp, cameraName[j]);
				strcpy(cameraName[j], cameraName[i]);
				strcpy(cameraName[i], temp);
				temp2 = capSub[j];
				capSub[j] = capSub[i];
				capSub[i] = temp2;
			}
		}
	}
	// Get and show video format
	for(i=0; i<camCnt; i++){
		// Get Pixel Format
		memset(&videoFormat[i], 0, sizeof(videoFormat[i]));
		capSub[i]->GetVideoFormat(&videoFormat[i]);
			
		printf("ch: %d, width: %d, height: %d\n", 
			i, videoFormat[i].width, videoFormat[i].height);
		printf("pixelFormat = %s\n", videoFormat[i].pixelFormat);
	}

	// Create window
	CreateWindow(camCnt, options);
	
	// Create Frame Converter
	if(options & SIMPLE_WINDOW){
		int pixelFormat = capture.GetPixelFormat(&videoFormat[0]);
		
		pFrameConverter[0] = new CFrameConverter(
			videoFormat[0].width, videoFormat[0].height, 1, options, pixelFormat);
	}else{
		for(i=0; i<camCnt; i++){
			int pixelFormat = capture.GetPixelFormat(&videoFormat[i]);
			pFrameConverter[i] = new CFrameConverter(videoFormat[i].width, videoFormat[i].height, 0, 
				options, pixelFormat);
		}
	}
	
	uint8_t* captureBufSame, *captureBufInd[MAX_CAMERA];
	if(options & SIMPLE_WINDOW){
		int width = videoFormat[0].width;
		int height = videoFormat[0].height;
		int bytePerPixel = (videoFormat[i].bitPerPixel + 7) / 8;
		captureBufSame = new uint8_t[width*height*bytePerPixel*4];
		for(i=0; i<MAX_CAMERA; i++){
			captureBufInd[i] = NULL;
		}
	}else{
		captureBufSame = NULL;
		for(i=0; i<MAX_CAMERA; i++){
			captureBufInd[i] = NULL;
			if(i < camCnt){
				int bytePerPixel = (videoFormat[i].bitPerPixel + 7) / 8;
				captureBufInd[i] = new uint8_t[
					videoFormat[i].width * videoFormat[i].height * bytePerPixel];
			}
		}
	}


	printf("q: exit\n");
	printf("i: show/hide info\n");
	//ShowUserOption();

	sMainLoopParam param;
	param.capSub = capSub;
	param.pFrameConverter = pFrameConverter;
	param.videoFormat = videoFormat;
	param.captureBufSame = captureBufSame;
	param.captureBufInd = captureBufInd;
	param.pCapture = &capture;
	param.threadData = NULL;
	param.camIdx = camIdx;
	
	Mainloop(&param, camCnt, options);


	capture.RecEnd(fpRecording);
	
	if(captureBufSame) delete [] captureBufSame;
	for(i=0; i<MAX_CAMERA; i++){
		if(captureBufInd[i]) delete [] captureBufInd[i];
		if(capSub[i]) delete capSub[i];
		if(pFrameConverter[i]) delete pFrameConverter[i];
	}
	
	
	
	return 0;
}


