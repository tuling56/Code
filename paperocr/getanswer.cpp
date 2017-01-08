#include "common.h"

using namespace cv;
using namespace std;


/*
 * 功能: 区域分解和识别
 * 输入：精确定位后的区域，该答题区域的标示areaflag
 * 输出：每个答题项的位置，意义和（内容）,locs用于存储最终的内容
 * 返回：学号信息（若识别不到学号信息，整个试卷的识别是无意义的）
 */
string areaDecomposeOCR( Mat preciseimg,string areaflag,vector<SLocAnswer> &locs)
{
	CV_Assert( !preciseimg.empty() );
	
	if (areaflag=="xuehaoti")	{		
		cout << "****学号区识别*****" << endl;
		string xuehaoinfo = xuehaotiProcess(preciseimg);
		if (xuehaoinfo=="")
			return "xuehaoerror";
		else
			return xuehaoinfo;
	}
	else if (areaflag=="xuanzeti"){
		cout << "****选择题区识别*****" << endl;
		selectProcess(preciseimg, areaflag, locs);    //此处选择题不返回识别结果
	}
	else if (areaflag.find("zuguanti_")!=string::npos){
		cout << "****解答题区识别******" << endl;
		string zuguaninfo=zuguantiProcess(preciseimg, areaflag, locs);	//处理的是每个主观题（小主观）
		return zuguaninfo;
	}
	else{
		cout << "非定义的识别区域" << endl;
		return "";
	}
}


/*
 * 功能：答题域识别入口 
 * 输入：精确定位后的区域，该答题区域的标示areaflag
 * 输出：每个答题项的位置，意义和（内容）
 * 返回：学号信息
 * 完善：高斯金字塔去噪
 */
string getanswer(Mat preciseimg,string areaflag,vector<SLocAnswer> &answerloc)
{
	//前置预处理程序
	int spatialRad = 20;
	int colorRad = 20;
	int maxPyrLevel = 2;
    
	Mat pyres;
	//pyrMeanShiftFiltering( preciseimg, pyres, spatialRad, colorRad, maxPyrLevel);
    
	
	//调用各个模块的识别
	string ocrinfo=areaDecomposeOCR(preciseimg,areaflag,answerloc);

	return ocrinfo;
}


//功能测试区
int main_getanswer()
{
	string filename = "E:\\Code\\Git\\Code\\paperocr\\samples\\zuguan1.bmp";
	Mat src = imread(filename);
	if (src.empty()){
		cout << "load fail" << endl;
		return 0;
	}

	vector<SLocAnswer> answerloc;
	getanswer(src,"xuanzeti",answerloc);

	return 0;
}
