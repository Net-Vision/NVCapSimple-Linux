// Video for Linux Wrapper Class
// To see detail of Video for Linux API, see
// https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/v4l2.html

#include "V4LCapture_Min.h"

#include <sys/time.h>
#include <unistd.h>
#include <inttypes.h>



#define TIMEOUT_SEC 5 // default value of timeout
#define REQ_COUNT 8 // default value of frame buffer
#define USE_DMA 0 // Currently not supported
#define USE_TIMECODE 0

CV4LCapture::CV4LCapture()
{
	m_initialized = 0;
	m_fd = 0;
	m_timeout = TIMEOUT_SEC;
	m_id = -1;
}
CV4LCapture::~CV4LCapture()
{
	if(m_initialized){
		StopCapture();
		CloseDevice();
	}
}

int CV4LCapture::ReadFrame(uint32_t count, uint8_t* outputBuf, 
	uint32_t bufSize, uint32_t* pSequence)
{
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
#if USE_DMA
	buf.memory = V4L2_MEMORY_DMABUF;
	buf.m.fd   = ???;
#else
	buf.memory = V4L2_MEMORY_MMAP;
#endif
	if(USE_TIMECODE) buf.type |= V4L2_BUF_FLAG_TIMECODE;
	

	// deque buffer from driver's queue
	if(xioctl(m_fd, VIDIOC_DQBUF, &buf) == -1) { 
		switch(errno){
		case EAGAIN:
			return 0;
		case EIO:
			/* Could ignore EIO, see spec. */
			/* fall through */
		default:
			errno_exit("VIDIOC_DQBUF");
		}
	}

//	if(USE_TIMECODE) printf("Timecode = %d\n", buf.timecode.frames);
	if(pSequence) *pSequence = buf.sequence; 

	if(count == 0){
		// save	
		uint32_t sz = m_pBuffer[buf.index].length;
		if(bufSize < sz) sz = bufSize;
		memcpy(outputBuf, m_pBuffer[buf.index].start, sz);
	}
	// enque buffer 
	if(xioctl(m_fd, VIDIOC_QBUF, &buf) == -1){
		errno_exit("VIDIOC_QBUF");
	}

  return 1;
}

void CV4LCapture::StopCapture(void)
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(xioctl(m_fd, VIDIOC_STREAMOFF, &type) == -1){
		errno_exit("VIDIOC_STREAMOFF");
	}
}

