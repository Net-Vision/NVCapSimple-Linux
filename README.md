# NVCapSimple_Linux

NVCapSimple is a simple example of capture (preview) software for our "SV series" board and other UVC camera.
This software supports "UYVY" and "YUY2" frame format, but other formats such as MJPG and RGB are not supported.

This example uses v4l2 driver to capture "raw" data of UYVY format and uses OpenCV to display video.

It includes another example to show "RAW10/RAW12 on UYVY" video with converting to monochrome.

# OS
Ubuntu 18.04 LTS (64 bit)

# Build

At first, install v4l-utils etc.
1.	sudo apt-get install v4l-utils cmake libx11-dev

Install OpenCV
1.	Download the source code from OpenCV web site (http://opencv.org/), then 
	extract archive and move to the extracted folder. 
	wget https://github.com/opencv/opencv/archive/3.0.0.zip
	unzip 3.0.0.zip
	cd opencv-3.0.0/
2.	Install build-essential packeage:
	sudo apt-get install build-essential
3.	Install libraries required for OpenCV:
	sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev \
	libavformat-dev libswscale-dev

4.	Install optional libraries internally used from OpenCV:
	sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev \
	libpng-dev libtiff-dev libjasper-dev
5.	Create a folder and move into it
	mkdir build && cd build
6a.	If you want to use Qt generate Makefile with corresponding option:
	cmake -D WITH_QT=ON -D WITH_OPENMP=ON ..
6b.	If you don't want to use Qt:
	cmake -D WITH_OPENMP=ON ..
7.	make 
8.	sudo make install

Then build
1.	cd build
2.	cmake .. .
3.	make

# Usage
NVCapSimple_YUV: show YUV video.
NVCapSimple_RAW10: show RAW10 video in grayscale.
NVCapSimple_RAW12: show RAW12 video in grayscale.

To open device 1 (/dev/video1),
---bash
./NVCapSimple_YUV 1
---

Press key 'q' to quit.
Press key 'i' to toggle info text.

