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

void LogInt(int i){
    char buffer [50];
	sprintf (buffer, "Logint: %d", i);
	LOGV(buffer);	
}


/**
 * Caclulates the center point of a rectangle.
 */
Point midPoint(Rect rect){
	float x = rect.x + (rect.width/ 2.0) ;
	float y = rect.y + (rect.height/ 2.0) ;
	return Point(x,y);

}

Rect cRects(Rect r1, Rect r2) {
	int newx = 0;
	int neww = 0;
	int newy = 0;
	int newh = 0;
	if (r1.x < r2.x) {
		newx = r1.x;
		if (r1.x + r1.width > r2.x + r2.width) {
			neww = r1.width;
		} else {
			neww = r2.x + r2.width - r1.x;
		}
		if (r1.y < r2.y) {
			newy = r1.y;
		} else {
			newy = r2.y;
		}
		if (r1.y + r1.height > r2.y + r2.height) {
			newh = r1.y + r1.height - newy;
		} else {
			newh = r2.y + r2.height - newy;
		}

	} else {
		newx = r2.x;
		if (r1.x + r1.width > r2.x + r2.width) {
			neww = r1.x + r1.width - r2.x;
		} else {
			neww = r1.width;
		}
		if (r1.y < r2.y) {
			newy = r1.y;
		} else {
			newy = r2.y;
		}
		if (r1.y + r1.height > r2.y + r2.height) {
			newh = r1.y + r1.height - newy;
		} else {
			newh = r2.y + r2.height - newy;
		}
	}
	return Rect(newx, newy, neww, newh);
}

/**
 * Looks if point lies in the given rectangle.
 */
bool inRect(Point p, Rect rect){
	bool result = false;
	if(p.x >= rect.x && p.x <= rect.x + rect.width && p.y >= rect.y && p.y <= rect.y + rect.height){
		result = true;
	}
	return result;
}

/**
 * Convert int to string.
 */
string convertInt(int number) {
	stringstream ss; //create a stringstream
	ss << number; //add number to the stream
	return ss.str(); //return a string with the contents of the stream
}

void bubbleSort(double num[], vector<Box> &lines) {
	unsigned int i, j, flag = 1; // set flag to 1 to start first pass
	double temp; // holding variable
	Box temp_p;
	unsigned int numLength = lines.size();
	for (i = 1; (i <= numLength) && flag; i++) {
		flag = 0;
		for (j = 0; j < (numLength - 1); j++) {
			if (num[j + 1] < num[j]){

				temp = num[j]; // swap elements
				temp_p = lines[j];

				num[j] = num[j + 1];
				lines[j] = lines[j+1];

				num[j + 1] = temp;
				lines[j + 1] = temp_p;

				flag = 1; // indicates that a swap occurred.
			}
		}
	}
}

void bubbleSort(double num[], vector<BoxLine> &lines) {
	unsigned int i, j, flag = 1; // set flag to 1 to start first pass
	double temp; // holding variable
	BoxLine temp_p;
	unsigned int numLength = lines.size();
	for (i = 1; (i <= numLength) && flag; i++) {
		flag = 0;
		for (j = 0; j < (numLength - 1); j++) {
			if (num[j + 1] < num[j]){

				temp = num[j]; // swap elements
				temp_p = lines[j];

				num[j] = num[j + 1];
				lines[j] = lines[j+1];

				num[j + 1] = temp;
				lines[j + 1] = temp_p;

				flag = 1; // indicates that a swap occurred.
			}
		}
	}
}

class Box {
public:
	Box();
	Box(Rect rect, Point line);
	Rect rect;
	Point line;
};

Box::Box(Rect rect, Point line){
	this->rect = rect;
	this->line = line;
}
Box::Box(){
}

class BoxLine {
public:
	BoxLine();
	BoxLine(vector<Box> v, int y);
	vector<Box> v;
	int y;
	bool br_afterline;
};
BoxLine::BoxLine(){
}
BoxLine::BoxLine(vector<Box> v, int y){
	this->v = v;
	this->y = y;
	this->br_afterline = false;
}















