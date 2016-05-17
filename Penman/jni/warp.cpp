#include <jni.h>

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <math.h>

#include "opencv2/core/core.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/nonfree/nonfree.hpp"

#include <android/log.h> 

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "libnav", __VA_ARGS__) 
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "libnav", __VA_ARGS__) 
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "libnav", __VA_ARGS__) 
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "libnav", __VA_ARGS__) 
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "libnav", __VA_ARGS__) 
#define UP    		0
#define DOWN  		1
#define LEFT  		2
#define RIGHT 		3
#define NODIRECTION 4

using namespace std;
using namespace cv;

using namespace std;
using namespace cv;

void LogInt(int i){
    char buffer [50];
	sprintf (buffer, "Logint: %d", i);
	LOGV(buffer);	
}



/**
 * Makes a rectangle from a keypoint, with the dimensions adjusted to the
 * destination image.
 */
void makeRectFromKeyPoint(KeyPoint kp, Rect &rect, Mat &img) {
	rect = Rect((int) kp.pt.x - kp.size / 2.0, (int) (kp.pt.y - kp.size / 2.0),
			(int) kp.size, (int) kp.size);

	if (rect.x < 0) {
		rect.x = 0;

	}
	if (rect.y < 0) {
		rect.y = 0;

	}
	if (rect.x + rect.width >= img.cols) {
		rect.width =
				(img.cols - rect.x < rect.height) ?
						img.cols - rect.x : rect.height;
		rect.height =
				(img.cols - rect.x < rect.height) ?
						img.cols - rect.x : rect.height;

	}
	if (rect.y + rect.height >= img.rows) {
		rect.width =
				(img.rows - rect.y < rect.width) ?
						img.rows - rect.y : rect.width;
		rect.height =
				(img.rows - rect.y < rect.width) ?
						img.rows - rect.y : rect.width;

	}

}


int findUpper(KeyPoint a, KeyPoint b, Mat image) {
	int result = 0;
	Rect rect[2];
	makeRectFromKeyPoint(a, rect[0], image);
	makeRectFromKeyPoint(b, rect[1], image);
	for (int i = 0; i < 2; i++) {
		Mat A(image, rect[i]);

		int erosion_size = 2;
		int erosion_type = MORPH_RECT;
		Mat element = getStructuringElement(erosion_type,
				Size(2 * erosion_size + 1, 1), Point(erosion_size, 0));
		morphologyEx(A, A, MORPH_CLOSE, element);

		adaptiveThreshold(A, A, 128, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY,
				5, 5);
		bool b = false;
		for (int i = 0; i < A.rows; i++) {
			for (int j = 0; j < A.cols; j++) {
				if (A.at<bool>(i, j) != 1) {
					b = true;
					result =
							(A.rows - i > A.rows - result) ?
									A.rows - i : A.rows - result;
					break;
				}
			}
			if (b) {
				break;
			}
		}

	}

	return result;
}
int findLower(KeyPoint a, KeyPoint b, Mat image) {
	int result = 0;
	Rect rect[2];
	makeRectFromKeyPoint(a, rect[0], image);
	makeRectFromKeyPoint(b, rect[1], image);
	for (int i = 0; i < 2; i++) {
		Mat A(image, rect[i]);

		int erosion_size = 2;
		int erosion_type = MORPH_RECT;
		Mat element = getStructuringElement(erosion_type,
				Size(2 * erosion_size + 1, 1), Point(erosion_size, 0));
		morphologyEx(A, A, MORPH_CLOSE, element);

		adaptiveThreshold(A, A, 128, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY,
				5, 5);
		bool b = false;
		for (int i = A.rows - 1; i >= 0; i--) {
			for (int j = 0; j < A.cols; j++) {
				if (A.at<bool>(i, j) != 1) {
					b = true;

					result = (i > result) ? i : result;
					break;
				}
			}
			if (b) {
				break;
			}
		}

	}

	return result;
}

