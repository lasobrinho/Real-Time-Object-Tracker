
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {

	// Check if the user input is correct to proceed
	if (argc != 2) {
		cerr << endl;
		cerr << " --> Wrong arguments list" << endl;
		cerr << " --> Usage ./RealTimeObjectTracker [video_path]" << endl << endl;
		return 0;
	}

	// Initialize program variables
	Mat frame, frameGray, background, diff;
	const char *frameWindowTitle = "frame", *frameGrayWindowTitle = "frameGray";
	const char *backgroundWindowTitle = "background", *diffWindowTitle = "diff";

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



		// Display images
		imshow(frameWindowTitle, frame);
		imshow(frameGrayWindowTitle, frameGray);
		imshow(backgroundWindowTitle, background);
		imshow(diffWindowTitle, diff);

		// Wait for user input in order to finish the program
		if (waitKey(10) >= 0) {
			return 1;
		}
	}

	return 1;
}