int findNext(vector<Rect> &boxes, Rect current, int size) {
	int result = -1;
	int minnx = current.x + current.width;
	int minny = current.y + (current.height * 1.1);

	int newx = 10000000;

	for (size_t i = 0; i < boxes.size(); i++) {
		Point p = midPoint2(boxes[i]);
		if (p.x > minnx && p.y < minny && p.x < newx) {
			newx = p.x;
			result = i;
		}
	}

	if (result != -1) {
		Point p = midPoint2(boxes[result]);
		int ny = ((current.y + current.height) - size);
		int nx = (p.x - 0.5 * size);
		bool change = false;
		if (boxes[result].height < (size * 0.5)) {
			boxes[result] = Rect(nx, ny, size, size);
			change = true;
		}

		int miny = ny;
		int minx = nx;
		int width = size;
		int height = size;

		for (size_t i = 0; i < boxes.size(); i++) {
			if (i != result) {
				if (inRect2(midPoint2(boxes[i]), boxes[result])) {
					if (boxes[i].x < minx) {
						width += minx - boxes[i].x;
						minx = boxes[i].x;
					}
					if (boxes[i].y < miny) {
						height += miny - boxes[i].y;
						miny = boxes[i].y;
					}

					if (boxes[i].y + boxes[i].height > miny + height) {
						//height += (boxes[i].y + height) - (miny + height);

					}
					if (boxes[i].x + boxes[i].width > minx + width) {
						width += (boxes[i].x + width) - (minx + width);
					}

					boxes.erase(boxes.begin() + i);
					i--;
				}
			}
		}
		if (change) {
			boxes[result] = Rect(minx, miny, width, height);
		}
	}

	return result;

}

Rect popFirst(vector<Rect> &boxes, long area) {

	int index = -1;
	for (size_t i = 0; i < boxes.size(); i++) {
		Point p = midPoint2(boxes[i]);
		long newarea = p.x * p.y;
		if (newarea < area) {

			area = newarea;
			index = i;
		}
	}
	Rect r = Rect(0, 0, 0, 0);
	if (index != -1) {
		r = boxes[index];
		boxes.erase(boxes.begin() + index);
	}
	return r;

}

void makeLarger(vector<Rect> &boxes) {
	for (size_t i = 0; i < boxes.size(); i++) {
		Rect cur = boxes[i];
		int diffheight = cur.height * .4;
		int diffwidth = cur.width * .4;
		int newy = cur.y - (diffheight * 0.5);
		int newx = cur.x - (diffwidth * 0.5);
		int newheight = cur.height + diffheight;
		int newwidth = cur.width + diffwidth;

		boxes[i] = Rect(newx, newy, newwidth, newheight);

	}
}

void findLineBreaks(vector<vector<Rect> > lines, vector<bool> &brs, int s) {

	if(lines.size() != 0){
		for (size_t i = 0; i < lines.size() - 1; i++) {
			bool result = false;
			if (lines[i].size() > 0 && lines[i + 1].size() > 0) {
				Point p1 = midPoint2(lines[i][0]);
				Point p2 = midPoint2(lines[i + 1][0]);
				if (abs(p1.y - p2.y) > s * 1.3) {
					result = true;
				}
			}
			brs.push_back(result);
		}
	}
	brs.push_back(false);

}


void finalLine(vector<Rect> &line) {
	int lineheight = 0;

	if(line.size() != 0){
		for (size_t i = 0; i < line.size() - 1; i++) {
			if (abs(line[i].y - line[i + 1].height) > lineheight) {
				lineheight = abs(line[i].y - line[i + 1].y + line[i + 1].height);
			}
		}
	}


	for (size_t i = 0; i < line.size(); i++) {
		if (i == 0) {
			Point m = midPoint2(line[i]);
			int y = m.y - lineheight * 0.5;
			line[0] = Rect(line[0].x, y, line[0].width, lineheight);
		} else {

			int diff = (line[i - 1].y + line[i - 1].height)
					- (line[i].y + line[i].height);

			int y = line[i - 1].y - diff;
			int x = line[i].x;
			int width = line[i].width;

			line[i] = Rect(line[0].x, y, line[0].width, lineheight);
			line[i] = Rect(x, y, width, lineheight);
		}
	}
	if(line.size() != 0){
		for (size_t i = 0; i < line.size() - 1; i++) {
			cout << "line.w: " << line[i].width << " < " << line[i].height << endl;
			cout << line[i].x + lineheight << " > " << line[i + 1].x << endl;

			if (line[i].width < line[i].height * 0.7
					&& line[i].x + lineheight > line[i + 1].x) {
				cout << "done!";
				line[i] = cRects(line[i], line[i + 1]);
				line.erase(line.begin() + i + 1);
				i--;
			}
		}
	}

}

