
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

	IplImage* frame = 0, *invert, *background, *foreground, *frameCinza;
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
	frameCinza = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
	background->origin = 0;
	cvCvtColor(frame, background, CV_RGB2GRAY); 
	cvSmooth(background, background, CV_BLUR);

	foreground = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
	foreground->origin = 0;

	cvNamedWindow("in", 0);
	cvNamedWindow("fore", 0);
	cvNamedWindow("frameCinza", 0);
	cvNamedWindow("background", 0);

	cvMoveWindow("in", 50, 50);
	cvMoveWindow("fore", 50, 375);
	cvMoveWindow("frameCinza", 375, 50);
	cvMoveWindow("background", 375, 375);

	IplConvKernel* structElem;

	for (;;) {
		if(!cvGrabFrame(capture)) 
			break;

		frame = cvRetrieveFrame(capture);

		cvSmooth(frame, frame, CV_BLUR);

		cvCvtColor(frame, frameCinza, CV_RGB2GRAY);		

	    cvAbsDiff(frameCinza, background, foreground);	

	    CvScalar brVal = cvScalarAll(abs(10.0));
    	cvAddS(foreground, brVal, foreground, NULL);
	    IplImage *pTempForeground = cvCreateImage(cvGetSize(foreground), IPL_DEPTH_8U, foreground->nChannels);
	    cvSet( pTempForeground, cvScalarAll(1), NULL );
	    double scale = 1.5;
	    cvMul(foreground, pTempForeground, foreground, scale);
	    cvReleaseImage(&pTempForeground);

		cvThreshold(foreground, foreground, 75, 255, CV_THRESH_BINARY);

		cvErode(foreground, foreground, NULL, 2);

		structElem = cvCreateStructuringElementEx(3, 5, 1, 2, 0);
		//structElem = cvCreateStructuringElementEx(5, 9, 3, 5, 2);

		cvDilate(foreground, foreground, structElem, 15);
		cvErode(foreground, foreground, structElem, 10);

		// for(int i=0;i<background->height;i++) 
		// 	for(int j=0;j<background->width;j++) {
		// 		((uchar*)(background->imageData + background->widthStep*i))[j] = 255;
		// 	}  

		cvShowImage("frameCinza", frameCinza);
		cvShowImage("background", background);
		cvShowImage("in", frame);
		cvShowImage("fore", foreground);

		if(cvWaitKey(10) >= 0)
			break;

	}

	cvReleaseCapture(&capture);

	cvDestroyWindow("in");
	cvDestroyWindow("fore");
	cvDestroyWindow("frameCinza");
	cvDestroyWindow("background");

	system("pause");

	return 0;
}
