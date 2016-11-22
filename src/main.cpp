
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

	Mat mat_frame, mat_frameGray, mat_background, mat_diff;
	const char *frameWindowTitle = "frame", *frameGrayWindowTitle = "frameGray";
	const char *backgroundWindowTitle = "background", *diffWindowTitle = "diff";

	VideoCapture videoCapture(argv[1]);
	if (!videoCapture.isOpened()) {
		cerr << " --> Could not initialize capturing for file " << argv[1] << "" << endl;
		cerr << " --> Program ended with errors" << endl;
		return 0;
	}

	videoCapture >> mat_frame;

	cvtColor(mat_frame, mat_background, CV_BGR2GRAY);
	for (int i = 1; i < 4; i = i + 2) {
		blur(mat_background, mat_background, Size(i, i), Point(-1, -1));
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

		if (!videoCapture.read(mat_frame)) {
			cout << " --> No more frames available for " << argv[1] << "" << endl;
			cout << " --> Program ended" << endl;
			return 0;
		}

		for (int i = 1; i < 4; i = i + 2) {
			blur(mat_frame, mat_frame, Size(i, i), Point(-1, -1));
		}
		cvtColor(mat_frame, mat_frameGray, CV_BGR2GRAY);

	    absdiff(mat_frameGray, mat_background, mat_diff);
		threshold(mat_diff, mat_diff, 75, 255, THRESH_BINARY);

		Mat structElementSquare = getStructuringElement(MORPH_RECT, Size(2, 2));
		erode(mat_diff, mat_diff, structElementSquare, Point(-1, -1), 1);

		Mat structElementRect = getStructuringElement(MORPH_RECT, Size(3, 5));
		dilate(mat_diff, mat_diff, structElementRect, Point(-1, -1), 15);
		erode(mat_diff, mat_diff, structElementRect, Point(-1, -1), 10);

		imshow(frameWindowTitle, mat_frame);
		imshow(frameGrayWindowTitle, mat_frameGray);
		imshow(backgroundWindowTitle, mat_background);
		imshow(diffWindowTitle, mat_diff);

		if (waitKey(10) >= 0) {
			return 1;
		}
	}

	return 1;
}