void finalTouch(vector<vector<Rect> > &lines) {

	for (size_t i = 0; i < lines.size(); i++) {
		finalLine(lines[i]);

	}

}

void findWithoutLines(vector<Rect> &boxes, Mat &img, int size, vector<Rect> &out) {
	makeLarger(boxes);
	Rect current;
	vector<vector<Rect> > lines;

	while (boxes.size() != 0) {
		vector<Rect> line;
		current = popFirst(boxes, ((long) img.cols * (long) img.rows));
		line.push_back(current);

		int next = findNext(boxes, current, size);
		while (next != -1) {
			current = boxes[next];
			boxes.erase(boxes.begin() + next);
			line.push_back(current);
			next = findNext(boxes, current, size);
		}

		lines.push_back(line);
	}

	vector<bool> brs;
	findLineBreaks(lines, brs, size);

	finalTouch(lines);

	int count = 0;
	for (size_t i = 0; i < lines.size(); i++) {
		RNG rng2(12345);
		Scalar color = randomColor2(rng2);

		for (size_t j = 0; j < lines[i].size(); j++) {
			count++;
			circle(img, midPoint2(lines[i][j]), 1, Scalar(255, 0, 0), 3);

			rectangle(img, lines[i][j], color);
			out.push_back(lines[i][j]);

			putText(img, convertInt3(count), midPoint2(lines[i][j]),
					CV_FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 0, 0), 1);

		}

		cout << brs.size() << ", " << i << ", " << lines[i].size() << endl;
		if (brs.size() > i && lines[i].size() > 0) {
			Scalar color;
			if (brs[i]) {
				color = Scalar(0, 255, 0);
			} else {
				color = Scalar(0, 0, 255);
			}
			int y = lines[i][0].y + lines[i][0].height;
			line(img, Point(0, y), Point(img.cols, y), color, 2);
		}
	}

}











/*********************************** findtext.cpp *********************************************/



/**
 * Filter the rectangles that are to large
 */
void filterRectangles(vector<Rect> &rects, Mat img){
	float margin = 0.8;
	int maxwidth = img.cols * margin;
	int maxheight = img.rows * margin;
	for(unsigned int i = 0; i < rects.size(); i++){
		if(rects[i].width > maxwidth || rects[i].height > maxheight){
			rects.erase(rects.begin() + i);
			i--;
			cout << "To large rectangle erased" << endl;
		}
	}
}



/**
 * Searches if the rectangle with the given index is in one of the other
 * rectangles.
 */
int inBox(int index, vector<Rect> rectangles) {
	int result = -1;
	Rect current = rectangles[index];
	unsigned int i = 0;

	for (i = 0; i < rectangles.size(); i++) {
		if (i != index) {
			for (int j = 0; j < 4; j++) {
				int x, y;


				if (inRect(Point(current.x, current.y), rectangles[i]) &&
						inRect(Point(current.x + current.width, current.y ), rectangles[i]) &&
						inRect(Point(current.x, current.y + current.height), rectangles[i]) &&
						inRect(Point(current.x + current.width, current.y + current.height), rectangles[i])) {
					result = i;
					break;
				}
			}
		}
	}

	return result;
}

/**
 * Searches a vector for the same point
 */
bool inPointVector(Point2d a, vector<Point2d> target){
	bool result = false;
	for(unsigned int i = 0; i < target.size(); i++){
		if(target[i].x == a.x && target[i].y == a.y){
			result = true;
			break;
		}
	}
	return result;

}
// Combines the rectangles: if a rectangle is inside another rectangle, it
// is erased from the vector.
void combineRects(vector<Rect> &rectangles) {
	bool change = true;
	while (change) {
		change = false;

		for (unsigned int i = 0; i < rectangles.size(); i++) {
			int result = inBox(i, rectangles);
			if(result != -1){
				rectangles.erase(rectangles.begin() + i);
				i--;
			}

		}
	}

}

/**
 * Generates a random color.
 */
Scalar randomColor(RNG& rng) {
	return Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
			rng.uniform(0, 255));
}


/**
 * Searches a vector for the same point
 */
