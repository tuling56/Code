#include "../online.h"

using namespace cv;
using namespace std;


//自定义排序函数 (sort by x) 
bool SortByX(const Rect &p1, const Rect &p2)//注意：本函数的参数的类型一定要与vector中元素的类型一致  
{
	return p1.tl().x < p2.tl().x;//升序排列

	if (abs(p1.tl().x - p2.tl().x)<p1.width / 4){
		return p1.tl().x < p2.tl().x;
	}
	else
	{
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


/*
* 完成答案和题号的绑定工作
* 输入：满足条件的答题域floodRects，该答题域的标示areaflag
* 输出：经过校正和编号的答题域
*/
int bindTihaoAnswer(Mat demo, vector<Rect> &floodRects, string areaflag, vector<Rect> &vrectify)
{
	if (floodRects.size() % 2 != 0)	{
		cout << "必须进行矫正"<< endl;
	}

	sort(floodRects.begin(), floodRects.end(), SortByWidth);
	int minwidth = floodRects[0].width;
	int maxwidth = floodRects[floodRects.size() - 1].width;

	sort(floodRects.begin(), floodRects.end(), SortByHeight);
	int minheight = floodRects[0].height;
	int maxheight = floodRects[floodRects.size() - 1].height;


	sort(floodRects.begin(), floodRects.end(), SortByY);
	int miny = floodRects[0].tl().y;
	int maxy = floodRects[floodRects.size() - 1].br().y;

	sort(floodRects.begin(), floodRects.end(), SortByX);
	int minx = floodRects[0].tl().x;
	int maxx = floodRects[floodRects.size() - 1].tl().x;

	int widthsum = 0;
	int heightsum = 0;

	int wflag = 0;
	int hflag = 0;
	for (vector<Rect>::iterator itrect = floodRects.begin(); itrect != floodRects.end(); itrect++)
	{
		rectangle(demo, *itrect, Scalar(255, 0, 255), -1);
		widthsum += itrect->width;
		heightsum += itrect->height;
	}
	int widthmean = 1.1*(widthsum - maxwidth - minwidth) / (floodRects.size() - 2);
	int heightmean = 1.1*(heightsum - maxheight - minheight) / (floodRects.size() - 2);


	//补框
	vector<Rect> recover;
	for (vector<Rect>::iterator itr = floodRects.begin(); itr != floodRects.end(); itr++)
	{
		//缺一补绿
		if (((itr != floodRects.end() - 1) && abs(itr->tl().x - (itr + 1)->tl().x) < 0.5*widthmean) ||
			(itr != floodRects.begin() && abs(itr->tl().x - (itr - 1)->tl().x) < 0.5*widthmean)) //满额
		{
			//cout << "满额" << endl;
			recover.push_back(*itr);
			//continue;
		}
		else if ((itr != floodRects.end() - 1 && (abs(itr->tl().x - (itr + 1)->tl().x) < 1.5*widthmean)) || itr == floodRects.end() - 1) //缺一补1
		{
			if (itr->tl().y > 0.5*heightmean + miny) //上缺
			{
				Rect up = *itr - Point(0, heightmean);
				recover.push_back(up);
				recover.push_back(*itr);
				rectangle(demo, up, Scalar(0, 255, 0), -1);

			}
			else
			{
				recover.push_back(*itr);
				Rect down = *itr + Point(0, heightmean);
				recover.push_back(down);
				rectangle(demo, down, Scalar(0, 255, 0), -1);
			}
		}

		//缺2补天蓝
		if (itr != floodRects.end() - 1 && (abs(itr->tl().x - (itr + 1)->tl().x) > 1.5*widthmean))
		{
			if (itr->tl().y<miny+0.5*heightmean) //补下框
			{
				Rect down = *itr + Point(0, heightmean);
				Rect rup = *itr + Point(widthmean, 0);
				Rect rdown = *itr + Point(widthmean, heightmean);
				recover.push_back(*itr);
				recover.push_back(down);
				recover.push_back(rup);
				recover.push_back(rdown);
				rectangle(demo, down, Scalar(255, 255,0 ), -1);
				rectangle(demo, rup, Scalar(255, 255, 0), -1);
				rectangle(demo, rdown, Scalar(255, 255, 0), -1);
			}
			else  
			{
				Rect up = *itr - Point(0, heightmean);
				Rect rright = *itr + Point(widthmean, 0);
				Rect rup = *itr + Point(widthmean, -heightmean);
				recover.push_back(up);
				recover.push_back(*itr);
				recover.push_back(rup);
				recover.push_back(rright);
				rectangle(demo, up, Scalar(255, 255, 0), -1);
				rectangle(demo, rup, Scalar(255, 255, 0), -1);
				rectangle(demo, rright, Scalar(255, 255, 0), -1);
			}

		}


		circle(demo, Point(itr->tl().x + widthmean / 2, itr->tl().y + heightmean / 2), 3, Scalar(0, 0, 255), 1);
	}

	//修框（黄标）
	int n = 1;
	int lrmove = widthmean / 5;
	for (vector<Rect>::iterator itr = recover.begin(); itr != recover.end(); itr++)
	{
		if (n % 2 == 0)
		{
			int  minx = (itr - 1)->tl().x<itr->tl().x ? (itr - 1)->tl().x : itr->tl().x;
			int  maxx = (itr - 1)->br().x>itr->br().x ? (itr - 1)->br().x : itr->br().x;
			bool mod = false;
			
			if (itr->tl().x>minx+lrmove || itr->br().x<maxx-lrmove)	{
				mod = true;
				Rect mod = Rect(Point(minx, itr->tl().y), Point(maxx, itr->br().y));
				vrectify.push_back(*(itr - 1));
				vrectify.push_back(mod);
				rectangle(demo, mod, Scalar(0, 255, 255), -1);
			}

			if ((itr - 1)->tl().x>minx+lrmove || (itr - 1)->br().x < maxx-lrmove)	{
				mod = true;
				Rect mod = Rect(Point(minx, (itr - 1)->tl().y), Point(maxx, (itr - 1)->br().y));
				vrectify.push_back(mod);
				vrectify.push_back(*itr);
				rectangle(demo, mod, Scalar(0, 255, 255), -1);
			}

			if (!mod)	{
				vrectify.push_back(*(itr - 1));
				vrectify.push_back(*itr);
			}


		}
		n++;
	}

 	if (0){
		imshow(areaflag, demo);
		waitKey();
	}


	return 0;
}


/*
* 单个选择项的图像处理
* 输入：
* 输出：
*/
int areaAdjust(Mat &img, Mat &srcn)
{
	//边界扩充
	Mat src;
	img.copyTo(src);
	copyMakeBorder(src, srcn, 5, 5, 5, 5, BORDER_CONSTANT, Scalar::all(255));

	
	//二值化
	int adaptive_method = CV_ADAPTIVE_THRESH_GAUSSIAN_C;
	int block_size = 3;
	double offset = 12.0;
	if (src.channels() == 3){
		cvtColor(srcn, srcn, CV_BGR2GRAY);
	}
	threshold(srcn, srcn, 200, 255, CV_THRESH_BINARY_INV|CV_THRESH_OTSU);
	//adaptiveThreshold(srcn, srcn, 255, adaptive_method, CV_THRESH_BINARY_INV, block_size, offset);
	cvtColor(srcn, srcn, CV_GRAY2BGR);

	//形态学处理
	int Absolute_offset = 1;
	Mat element = getStructuringElement(MORPH_CROSS, Size(Absolute_offset * 2 + 1, Absolute_offset * 2 + 1), Point(Absolute_offset, Absolute_offset));
	dilate(srcn, srcn, element);
	erode(srcn, srcn, element);

	resize(srcn, srcn, Size(), 2, 2); //图像区域调整

	//选取最大连通域



	return 0;
}



/* 选择题处理
 * 输入：精定位图像，区域标示
 * 输出：位置和识别结果
 */
int selectProcess(Mat preciseimg, string areaflag, vector<SLocAnswer> &locs)
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
	Mat now(preciseimg.rows + 2, preciseimg.cols + 2, CV_8UC3, Scalar::all(0));

	const Scalar& colorDiff = Scalar::all(50);
	int flag = 4 | (255 << 8);
	int downarea = 200; // img.cols*img.rows / 35;
	int uparea = preciseimg.cols*preciseimg.rows / 5;

	vector<int> floodArea;
	vector<float> floodRatio;
	vector<Rect> floodRects;
	for (int y = 0; y < preciseimg.rows; y++)
	{
		for (int x = 0; x < preciseimg.cols; x++)
		{
			if (mask.at<uchar>(y + 1, x + 1) == 0)
			{
				Scalar newVal(rng(256), rng(256), rng(256));
				Rect floodRect;
				int area = floodFill(floodimg, mask, Point(x, y), newVal, &floodRect, colorDiff, colorDiff,flag);

				float  wrap_ratio = min(float(floodRect.width) / floodRect.height, float(floodRect.height) / floodRect.width);
				float  occupation_ratio = float(area) / float(floodRect.area());
				if (area<downarea || area>uparea)
					continue;

				if (wrap_ratio < 0.7 || occupation_ratio < 0.7)
					continue;

				floodArea.push_back(area);
				floodRatio.push_back(wrap_ratio);
				floodRects.push_back(floodRect);

				imgbak.copyTo(now, mask);
				//rectangle(now, floodRect, Scalar(0, 0, 255), 1, CV_AA);

			}
		}
	}

	if (floodRects.size() == 0){
		cout << "Detected no fit areas" << endl;
		return 0;
	}

	//初始化引擎
	tesseract::TessBaseAPI tess;
	initOCR(tess);

	//答案定位和识别
	vector<Rect> recover;
	bindTihaoAnswer(now, floodRects, "xuanzeti", recover);
	sort(recover.begin(), recover.end(), SortByX);
	int n = 1, j = 0;
	char s[50];
	vector< vector<float> > allconfidences;
	for (vector<Rect>::iterator itrect = recover.begin(); itrect != recover.end(); itrect++)
	{
		rectangle(now, *itrect, Scalar(0, 0, 255), 1, CV_AA);
		//cout << "curponit:\tx:" << itrect->tl().x << " y:" << itrect->tl().y << endl;

		if (n % 2 == 0)
		{
			SLocAnswer now_answer, now_tihao;
			ostringstream s1, s2;
			s1 << areaflag << "_tihao_" << j;
			s2 << areaflag << "_answer_" << j;

			now_tihao.what = s1.str();
			now_answer.what = s2.str();

			now_tihao.where = (itrect - 1)->tl().y < itrect->tl().y ? *(itrect - 1) : *itrect;
			now_answer.where = (itrect - 1)->tl().y > itrect->tl().y ? *(itrect - 1) : *itrect;

			//矩形缩小,避免边缘干扰
			//Size size(2, 2);
			//now_tihao.where = Rect(now_tihao.where.tl() + Point(1, 1), now_tihao.where.br() - Point(1, 1));
			//now_answer.where = Rect(now_answer.where.tl() + Point(1, 1), now_answer.where.br() - Point(1, 1));

			Mat tihao = imgbak(now_tihao.where);
			Mat answer = imgbak(now_answer.where);

			//边界扩充
			Mat tihaon, answern;
			areaAdjust(tihao, tihaon);
			areaAdjust(answer, answern);

			string tihaovalue;
			int conf=0;
			vector<string> tihaocontent;
			vector<float> tihaoconfidences;
			OCR(tess, tihaon, tihaovalue,conf,tihaocontent, tihaoconfidences);
			now_tihao.pic = tihao;
			now_tihao.content = tihaovalue;
			now_tihao.confidences = tihaoconfidences;

			string answervalue;
			vector<string> answercontent;
			vector<float> answerconfidences;
			OCR(tess, answern, answervalue,conf,answercontent, answerconfidences);
			now_answer.pic = answer;
			now_answer.content = answervalue;
			now_answer.confidences = answerconfidences;

			locs.push_back(now_tihao);
			locs.push_back(now_answer);

			//结果标注
			float scale_img = (float)(600.f / preciseimg.rows);
			float scale_font = 0.7; // (float)(abs(2 - scale_img)) / 1.2f;
			Size word_size = getTextSize(answervalue, FONT_HERSHEY_SIMPLEX, (double)scale_font, (int)(3 * scale_font), NULL);
			putText(preciseimg, answervalue, now_answer.where.tl(), FONT_HERSHEY_SIMPLEX, scale_font, Scalar(0, 0, 255), (int)(2 * scale_font));

			/*
			sprintf(s, "segres/t/tihao_%d.jpg", j);
			imwrite(s, tihaon);
			sprintf(s, "segres/t/answer_%d.jpg", j);
			imwrite(s, answern);
			*/

			j++;
		}
		n++;
	}

	//关闭引擎
	closeOCR(tess);

	return 0;
}