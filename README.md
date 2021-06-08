To build:

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
