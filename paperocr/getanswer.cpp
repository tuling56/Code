#include "online.h"

using namespace cv;
using namespace std;


/*
 * 功能: 区域分解和识别
 * 输入：精确定位后的区域，该答题区域的标示areaflag
 * 输出：每个答题项的位置，意义和（内容）
 */
int areaDecomposeOCR( Mat img,string areaflag,vector<SLocAnswer> &locs)
{
	CV_Assert( !img.empty() );
	
	if (areaflag=="xuehao")	{
		cout << "学号区" << endl;
		//do something
	}
	else if (areaflag=="xuanzeti"){
		selectProcess(img, areaflag, locs);
	}
	else if (areaflag=="jieda")	{
		cout << "解答题区" << endl;
		//do something
	}
	else{
		cout << "非定义区域" << endl;
		//do something
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
	
	int spatialRad = 20;
	int colorRad = 20;
	int maxPyrLevel = 2;
    
	Mat pyres;
	//pyrMeanShiftFiltering( preciseimg, pyres, spatialRad, colorRad, maxPyrLevel);
    areaDecomposeOCR(preciseimg,areaflag,answerloc);

	return 0;
}


//功能测试区
/*
int main()
{
	string filename = "./data/img072.jpg";
	Mat src = imread(filename);
	if (src.empty()){
		cout << "load fail" << endl;
		return 0;
	}

	vector<SLocAnswer> answerloc;
	getanswer(src,"xuanzeti",answerloc);

	return 0;
}
*/