int inBoxLines(int y, vector<BoxLine> target){
	int result = -1;
	for(unsigned int i = 0; i < target.size(); i++){
		if(target[i].y == y){
			result = i;
			break;
		}
	}
	return result;

}

/**
 * Searches a image for lines that are in the full length. If they are found,
 * they are erased. The found lines are returned so that we can determine the
 * order of the words/boxes later.
 */
void eraseLines(Mat &origional, Mat &dst, int lineheight, vector<Point2d> &out_lines) {
	Mat grey = origional.clone();
	adaptiveThreshold(origional, grey, 128, ADAPTIVE_THRESH_GAUSSIAN_C,
			THRESH_BINARY, 7, 5);

	Mat canny;
	Canny(grey, canny, 50, 200, 3);
	vector<Vec4i> p_lines;

	/// 2. Use Probabilistic Hough Transform
	HoughLinesP(canny, p_lines, 1, CV_PI / 180, 120, 100, 100);

	/// Show the result

	vector<vector<Vec4i> > lines;

	while (p_lines.size() != 0) {
		int lowesty = canny.rows;
		Vec4i lowestline;
		int lowestindex = -1;
		vector<Vec4i> temp;


		//find the lowest segment of each line
		for (size_t i = 0; i < p_lines.size(); i++) {
			Vec4i l = p_lines[i];

			if (l[1] < lowesty || l[3] < lowesty) {
				lowestline = l;
				lowesty = (l[1] < l[3] ? l[1] : l[3]);
				lowestindex = i;
			}
		}

		//erase first line and push back to temp
		p_lines.erase(p_lines.begin() + lowestindex);
		temp.push_back(lowestline);
		double midy = (lowestline[1] + lowestline[3]) / 2.0;

		for (size_t i = 0; i < p_lines.size(); i++) {
			Vec4i l = p_lines[i];

			if (l[1] < midy + lineheight / 2.0 && l[1] > midy - lineheight / 2.0
					&& l[3] < lowesty + lineheight / 2.0
					&& l[3] > lowesty - lineheight / 2.0) {
				temp.push_back(l);
				p_lines.erase(p_lines.begin() + i);
				i--;
			}
		}
		lines.push_back(temp);

		for (size_t i = 0; i < lines.size(); i++) {

			int min_x = canny.cols;
			int min_y = canny.rows;
			int max_x = 0;
			int max_y = 0;
			for (size_t j = 0; j < lines[i].size(); j++) {
				Vec4i l = lines[i][j];
				line(dst, Point(l[0], l[1]), Point( l[2],  l[3]), Scalar(128), 2,
													CV_AA);
				if (l[0] < min_x) {
					min_x = l[0];
					min_y = l[1];
				}
				if (l[2] < min_x) {
					min_x = l[2];
					min_y = l[3];
				}
				if (l[0] > max_x) {
					max_x = l[0];
					max_y = l[1];
				}
				if (l[2] > max_x) {
					max_x = l[2];
					max_y = l[3];
				}

			}
			int dx = max_x - min_x;
			int dy = max_y - min_y;

			cout << min_x << ", " << max_x << ", " << min_y << ", " << max_y
					<< ", " << endl;
			if ((dx) > (0.8 * dst.cols) && dy < 0.5 * lineheight) {
				double a = (double) dy / (double) dx;

				double b = min_y - (min_x * a);



				double zz = a * dst.cols + b;

				//int begin = (int) (b + 0.5);
				//int end = (int) (zz + 0.5);
				Point2d out_l = Point2d(a,b);
				if(!inPointVector(out_l, out_lines)){
					out_lines.push_back(out_l);
				}


			}

		}


	}
	namedWindow("lines");
	imshow("lines", dst);
}

/**
 * Split rectangles that are larger than the given size.
 */
