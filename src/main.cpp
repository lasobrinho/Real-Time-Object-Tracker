#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
//#include <conio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
//#include <opencv2/cvcam.h>


/************************************************************************
 * 
 ************************************************************************/
int main( int argc, char** argv )
{
  IplImage* frame = 0, *invert, *background, *foreground, *framecinza;
  CvCapture* capture = 0;
  int pix;
   
  // capture = cvCaptureFromCAM(0);

  //capture = cvCaptureFromAVI( "c:\\v4.mpeg" ); 
  capture = cvCaptureFromFile("/home/lucas/Downloads/Bases Video/v2.mpg");
  
  if( !capture )
  {
    fprintf(stderr,"Could not initialize capturing...\n");
     system("pause");
  }
  
  frame = cvQueryFrame( capture );
  frame->origin = 1;
  
  background = cvCreateImage(cvSize(frame->width, frame->height),frame->depth,1);
  framecinza = cvCreateImage(cvSize(frame->width, frame->height),frame->depth,1);
  background->origin = 1;
  cvCvtColor(frame,background,CV_RGB2GRAY); 

  foreground = cvCreateImage(cvSize(frame->width, frame->height),frame->depth,1);
  foreground->origin = 1;

  cvNamedWindow( "in", 0 );
  cvNamedWindow( "fore", 0 );
  

  for(;;)
  {
    if(!cvGrabFrame(capture)) 
      break;
 
    frame = cvRetrieveFrame(capture);
    cvCvtColor(frame,framecinza,CV_RGB2GRAY);
    
    cvAbsDiff(framecinza,background,foreground);	
    cvThreshold(foreground, foreground,125,255,CV_THRESH_BINARY);

     cvErode(foreground, foreground, NULL, 1);
     cvDilate( foreground,foreground,NULL, 2);
   
/*   
    for(int i=0;i<background->height;i++) 
              for(int j=0;j<background->width;j++) {
               ((uchar*)(background->imageData + background->widthStep*i))[j] = 255;
    }
*/    
    cvShowImage("in", frame );
    cvShowImage("fore", foreground );
    
   if( cvWaitKey(10) >= 0 )
        break;
  }

  cvReleaseCapture( &capture );
  cvDestroyWindow("in");
  cvDestroyWindow("fore");
  system("pause");
  return 0;
}
