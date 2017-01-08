#include "common.h"
#include <stdlib.h>
#include <unistd.h>

using namespace cv;
using namespace std;

//matÍ¼ÏñÏòÁ¿»¯
string mat2vecstr(Mat image)
{
	if (image.channels() == 3){
		cvtColor(image, image, CV_BGR2GRAY);
	}

	//×ª»¯ºÍ¹éÒ»»¯
	image.convertTo(image, CV_32FC1);
	double min = 0;
	double max = 0;
	minMaxIdx(image, &min, &max);
	image = (image-min) / (max - min);

	//¶þÖµ»¯
	//threshold(image, image, 50, 1, THRESH_BINARY_INV);

	char tmp[6];

	ostringstream res;
	res << "[";
	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
			sprintf(tmp, "%5.4f", image.at<float>(j, i));
			if (i == image.rows - 1 && j == image.cols - 1)
				res << tmp;
			else
				res << tmp << ",";
		}
	}
	res << "]";

	return res.str();
}

//¹¦ÄÜ£ºÅÐ¶ÏÁ½¸ö¾ØÐÎÊÇ·ñÏà½»£¬ºÍÖØµþÂÊ
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
	float ovlap3 = intersection / (area1 + area2 - intersection); //ÖØµþÃæ»ýÕ¼×ÜÃæ»ýµÄ±È

	//×î´óµÄÖØµþÂÊ·µ»Ø
	float ovlapmax = ovlap1;
	if (ovlap2 > ovlapmax)
		ovlapmax = ovlap2;
	if (ovlap3 > ovlapmax)
		ovlapmax = ovlap3;

	return ovlapmax;

}

// ¼ÆËãÁ½ÌõÏß¶ÎµÄ¼Ð½Ç:pt0->pt1 and from pt0->pt2
float angle(Point pt1, Point pt2, Point pt0)
{
	float dx1 = pt1.x - pt0.x;
	float dy1 = pt1.y - pt0.y;
	float dx2 = pt2.x - pt0.x;
	float dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}


// ¹¦ÄÜ£º×Ô¶¨ÒåÅÅÐòº¯ÊýÏµÁÐ
bool SortByX(const Rect &p1, const Rect &p2)//×¢Òâ£º±¾º¯ÊýµÄ²ÎÊýµÄÀàÐÍÒ»¶¨ÒªÓëvectorÖÐÔªËØµÄÀàÐÍÒ»ÖÂ  
{
	return p1.tl().x < p2.tl().x;			//ÉýÐòÅÅÁÐ

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
	return rs1.tl().x < rs2.tl().x;			//ÉýÐòÅÅÁÐ
}

//vector<num>Í³¼Æ
int vectorstat(vector<float>  invec,vector<float> & outvec)
{
	float sum = 0;
	float minv = 0;
	float maxv = 0;
	for (vector<float>::iterator it = invec.begin(); it != invec.end(); it++)
	{
		cout<<"now"<<*it<<endl;
		sum = sum + *it;
		if (*it > maxv)
			maxv = *it;
		if (*it < minv)
			minv = *it;
	}
	float mean = sum / invec.size();


	//¼ÆËã·½²î
	float acsum = 0;
	for (vector<float>::iterator it = invec.begin(); it != invec.end(); it++)
	{
		acsum += (*it - mean)*(*it - mean);
	}
	float stdev = sqrt(acsum / (invec.size() - 1));



	//·µ»Ø
	outvec.clear();
	outvec.push_back(sum);
	outvec.push_back(mean);
	outvec.push_back(stdev);
	outvec.push_back(maxv);
	outvec.push_back(minv);

	cout<<"sum"<<sum<<"mean"<<mean<<"stdev"<<stdev<<"maxv"<<maxv<<"minv"<<minv<<endl;

	return 0;
}


//»ñÈ¡µ±Ç°Â·¾¶
string getcurpath()
{
	//µ±Ç°Â·¾¶»ñÈ¡£¬ÓÃÓÚÈ·¶¨Ä£¿éÂ·¾¶µÄÎ»ÖÃ
	char *curpath_tmp;
	curpath_tmp = (char *)malloc(60);
	getcwd(curpath_tmp, 60);
	//printf("µ±Ç°Ö´ÐÐ³ÌÐòÂ·¾¶:%s\n", curpath_tmp);
	string curpath = curpath_tmp;
	free(curpath_tmp);

	return curpath;
}