void splitLargeRectangles(vector<Rect> &rects, int size){
	//cout << "splitLargeRectangles: " << rects.size() << endl;
	int msize = size * 1.0;
	vector<Rect> tohigh;
	for(unsigned int i; i < rects.size(); i++){
		int factor = (rects[i].height / msize) + 1;
		//cout << "height: " << rects[i].height << ", size: " << size  << endl;
		//cout << "tohigh: " << (rects[i].height / size) + 1.0  << endl;
		if(factor > 1){
			//cout << "tohigh: " << rects[i].height << endl;
			tohigh.push_back(rects[i]);
			rects.erase(rects.begin() + i);

			i--;

		}
	}
	for(unsigned int i = 0; i < tohigh.size(); i++){
		int factor = (tohigh[i].height / size);
		int remainder = tohigh[i].height % size;
		//cout << tohigh[i].height << ", " << (factor + 0.5) << endl;
		//cout << "remainder: " << remainder << endl;
		if(remainder != 0){
			Rect top = Rect(tohigh[i].x, tohigh[i].y, tohigh[i].width, remainder);
			rects.push_back(top);
		}
		int newheight = ((tohigh[i].height - remainder) / factor);
		for(int j = 0; j < factor; j++){
			int newy = (tohigh[i].y + remainder) + j * newheight;
			Rect n = Rect(tohigh[i].x, newy, tohigh[i].width, newheight);
			rects.push_back(n);
		}





	}




}



/**
 *
 */
int findLineHeight(vector<Rect> boxes, vector<Point2d> lines, int maxheight){
	int result;
	if(lines.size() > 1){
		int lineheight = 0;
		int count = 0;
		for(unsigned int i = 0; i < lines.size(); i++){
			int min = maxheight;
			for(unsigned int j = 0; j < lines.size(); j++){
				if(i!=j){
					unsigned int s = abs((lines[i].y - lines[j].y));
					if(s < min){
						min = s;
					}
				}


			}

			lineheight += min;

		}
		result = lineheight/lines.size();
	} else{
		int lineheight = 0;
		for(unsigned int i = 0; i < boxes.size(); i++){
			lineheight += boxes[i].height + boxes[i].height * 0.15;
		}
		result = lineheight/lines.size();
	}
	return result;
}




/**
 * Order lines
 */
void orderBoxes(vector<Box> &boxes){
	cout << "sort" << endl;
	double num[boxes.size()];

	for(unsigned int i = 0; i < boxes.size(); i++){
		num[i] = boxes[i].rect.x;
	}
	cout << "sort" << endl;
	bubbleSort(num, boxes);
	for(unsigned int i = 0; i < boxes.size(); i++){
		cout << boxes[i].rect.x << endl;
	}



	cout << "sort" << endl;

}



/**
 * Order lines
 */
void orderLines(vector<BoxLine> &lines){
	cout << "sort" << endl;
	double num[lines.size()];

	for(unsigned int i = 0; i < lines.size(); i++){
		num[i] = lines[i].y;
	}
	cout << "sort" << endl;
	bubbleSort(num, lines);
	for(unsigned int i = 0; i < lines.size(); i++){
		cout << lines[i].y << endl;
	}

	for(unsigned int i = 0; i < lines.size(); i++){
		orderBoxes(lines[i].v);
	}



	cout << "sort" << endl;

}

Box createEnclosingBox(vector<Box> boxes, int height){
	int minx = 100000;
	int maxx = 0;
	int miny = 100000;
	int maxy = 0;
	//cout << boxes[0].line.x << endl;
	Point p = boxes[0].line;

	for(size_t i = 0; i < boxes.size(); i++){
		if(boxes[i].rect.x < minx){
			minx = boxes[i].rect.x;

		}
		if(boxes[i].rect.x + boxes[i].rect.width > maxx){
			maxx = boxes[i].rect.x + boxes[i].rect.width;
		}
		if(boxes[i].rect.y < miny){
			miny = boxes[i].rect.y;

		}
		if(boxes[i].rect.y + boxes[i].rect.height > maxy){
			maxy = boxes[i].rect.y + boxes[i].rect.height;
		}
	}

	int newy = p.x * minx + p.y - (height * 0.8);


	int width = (maxx- minx) * 1.4;
	int h = maxy - miny;
	int newx = minx - ((maxx- minx) * 0.2);

	Rect r = Rect(newx, newy, width, height *1.3);
	return Box(r,p);


}

void inSameXRegion(vector<Box> &boxes, vector<Box> &nboxes){
	Box current = boxes[0];
	boxes.erase(boxes.begin());
	nboxes.clear();
	nboxes.push_back(current);
	for(size_t i = 0; i < boxes.size(); i++){
		if(boxes[i].rect.x < current.rect.x + current.rect.width && boxes[i].rect.x > current.rect.x){
			nboxes.push_back(boxes[i]);
			boxes.erase(boxes.begin() + i);
			i--;
		}
	}

}

