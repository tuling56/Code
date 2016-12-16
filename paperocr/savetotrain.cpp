/************************************************************************
* Copyright(c) 2015 tuling56
*
* File:	saverults.cpp
* Brief:将结果保存为xml文件,txt文件及识别和定位的结果
* Status:
************************************************************************/
#include <sstream>
#include <fstream>
#include "online.h"

using namespace cv;
using namespace std;

/*
 *	功能：将一个学生试卷的定位和识别结果写入json文件,交互接口
 *	输入:stuNum（二维码识别结果）,vector<SLocAnswer> locanswers (选择题每个部分和主观题每个部分)
 * 
 */
int savetojson(string stuNum, vector<SLocAnswer> locanswers)
{
	string resfile = stuNum + "_tess.json";
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
 *	功能：保存处理后的答题区域,每个答题域一个图像，用于训练样本收集
 *	输入：outpath(训练图像保存位置)，
 *	      
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
