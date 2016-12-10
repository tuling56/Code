/************************************************************************
* Copyright(c) 2015 tuling56
*
* File:	saverults.cpp
* Brief:将结果保存为xml文件,txt文件及识别和定位的结果
* Status:
************************************************************************/
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "../online.h"

using namespace cv;
using namespace std;

/*
 *	功能：将一个学生试卷的定位和识别结果写入json文件 
 *	      交互接口
 * 
 */
int savetojson(string stuNum, vector<SLocAnswer> locanswers)
{
	string resfile = stuNum + "_tess.txt";
	ofstream ocrres(resfile.c_str());
	for (vector<SLocAnswer>::iterator it = locanswers.begin(); it != locanswers.end(); it++)
	{
		ocrres << it->what << ":" << it->content << endl;
	}

	ocrres.close();

	cout<<"识别结果文件保存在:"<<resfile<<endl;
	return 0;
}


/*
 *	功能：保存处理后的答题区域,每个答题域一个图像
 *	      用于训练样本收集
 */
int savetotrain(string outpath, vector<SLocAnswer> locanswers)
{
	for (vector<SLocAnswer>::iterator it = locanswers.begin(); it != locanswers.end(); it++)
	{
		string filename = outpath + it->what + ".png";
		imwrite(filename, it->pic);
	}

	cout<<"答题域文件保存在:"<<outpath<<endl;
	return 0;
}
