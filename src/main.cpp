
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{

	if (argc != 2) {
		cerr << endl;
		cerr << " --> Wrong argument list" << endl;
		cerr << " --> Usage ./RealTimeObjectTracker [video_path]" << endl << endl;
		return 0;
	}

	Mat frame, frameGray, background, diff;
	const char *frameWindowTitle = "frame", *frameGrayWindowTitle = "frameGray";
	const char *backgroundWindowTitle = "background", *diffWindowTitle = "diff";

	VideoCapture videoCapture(argv[1]);
	if (!videoCapture.isOpened()) {
		cerr << " --> Could not initialize capturing for file " << argv[1] << "" << endl;
		cerr << " --> Program ended with errors" << endl;
		return 0;
	}

	videoCapture >> frame;

	cvtColor(frame, background, CV_BGR2GRAY);
	for (int i = 1; i < 4; i = i + 2) {
		blur(background, background, Size(i, i), Point(-1, -1));
	}

	namedWindow(frameWindowTitle, CV_WINDOW_AUTOSIZE);
	namedWindow(frameGrayWindowTitle, CV_WINDOW_AUTOSIZE);
	namedWindow(backgroundWindowTitle, CV_WINDOW_AUTOSIZE);
	namedWindow(diffWindowTitle, CV_WINDOW_AUTOSIZE);

	moveWindow(frameWindowTitle, 50, 50);
	moveWindow(frameGrayWindowTitle, 50, 325);
	moveWindow(backgroundWindowTitle, 375, 50);
	moveWindow(diffWindowTitle, 375, 325);

	for (;;) {

		if (!videoCapture.read(frame)) {
			cout << " --> No more frames available for " << argv[1] << "" << endl;
			cout << " --> Program ended" << endl;
			return 0;
		}

		for (int i = 1; i < 4; i = i + 2) {
			blur(frame, frame, Size(i, i), Point(-1, -1));
		}
		cvtColor(frame, frameGray, CV_BGR2GRAY);

	    absdiff(frameGray, background, diff);
		threshold(diff, diff, 75, 255, THRESH_BINARY);

		Mat structElementSquare = getStructuringElement(MORPH_RECT, Size(2, 2));
		erode(diff, diff, structElementSquare, Point(-1, -1), 1);

		Mat structElementRect = getStructuringElement(MORPH_RECT, Size(3, 5));
		dilate(diff, diff, structElementRect, Point(-1, -1), 15);
		erode(diff, diff, structElementRect, Point(-1, -1), 10);

		imshow(frameWindowTitle, frame);
		imshow(frameGrayWindowTitle, frameGray);
		imshow(backgroundWindowTitle, background);
		imshow(diffWindowTitle, diff);

		if (waitKey(10) >= 0) {
			return 1;
		}
	}

	return 1;
}
