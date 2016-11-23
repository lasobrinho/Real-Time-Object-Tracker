
#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

typedef struct FrameObject
{
    int id;
    int centerX;
    int centerY;
    int area;
    Rect rect;
    Scalar color;
    FrameObject (int _id, int _centerX, int _centerY, int _area, Rect _rect, Scalar _color) : id(_id), centerX(_centerX), centerY(_centerY), area(_area), rect(_rect), color(_color) {};
}
FrameObject;

int main(int argc, char** argv) {

	// Check if the user input is correct to proceed
	if (argc != 2) {
		cerr << endl;
		cerr << " --> Wrong arguments list" << endl;
		cerr << " --> Usage ./RealTimeObjectTracker [video_path]" << endl << endl;
		return 0;
	}

	// Initialize program variables
	Mat frame, frameGray, background, diff, diff_clone;
	const char *frameWindowTitle = "frame", *frameGrayWindowTitle = "frameGray";
	const char *backgroundWindowTitle = "background", *diffWindowTitle = "diff_copy";
	vector<vector<Point> > contours;
  	vector<Vec4i> hierarchy;
  	RNG rng(15379);
  	vector<FrameObject> frameObjects;

	// Start VideoCapture using the file name passed as program argument
	VideoCapture videoCapture(argv[1]);
	if (!videoCapture.isOpened()) {
		cerr << " --> Could not initialize capturing for file " << argv[1] << "" << endl;
		cerr << " --> Program ended with errors" << endl;
		return 0;
	}

	// Capture the background frame, convert it to gray and blur it
	videoCapture >> frame;
	cvtColor(frame, background, CV_BGR2GRAY);
	blur(background, background, Size(3, 3));

	// Initialize windows to display the output
	namedWindow(frameWindowTitle, CV_WINDOW_AUTOSIZE);
	namedWindow(frameGrayWindowTitle, CV_WINDOW_AUTOSIZE);
	namedWindow(backgroundWindowTitle, CV_WINDOW_AUTOSIZE);
	namedWindow(diffWindowTitle, CV_WINDOW_AUTOSIZE);

	// Position output windows on the screen
	moveWindow(frameWindowTitle, 50, 50);
	moveWindow(frameGrayWindowTitle, 50, 325);
	moveWindow(backgroundWindowTitle, 375, 50);
	moveWindow(diffWindowTitle, 375, 325);

	// Main program loop
	for (;;) {

		// Read next frame from the video input
		if (!videoCapture.read(frame)) {
			cout << " --> No more frames available for " << argv[1] << "" << endl;
			cout << " --> Program ended" << endl;
			return 0;
		}

		// Convert to gray and blur the frame
		cvtColor(frame, frameGray, CV_BGR2GRAY);
		blur(frameGray, frameGray, Size(3, 3));

	    // Find the difference between the gray frame and the background
	    absdiff(frameGray, background, diff);
		threshold(diff, diff, 75, 255, THRESH_BINARY);

		// Remove noise using erode() function
		Mat structElementSquare = getStructuringElement(MORPH_RECT, Size(2, 2));
		erode(diff, diff, structElementSquare, Point(-1, -1), 1);

		// Remove gaps inside the objects found
		Mat structElementRect = getStructuringElement(MORPH_RECT, Size(3, 5));
		dilate(diff, diff, structElementRect, Point(-1, -1), 15);
		erode(diff, diff, structElementRect, Point(-1, -1), 10);

		// Make a copy of the diff Mat, as it will be modified when applying findContours()
		diff_clone = diff.clone();

		// Find contours, get bounding rects and track objects in the frame
		findContours(diff, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		vector<vector<Point> > contours_poly(contours.size());
		vector<Rect> boundRect(contours.size());
		vector<Point2f> center(contours.size());
		vector<float>radius(contours.size());

		if (contours.size() == 0) {
			frameObjects.clear();
		}
		if (contours.size() != frameObjects.size()) {
			frameObjects.clear();
		}

		for (int i = 0; i < contours.size(); i++) {
			approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
			boundRect[i] = boundingRect(Mat(contours_poly[i]));
			minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
			int area = boundRect[i].width * boundRect[i].height;

			if (area > 2500 || area < 1000) {
				break;
			}

			bool found = false;
			
			for (int j = 0; j < frameObjects.size(); j++) {
				int dX = abs(center[i].x - frameObjects[j].centerX);
				int dY = abs(center[i].y - frameObjects[j].centerY);
				int dArea = abs(area - frameObjects[j].area);
					
				if (dX <= 100 && dY <= 100 && dArea <= 1000 ) {
					frameObjects[j].centerX = center[i].x;
					frameObjects[j].centerY = center[i].y;
					frameObjects[j].area = area;
					frameObjects[j].rect = boundRect[i];
					found = true;
					break;
				}
			}
			if (!found) {
				Scalar color = Scalar(rng.uniform(100, 255), rng.uniform(100,255), rng.uniform(100,255));
				FrameObject currentObject(frameObjects.size(), center[i].x, center[i].y, area, boundRect[i], color);
				frameObjects.push_back(currentObject);
			}
		}

		// Draw bounding rects and numbers around detected objects
		for (int i = 0; i < frameObjects.size(); i++) {
			rectangle(frame, frameObjects[i].rect.tl(), frameObjects[i].rect.br(), frameObjects[i].color, 1, 8, 0);
			rectangle(diff_clone, frameObjects[i].rect.tl(), frameObjects[i].rect.br(), frameObjects[i].color, 1, 8, 0);
			putText(frame, to_string(frameObjects[i].id), Point(frameObjects[i].centerX - 5, frameObjects[i].centerY), FONT_HERSHEY_SIMPLEX, 0.75, frameObjects[i].color, 1);
			putText(diff_clone, to_string(frameObjects[i].id), Point(frameObjects[i].centerX - 5, frameObjects[i].centerY), FONT_HERSHEY_SIMPLEX, 0.75, frameObjects[i].color, 1);
		}

		// Display images
		imshow(frameWindowTitle, frame);
		imshow(frameGrayWindowTitle, frameGray);
		imshow(backgroundWindowTitle, background);
		imshow(diffWindowTitle, diff_clone);

		// Wait for user input in order to finish the program
		if (waitKey(25) >= 0) {
			return 1;
		}
	}

	return 1;
}
