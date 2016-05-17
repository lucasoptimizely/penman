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


using namespace std;
using namespace cv;

using namespace std;
using namespace cv;


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

void putInOrder(KeyPoint newpts[4]) {
	double atan2results[4];

	double avx = 0;
	double avy = 0;

	for (int i = 0; i < 4; i++) {
		avx += newpts[i].pt.x;
		avy += newpts[i].pt.y;
	}
	avx /= 4;
	avy /= 4;

	Point2d temp[4];
	for (int i = 0; i < 4; i++) {
		temp[i] = Point2f(newpts[i].pt.x - avx, newpts[i].pt.y - avy);
	}

	for (int i = 0; i < 4; i++) {
		atan2results[i] = atan2((double) temp[i].y, (double) temp[i].x);
	}
	bubbleSort(atan2results, newpts);

}

void createDescriptors(Mat &descriptors, vector<KeyPoint> &kps, Mat &image) {
/*	SurfDescriptorExtractor extractor = SurfDescriptorExtractor(0,0,false,false);
	extractor.compute(image, kps, descriptors);*/

	SURF s = SURF(2000, 4, 2, false, false );
	s.operator()(image,Mat(),kps,descriptors,true);



}

/**
 * Draws a list of keypoints.
 */
void drawPoints(Mat &image, std::vector<KeyPoint> points, Scalar color) {
	for (unsigned int i = 0; i < points.size(); i++) {
		KeyPoint kp = points[i];
		CvRect rect = cvRect(kp.pt.x - ((int) kp.size / 2),
				kp.pt.y - ((int) kp.size / 2), (int) kp.size, (int) kp.size);
		rectangle(image, cvPoint(rect.x, rect.y),
				cvPoint(rect.x + rect.width, rect.y + rect.height), color, 4, 8,0);
	}

}

void LogInt(int i){
    char buffer [50];
	sprintf (buffer, "Logint: %d", i);
	LOGV(buffer);	
}
/**
 * Searches if a keypoint that is in the points vector with index "index" is in
 * another keypoint-box.
 */
int inBox(int index, vector<KeyPoint> points) {
	int result = -1;
	bool stop = false;

	vector<Rect> rectangles;
	for (unsigned int i = 0; i < points.size(); i++) {
		KeyPoint kp = points[i];
		Rect rect = cvRect(kp.pt.x - ((int) kp.size / 2),
				kp.pt.y - ((int) kp.size / 2), (int) kp.size, (int) kp.size);
		rectangles.push_back(rect);
	}

	unsigned int newindex = index;

	Rect current = rectangles[index];
	unsigned int i = 0;
	for (i = 0; i < rectangles.size(); i++) {
		if (i != newindex) {
			for (int j = 0; j < 4; j++) {
				int x, y;
				if (j == 0) {
					x = current.x;
					y = current.y;
				} else if (j == 1) {
					x = current.x + current.width;
					y = current.y;
				} else if (j == 2) {
					x = current.x;
					y = current.y + current.height;
				} else {
					x = current.x + current.width;
					y = current.y + current.height;
				}
				/*				cout << x << " > " << (rectangles[i].x) << " && " << x << " < "
				 << (rectangles[i].x + rectangles[i].width) << " && "
				 << y << " > " << (rectangles[i].y) << " && " << y
				 << " < " << (rectangles[i].y + rectangles[i].height)
				 << endl;*/

				if (x >= (rectangles[i].x)
						&& x <= (rectangles[i].x + rectangles[i].width)
						&& y >= (rectangles[i].y)
						&& y <= (rectangles[i].y + rectangles[i].height)) {
					result = i;
					//cout << "FOUND!!!!" << endl;
					stop = true;
					break;
				}
			}
			if (stop) {
				break;
			}
			for (int j = 0; j < 4; j++) {
				int x, y;
				if (j == 0) {
					x = rectangles[i].x;
					y = rectangles[i].y;
				} else if (j == 1) {
					x = rectangles[i].x + rectangles[i].width;
					y = rectangles[i].y;
				} else if (j == 2) {
					x = rectangles[i].x;
					y = rectangles[i].y + rectangles[i].height;
				} else {
					x = rectangles[i].x + rectangles[i].width;
					y = rectangles[i].y + rectangles[i].height;
				}

				if (x >= (current.x) && x <= (current.x + current.width)
						&& y >= (current.y)
						&& y <= (current.y + current.height)) {
					result = i;
					//cout << "FOUND!!!!" << endl;
					stop = true;
					break;
				}
			}
			if (stop) {
				break;
			}

		}
	}

	return result;
}

/**
 * Looks if a keypoint is in a box that is bigger. The smallest keypoint is
 * removed from the keypoints list and added to the bad keypoints list.
 */