void CV4LCapture::StartCapture(void)
{
	int i;
	enum v4l2_buf_type type;

	for(i=0; i<m_bufferCnt; i++){
#if USE_DMA
#else
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;

		if(xioctl(m_fd, VIDIOC_QBUF, &buf) == -1){
			errno_exit("VIDIOC_QBUF");
		}
#endif
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(xioctl(m_fd, VIDIOC_STREAMON, &type) == -1){
		errno_exit("VIDIOC_STREAMON");
	}
}

void CV4LCapture::CloseDevice(void)
{
	int i;
	for(i=0; i<m_bufferCnt; i++){
		if(munmap(m_pBuffer[i].start, m_pBuffer[i].length) == -1){
			errno_exit("munmap");
		}
	}
  	delete [] m_pBuffer;

	if(m_fd > 0) close(m_fd);
}
void CV4LCapture::InitMmap(char* dev_name, int reqCount)
{
	struct v4l2_requestbuffers req;

	memset(&req, 0, sizeof(req));

	if(reqCount <= 0) reqCount = REQ_COUNT;
	req.count = reqCount;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
#if USE_DMA
#else
	req.memory = V4L2_MEMORY_MMAP;
#endif
  
	if(xioctl(m_fd, VIDIOC_REQBUFS, &req) == -1) {
		exit(EXIT_FAILURE);
	}
	if(req.count < 2)
	{
		exit(EXIT_FAILURE);
	}
	m_pBuffer = new sBuffer[req.count];

	m_bufferCnt = req.count;
  
	int i;
	for(i=0; i<req.count; i++)
	{
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if(xioctl(m_fd, VIDIOC_QUERYBUF, &buf) == -1){
			errno_exit("VIDIOC_QUERYBUF");
		}

		m_pBuffer[i].length = buf.length;
		m_pBuffer[i].start =
		mmap(NULL, buf.length,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			m_fd,
			buf.m.offset);

		if(m_pBuffer[i].start == MAP_FAILED){
			errno_exit("mmap");
		}
	}
}

int CV4LCapture::ReOpen(void)
{
	return Init(m_id, m_width, m_height);
}

int CV4LCapture::Init(int id, int width, int height)
{
	return Init(id, width, height, TIMEOUT_SEC, REQ_COUNT);
}

int CV4LCapture::Init(int id, 
	int width, int height, unsigned int timeout, unsigned int queueSize)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	
	if(m_initialized){
		StopCapture();
		CloseDevice();
	}

	m_id = id;
	m_width = width;
	m_height = height;

	char dev_name[128];
	sprintf(dev_name, "/dev/video%d", id);

	m_timeout = timeout;
	
	// try to get device name
	GetCameraName(id, m_deviceName, 127);

	struct stat st; 

	{
		if(stat(dev_name, &st) == -1)
		{
			fprintf(stderr, "Cannot identify '%s': %d, %s\n",
				dev_name, errno, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if(!S_ISCHR(st.st_mode))
		{
			fprintf(stderr, "%s is not a valid device\n", dev_name);
			exit(EXIT_FAILURE);
		}
		m_fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
		if(m_fd == -1)
		{
			fprintf(stderr, "Cannot open '%s': %d, %s\n",
				dev_name, errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
  
	if(xioctl(m_fd, VIDIOC_QUERYCAP, &cap) == -1)
	{
		if(EINVAL == errno)
		{
			fprintf(stderr, "%s is not V4L2 device\n",dev_name);
			exit(EXIT_FAILURE);
		}
		else
		{
			errno_exit("VIDIOC_QUERYCAP");
		}
	}
	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		fprintf(stderr, "%s is not video capture device\n",dev_name);
		exit(EXIT_FAILURE);
	}
	if(!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		fprintf(stderr, "%s does not support streaming i/o\n",
		dev_name);
		exit(EXIT_FAILURE);
	}

	memset(&cropcap, 0, sizeof(cropcap));
  
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(xioctl(m_fd, VIDIOC_CROPCAP, &cropcap) == 0){
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect;

		xioctl(m_fd, VIDIOC_S_CROP, &crop);
	}
	memset(&fmt, 0, sizeof(fmt));
  
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = width;
	fmt.fmt.pix.height      = height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       = V4L2_FIELD_NONE;

	if(xioctl(m_fd, VIDIOC_S_FMT, &fmt) == -1){ // Set video format
		errno_exit("VIDIOC_S_FMT");
	}
	
	// ?
	uint32_t min = fmt.fmt.pix.width * 2;
	if(fmt.fmt.pix.bytesperline < min) fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if(fmt.fmt.pix.sizeimage < min) fmt.fmt.pix.sizeimage = min;

	InitMmap(dev_name, queueSize);

	StartCapture();

	m_initialized = 1;
	printf("Init OK.\n"); // debug msg

	return 0;
}

int CV4LCapture::GetVideoFormat(CV4LCapture::sVideoFormat* pFmt)
{
	struct v4l2_format fmt;
	if(m_fd == 0) return -1;

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(xioctl(m_fd, VIDIOC_G_FMT, &fmt) == -1){
		return -1;
	}
	pFmt->width = fmt.fmt.pix.width;
	pFmt->height = fmt.fmt.pix.height;
	pFmt->pixelFormat[0] = fmt.fmt.pix.pixelformat >> 0;
	pFmt->pixelFormat[1] = fmt.fmt.pix.pixelformat >> 8;
	pFmt->pixelFormat[2] = fmt.fmt.pix.pixelformat >> 16;
	pFmt->pixelFormat[3] = fmt.fmt.pix.pixelformat >> 24;
	
	pFmt->bitPerPixel = 16;
	if(pFmt->pixelFormat[0] == '\0') pFmt->bitPerPixel = 24; // might be RGB24
	if(strncmp((const char*)pFmt->pixelFormat, "Y8  ", 4) == 0) pFmt->bitPerPixel = 8;
	if(strncmp((const char*)pFmt->pixelFormat, "Y10 ", 4) == 0) pFmt->bitPerPixel = 10;
	if(strncmp((const char*)pFmt->pixelFormat, "Y12 ", 4) == 0) pFmt->bitPerPixel = 12;
	
	// device name
	strcpy(pFmt->deviceName, m_deviceName);
	return 0;
	
}

// Capture 1 frame
int CV4LCapture::CaptureFrame(unsigned char* buf, uint32_t bufSize, uint32_t* pSequence)
{
	static uint32_t count = 0;
	do{
		fd_set fds;
		struct timeval tv;
		int ret;
		FD_ZERO(&fds);
		FD_SET(m_fd, &fds);
		/* Timeout. */
		if(m_timeout >= 0x80000000){
			tv.tv_sec = 0;
			tv.tv_usec = m_timeout & 0x7FFFFFFF;
		}else{
			tv.tv_sec = m_timeout;
			tv.tv_usec = 0;
		}
		ret = select(m_fd + 1, &fds, NULL, NULL, &tv);
		if(ret == -1)
		{
			if(EINTR == errno)
			continue;
			errno_exit("select");
		}
		if(ret == 0)
		{
			// [18/03/01]
			printf("select() timeout (CaptureFrame)\n");
			return -1;
			//fprintf(stderr, "select() timeout (CaptureFrame)\n");
			//exit(EXIT_FAILURE);
		}
		if(ReadFrame(count, buf, bufSize, pSequence)) break; // read

	}while(0);

	return 0;
}


