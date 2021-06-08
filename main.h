#ifndef _MAIN_H_
#define _MAIN_H_


#include <opencv2/opencv.hpp>
#include <ctype.h>

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

// priority
#include <sys/time.h>
#include <sys/resource.h>

//#include <gtk/gtk.h>

// for debug use
#define TEST1  0
#define TEST2  0
#define TEST3  1 // CFrameConverter

#define MAX_CAMERA 8

// default camera resolution

//#define WIDTH  1280
//#define HEIGHT 800
//#define HEIGHT 720
//#define HEIGHT 1086
//#define WIDTH  640
//#define HEIGHT 480
#define WIDTH	1920
#define HEIGHT	1080

// Flag bits
#define SHOW_FRAME 1
#define SAVE_FILE 2
#define DEBUG_MODE 4
#define SIMPLE_WINDOW 8
#define HALF_WINDOW 16
#define SIMPLE_GUI 32

// Flag bits 2 (Active Flags)
#define AF_ROTATION 1
#define AF_FACE_DETECTION 2
#define SHOW_INFO_MSG 4
#define CAPTURE_ON 8
#define AF_FLIP 16

// Application priority (Higher value requires authority)
#define CPU_PRIORITY -19

// debug
//#include "TimeConsManager.h"
//extern CTimeConsManager g_cTimeConsManager;

using namespace cv;
using namespace std;

#endif

