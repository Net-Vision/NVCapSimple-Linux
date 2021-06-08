// Linux SVM UVC Manager
// 2015.06.xx

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <sys/time.h>
#include <unistd.h>

// v42l header
#include <linux/videodev2.h>
#include <libv4l2.h>

#include <linux/uvcvideo.h>
#include <linux/usb/video.h>

#include "UVCManager.h"

CUVCManager::CUVCManager(){
	m_fd = 0;
	m_deviceID = -1;
}
CUVCManager::CUVCManager(int deviceID){
	OpenDevice(deviceID);
}
CUVCManager::~CUVCManager(){
	CloseDevice();
}

int CUVCManager::GetDeviceName(int deviceID, char* name, int nameSize)
{
   char buf[128];
   sprintf(buf, "/sys/class/video4linux/video%d/name", deviceID);
   FILE* fp = fopen(buf, "r");
   if(fp == NULL) return -1;

	char* p = fgets(name, nameSize-1, fp);
	int l = strlen(name);
	name[l-1] = '\0';
	fclose(fp);

	return 0;
}
int CUVCManager::GetDeviceName(char* name, int nameSize){
	if(m_deviceID >= 0){
		GetDeviceName(m_deviceID, name, nameSize);
	}else{
		strcpy(name, "(Device not opened.)");
	}
	return 0;
}
int CUVCManager::OpenDevice(int deviceID)
{
	if(m_fd > 0) CloseDevice();
	
	char buf[128];
	sprintf(buf, "/dev/video%d", deviceID);
	m_fd = open(buf, O_RDWR);
	m_deviceID = (m_fd>0) ? deviceID : -1;
	return m_fd;
}

void CUVCManager::CloseDevice()
{
	if(m_fd > 0) close(m_fd);
}

int CUVCManager::QueryControl(struct v4l2_queryctrl* pQueryCtrl)
{
	if(m_fd <= 0) return -1;
	// set pQueryCtrl->id
	return ioctl(m_fd, VIDIOC_QUERYCTRL, pQueryCtrl);
}
int CUVCManager::SetControl(unsigned int id, int value)
{
	if(m_fd <= 0) return -1;
	struct v4l2_control ctrl;
	ctrl.id = id;
	ctrl.value = value;
	return ioctl(m_fd, VIDIOC_S_CTRL, &ctrl);
}
int CUVCManager::GetControl(unsigned int id, int* pValue)
{
	if(m_fd <= 0) return -1;
	struct v4l2_control ctrl;
	ctrl.id = id;
	ctrl.value = 0;
	int ret = ioctl(m_fd, VIDIOC_G_CTRL, &ctrl);
	*pValue = ctrl.value;
	return ret;
}




