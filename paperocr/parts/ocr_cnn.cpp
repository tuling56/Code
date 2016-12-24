/************************************************************************
* Copyright(c) 2015 tuling56
*
* File:ocr_cnn.cpp
* Brief: cpp调用Python的Tensor识别模块
* Status: 
* Date:	[6/25/2015 jmy]
************************************************************************/
#include<Python.h> //#include<python2.7/Python.h>  
#include "../common.h"

using namespace std;

/*	功能：python cnn识别的cpp调用
 *	输入：Mat 图像,cnn模块的路径,模型文件路径，要识别的内容的类型
 *	输出：识别结果
 */
string  cnn_ocr(cv::Mat src,string cnnpypath,string modulepath,string whats)
{
	string picvecstr = mat2vecstr(src);

	printf("CNN识别阶段:\n");
	Py_Initialize();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString(cnnpypath.c_str());

	PyObject *pMode = NULL;
	PyObject *pfunc = NULL;
	PyObject *pArg = NULL;
	PyObject *pRet = NULL;

	pMode = PyImport_ImportModule("cnn_ocr");
	pfunc = PyObject_GetAttrString(pMode, "cnn_ocr");
	pArg = Py_BuildValue("sss", picvecstr,modulepath.c_str(),whats.c_str());
	pRet = PyEval_CallObject(pfunc,pArg);

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
/*int main()
{
	string propath="/home/roo/Documents/pythoncpp/paperocr";
	cnn_ocr(propath);
	return 0;

}*/
