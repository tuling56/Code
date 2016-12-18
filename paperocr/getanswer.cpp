#include "online.h"

using namespace cv;
using namespace std;


/*
 * 功能: 区域分解和识别
 * 输入：精确定位后的区域，该答题区域的标示areaflag
 * 输出：每个答题项的位置，意义和（内容）
 */
int areaDecomposeOCR( Mat preciseimg,string areaflag,vector<SLocAnswer> &locs)
{
	CV_Assert( !preciseimg.empty() );
	
	if (areaflag=="xuehao")	{			//调用python接口返回学号
		//cout << "学号区" << endl;
		string xuehaopath = "";
		string xuehaoimg = "";
		string xuehao = xuehaotiProcess(xuehaopath, xuehaoimg);
	}
	else if (areaflag=="xuanzeti"){
		selectProcess(preciseimg, areaflag, locs);
	}
	else if (areaflag=="zuguanti")	{
		//cout << "解答题区" << endl;
		zuguantiProcess(preciseimg, areaflag, locs); //处理的是每个主观题
	}
	else{
		//cout << "非定义区域" << endl;
		//do something
		return 0;
	}

	return 0;	
}


/*
 * 功能：答题域识别入口 
 * 输入：精确定位后的区域，该答题区域的标示areaflag
 * 输出：每个答题项的位置，意义和（内容）
 * 完善：高斯金字塔去噪
 */
int getanswer(Mat preciseimg,string areaflag,vector<SLocAnswer> &answerloc)
{
	
	//前置预处理程序
	int spatialRad = 20;
	int colorRad = 20;
	int maxPyrLevel = 2;
    
	Mat pyres;
	//pyrMeanShiftFiltering( preciseimg, pyres, spatialRad, colorRad, maxPyrLevel);
    
	
	//调用各个模块的识别
	areaDecomposeOCR(preciseimg,areaflag,answerloc);

	return 0;
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
