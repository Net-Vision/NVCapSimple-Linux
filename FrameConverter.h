#ifndef _FRAME_CONVERTER_H_
#define _FRAME_CONVERTER_H_

//#include "main.h"

#define FC_FORMAT_UYVY	0
#define FC_FORMAT_YUYV	1
//#define FC_FORMAT_RGB	2
#define FC_FORMAT_Y8	8
#define FC_FORMAT_Y10	9
#define FC_FORMAT_Y12	10
#define FC_FORMAT_RGB24	16

// [15/07/15] UYVY or YUYV to RGB
class CFrameConverter{
public:
	CFrameConverter(int width, int height, int oneWindow, int options, int pixelFormat = 0){
		m_oneWindow = oneWindow;
		m_width = width;
		m_height = height;
		// reserve internal buffer
		if(options & HALF_WINDOW){
			if(oneWindow){
				m_frame2 = Mat::zeros(Size(width, height), CV_8UC3); // 
			}else{
				m_frame2 = Mat::zeros(Size(width/2, height/2), CV_8UC3); // 
			}
		}else{
			if(oneWindow){
				m_frame2 = Mat::zeros(Size(width*2, height*2), CV_8UC3); // 
			}else{
				m_frame2 = Mat::zeros(Size(width, height), CV_8UC3); // 
			}
		}
		m_pixelFormat = pixelFormat;
	};
	~CFrameConverter(){
	};
	
private:
	void ConvertFrame_sub_rgb24(int bufOffs, const uint8_t* captureBuf, Mat& frame, int options)
	{
		if(options & HALF_WINDOW){
			frame = Mat::zeros(Size(m_width/2, m_height/2), CV_8UC3); // 
			uint8_t* data = &frame.data[0];

			for(int y=0; y<m_height/2; y+=1){
				for(int x=0; x<m_width/2; x+=1){
					data[3*(y*m_width/2 + x)+0] = captureBuf[3*(2*y*m_width+x*2)+0];
					data[3*(y*m_width/2 + x)+1] = captureBuf[3*(2*y*m_width+x*2)+1];
					data[3*(y*m_width/2 + x)+2] = captureBuf[3*(2*y*m_width+x*2)+2];
				}
			}
		}else{
			frame = Mat(Size(m_width, m_height), CV_8UC3, (void*)captureBuf); // 
		}
	}
	// 222 -> 244 Conversion
	void ConvertFrame_sub(int bufOffs, const uint8_t* captureBuf, Mat& frame, int options)
	{
		// show individual window
		unsigned char lastCr = 128;
		unsigned char lastCb = 128;
		const int offs = 0;
		int x = m_width, y = m_height;
		uint8_t* data = &frame.data[bufOffs];
		if(options & HALF_WINDOW){
			for(int y=0; y<m_height/2; y+=1){
				for(int x=0; x<m_width/2; x+=1){
					int j = y*m_width/2+x;
					int idx = y*2*m_width+x*2;
					int j3 = j*3;
					int idx2 = idx*2;
					
					if(m_pixelFormat == FC_FORMAT_YUYV){ // YUYV
						data[j3] = captureBuf[offs+idx2]; // Y
						data[j3+1] = captureBuf[offs+idx2+1]; // Cr
						data[j3+2] = captureBuf[offs+idx2+3]; // Cb
					}else if(m_pixelFormat == FC_FORMAT_UYVY){ // UYVY
						data[j3] = captureBuf[offs+idx2+1]; // Y
						data[j3+1] = captureBuf[offs+idx2]; // Cr
						data[j3+2] = captureBuf[offs+idx2+2]; // Cb
					}else if(m_pixelFormat == FC_FORMAT_Y8){
						data[j3] = captureBuf[offs+idx]; // Y
						data[j3+1] = data[j3+2] = 0x80;
					}
				}
			}
		}else{
			for(int j=0; j<x*y; j+=1){
				int j3 = j*3;
				int j2 = j*2;
				
				if(m_pixelFormat == FC_FORMAT_YUYV){// YUYV
					data[j*3] = captureBuf[offs+j2]; // Y
					if(j&1){
						data[j3+1] = lastCr; // Cr
						data[j3+2] = lastCb = captureBuf[offs+j2+1]; // Cb
					}else{
						data[j3+1] = lastCr = captureBuf[offs+j2+1]; // Cr
						data[j3+2] = lastCb; // Cb
					}
				}else if(m_pixelFormat == FC_FORMAT_UYVY){
					data[j3] = captureBuf[offs+j*2+1]; // Y
					if(j&1){
						data[j3+1] = lastCr; // Cr
						data[j3+2] = lastCb = captureBuf[offs+j2]; // Cb
					}else{
						data[j3+1] = lastCr = captureBuf[offs+j2]; // Cr
						data[j3+2] = lastCb; // Cb
					}
				}else if(m_pixelFormat == FC_FORMAT_Y8){
					data[j3] = captureBuf[offs+j]; // Y
					data[j3+1] = data[j3+2] = 0x80;
				}
			}
		}
	}
	
public:
	void ConvertFrame_2(int camCnt, const uint8_t* captureBuf, Mat& frame, 
		int options) 
	{
#if TEST2
g_cTimeConsManager.Start(8);
#endif
		// captureBuf -> Mat
		Mat buf = Mat::zeros(Size(m_width, m_height), CV_8UC2);
		memcpy(buf.data, captureBuf, m_width*m_height*2);
		
		// YUV -> RGB
		if(options == FC_FORMAT_YUYV){
			cvtColor(buf, frame, COLOR_YUV2BGR_YUY2);
		}else{
			cvtColor(buf, frame, COLOR_YUV2BGR_UYVY);
		}
		
#if TEST2
g_cTimeConsManager.End(8);
#endif
	}