void addToKeypoints(vector<KeyPoint> &kps, KeyPoint kp,
		vector<KeyPoint> &bad_kps) {
	kps.push_back(kp);
	int newitemindex = kps.size() - 1;
	int found = inBox(newitemindex, kps);

	if (found != -1) {
		KeyPoint tmp;
		if (kps[newitemindex].size > kps[found].size) {
			tmp = kps[found];
			kps.erase(kps.begin() + found);
			bad_kps.push_back(tmp);
		} else {
			tmp = kps[newitemindex];
			kps.erase(kps.begin() + newitemindex);

			bad_kps.push_back(tmp);
		}
	}

}


extern "C" {

JNIEXPORT jlong JNICALL Java_com_noregular_penman_mod_SVM_nativeCreateObject
(JNIEnv * jenv, jclass jobj, jstring jFileName)
{
    const char* jnamestr = jenv->GetStringUTFChars(jFileName, NULL);    
    string stdFileName(jnamestr);

	LOGD("test");	
    LOGD(jnamestr);

    jlong result = 0;
    
    try
    {

    	CvSVM* mysvm = new CvSVM();
    	(*mysvm).load(jnamestr);
    	result = (jlong)mysvm;

    }
    catch(cv::Exception e) {
		LOGD("nativeCreateObject catched cv::Exception: %s", e.what());
		jclass je = jenv->FindClass("org/opencv/core/CvException");
		if(!je) 
	    	je = jenv->FindClass("java/lang/Exception");
		jenv->ThrowNew(je, e.what());
    }
    catch (...){
		LOGD("nativeCreateObject catched unknown exception");
		jclass je = jenv->FindClass("java/lang/Exception");
		jenv->ThrowNew(je, "Unknown exception in JNI code {highgui::VideoCapture_n_1VideoCapture__()}");
		return 0;
    }
    
    return result;
}

JNIEXPORT void JNICALL Java_com_noregular_penman_mod_SVM_nativeDestroyObject
(JNIEnv * jenv, jclass jobj, jlong thiz)
{
	CvSVM* mysvm = ((CvSVM*)thiz);
	if(mysvm)
		delete ((CvSVM*)thiz); 
}



JNIEXPORT jboolean JNICALL Java_com_noregular_penman_mod_SVM_nativeDetect
(JNIEnv * jenv, jclass jobj, jlong thiz, jlong grey, jlong color, jlong out)
{
	jboolean result = false;
    try{

    	CvSVM* mysvm = ((CvSVM*)thiz);
    	Mat* searchimage = ((Mat*) grey);
    	Mat* displayimage = ((Mat*) color);
    	Mat* outmat = ((Mat*) out);

    	Mat small;

    	int origwidth = searchimage->cols;
    	int origheight = searchimage->rows;
    	int newwidth = 450;
    	int newheight = 320;

    	float factorw = origwidth / (float) newwidth;
    	float factorh = origheight / (float) newheight;

    	float factors = (factorw + factorh) / (float)2;



    	Size s = Size(newwidth, newheight);

    	resize(*searchimage, small, s);




    	int minHessian = 2000;
		std::vector<KeyPoint> keypoints_scene, keypoints_not, keypoints_triangles;
		Mat mask = Mat();    	
    	SURF surfobject = SURF(minHessian, 3, 2, false, false);
		surfobject.operator ()(small, mask, keypoints_scene);

		Mat descriptors = Mat(keypoints_scene.size(), 64, CV_32FC1);
		createDescriptors(descriptors, keypoints_scene, small);


		for (int i = 0; i < descriptors.rows; i++) {
			float result = mysvm->predict(descriptors.row(i));
			keypoints_scene[i].pt.x = keypoints_scene[i].pt.x * factorw;
			keypoints_scene[i].pt.y = keypoints_scene[i].pt.y * factorh;
			keypoints_scene[i].size = keypoints_scene[i].size * factors;


			if (result == 1) {
				addToKeypoints(keypoints_triangles, keypoints_scene[i],
						keypoints_not);
			} else {
				keypoints_not.push_back(keypoints_scene[i]);

			}
		}

		
		if(keypoints_triangles.size() == 4){
			result = true;

			KeyPoint in_order[4] = {keypoints_triangles[0], keypoints_triangles[1], keypoints_triangles[2], keypoints_triangles[3]};
			putInOrder(in_order);

			double smallest = 720;


			double avsize = 0;
			for(int i = 0; i < 4; i++){
				avsize += keypoints_triangles[i].size;

			}
			avsize /= 4.;


			int nwidth = displayimage->cols;
			int nheight = displayimage->rows;

			drawPoints(*displayimage, keypoints_triangles, Scalar(0, 255, 0, 255));


			

			// width and height of image capture
			LogInt(nwidth);
			LogInt(nheight);
			outmat->at<double>(0,0) = nwidth;
			outmat->at<double>(1,0) = nheight;

			//pt 1
			outmat->at<double>(2,0) = in_order[0].size;
			outmat->at<double>(3,0) = in_order[0].pt.x;
			outmat->at<double>(4,0) = in_order[0].pt.y;

			//pt 2
			outmat->at<double>(5,0) = in_order[1].size;
			outmat->at<double>(6,0) = in_order[1].pt.x;
			outmat->at<double>(7,0) = in_order[1].pt.y;

			//pt 3
			outmat->at<double>(8,0) = in_order[2].size;
			outmat->at<double>(9,0) = in_order[2].pt.x;
			outmat->at<double>(10,0) = in_order[2].pt.y;

			//pt 4
			outmat->at<double>(11,0) = in_order[3].size;
			outmat->at<double>(12,0) = in_order[3].pt.x;
			outmat->at<double>(13,0) = in_order[3].pt.y;

		} else {
			drawPoints(*displayimage, keypoints_triangles, Scalar(0, 0, 255, 255));
		}


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

    return result;


}


JNIEXPORT jboolean JNICALL Java_com_noregular_penman_mod_SVM_nativeDetect2
(JNIEnv * jenv, jclass jobj, jlong thiz, jlong grey, jlong out)
{
	jboolean result = false;
    try{

    	CvSVM* mysvm = ((CvSVM*)thiz);
    	Mat* searchimage = ((Mat*) grey);
    	Mat* outmat = ((Mat*) out);

    	Mat small;

    	int origwidth = searchimage->cols;
    	int origheight = searchimage->rows;
    	int newwidth = 450;
    	int newheight = 320;

    	float factorw = origwidth / (float) newwidth;
    	float factorh = origheight / (float) newheight;

    	float factors = (factorw + factorh) / (float)2;



    	Size s = Size(newwidth, newheight);

    	resize(*searchimage, small, s);




    	int minHessian = 2000;
		std::vector<KeyPoint> keypoints_scene, keypoints_not, keypoints_triangles;
		Mat mask = Mat();    	
    	SURF surfobject = SURF(minHessian, 3, 2, false, false);
		surfobject.operator ()(small, mask, keypoints_scene);

		Mat descriptors = Mat(keypoints_scene.size(), 64, CV_32FC1);
		createDescriptors(descriptors, keypoints_scene, small);


		for (int i = 0; i < descriptors.rows; i++) {
			float result = mysvm->predict(descriptors.row(i));
			keypoints_scene[i].pt.x = keypoints_scene[i].pt.x * factorw;
			keypoints_scene[i].pt.y = keypoints_scene[i].pt.y * factorh;
			keypoints_scene[i].size = keypoints_scene[i].size * factors;


			if (result == 1) {
				addToKeypoints(keypoints_triangles, keypoints_scene[i],
						keypoints_not);
			} else {
				keypoints_not.push_back(keypoints_scene[i]);

			}
		}

		
		if(keypoints_triangles.size() == 4){
			result = true;

			KeyPoint in_order[4] = {keypoints_triangles[0], keypoints_triangles[1], keypoints_triangles[2], keypoints_triangles[3]};
			putInOrder(in_order);

			double smallest = 720;


			double avsize = 0;
			for(int i = 0; i < 4; i++){
				avsize += keypoints_triangles[i].size;

			}
			avsize /= 4.;


			int nwidth = searchimage->cols;
			int nheight = searchimage->rows;

			

			// width and height of image capture
			LogInt(nwidth);
			LogInt(nheight);
			outmat->at<double>(0,0) = nwidth;
			outmat->at<double>(1,0) = nheight;

			//pt 1
			outmat->at<double>(2,0) = in_order[0].size;
			outmat->at<double>(3,0) = in_order[0].pt.x;
			outmat->at<double>(4,0) = in_order[0].pt.y;

			//pt 2
			outmat->at<double>(5,0) = in_order[1].size;
			outmat->at<double>(6,0) = in_order[1].pt.x;
			outmat->at<double>(7,0) = in_order[1].pt.y;

			//pt 3
			outmat->at<double>(8,0) = in_order[2].size;
			outmat->at<double>(9,0) = in_order[2].pt.x;
			outmat->at<double>(10,0) = in_order[2].pt.y;

			//pt 4
			outmat->at<double>(11,0) = in_order[3].size;
			outmat->at<double>(12,0) = in_order[3].pt.x;
			outmat->at<double>(13,0) = in_order[3].pt.y;

		} 


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

    return result;


}

}