#include "online.h"
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

using namespace cv;
using namespace std;

//单文件处理
string  singleproc(string filename,string resTrainpath)
{
	Mat src = imread(filename, 1);
	if (src.empty()){
		cout << "load fail" << endl;
		return 0;
	}

    //输入文件
    string fnameExt=filename.substr(filename.rfind("/")+1);
	string fname=fnameExt.substr(0,fnameExt.rfind("."));
	cout<<"文件名:"<<fnameExt<<endl;
	
    //输出以作训练的图像的父目录
	if (NULL==opendir(resTrainpath.c_str())){
		mkdir(resTrainpath.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
	string output = resTrainpath +fname;


	vector<SRPart> rough_area;		//粗定位的区域（相对于整体的位置）
	vector<SLocAnswer> precise_area;	//精确定位区域（相对于粗定位的位置）
	vector<SLocAnswer> loc_answer;	//答题区域和答案(含选择题和主观题)
	
	//step1:粗定位
	roughloc(src, rough_area);
	
	//step2:精确定位
	for (vector<SRPart>::iterator itr=rough_area.begin();itr!=rough_area.end();itr++)
	{
		string areaflag = itr->what;
		precise_area.clear();
		
		Mat rough = src(itr->where);
		int precise_boxnum=preciseloc(rough,areaflag,precise_area);

		if(precise_boxnum==-1) continue;
		
		//对精定位的结果进行识别
		if (precise_boxnum == 0){
			getanswer(rough, areaflag, loc_answer);//如果没定位到的精确区域，则将整个粗定位作为精确定位进行识别
			imwrite(output + "_ocr.png", rough);
			continue;
		}

		for (vector<SLocAnswer>::iterator itp = precise_area.begin(); itp != precise_area.end();itp++)
		{
			Mat precise = rough(itp->where);
			getanswer(precise,areaflag,loc_answer);
			imwrite(output + "_ocr.png", precise);

		}
	}


	//结果文件保存以再训练
	savetotrain(resTrainpath, loc_answer);
	string fres=savetojson(std::string filename, std::vector<SLocAnswer > locanswers);

	return fres;
}


//程序入口
int  main(int argc,char*argv[])
{

	string filename = "./data/img073.jpg";
    string respath  = "./totrain";
	string result=singleproc(filename,respath);
	cout<<"最终识别结果:"<<result<<endl;

	return 0;
}