	// gain, gamma options are not valid in this time.
	void ConvertRAW2Mono(uint8_t* captureBuf, int rawBitWidth, double gain, double gamma) 
	{
		int idx = 0;
		int oe = 0;
		if(m_pixelFormat == FC_FORMAT_YUYV) oe = 1;
		for(idx=0; idx<m_height*m_width; idx+=1)
		{
			int v = ((int)captureBuf[idx*2+0] << 8) | captureBuf[idx*2+1];
			captureBuf[idx*2+oe] = 0x80; // uv
			if(rawBitWidth == 12)
			{
				captureBuf[idx*2+(oe^1)] = v >> 4; // y
			}else if(rawBitWidth == 10)
			{
				captureBuf[idx*2+(oe^1)] = v >> 2; // y
			}
		}
	}
	
	// 
	void ConvertFrame(int camCnt, const uint8_t* captureBuf, Mat& frame, 
		int options) 
	{
	
#if TEST2
g_cTimeConsManager.Start(7);
#endif

	// Set in Matrix (420 -> 422)
	if(m_oneWindow == 0){
		if(m_pixelFormat == FC_FORMAT_RGB24){
			ConvertFrame_sub_rgb24(0, captureBuf, frame, options);
#if TEST2
g_cTimeConsManager.End(7);
#endif
			return;
		}else{
			ConvertFrame_sub(0, captureBuf, m_frame2, options);
		}
	}else{
		// In this option not support RGB24 pixel format!
		if(m_pixelFormat == FC_FORMAT_RGB24) return;
		
		// show all in one window
		unsigned char lastCr = 128;
		unsigned char lastCb = 128;
		int d = 1;
		if(options & HALF_WINDOW) d = 2;
		
		//memset(m_frame2.data, 128, m_height*m_width*4/d/d*3);

		// 15/09/18 Revised
		Mat yuvBuf;
		yuvBuf = Mat::zeros(Size(m_width, m_height), CV_8UC3);
		for(int camIdx = 0; camIdx < camCnt; camIdx++){
			ConvertFrame_sub(0, &captureBuf[camIdx*2*(m_width*m_height)], yuvBuf, options);
			
			
			// yuvbuf -> frame2
			if(d == 1){
				int yoffs = (camIdx/2)*m_height;
				int xoffs = (camIdx%2)*m_width;
				for(int y=0; y<m_height; y++){
					int idx1 = ((y+yoffs)*(m_width*2) + 0+xoffs)*3;
					int idx2 = (y*m_width + 0)*3;
					for(int x=0; x<m_width; x++){
						m_frame2.data[ idx1 + 0] = 
							yuvBuf.data[ idx2 + 0 ];
						m_frame2.data[ idx1 + 1] = 
							yuvBuf.data[ idx2 + 1 ];
						m_frame2.data[ idx1 + 2] = 
							yuvBuf.data[ idx2 + 2 ];
						idx1 += 3;
						idx2 += 3;
					}
				}
			}else{
				int yoffs = (camIdx/2)*m_height/2;
				int xoffs = (camIdx%2)*m_width/2;
				for(int y=0; y<m_height/2; y++){
					int idx1 = ((y+yoffs)*(m_width*2/2) + 0+xoffs)*3;
					int idx2 = (y*m_width/2 + 0)*3;
					for(int x=0; x<m_width/2; x++){
						m_frame2.data[ idx1 + 0] = 
							yuvBuf.data[ idx2 + 0 ];
						m_frame2.data[ idx1 + 1] = 
							yuvBuf.data[ idx2 + 1 ];
						m_frame2.data[ idx1 + 2] = 
							yuvBuf.data[ idx2 + 2 ];
						idx1 += 3;
						idx2 += 3;
					}
				}
			}
			
		}
	}
	

#if TEST2
g_cTimeConsManager.End(7);
g_cTimeConsManager.Start(8);
#endif

	// Color Conversion
	cvtColor(m_frame2, frame, COLOR_YCrCb2RGB);
//	frame = m_frame2;

#if TEST2
g_cTimeConsManager.End(8);
#endif

	}
private:
	int m_oneWindow;
	int m_width, m_height;
	Mat m_frame2; // intermediate buffer
	int m_pixelFormat;
};



#endif

