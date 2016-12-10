#ifndef ONLINE_H
#define ONLINE_H

#include <vector>
#include <string>
#include <iostream>
#include <string>
#include <stdio.h>
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
struct SRPart
{
	std::string what;   				//意义
	cv::Rect where;     				//位置	
};


//定义答题位置和答案（细定位和识别）
struct SLocAnswer{
	std::string what;			        //意义		
	cv::Rect where;					    //位置
	cv::Mat pic;						//图像
	std::string content;				//图像文字内容
	std::vector<float> confidences;     //置信率(扩充多选答案的时候)
};


/*****************************工具声明*******************************/
//文件遍历
std::string getPath(std::string s);
std::string getFilenameWithExt(std::string s);
std::string getFilenameWithNoExt(std::string s);


/*****************************功能声明*******************************/
//粗定位（得到答题的大致区域）
int roughloc(cv::Mat src, std::vector<SRPart> &rougharea);

//精确定位(得到精确的整个答题区)
int preciseloc(cv::Mat src, std::string areaflag,std::vector<SRPart> &locs);

//获取答题内容（精确到每道题和答案）
int getanswer(cv::Mat src, std::string areaflag,std::vector<SLocAnswer> &answers);

//识别答题区（后期将初始化引擎工作提出到外层）
/*part1；tesseract*/
int ocranswer(cv::Mat src, std::string & output, std::vector<std::string> &detect_words, std::vector<float> & detect_confidences);
int ocranswer_seqs(std::vector<cv::Mat> srcs, std::vector<std::string> & outputs, std::vector<std::vector<std::string> > &detect_chars, std::vector<std::vector<float> > & detect_confidences);

int selectProcess(cv::Mat preciseimg, std::string areaflag, std::vector<SLocAnswer> &locs);
int initOCR(tesseract::TessBaseAPI &tess);
int OCR(tesseract::TessBaseAPI &tess, cv::Mat src,std::string&output,int &conf,std::vector<std::string> &detect_words, std::vector<float> & detect_confidences);
int closeOCR(tesseract::TessBaseAPI &tess);

/*part2:cnn*/
int cnn_ocr(std::string projectpath);


//识别结果保存
int savetojson(std::string stuNum, std::vector<SLocAnswer> locanswers);
int savetotrain(std::string outpath, std::vector<SLocAnswer> locanswers);


#endif