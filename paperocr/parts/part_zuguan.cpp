#include "../common.h"

using namespace cv;
using namespace std;



/* 主观题处理
 * 输入：主观题精定位图像，区域标示（例如：zguanti_1）
 * 输出：（对多数字的支持）位置的单独识别结果（vector<SLocAnswer> &locs）
 * 返回：整个主观题的识别结果
 * 问题： 此处没有记录整个主观题的结果
 */
string zuguantiProcess(Mat preciseimg, string areaflag, vector<SLocAnswer> &wlocs)
{
	cout<<areaflag<<" 的识别"<<endl;
    CV_Assert(!preciseimg.empty());
	RNG rng = theRNG();

	//首先将主观题分割成单个字符
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

	vector<SLocAnswer> &locs
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
	
				//处理的是主观题的细节
				SLocAnswer s;
				s.what = areaflag;
				s.where = floodRect;
				s.pic = tosave;
				locs.push_back(s);
			}
		}
	}

	//对符合条件的主观题局部进行识别

	/***part1:cnn识别和保存****/
	//cout<<">>>主观题的cnn识别(拼接细分)"<<endl;
	string whats = "0123456789";
	sort(locs.begin(), locs.end(), SortBySx); 	   //对主观题多数字情况下进行位置左右划分(此处也有问题啊)
	ostringstream cnn_res;
	for (vector<SLocAnswer>::iterator it = locs.begin(); it!= locs.end(); it++)  
	{
		string res="cnn结果";//cnn_ocr(it->pic,whats);
		it->content = res;
		cnn_res << res;		//cnn识别的组合结果
	}

	string cnnocr = cnn_res.str();

	/***part2:tess识别和保存*****/
    //cout<<">>>主观题的tess识别(整体)"<<endl;
	tesseract::TessBaseAPI tess;
	initOCR(tess);

	int conf = 0;
	string tessocr;
	vector<string> tessocr_detail;
	vector<Mat> tessocr_detail_pic;
	vector<float> tessocr_detail_confidence;
	tess_ocr(tess, preciseimg, tessocr, conf, tessocr_detail,confidences);

	closeOCR(tess);

	//细节存入
	vector<Sinfo> dinfo;
	for (int i = 0; i < tessocr_detail.size(); ++i)
	{
		Struct sinfo;
		sinfo.content=tessocr_detail[i];
		sinfo.pic=tessocr_detail_pic[i]
		sinfo.confidence=tessocr_detail_confidence[i];
		dinfo.push_back(sinfo);
	}
	now_answer.dinfo=dinfo;

	wlocs.insert(locs.begin(), locs.end()); //将主观题的处理结果插入最终的识别结果序列

	/***part3:结果评判***/
	if(confidences.size()==0)
	{
		if (conf>70)
		{
			cout<<"无细节，返回tess的结果"<<endl;
			return tessocr;
		}
		else
		{
			cout<<"无细节，返回cnn的结果"<<endl;
			return cnnocr;
		}
	}
	else
	{
		vector<float> statres;
		vectorstat(confidences, statres);
		if (statres[1] > 70)
	    {	
	        cout<<"返回tess的结果"<<endl;
	        return tessocr;
	    }
		else
	    {
	    	cout<<"返回cnn的结果"<<endl;
			return cnnocr;
	    }
    }
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
