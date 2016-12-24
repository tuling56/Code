#include "common.h"
#include <stdlib.h>

using namespace cv;
using namespace std;

//mat图像向量化
string mat2vecstr(Mat image)
{
	if (image.channels() == 3){
		cvtColor(image, image, CV_BGR2GRAY);
	}

	//转化和归一化
	/*
	image.convertTo(image, CV_32FC3);
	double min = 0;
	double max = 0;
	minMaxIdx(image, &min, &max);
	image = (image-min) / (max - 50);
	*/
	threshold(image, image, 50, 1, THRESH_BINARY_INV);

	char tmp[6];

	ostringstream res;
	res << "[";
	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
			sprintf(tmp, "%d", image.at<uchar>(j, i));
			if (i == image.rows - 1 && j == image.cols - 1)
				res << tmp;
			else
				res << tmp << ",";
		}
	}
	res << "]";

	return res.str();
}

//功能：判断两个矩形是否相交，和重叠率
float bbOverlap(Rect& box1, Rect& box2)
{
	if (box1.x > box2.x + box2.width) { return 0.0; }
	if (box1.y > box2.y + box2.height) { return 0.0; }
	if (box1.x + box1.width < box2.x) { return 0.0; }
	if (box1.y + box1.height < box2.y) { return 0.0; }

	float colInt = min(box1.x + box1.width, box2.x + box2.width) - max(box1.x, box2.x);
	float rowInt = min(box1.y + box1.height, box2.y + box2.height) - max(box1.y, box2.y);
	float intersection = colInt * rowInt;

	float area1 = box1.width*box1.height;
	float area2 = box2.width*box2.height;


	float ovlap1 = intersection / area1;
	float ovlap2 = intersection / area2;
	float ovlap3 = intersection / (area1 + area2 - intersection); //重叠面积占总面积的比

	//最大的重叠率返回
	float ovlapmax = ovlap1;
	if (ovlap2 > ovlapmax)
		ovlapmax = ovlap2;
	if (ovlap3 > ovlapmax)
		ovlapmax = ovlap3;

	return ovlapmax;

}


// 计算两条线段的夹角:pt0->pt1 and from pt0->pt2
double angle(Point pt1, Point pt2, Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}



/*
 * 功能：自定义排序函数系列
 */
bool SortByX(const Rect &p1, const Rect &p2)//注意：本函数的参数的类型一定要与vector中元素的类型一致  
{
	return p1.tl().x < p2.tl().x;			//升序排列

	if (abs(p1.tl().x - p2.tl().x) < p1.width / 4){
		return p1.tl().x < p2.tl().x;
	}
	else{
		return false;
	}
}

bool SortByY(const Rect &p1, const Rect &p2)
{
	return p1.tl().y < p2.tl().y;
}

bool SortByWidth(const Rect &r1, const Rect &r2)
{
	return r1.width < r2.width;
}

bool SortByHeight(const Rect &r1, const Rect &r2)
{
	return r1.height < r2.height;
}

bool SortByPx(const Point &p1, const Point &p2)
{
	return p1.y < p2.y;
}

bool SortBySx(const SLocAnswer &s1, const SLocAnswer &s2)
{
	Rect rs1 = s1.where;
	Rect rs2 = s2.where;
	return rs1.tl().x < rs2.tl().x;			//升序排列
}