void finalLine(BoxLine &line, int lineheight){
	vector<Box> nline;
	vector<Box> result;
	while(line.v.size() != 0){
		inSameXRegion(line.v, nline);
		result.push_back(createEnclosingBox(nline, lineheight));
		//cout << result[result.size() -1].line.x << endl;
	}
	line.v = result;


	if(line.v.size() != 0){
		for (size_t i = 0; i < line.v.size() - 1; i++) {
			cout << "size: " << line.v.size() << ", i: " << i << endl;

			if (line.v[i].rect.width < line.v[i].rect.height * 0.7
					&& line.v[i].rect.x + lineheight > line.v[i + 1].rect.x) {
				cout << "done!";
				line.v[i].rect = cRects2(line.v[i].rect, line.v[i + 1].rect);
				line.v.erase(line.v.begin() + i + 1);
				i--;
			}
		}
	}



}

void filterBoxes(vector<Box> &boxes, int height){
	for(size_t i = 0; i < boxes.size(); i++){
		Point p = midPoint(boxes[i].rect);
		int miny = boxes[i].line.x * p.x + boxes[i].line.y - (height * 0.6);
		if(p.y < miny && boxes[i].rect.area() < (0.3 * height * height)){
			boxes.erase(boxes.begin() + i);
			i--;
		}
	}
}

void filterLines(vector<BoxLine> &lines, int height){
	for(size_t i = 0; i < lines.size(); i++){
		filterBoxes(lines[i].v, height);
	}

}

/**
 * Combine combine rectangles that are in the same x region and find divided
 * words.
 */
void finalPreparations(vector<BoxLine> &lines, int lineheight){
	for(size_t i = 0; i < lines.size(); i++){
		cout << "i: " << i << ", size: " << lines.size() << endl;
		finalLine(lines[i], lineheight);
		cout << "i: " << i << ", size: " << lines.size() << endl;
		if(lines[i].v.size() ==0){
			lines.erase(lines.begin() + i);
			i--;
		}
	}

	if(lines.size() != 0){
		for(size_t i = 0; i < lines.size() - 1; i++){
			if(abs(lines[i].v[0].line.y - lines[i + 1].v[0].line.y) > lineheight * 1.5){
				lines[i].br_afterline = true;
			}
		}
	}



}


/**
 * Tries to find a order using the found lines.
 */
void organizeWithLines(vector<Rect> &boxes, vector<Point2d> lines, Mat &img, int size, vector<Rect> &out){
	RNG rng(12345);
	vector<Scalar> colors;
	for(unsigned int j = 0; j < lines.size() + 1; j++){
		colors.push_back(randomColor(rng));
	}

	vector<Point2d> linestarts;

	vector<Box> ordened;
	vector<BoxLine> ordenedl;

	for(unsigned int i = 0; i < boxes.size(); i++){
		Point p = midPoint(boxes[i]);
		int maxliney = img.rows;
		unsigned int maxindex = lines.size();


		Box nn;
		Point nnn;
		Scalar c = randomColor(rng);
		for(unsigned int j = 0; j < lines.size(); j++){
			double lineval = lines[j].x * p.x + lines[j].y;
			if(p.y < lineval &&

					lineval < maxliney ){
				maxliney = lines[j].x * p.x + lines[j].y;
				maxindex = j;
				nn = Box(boxes[i], Point(lines[j].x,lines[j].y));
				nnn = Point2d(0,lines[j].y);

			}
		}

		if(!inPointVector(nnn,linestarts)){
			linestarts.push_back(nnn);
		}

		int index = inBoxLines(nnn.y, ordenedl);
		if(index == -1){
			ordened.clear();
			ordened.push_back(nn);
			ordenedl.push_back(BoxLine(ordened, nnn.y));
		}else {
			ordenedl[index].v.push_back(nn);
		}
		ordened.push_back(nn);



	}
	int lineheight = findLineHeight(boxes, lines, size);
	//cout << lineheight << endl;

	orderLines(ordenedl);
	int count = 0;


	filterLines(ordenedl,lineheight);
	finalPreparations(ordenedl, lineheight);


	for(size_t i = 0; i < ordenedl.size(); i++){
		for(size_t j = 0; j < ordenedl[i].v.size(); j++){
			count++;
			int x = ordenedl[i].v[j].rect.x;
			int y = ordenedl[i].v[j].rect.y;
			putText(img, convertInt2(count), Point(x,y),
							CV_FONT_HERSHEY_COMPLEX, 1.0, Scalar(0, 0, 0), 2);
			rectangle(img,ordenedl[i].v[j].rect, Scalar(0));
			out.push_back(ordenedl[i].v[j].rect);

		}
		int endco = ordenedl[i].v[0].line.x * img.cols + ordenedl[i].v[0].line.y;
		Point p1 = Point(0, ordenedl[i].v[0].line.y);
		Point p2 = Point(img.cols, endco);

		if(ordenedl[i].br_afterline){
			line(img, p1, p2, Scalar(0,255,0),2);

		} else {
			line(img, p1, p2, Scalar(0,0,255),2);
		}
	}


}

