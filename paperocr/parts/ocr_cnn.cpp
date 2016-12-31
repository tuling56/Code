/************************************************************************
* Copyright(c) 2015 tuling56
*
* File:ocr_cnn.cpp
* Brief: cpp调用Python的Tensor识别模块
* Status: 
* Date:	[6/25/2015 jmy]
************************************************************************/
#include "../common.h"
#include<python2.7/Python.h>  

using namespace cv;
using namespace std;

/*	功能：python cnn识别的cpp调用
 *	输入：Mat 图像,要识别的内容序列
 *	输出：识别结果
 */
string  cnn_ocr(cv::Mat src,string whats)
{

	string modelname;
	if (whats.find("ABCD") != string::npos)
		modelname = "Model_ABCD";
	else if (whats.find("0123456789") != string::npos)
		modelname = "Model_0-9";
	else{
		cout << "whats indict error" << endl;
		return "model error";
	}
	

	string curpath = getcurpath();
	string cnnpypath = curpath + "/parts/";
	string model = curpath + "/parts/models/cnn/"+modelname;

	string picvecstr = mat2vecstr(src);

	cout<<"CNN识别阶段:"<<endl;
	Py_Initialize();
    //PySys_SetArgv(argc, argv);
    PyRun_SimpleString("import sys");
	string importstr="sys.path.append(\""+cnnpypath+"\")";
    //cout<<"导入模块路径:"<<importstr<<endl;
    PyRun_SimpleString(importstr.c_str());

	PyObject *pMode = NULL;
	PyObject *pfunc = NULL;
	PyObject *pArgs = NULL;
	PyObject *pRet = NULL;

	pMode = PyImport_ImportModule("ocr_cnn");
	pfunc = PyObject_GetAttrString(pMode, "ocr_cnn_api");
    if (!pfunc){
        printf("cann't find ocr_cnn_api\n");
        return "load error";
    }

    //传递三个参数进去
	//cout<<"[cpp]cnnpypath:"<<cnnpypath<<endl;
    cout<<"[cpp]picvecstr:"<<picvecstr<<endl;
    cout<<"[cpp]modelname:"<<model<<endl;
    cout<<"[cpp]whats:"<<whats<<endl;

    pArgs = PyTuple_New(3);
    PyTuple_SetItem(pArgs,0,Py_BuildValue("s",picvecstr.c_str()));
    PyTuple_SetItem(pArgs,1,Py_BuildValue("s",model.c_str()));
    PyTuple_SetItem(pArgs,2,Py_BuildValue("s",whats.c_str()));
	pRet = PyEval_CallObject(pfunc,pArgs);

	//获取返回结果
	char * cnn_res=NULL;
	int retstatus=PyArg_Parse(pRet,"s",&cnn_res);
	Py_DECREF(pfunc);
	Py_Finalize();

	if (retstatus!=0){
		string res=cnn_res;
		return res;
	}
	else
		return "error";	
}

//测试
int main()
{
	Mat src = imread("samples/1.bmp", 0);
	if (src.empty()){
		cout << "load fail" << endl;
		return 0;
	}
	cnn_ocr(src,"ABCD");

	return 0;

}
