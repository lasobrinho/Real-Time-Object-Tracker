
#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace std;

int main( int argc, char** argv )
{

	if (argc != 2) {
		cerr << endl;
		cerr << "--> [ERROR] Wrong argument list." << endl;
		cerr << "--> Usage ./RealTimeObjectTracker [video_path]" << endl << endl;
		return 0;
	}

	IplImage* frame = 0, *invert, *background, *foreground, *framecinza;
	CvCapture* capture = 0;
	int pix;

	capture = cvCaptureFromFile(argv[1]);

	if (!capture) {
		cerr << "Could not initialize capturing" << endl;
		return 0;
	}

	frame = cvQueryFrame(capture);
	frame->origin = 0;

	background = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
	framecinza = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
	background->origin = 0;
	cvCvtColor(frame, background, CV_RGB2GRAY); 

	foreground = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
	foreground->origin = 0;

	cvNamedWindow("in", 0);
	cvNamedWindow("fore", 0);


	for (;;) {
		if(!cvGrabFrame(capture)) 
			break;

		frame = cvRetrieveFrame(capture);
		cvCvtColor(frame, framecinza, CV_RGB2GRAY);

		cvAbsDiff(framecinza, background, foreground);	
		cvThreshold(foreground, foreground, 125, 255, CV_THRESH_BINARY);

		cvErode(foreground, foreground, NULL, 1);
		cvDilate(foreground, foreground, NULL, 2);

		// for(int i=0;i<background->height;i++) 
		// 	for(int j=0;j<background->width;j++) {
		// 		((uchar*)(background->imageData + background->widthStep*i))[j] = 255;
		// 	}  

		cvShowImage("in", frame );
		cvShowImage("fore", foreground );

		if(cvWaitKey(10) >= 0)
			break;

	}

	cvReleaseCapture(&capture);
	cvDestroyWindow("in");
	cvDestroyWindow("fore");
	system("pause");

	return 0;
}
