#ifndef ONLINE_H
#define ONLINE_H

#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>


//tesseatct
#include <baseapi.h>
#include <basedir.h>
//leptonica
#include <leptonica/allheaders.h>

//opencv2
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//定义区块（粗定位）
struct SRPart{
	std::string what;   				//意义
	cv::Rect where;     				//位置	
};


//定义答题位置和答案（细定位和识别）
struct SLocAnswer{
	std::string what;			        //意义		
	cv::Rect where;					    //位置
	cv::Mat pic;						//图像
	std::string content;				//图像整体识别结果
	std::vector<std::string> contents;	//图像内部每个字符的识别结果
	std::vector<float> confidences;    //图像内部每个字符的识别置信率(扩充多选答案的时候)
};


/*****************************工具声明**********************************/
//文件遍历
std::string getPath(std::string s);
std::string getFilenameWithExt(std::string s);
std::string getFilenameWithNoExt(std::string s);

std::string mat2vecstr(cv::Mat image);
float bbOverlap(cv::Rect& box1, cv::Rect& box2);
float angle(cv::Point pt1, cv::Point pt2, cv::Point pt0);

bool SortByX(const cv::Rect &r1, const cv::Rect &r2);
bool SortByY(const cv::Rect &r1, const cv::Rect &r2);
bool SortByWidth(const cv::Rect &r1, const cv::Rect &r2);
bool SortByHeight(const cv::Rect &r1, const cv::Rect &r2);
bool SortByPx(const cv::Point &p1, const cv::Point &p2);
bool SortBySx(const SLocAnswer &s1, const SLocAnswer &s2);

//依次返回和，均值，方差，最大值，最小值
int vectorstat(std::vector<float> invec, std::vector<float> outvec);



/*****************************流程函数声明*******************************/
//粗定位（得到答题的大致区域）
int roughloc(cv::Mat src, std::vector<SRPart > &rougharea);

//精确定位(得到精确的整个答题区)
int preciseloc(cv::Mat src, std::string areaflag,std::vector<SLocAnswer > &locs);

//获取答题内容（精确到每道题和答案）
std::string getanswer(cv::Mat src, std::string areaflag,std::vector<SLocAnswer > &answers);

//*****子系统
std::string xuehaotiProcess(cv::Mat qrgray);
std::string selectProcess(cv::Mat preciseimg, std::string areaflag, std::vector<SLocAnswer > &locs);
std::string zuguantiProcess(cv::Mat preciseimg, std::string areaflag, std::vector<SLocAnswer > &locs);


//识别结果保存和再训练
std::string savetojson(std::string filename,std::string xuehao,std::vector<SLocAnswer > locanswers);
int savetotrain(std::string outpath,std::vector<SLocAnswer > locanswers);


/*****************************功能函数声明*******************************/
//识别答题区（后期将初始化引擎工作提出到外层）
/*part1:tesseract*/
int ocranswer(cv::Mat src, std::string & output, std::vector<std::string> &detect_words, std::vector<float> & detect_confidences);
int ocranswer_seqs(std::vector<cv::Mat> srcs, std::vector<std::string> & outputs, std::vector<std::vector<std::string> > &detect_chars, std::vector<std::vector<float> > & detect_confidences);

int initOCR(tesseract::TessBaseAPI &tess);
int tess_ocr(tesseract::TessBaseAPI &tess, cv::Mat src,std::string&output,int &conf,std::vector<std::string> &detect_words, std::vector<float> & detect_confidences);
int closeOCR(tesseract::TessBaseAPI &tess);

/*part2:cnn*/
std::string cnn_ocr(cv::Mat src,std::string whats);


#endif