void findWords(Mat &grey, Mat &color, int recwidth, vector<Rect > &out) {
	Mat blurt;
	Mat lines = grey.clone();

	int erosion_size = 11;



	blur(grey, blurt, Size(3, 3));
	int erosion_type = MORPH_RECT;

	cout << "RecWidth: " << recwidth << endl;

	Mat element = getStructuringElement(erosion_type,
			Size(2 * erosion_size + 1, 3), Point(erosion_size, 1));

	Mat blur_black;
	adaptiveThreshold(blurt, blur_black, 128, ADAPTIVE_THRESH_GAUSSIAN_C,
			THRESH_BINARY, 7, 5);

	vector<Point2d> paper_lines;
	eraseLines(lines, blur_black, recwidth, paper_lines);

	morphologyEx(blur_black, blur_black, MORPH_OPEN, element);

	namedWindow("morph");
	imshow("morph", blur_black);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(blur_black, contours, hierarchy, CV_RETR_TREE,
			CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Find the convex hull object for each contour
	int imagewidth = blur_black.cols;
	int imageheight = blur_black.rows;

	//filterContours(imagewidth, imageheight, contours);
	vector<vector<Point> > hull(contours.size());
	for (unsigned int i = 0; i < contours.size(); i++) {
		convexHull(Mat(contours[i]), hull[i], false);
		for (unsigned int j = 0; j < contours[i].size(); j++) {
			Point t = contours[i][j];

			//cout << "x: " << t.x << ", y: " << t.y << "\n";
			if (t.x >= imagewidth || t.y >= imageheight || t.x <= 0
					|| t.y <= 0) {
				cout << "x: " << t.x << ", y: " << t.y << "\n";
			}
		}
	}


	RNG rng(54321);


	vector<Rect> bounding(hull.size());

	vector<Rect> boxes;
	for (unsigned int i = 0; i < contours.size(); i++) {
		Rect box = boundingRect(Mat(contours[i]));
		boxes.push_back(box);


	}



	filterRectangles(boxes, color);
	splitLargeRectangles(boxes, recwidth);
	combineRects(boxes);

	if(paper_lines.size() > 0){
		organizeWithLines(boxes, paper_lines, color, recwidth, out);
	}else{

		findWithoutLines(boxes, color, recwidth, out);

	}


}


extern "C" {

	JNIEXPORT void JNICALL Java_com_noregular_penman_mod_WarpField_nativeWord
	(JNIEnv * jenv, jclass jobj, jlong color, jlong grey, jlong out)
	{
		LOGD("Warping the field natively");
		Mat* displayimage = ((Mat*) color);
		Mat* greyimage = ((Mat*) grey);
		Mat* outimage = ((Mat*) out);

	    try{






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


	JNIEXPORT void JNICALL Java_com_noregular_penman_mod_WarpField_nativeCut
	(JNIEnv * jenv, jclass jobj, jlong color, jdoubleArray jarray, jlong out)
	{
		LOGD("Warping the field natively");
		Mat* displayimage = ((Mat*) color);
		Mat* outimage = ((Mat*) out);

	    try{
		    jboolean isCopy1;
		    jdouble* ipoints = 
		           jenv->GetDoubleArrayElements(jarray, &isCopy1);
		    jint n = jenv->GetArrayLength(jarray);

			Rect r  = Rect(ipoints[0],ipoints[1],ipoints[2],ipoints[3]);

			Mat(*displayimage, r).copyTo(*outimage);


			if (isCopy1 == JNI_TRUE) {
		       jenv->ReleaseDoubleArrayElements(jarray, ipoints, JNI_ABORT);
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



	}


}