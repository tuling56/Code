/************************************************************************
* Copyright(c) 2015 tuling56
*
* File:	saverults.cpp
* Brief:将结果保存为json文件,待训练图像按识别结果分类保存
* Status:
************************************************************************/
#include "online.h"
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <jsoncpp/json/json.h>

using namespace cv;
using namespace std;
using namespace Json;

/*
 *	功能：将一个学生试卷的定位和识别结果写入json文件,交互接口
 *	输入:stuNum（二维码识别结果）,vector<SLocAnswer> locanswers (选择题每个部分和主观题每个部分)
 * 
 */
string savetojson(string stuNum, vector<SLocAnswer> locanswers)
{
	string resfile = stuNum + "_tess.json";
	ofstream ocrres(resfile.c_str());

	Json::Value paper_res;
	paper_res["image_path"]="/data/img/xxx.jpg";
	paper_res["qr_code"]="paper_id=xxxxxxx&page_num=1"; 
	paper_res["student_id"]="20161202";

	//涵盖选择题和主观题
	for (vector<SLocAnswer>::iterator it = locanswers.begin(); it != locanswers.end(); it++)
	{
		paper_res[it->what]=it->content;
	}

	Json::StyledWriter styled_writer;
	ocrres<<styled_writer(paper_res);
	ocrres.close();

	//返回识别结果
	return styled_writer.write(paper_res);
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
		//创建子目录
		if (NULL == opendir(it->what.c_str())){
			mkdir(resTrainpath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}
		string filename = outpath +"/"+it->what+"/"+it->what + ".png";
		imwrite(filename, it->pic);
	}

	return 0;
}
