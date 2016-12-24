#include "common.h"

using namespace cv;
using namespace std;



/*	功能：点排序（确定每一个点的对应位置）
 *	输入：乱序点；输出：按指定规则排序后的点
 */
int SortPoint(vector<Point> locmarks,vector<Point> &markps)
{
	if (0){
		cout << "排序前:" << endl;
		for (std::vector<Point>::iterator it = locmarks.begin(); it!=locmarks.end(); it++){
			std::cout << it->x << '\t' << it->y << std::endl;
		}
	}
	sort(locmarks.begin(), locmarks.end(), SortByPx);
	if (0){
		cout << "排序后:"<< endl;
		for (std::vector<Point>::iterator it = locmarks.begin(); it!=locmarks.end(); it++){
			std::cout << it->x << '\t' << it->y << std::endl;
		}
	}

	Point p1 = locmarks[0].x < locmarks[1].x ? locmarks[0] : locmarks[1];
	markps.push_back(p1);
	Point p2 = locmarks[0].x > locmarks[1].x ? locmarks[0] : locmarks[1];
	markps.push_back(p2);
	Point p3 = locmarks[2].x < locmarks[3].x ? locmarks[2] : locmarks[3];
	markps.push_back(p3);
	Point p4 = locmarks[2].x > locmarks[3].x ? locmarks[2] : locmarks[3];
	markps.push_back(p4);
	Point p5 = locmarks[4].x < locmarks[5].x ? locmarks[4] : locmarks[5];
	markps.push_back(p5);
	Point p6 = locmarks[4].x > locmarks[5].x ? locmarks[4] : locmarks[5];
	markps.push_back(p6);

	if (0){
		cout << "最终结果:(从左到右,从上到下)" << endl;
		for (std::vector<Point>::iterator it = markps.begin(); it!=markps.end(); it++)
		{
			std::cout << it->x << '\t' << it->y << std::endl;
		}
	}

	return 0;

}



/*
 * 功能：给出矩形，求6个点序列
 * 输入: 矩形；输出：按指定规则排序后的点	
 *
 */
int rect2point(Rect rect, vector<Point> & marks)
{	
	marks.clear();
	Point p1 = rect.tl();
	Point p2 = Point(p1.x + rect.width / 2, p1.y);
	Point p3 = Point(p1.x, p1.y + rect.height / 2);
	Point p4 = Point(p1.x + rect.width, p1.y + rect.height / 2);
	Point p5 = Point(p1.x, p1.y + rect.height);
	Point p6 = rect.br();
	marks.push_back(p1);
	marks.push_back(p2);
	marks.push_back(p3);
	marks.push_back(p4);
	marks.push_back(p5);
	marks.push_back(p6);

	return marks.size();
}



/* 功能：粗定位：轮廓和拟合(确定学号和答题卡区的大致位置)
 * 输入：待定位图像（原始输入）
 * 输出：粗定位结果
 */

int roughloc(Mat src, vector<SRPart>&rougharea)
{
	Mat img, demo;
	src.copyTo(img);
	src.copyTo(demo);

	//高斯模糊平滑 
	if (src.channels() == 3){
		cvtColor(img, img, CV_BGR2GRAY);
	}
	
	//设定ROI区域
	Rect rroi = Rect(0, 0, src.cols, src.rows);
	Mat imgroi = img(rroi);

	//cout<<"morphy process"<<endl;
	GaussianBlur(imgroi, imgroi, Size(9, 9), 2, 2);
	threshold(imgroi, imgroi, 50, 250, CV_THRESH_BINARY_INV);
	int Absolute_offset = 3;
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(Absolute_offset * 2 + 1, Absolute_offset * 2 + 1), Point(Absolute_offset, Absolute_offset));
	morphologyEx(imgroi, imgroi, CV_MOP_OPEN, element);


	//cout<<"lunkuo search"<<endl;
	vector<vector<Point> > contours;
	findContours(imgroi, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	drawContours(demo, contours, -1, Scalar(0, 255, 255), 1);

	//cout<<"lunkuo filter"<<endl;
	vector<vector<Point> > approx;
	vector<Point> locmarks;
	for (size_t i = 0; i < contours.size(); i++)
	{
		float contour_area = contourArea(contours[i]);
		//approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
		//if (fabs(contourArea(Mat(contours[i]))) > 10 && isContourConvex(Mat(contours[i])))
		{
			float radius;
			Point2f center;
			minEnclosingCircle(Mat(contours[i]), center, radius);
			
			float circle_area = 3.14*radius*radius;
			float ratio = circle_area / contour_area;
			if (ratio > 1.2&&ratio<2 && (center.x<imgroi.cols / 4 || center.x>imgroi.cols * 3 / 4))
			{
				circle(demo, Point2f(center), 2, Scalar(0, 0, 255), 2);
				circle(demo, Point2f(center), static_cast<int>(radius), Scalar(0, 255, 0), 2);
				locmarks.push_back(center);
				//cout << "contour_area:" << contour_area << " circle_area:" << circle_area << "  ratio:" << ratio << endl;
			}
			
		}
	}

	//cout<<"loc points"<<endl;
	vector<Point> markps;
	if (locmarks.size() == 6){
		//cout << "we check six" << endl;
		SortPoint(locmarks, markps);
		Rect loc = boundingRect(Mat(locmarks));//定位点出错？
		rectangle(demo, loc-Point(5,5)+ Size(10, 10), Scalar(0, 0, 255), 2);
	}
	else{
		//cout << "we polyfit" << endl;
		Rect loc = boundingRect(Mat(locmarks)); 	
		rect2point(loc, markps);
		rectangle(demo, loc+Size(5,5), Scalar(0, 0, 255), 2);
	}

	//学号区（前4个点）
	vector<Point> xuehao;
	for (int i = 0; i < 4; i++) {
		xuehao.push_back(markps[i]);
	}

	//选择题区（后4个点，其中与学号区共用4个点）
	vector<Point> xuanzeti;
	for (int i = 2; i < 6; i++)	{
		xuanzeti.push_back(markps[i]);
	}

	Rect xhloc = boundingRect(Mat(xuehao));
	Rect xzloc = boundingRect(Mat(xuanzeti));
	Rect zgloc = Rect(markps[4].x,markps[4].y, xzloc.width, src.rows-markps[4].y);

	SRPart sxuehao, sxuanze,szuguan;
	sxuehao.what = "xuehao";
	sxuehao.where = xhloc;

	sxuanze.what = "xuanzeti";
	sxuanze.where = xzloc;

	szuguan.what="zuguanti";
	szuguan.where=zgloc;

	rougharea.push_back(sxuehao);
	rougharea.push_back(sxuanze);
	rougharea.push_back(szuguan);


	if (1){
		rectangle(demo, xhloc, Scalar(255, 255, 0), 2);
		rectangle(demo, xzloc, Scalar(0, 255, 255), 2);
		rectangle(demo, zgloc, Scalar(255, 0, 255), 2);
		//resize(demo, demo, Size(), 0.5, 0.5);
		imshow("rougloc", demo);
		waitKey();
	}

	return 0;
}


//功能测试区
int main_roughloc()
{
	string filename = "./samples/img072.jpg";
	Mat src = imread(filename);
	if (src.empty()){
		cout << "load fail" << endl;
		return 0;
	}

	vector<SRPart> rougharea;
	roughloc(src,rougharea);

	return 0;

}
