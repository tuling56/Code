#include "../online.h"

using namespace cv;
using namespace std;


/* 主观题处理
 * 输入：精定位图像，区域标示（例如：zguanti_1）
 * 输出：位置和识别结果
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

	//对主观题进行位置左右划分
	sort(floodRects.begin(), floodRects.end(), SortByX);

	//识别和保存
	char s[50];
	vector< vector<float> > allconfidences;
	for (vector<Rect>::iterator itrect = floodRects.begin(); itrect != floodRects.end(); itrect++)
	{
			rectangle(now, *itrect, Scalar(0, 0, 255), 1, CV_AA);
			SLocAnswer now_answer;
			ostringstream s2;
			s2 << areaflag << "_answer_" << j;

			now_answer.what = s2.str();
			now_answer.where = (itrect - 1)->tl().y > itrect->tl().y ? *(itrect - 1) : *itrect;

			//矩形缩小,避免边缘干扰
			//Size size(2, 2);
			//now_answer.where = Rect(now_answer.where.tl() + Point(1, 1), now_answer.where.br() - Point(1, 1));

			Mat answer = imgbak(now_answer.where);

			int conf=0;
			string answervalue;
			vector<string> answercontent;
			vector<float> answerconfidences;
			OCR(tess, answern, answervalue,conf,answercontent, answerconfidences);
			now_answer.pic = answer;
			now_answer.content = answervalue;
			now_answer.confidences = answerconfidences;

			locs.push_back(now_answer);

			//结果标注
			float scale_img = (float)(600.f / preciseimg.rows);
			float scale_font = 0.7; // (float)(abs(2 - scale_img)) / 1.2f;
			Size word_size = getTextSize(answervalue, FONT_HERSHEY_SIMPLEX, (double)scale_font, (int)(3 * scale_font), NULL);
			putText(preciseimg, answervalue, now_answer.where.tl(), FONT_HERSHEY_SIMPLEX, scale_font, Scalar(0, 0, 255), (int)(2 * scale_font));
	}

	//关闭引擎
	closeOCR(tess);
}