void removeEmptyBorder(Mat &triangle, Mat &newtriangle) {
	int countrows[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int countcols[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int first[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int last[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int firsty[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int lasty[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	bool found[16] = { false, false, false, false, false, false, false, false,
			false, false, false, false, false, false, false, false };

	int cutup = 0;
	int cutdown = 0;
	int cutleft = 0;
	int cutright = 0;

	for (int i = 0; i < triangle.rows; i++) {
		bool firstfound = false;
		for (int j = 0; j < triangle.cols; j++) {
			if (triangle.at<bool>(i, j) != 1) {
				countcols[j]++;
				countrows[i]++;
				if (!firstfound) {
					firstfound = true;
					first[i] = j;
				}
				if (!found[j]) {
					found[j] = true;
					firsty[j] = i;
				}
				lasty[j] = i;
				last[i] = j;
			}
		}
	}
	int limit = 1;

	for (int i = 0; i < 16; i++) {
		if (countcols[i] >= limit) {
			break;
		} else {
			cutleft++;
		}
	}
	for (int i = 15; i >= 0; i--) {
		if (countcols[i] >= limit) {
			break;
		} else {
			cutright++;
		}
	}

	for (int i = 0; i < 16; i++) {
		if (countrows[i] >= limit) {
			break;
		} else {
			cutup++;
		}
	}
	for (int i = 0; i < 16; i++) {
		if (countrows[i] >= limit) {
			break;
		} else {
			cutdown++;
		}
	}
	Mat tmp = triangle.clone();
	for (int i = 0; i < tmp.rows; i++) {
		for (int j = 0; j < 16; j++) {
			if (j > first[i] && j < last[i]) {
				tmp.at<bool>(i, j) = 0;
			} else if (i > firsty[j] && i < lasty[j]) {
				tmp.at<bool>(i, j) = 0;
			}

		}
	}

	newtriangle = triangle;
	Rect rect = cvRect(cutleft, cutup, 16 - cutleft - cutright,
			16 - cutdown - cutup);
	if (rect.width > 0 && rect.height > 0) {
		Mat A(tmp, rect);
		if (A.size().area() > 0) {
			resize(A, newtriangle, cvSize(16, 16), 0, 0, 0);
		}
	}
}

bool checkQuadrants(Mat &triangle, int &direction, int quads[4],
		int borders[4]) {
	Mat newtriangle;
	removeEmptyBorder(triangle, newtriangle);

	triangle = newtriangle;

	memset(quads, 0, sizeof(int) * 4);
	memset(borders, 0, sizeof(int) * 4);

	bool result = false;

	/* Quads:
	 *  #---------------#
	 *  |       |       |
	 *  |   0   |   1   |
	 *  |       |       |
	 *  |-------|-------|
	 *  |       |       |
	 *  |   2   |   3   |
	 *  |       |       |
	 *  #---------------#
	 *
	 *
	 */

	/* Borders:
	 *  #---------------#
	 *  |*|     0     |*|
	 *  |---------------|
	 *  | |           | |
	 *  |3|           |1|
	 *  | |           | |
	 *  |---------------|
	 *  |*|     2     |*|
	 *  #---------------#
	 *
	 *
	 */

	int total = 0;
	for (int i = 0; i < triangle.rows; i++) {
		for (int j = 0; j < triangle.cols; j++) {
			// quadrant upper, left
			//cout << triangle.type();
			int y = triangle.at<bool>(i, j);
			if (triangle.at<bool>(i, j) != 1) {
				if (i < (triangle.rows / 2) && j < 8) {
					quads[0]++;
				}
				// quadrant upper, right
				else if (i < (triangle.rows / 2) && j >= 8) {
					quads[1]++;
				}
				// quadrant lower, left
				else if (i >= (triangle.rows / 2) && j < 8) {
					quads[2]++;
				}
				// quadrant lower, right
				else {
					quads[3]++;
				}

				if (i < 4 && j > 3 && j < 12) {
					borders[DOWN]++;
				}
				// quadrant upper, right
				else if (j > 11 && i > 3 && i < 12) {
					borders[LEFT]++;
				}
				// quadrant lower, left
				else if (i > 11 && j > 3 && j < 12) {
					borders[UP]++;
				}
				// quadrant lower, right
				else if (j < 4 && i > 3 && i < 12) {
					borders[RIGHT]++;
				}

				total++;
			}
		}
	}

	int up = quads[0] + quads[1];
	int right = quads[1] + quads[3];
	int down = quads[2] + quads[3];
	int left = quads[0] + quads[2];

	quads[DOWN] = up;
	quads[UP] = down;
	quads[LEFT] = right;
	quads[RIGHT] = left;

	LOGD("Results: ");
	for(int i = 0; i < 4; i++){
		LOGD("Border: ");
		LogInt(borders[i]);
		LOGD("Quads: ");
		LogInt(quads[i]);

	}


	int h = 0;
	int hindex = -1;
	for (int i = 0; i < 4; i++) {
		if (borders[i] > h) {
			h = borders[i];
			hindex = i;
		}
	}

	float compare = 7.7 / 18.0;
	if ((up < right && up < down && up < left)) {
		if ((compare) * total > up) {
			result = true;

		}
		direction = UP;

	} else if (right < up && right < down && right < left) {
		if ((compare) * total > right) {
			result = true;

		}
		direction = RIGHT;
	} else if (down < up && down < right && down < left) {
		if ((compare) * total > down) {
			result = true;

		}
		direction = DOWN;
	} else if (left < up && left < right && left < down) {
		if ((compare) * total > left) {
			result = true;

		}
		direction = LEFT;
	} else {
		result = false;
		direction = NODIRECTION;
	}
	if (hindex == 0) {
		direction = DOWN;
	} else if (hindex == 1) {
		direction = LEFT;
	} else if (hindex == 2) {
		direction = UP;
	} else if (hindex == 3) {
		direction = RIGHT;
	}
	quads[DOWN] = up;
	quads[UP] = down;
	quads[LEFT] = right;
	quads[RIGHT] = left;


	return result;
}

void bubbleSort(double num[4], KeyPoint newpts[4]) {
	int i, j, flag = 1; // set flag to 1 to start first pass
	double temp; // holding variable
	KeyPoint temp_p;
	int numLength = 4;
	for (i = 1; (i <= numLength) && flag; i++) {
		flag = 0;
		for (j = 0; j < (numLength - 1); j++) {
			if (num[j + 1] > num[j]) {

				temp = num[j]; // swap elements
				temp_p = newpts[j];

				num[j] = num[j + 1];
				newpts[j] = newpts[j + 1];

				num[j + 1] = temp;
				newpts[j + 1] = temp_p;

				flag = 1; // indicates that a swap occurred.
			}
		}
	}
}

void putInOrder(vector<KeyPoint> old, KeyPoint newpts[4]) {
	double atan2results[4];

	double avx = 0;
	double avy = 0;

	for (int i = 0; i < 4; i++) {
		avx += old[i].pt.x;
		avy += old[i].pt.y;
	}
	avx /= 4;
	avy /= 4;

	Point2d temp[4];
	for (int i = 0; i < 4; i++) {
		temp[i] = Point2f(old[i].pt.x - avx, old[i].pt.y - avy);
	}

	for (int i = 0; i < 4; i++) {
		atan2results[i] = atan2((double) temp[i].y, (double) temp[i].x);
	}
	bubbleSort(atan2results, newpts);

}
void putInOrder(KeyPoint old[4]) {
	double atan2results[4];

	double avx = 0;
	double avy = 0;

	for (int i = 0; i < 4; i++) {
		avx += old[i].pt.x;
		avy += old[i].pt.y;
	}
	avx /= 4;
	avy /= 4;

	Point2d temp[4];
	for (int i = 0; i < 4; i++) {
		temp[i] = Point2f(old[i].pt.x - avx, old[i].pt.y - avy);
	}

	for (int i = 0; i < 4; i++) {
		atan2results[i] = atan2((double) temp[i].y, (double) temp[i].x);
	}
	bubbleSort(atan2results, old);

}

void warpField(vector<KeyPoint> kps, Mat displayimage, Mat searchimage, Mat &result, Mat &result_color, int &recwidth) {
	
	LOGD("Dimensions: ");
	LogInt(displayimage.cols);
	LogInt(displayimage.rows);

	int size = 0;

	for (unsigned int i = 0; i < kps.size(); i++) {

		if (kps[i].size > size) {
			size = kps[i].size;
		}

	}

	/*

	 * a----------b
	 * |          |
	 * |          |
	 * |          |
	 * c----------d
	 *


	/*  a----------b
	 *  |          |
	 *  |          |
	 *  |          |
	 *  d----------c
	 */
	KeyPoint origional[4] = { kps[0], kps[1], kps[2], kps[3] };

	putInOrder(kps, origional);

	//Point2f org_pts[4] = { a.pt, b.pt, c.pt, d.pt };

	/*	int width = ((((int) abs(b.pt.x - a.pt.x) << 1)
	 + ((int) abs(b.pt.y - a.pt.y) << 1)) >> 1);
	 int height = ((((int) abs(c.pt.x - a.pt.x) << 1)
	 + ((int) abs(c.pt.y - a.pt.y) << 1)) >> 1);*/

	int dxw = (origional[1].pt.x - origional[0].pt.x);
	int dyw = (origional[1].pt.y - origional[0].pt.y);
	int dxh = (origional[2].pt.x - origional[1].pt.x);
	int dyh = (origional[2].pt.y - origional[1].pt.y);

	float height = (float) sqrt((double) (dxw * dxw + dyw * dyw));
	float width = (float) sqrt((double) (dxh * dxh + dyh * dyh));

	cout << width << ", " << height << '\n';

	float margin = size / 2;

	Point2f new_pts_with_triangles[4] = { Point2f(width + margin,
			height + margin), Point2f(margin + width, margin), Point2f(margin,
			margin), Point2f(margin, margin + height) };

	/*	Point2f new_pts_wo_triangles[4] = {
	 Point2f(width - margin, height - margin),
	 Point2f(-margin + width, -margin),
	 Point2f(-margin, -margin),
	 Point2f(-margin, -margin + height) };*/

	for (int i = 0; i < 4; i++) {
		cout << "x: " << new_pts_with_triangles[i].x << ", y: "
				<< new_pts_with_triangles[i].y << "\n";
	}
	Point2f org_pts[4] = { origional[0].pt, origional[1].pt, origional[2].pt,
			origional[3].pt };

	/*	Mat perspective_matrix_wo_triangles = getPerspectiveTransform(org_pts,
	 new_pts_wo_triangles);*/
	Mat perspective_matrix = getPerspectiveTransform(org_pts,
			new_pts_with_triangles);

	Mat dst_img;
	dst_img = Scalar::all(0);
	Mat dst_img_color;
	dst_img_color = Scalar::all(0);

	warpPerspective(searchimage, dst_img, perspective_matrix,
			Size(width + (2 * margin), height + (2 * margin)),
			cv::INTER_LANCZOS4);

	warpPerspective(displayimage, dst_img_color, perspective_matrix,
			Size(width + (2 * margin), height + (2 * margin)),
			cv::INTER_LANCZOS4);

	/*	warpPerspective(displayimage, dst_img_wo, perspective_matrix_wo_triangles,
	 Size(width - (2 * margin), height - (2 * margin)),
	 cv::INTER_LANCZOS4);*/

	KeyPoint newkps[4];
	int votes[5] = { 0, 0, 0, 0, 0 };
	for (int i = 0; i < 4; i++) {

		newkps[i] = KeyPoint(new_pts_with_triangles[i], origional[i].size);
		cout << "dst.cols: " << dst_img.cols << ", dst.rows: " << dst_img.rows
				<< "\n";
		cout << "x: " << newkps[i].pt.x << ", y: " << newkps[i].pt.y
				<< ", size: " << newkps[i].size << "\n";

		/*		Rect rect = cvRect((int)newkps[i].pt.x - ((int) newkps[i].size / 2),
		 (int)newkps[i].pt.y - ((int) newkps[i].size / 2),
		 );*/
		Rect rect = Rect((int) newkps[i].pt.x - newkps[i].size / 2.0,
				(int) (newkps[i].pt.y - newkps[i].size / 2.0),
				(int) newkps[i].size, (int) newkps[i].size);

		if (rect.x < 0) {
			rect.x = 0;

		}
		if (rect.y < 0) {
			rect.y = 0;

		}
		if (rect.x + rect.width >= dst_img.cols) {
			rect.width =
					(dst_img.cols - rect.x < rect.height) ?
							dst_img.cols - rect.x : rect.height;
			rect.height =
					(dst_img.cols - rect.x < rect.height) ?
							dst_img.cols - rect.x : rect.height;

		}
		if (rect.y + rect.height >= dst_img.rows) {
			rect.width =
					(dst_img.rows - rect.y < rect.width) ?
							dst_img.rows - rect.y : rect.width;
			rect.height =
					(dst_img.rows - rect.y < rect.width) ?
							dst_img.rows - rect.y : rect.width;

		}

		Mat A(dst_img, rect);

		Mat E;
		resize(A, E, cvSize(16, 16), 0, 0, 0);

		int erosion_size = 2;
		int erosion_type = MORPH_RECT;

		Mat element = getStructuringElement(erosion_type,
				Size(2 * erosion_size + 1, 1), Point(erosion_size, 0));

		//erode(temp, erosion_dst, element);
		morphologyEx(E, E, MORPH_CLOSE, element);

		//blur(D, D, Size(3, 3));
		//threshold(B, C, 128, 255, THRESH_BINARY);
		adaptiveThreshold(E, E, 128, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY,
				5, 5);

		int direction;
		int borders[4] = { 0, 0, 0, 0 };
		int quads[4] = { 0, 0, 0, 0 };
		checkQuadrants(E, direction, quads, borders);
		for (int i = 0; i < 4; i++) {
			votes[i] += quads[i] + 2 * borders[i];
		}

	}

	// Find highest:
	int index_highest = -1;
	int highest = -1;
	for (int i = 0; i < 4; i++) {
		LOGD("Value for direction ");
		LogInt(i);
		LOGD(":");
		LogInt(votes[i]);
		if (votes[i] > highest) {
			index_highest = i;
			highest = votes[i];
		}
	}
	Mat dst;
	dst = Scalar::all(0);
	Mat dst_color;
	dst_color = Scalar::all(0);

	cout << index_highest << "\n";
	KeyPoint rotatedkps[4];
	Mat rot_mat;
	if (index_highest == DOWN) {
		Point2f src_center(dst_img.cols / 2.0F, dst_img.rows / 2.0F);
		rot_mat = getRotationMatrix2D(src_center, 180, 1.0);

		warpAffine(dst_img, dst, rot_mat, dst_img.size());
		warpAffine(dst_img_color, dst_color, rot_mat, dst_img.size());
	} else if (index_highest == LEFT) {
		Point2f src_center(dst_img.cols / 2.0F, dst_img.rows / 2.0F);
		rot_mat = getRotationMatrix2D(src_center, -90, 1.0);

		rot_mat.at<double>(0, 2) = (double) dst_img.rows;
		rot_mat.at<double>(1, 2) = (double) 0.0;

		warpAffine(dst_img, dst, rot_mat, Size(dst_img.rows, dst_img.cols));
		warpAffine(dst_img_color, dst_color, rot_mat,
				Size(dst_img.rows, dst_img.cols));

		//warpAffine(dst_img, dst, rot_mat, Size(dst_img.rows, dst_img.cols));
	} else if (index_highest == RIGHT) {
		//rotatedkps

		Point2f src_center(dst_img.cols / 2.0F, dst_img.rows / 2.0F);
		rot_mat = getRotationMatrix2D(src_center, 90, 1.0);

		rot_mat.at<double>(0, 2) = (double) 0.0;
		rot_mat.at<double>(1, 2) = (double) dst_img.cols;

		warpAffine(dst_img, dst, rot_mat, Size(dst_img.rows, dst_img.cols));
		warpAffine(dst_img_color, dst_color, rot_mat,
				Size(dst_img.rows, dst_img.cols));
	} else {

		dst = dst_img;
		dst_color = dst_img_color;
	}
	if (index_highest != UP) {
		for (int i = 0; i < 4; i++) {
			double nx = rot_mat.at<double>(0, 0) * newkps[i].pt.x
					+ rot_mat.at<double>(0, 1) * newkps[i].pt.y
					+ rot_mat.at<double>(0, 2);
			double ny = rot_mat.at<double>(1, 0) * newkps[i].pt.x
					+ rot_mat.at<double>(1, 1) * newkps[i].pt.y
					+ rot_mat.at<double>(1, 2);

			rotatedkps[i] = KeyPoint(Point(nx, ny), newkps[i].size);

			cout << "nx: " << rotatedkps[i].pt.x << ", ny: "
					<< rotatedkps[i].pt.y << "\n";
			//drawPoint(dst, rotatedkps[i], Scalar(0,0,255));
		}
		// So that we know which points are on the lower side of the rectangle and which are on the upper side.
		putInOrder(rotatedkps);
	} else {
		for (int i = 0; i < 4; i++) {
			rotatedkps[i] = newkps[i];
		}
		putInOrder(rotatedkps);

	}
	int avsize = 0;
	for (int i = 0; i < 4; i++) {
		cout << rotatedkps[i].pt << endl;
		avsize += rotatedkps[i].size;
	}
	avsize /= 4;

	int bottomborder = findUpper(rotatedkps[0], rotatedkps[1], dst.clone());
	int topborder = findLower(rotatedkps[2], rotatedkps[3], dst.clone());

	Rect rect = Rect(0, topborder, dst.cols,
			dst.rows - topborder - bottomborder);



	Mat dst_img_wo(dst, rect);
	Mat dst_img_wo_color(dst_color, rect);



	result = dst_img_wo.clone();
	result_color = dst_img_wo_color.clone();
	recwidth = avsize;

}



/**
 * Draws a list of keypoints.
 */
void drawPoints(Mat &image, std::vector<KeyPoint> points, Scalar color) {
	for (unsigned int i = 0; i < points.size(); i++) {
		LOGD("Drawpoints:");
		LOGD("Keypoint size:");
		LogInt((int)points[i].size);
		LOGD("Keypoint location:");
		LogInt((int)points[i].pt.x);
		LogInt((int)points[i].pt.y);

		KeyPoint kp = points[i];
		CvRect rect = cvRect(kp.pt.x - ((int) kp.size / 2),
				kp.pt.y - ((int) kp.size / 2), (int) kp.size, (int) kp.size);
		rectangle(image, cvPoint(rect.x, rect.y),
				cvPoint(rect.x + rect.width, rect.y + rect.height), color, 4, 8,0);
	}

}



extern "C" {




	JNIEXPORT void JNICALL Java_com_noregular_penman_mod_WarpField_nativeWarp
	(JNIEnv * jenv, jclass jobj, jlong color, jlong grey, jdoubleArray jarray, jlong lalt0, jlong lalt1, jlong lalt2, jlong lalt3, jlong recw)
	{
		LOGD("Warping the field natively");
		Mat* displayimage = ((Mat*) color);
		Mat* searchimage = ((Mat*) grey);
		Mat* outMat = ((Mat*) recw)



		long result = 0;
	    try{
		    jboolean isCopy1;
		    jdouble* ipoints = 
		           jenv->GetDoubleArrayElements(jarray, &isCopy1);
		    jint n = jenv->GetArrayLength(jarray);


		    int avkps = 0;
			vector<KeyPoint > kps;
			for(int i = 0; i < 4; i++){
				int r = i * 3;
				float i1 = (float) (double)ipoints[r];
				float i2 = (float) (double)ipoints[r + 1];
				float i3 = (float) (double)ipoints[r + 2];
				Point2d p2f = Point2f(i2, i3);
				KeyPoint kp = KeyPoint(p2f, i1);
				kps.push_back(kp);
				avkps += (int)i1;
			}
			avkps /= 4;


		    if (isCopy1 == JNI_TRUE) {
		       jenv->ReleaseDoubleArrayElements(jarray, ipoints, JNI_ABORT);
		    }

			//drawPoints(*displayimage, kps, Scalar(0, 0, 255, 255));
	    	int newwidth = 450;
	    	int newheight = 320;




	    	Mat text, text_color;
	    	int recwidth;

			warpField(kps, *displayimage, *searchimage, text, text_color, recwidth);


			Mat* malt0 = ((Mat*) lalt0);
			Mat* malt1 = ((Mat*) lalt1);
			Mat* malt2 = ((Mat*) lalt2);
			Mat* malt3 = ((Mat*) lalt3);


			outmat->at<double>(0,0) = nwidth;
			outmat->at<double>(1,0) = nheight;


			Mat tmp = Mat();
			// Malt 0: 0 degrees.
			Size s;
			newwidth = 800;
			newheight = (int)(text_color.rows * (800.0 / (double)text_color.cols));
			s = Size(newwidth, newheight);
			resize(text_color, *malt0, s);

			outmat->at<double>(0,0) = avkps * (800.0 / (double)tmp.cols);

			// Malt 1: 90 degrees.
			Point2f src_center = Point2f(text_color.cols / 2.0F, text_color.rows / 2.0F);
			Mat rot_mat = getRotationMatrix2D(src_center, 90, 1.0);

			rot_mat.at<double>(0, 2) = (double) 0.0;
			rot_mat.at<double>(1, 2) = (double) text_color.cols;
			warpAffine(text_color, tmp, rot_mat, Size(text_color.rows, text_color.cols));

			newheight = (int)(tmp.rows * (800.0 / (double)tmp.cols));
			s = Size(newwidth, newheight);
			resize(tmp, *malt1, s);

			outmat->at<double>(0,1) = avkps * (800.0 / (double)tmp.cols);

			// Malt 2: 180 degrees.
			src_center = Point2f(text_color.cols / 2.0F, text_color.rows / 2.0F);
			rot_mat = getRotationMatrix2D(src_center, 180, 1.0);
			warpAffine(text_color, tmp, rot_mat, text_color.size());
			newheight = (int)(tmp.rows * (800.0 / (double)tmp.cols));
			s = Size(newwidth, newheight);
			resize(tmp, *malt2, s);	


			// Malt 3: 270 degrees.
			src_center = Point2f(text_color.cols / 2.0F, text_color.rows / 2.0F);
			rot_mat = getRotationMatrix2D(src_center, 270, 1.0);

			rot_mat.at<double>(0, 2) = (double) text_color.rows;
			rot_mat.at<double>(1, 2) = (double) 0.0;
			warpAffine(text_color, tmp, rot_mat, Size(text_color.rows, text_color.cols));

			newheight = (int)(tmp.rows * (800.0 / (double)tmp.cols));
			s = Size(newwidth, newheight);
			resize(tmp, *malt3, s);


			
	    }
	    
	    catch(cv::Exception e){
			LOGD("nativeCreateObject catched cv::Exception: %s", e.what());
			jclass je = jenv->FindClass("org/opencv/core/CvException");
			if(!je)
		    	je = jenv->FindClass("java/lang/Exception");
			jenv->ThrowNew(je, e.what());
	    }
	    catch (...){
			LOGD("nativeDetect catched unknown exception");
			jclass je = jenv->FindClass("java/lang/Exception");
			jenv->ThrowNew(je, "Unknown exception in JNI code {highgui::VideoCapture_n_1VideoCapture__()}");
	    }



	}


}