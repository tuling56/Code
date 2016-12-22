#include "../online.h"

using namespace cv;
using namespace std;


//自定义排序函数 (sort by x) 
bool LSortByX(const SLocAnswer &s1, const SLocAnswer &s2)//注意：本函数的参数的类型一定要与vector中元素的类型一致  
{
	Rect rs1 = s1.where;
	Rect rs2 = s2.where;
	return rs1.tl().x < rs2.tl().x;			//升序排列
}


/*
*	功能：判断两个矩形是否相交，和重叠率
*  输入：Rect r1,Rect r2
*  输出：重叠率
*  状态：
*/
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
	float ovlap3= intersection / (area1 + area2 - intersection); //重叠面积占总面积的比

	//最大的重叠率返回
	float ovlapmax = ovlap1;
	if (ovlap2 > ovlapmax)
		ovlapmax = ovlap2;
	if (ovlap3 > ovlapmax)
		ovlapmax = ovlap3;

	return ovlapmax;

}


/* 主观题处理
 * 输入：精定位图像，区域标示（例如：zguanti_1）
 * 输出：（对多数字的支持）位置和识别结果
 */
int zuguantiProcess(Mat preciseimg, string areaflag, vector<SLocAnswer> &locs)
{
	CV_Assert(!preciseimg.empty());
	RNG rng = theRNG();

	//图像漫水和分割（先二值化再漫水）
	Mat floodimg;
	preciseimg.copyTo(floodimg);
	cvtColor(floodimg, floodimg, CV_RGB2GRAY);
	threshold(floodimg, floodimg, 180, 255, CV_THRESH_BINARY_INV);
	int Absolute_offset = 1;
	Mat element = getStructuringElement(MORPH_CROSS, Size(Absolute_offset * 2 + 1, Absolute_offset * 2 + 1), Point(Absolute_offset, Absolute_offset));
	morphologyEx(floodimg, floodimg, CV_MOP_CLOSE, element);
	cvtColor(floodimg, floodimg, CV_GRAY2BGR);

	Mat imgbak;
	copyMakeBorder(preciseimg, imgbak, 1, 1, 1, 1, BORDER_REPLICATE);
	Mat mask(preciseimg.rows + 2, preciseimg.cols + 2, CV_8UC1, Scalar::all(0));
	Mat premask(preciseimg.rows + 2, preciseimg.cols + 2, CV_8UC1, Scalar::all(0));
	Mat now(preciseimg.rows + 2, preciseimg.cols + 2, CV_8UC3, Scalar::all(0));

	const Scalar& colorDiff = Scalar::all(50);
	int flag = 4 | (255 << 8);
	int downarea = 200; // img.cols*img.rows / 35;
	int uparea = preciseimg.cols*preciseimg.rows / 5;
	int widelimit = 0.5* floodimg.cols;

	bool iscontain = false;
	vector<Rect> floodRects;
	for (int y = 0; y < preciseimg.rows; y++)
	{
		for (int x = 0; x < preciseimg.cols; x++)
		{
			if (mask.at<uchar>(y + 1, x + 1) == 0)
			{
				Scalar newVal(rng(256), rng(256), rng(256));
				Rect floodRect;
				mask.copyTo(premask);
				int area = floodFill(floodimg, mask, Point(x, y), newVal, &floodRect, colorDiff, colorDiff,flag);

				if (floodRect.width >widelimit)
					continue;

				//判断是否有嵌套的漫水域
				for (vector<Rect>::iterator it = floodRects.begin(); it != floodRects.end(); it++)
				{
					float bbo=bbOverlap(*it, floodRect);
					if (bbo > 0.8){
						iscontain = true;
						break;
					}		
				}
				if (iscontain)
					continue;

	
				Mat nowmask = mask - premask; //当前受影响域
				imgbak.copyTo(now, nowmask);  //当前受影响域单独提出
				rectangle(imgbak, floodRect, Scalar(0, 0, 255), 1, CV_AA); //在当前受影响域上标注出当前图像
				floodRects.push_back(floodRect);

				Mat tosave = nowmask(floodRect);
				copyMakeBorder(tosave, tosave, 10,10,10,10, BORDER_CONSTANT,Scalar(0));
	
				
				SLocAnswer s;
				s.what = areaflag;
				s.where = floodRect;
				s.pic = tosave;
				locs.push_back(s);
			}
		}
	}


	//step1:初始化引擎
	tesseract::TessBaseAPI tess;
	initOCR(tess);

	//对主观题进行位置左右划分
	sort(locs.begin(),locs.end(), LSortByX);

	//step2:识别和保存
	char s[50];
	int j = 0;
	vector< vector<float> > allconfidences;
	for (vector<SLocAnswer>::iterator it = locs.begin(); it!= locs.end(); it++)
	{
			int conf=0;
			string answervalue;
			vector<string> answercontent;
			vector<float> answerconfidences;
			tess_ocr(tess, it->pic, answervalue,conf,answercontent, answerconfidences);
			//string cnn_res=cnn_ocr(picf,cnnpypath,modulefile,whats);
			it->content = answervalue;
			it->confidences = answerconfidences;
	}

	//step3:关闭引擎
	closeOCR(tess);

	return 0;
}



//功能测试区
int main_partzuguan()
{

	//string filename = "./samples/zuguan_1.bmp";
	string filename = "E:\\Code\\Git\\Code\\paperocr\\samples\\zuguan_1.bmp";
	Mat src = imread(filename);
	if (src.empty()){
		cout << "load fail" << endl;
		return 0;
	}

	vector<SLocAnswer> locs;
	zuguantiProcess(src, "zuguan_1",locs);

	return 0;


}