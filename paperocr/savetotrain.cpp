/************************************************************************
* Copyright(c) 2015 tuling56
*
* File:	saverults.cpp
* Brief:将结果保存为json文件,待训练图像按识别结果分类保存
* Status:
************************************************************************/
#include "common.h"
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <jsoncpp/json/json.h>

using namespace cv;
using namespace Json;
using namespace std;


/*
 *	功能：将一个学生试卷的定位和识别结果写入json文件,交互接口
 *	输入:imagepath 文件路径，xuehao（二维码识别结果）,vector<SLocAnswer> locanswers (选择题每个部分和主观题每个部分)
 * 
 */
string savetojson(string imagepath,string xuehao, vector<SLocAnswer> locanswers)
{

	Json::Value paper_res;
	paper_res["image_path"]=imagepath;
	paper_res["qr_code"]=xuehao; 
	paper_res["student_id"]="20161202";

	//涵盖选择题和主观题
	for (vector<SLocAnswer>::iterator it = locanswers.begin(); it != locanswers.end(); it++)
	{
		paper_res[it->what]=it->content;
	}

	Json::StyledWriter styled_writer;

	//文件保存
	string resfile = imagepath + "_ocr.json";

	std::ofstream jf;
	jf.open(resfile.c_str(), std::ios::binary);
	jf << styled_writer.write(paper_res);
	jf.close();

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
		//创建子目录(以识别结果为准)
        string contentdir;
        if(it->content=="")
            contentdir="null";
        else
            contentdir=it->content;
            
		string subdir = outpath + "/" + contentdir;
		if (NULL == opendir(subdir.c_str())){
			mkdir(subdir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}
		ostringstream idstr;
		Rect loc = it->where;
		idstr <<"_"<<loc.x << "." << loc.y << "." << loc.width << "." << loc.height;
		string id = idstr.str();
		string filename=subdir+"/"+it->content +id+ ".png";
		imwrite(filename, it->pic);
	}

	return 0;
}
