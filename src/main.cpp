
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
    vector<Point> trajectory;
    FrameObject (int _id, int _centerX, int _centerY, int _area, Rect _rect, Scalar _color) : id(_id), centerX(_centerX), centerY(_centerY), area(_area), rect(_rect), color(_color) {};
}
FrameObject;

typedef struct Background
{
    Mat bgFrame;
    vector<vector<int> > ttl;
    Background (Mat bgF) : bgFrame(bgF) {};
}
Background;

// Applies blur to a specific Mat pointer
// Repeats the process accordingly to the iterations parameter value
void applyBlur(Mat *frame, int iterations) {
	for (int i = 1; i <= iterations; i++) {
		blur(*frame, *frame, Size(3, 3));
	}
}

void removeNoise(Mat *diff, Background *bg) {
	int rows = diff->rows;
	int cols = diff->cols;
	
	for(int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (diff->at<uchar>(i-1, j-1) == 0 && diff->at<uchar>(i, j-1) == 0 && diff->at<uchar>(i+1, j-1) == 0
				&&
				diff->at<uchar>(i-1, j)   == 0 && diff->at<uchar>(i, j) == 255 && diff->at<uchar>(i+1, j) 	== 0
				&&
				diff->at<uchar>(i-1, j+1) == 0 && diff->at<uchar>(i, j+1) == 0 && diff->at<uchar>(i+1, j+1) == 0) 
			{
				diff->at<uchar>(i, j) = 0;
			}
		}
	}

	int threshold = 25;

	for(int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {

			if (bg->ttl.at(i).at(j) <= threshold) {
				if (diff->at<uchar>(i, j) == 255) {
					bg->ttl.at(i).at(j) = bg->ttl.at(i).at(j) + (threshold / 10);
					if (bg->ttl.at(i).at(j) > threshold) {
						diff->at<uchar>(i, j) = 0;
					}
				} else {
					bg->ttl.at(i).at(j)--;
				}			
			} else {
				if (bg->ttl.at(i).at(j) < (threshold * threshold)) {
					bg->ttl.at(i).at(j)++;
					diff->at<uchar>(i, j) = 0;
				} else {
					bg->ttl.at(i).at(j) = 0;
				}
			}
		}
	}

	for(int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (diff->at<uchar>(i-1, j-1) == 0 && diff->at<uchar>(i, j-1) == 0 && diff->at<uchar>(i+1, j-1) == 0
				&&
				diff->at<uchar>(i-1, j)   == 0 && diff->at<uchar>(i, j) == 255 && diff->at<uchar>(i+1, j) 	== 0
				&&
				diff->at<uchar>(i-1, j+1) == 0 && diff->at<uchar>(i, j+1) == 0 && diff->at<uchar>(i+1, j+1) == 0) 
			{
				diff->at<uchar>(i, j) = 0;
			}
		}
	}
}

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
	Mat test_mat;
	const char *frameWindowTitle = "frame", *frameGrayWindowTitle = "frameGray";
	const char *backgroundWindowTitle = "background", *diffWindowTitle = "diff";
	const char *testWindowTitle = "test";
	vector<vector<Point> > contours;
  	vector<Vec4i> hierarchy;
  	RNG rng(time(NULL));
  	vector<FrameObject> frameObjects;
  	int delayTime;
  	int frameCounter = 0;

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
	background.convertTo(background, -1, 1.5, 0);
	applyBlur(&background, 1);

	Background bg_test(background);
	vector<vector<int> > ttl_temp(background.rows, vector<int>(background.cols));
	bg_test.ttl = ttl_temp;

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
		frameGray.convertTo(frameGray, -1, 1.5, 0);
		applyBlur(&frameGray, 1);

	    // Find the difference between the gray frame and the background
	    absdiff(frameGray, background, diff);
		threshold(diff, diff, 60, 255, THRESH_BINARY);

		removeNoise(&diff, &bg_test);
		//adaptativeBackground(&diff, &bg_test);

		// Remove noise using erode() function
		Mat structElementSquare = getStructuringElement(MORPH_RECT, Size(2, 2));
		erode(diff, diff, structElementSquare, Point(-1, -1), 2);

		// Remove gaps inside the objects found
		Mat structElementRect = getStructuringElement(MORPH_RECT, Size(3, 5));
		dilate(diff, diff, structElementRect, Point(-1, -1), 10);
		erode(diff, diff, structElementRect, Point(-1, -1), 7);

		// Make a copy of the diff Mat, as it will be modified when applying findContours()
		diff_clone = diff.clone();

		// Find contours, get bounding rects and track objects in the frame
		findContours(diff, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		vector<vector<Point> > contours_poly(contours.size());
		vector<Rect> boundRect(contours.size());
		vector<Point2f> center(contours.size());
		vector<float> radius(contours.size());

		int dXYThreshold = 25;

		for (int i = 0; i < contours.size(); i++) {
			approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
			boundRect[i] = boundingRect(Mat(contours_poly[i]));
			minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
			int area = boundRect[i].width * boundRect[i].height;
				
			bool found = false;

			for (int j = 0; j < frameObjects.size(); j++) {
				int dX = abs(center[i].x - frameObjects[j].centerX);
				int dY = abs(center[i].y - frameObjects[j].centerY);
				int dArea = abs(area - frameObjects[j].area);

				if (dX <= dXYThreshold && dY <= dXYThreshold) {
					frameObjects[j].centerX = center[i].x;
					frameObjects[j].centerY = center[i].y;
					frameObjects[j].area = area;
					frameObjects[j].rect = boundRect[i];
					frameObjects[j].trajectory.push_back(center[i]);
					found = true;
					break;
				}
			}
			if (!found) {
				Scalar color = Scalar(rng.uniform(127, 255), rng.uniform(127,255), rng.uniform(127,255));
				FrameObject currentObject(rng.uniform(0, 100), center[i].x, center[i].y, area, boundRect[i], color);
				frameObjects.push_back(currentObject);
			}			
		}

		// Clear frameObjects vector if there are no objects in the frame
		if (contours.size() == 0) {
			frameObjects.clear();
		}

		// Check for objects that left the scene and stop tracking them
		if (contours.size() != frameObjects.size()) {
			for (int i = 0; i < contours.size(); i++) {
				approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
				boundRect[i] = boundingRect(Mat(contours_poly[i]));
				minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
				int area = boundRect[i].width * boundRect[i].height;
				
				for (int j = 0; j < frameObjects.size(); j++) {
					int dX = abs(center[i].x - frameObjects[j].centerX);
					int dY = abs(center[i].y - frameObjects[j].centerY);
					int dArea = abs(area - frameObjects[j].area);
										
					if (!(dX <= dXYThreshold && dY <= dXYThreshold)) {
						frameObjects.erase(frameObjects.begin() + j);
					}
				}
			}
		}

		// Draw bounding rects and numbers around detected objects
		cvtColor(diff_clone, diff_clone, CV_GRAY2RGB);
		for (int i = 0; i < frameObjects.size(); i++) {
			rectangle(frame, frameObjects[i].rect.tl(), frameObjects[i].rect.br(), frameObjects[i].color, 1, 8, 0);
			rectangle(diff_clone, frameObjects[i].rect.tl(), frameObjects[i].rect.br(), frameObjects[i].color, 1, 8, 0);
			putText(frame, to_string(frameObjects[i].id), Point(frameObjects[i].centerX - 5, frameObjects[i].centerY - 40), FONT_HERSHEY_COMPLEX_SMALL, 0.7, frameObjects[i].color, 1);
			putText(diff_clone, to_string(frameObjects[i].id), Point(frameObjects[i].centerX - 5, frameObjects[i].centerY - 40), FONT_HERSHEY_COMPLEX_SMALL, 0.7, frameObjects[i].color, 1);
			for (int j = 0; j < frameObjects[i].trajectory.size(); j++) {
				line(frame, frameObjects[i].trajectory[j], frameObjects[i].trajectory[j], frameObjects[i].color, 1, 8);
				line(diff_clone, frameObjects[i].trajectory[j], frameObjects[i].trajectory[j], frameObjects[i].color, 1, 8);
			}
			
		}

		// Display images
		imshow(frameWindowTitle, frame);
		imshow(frameGrayWindowTitle, frameGray);
		imshow(backgroundWindowTitle, background);
		imshow(diffWindowTitle, diff_clone);
		//imshow(testWindowTitle, test_mat);

		// Set delay between frames based whether the frame has objects or not
		if (frameObjects.size() > 0) {
			delayTime = 5;
		} else {
			delayTime = 2;
		}

		// Wait for user input in order to finish the program
		if (waitKey(delayTime) >= 0) {
			return 1;
		}

		frameCounter++;

	}

	return 1;
}
