/************************************************************************
* Copyright(c) 2015 tuling56
*
* File:ocr_cnn.cpp
* Brief: cpp调用Python的Tensor识别模块
* Status: 
* Date:	[6/25/2015 jmy]
************************************************************************/
#include<Python.h> //#include<python2.7/Python.h>  
#include<stdio.h>
#include<string>

using namespace std;

/*	功能：python cnn识别的cpp调用
 *	输入：python cnn模块的路径，分割和定位后的图像路径（和待训练结果保存的异同？）
 *	输出：识别结果
 */
int cnn_ocr(string cnnpypath,string vdatapath)
{
	printf("CNN识别阶段:\n");
	Py_Initialize();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString(cnnpypath.c_str());

	PyObject *pMode = NULL;
	PyObject *pfunc = NULL;
	PyObject *pArg = NULL;
	
	printf("\tstep1:图像向量化....\n");
	pMode = PyImport_ImportModule("img_proc");
	pfunc = PyObject_GetAttrString(pMode, "img_proc");
	pArg = Py_BuildValue("(s)", vdatapath.c_str());  
	PyEval_CallObject(pfunc, pArg);
	Py_DECREF(pfunc);

	
	printf("\tstep2:cnn识别....\n");
	pMode = PyImport_ImportModule("cnn_ocr");
	pfunc = PyObject_GetAttrString(pMode, "cnn_ocr");
	PyEval_CallObject(pfunc,pArg);
	Py_DECREF(pfunc);

	Py_Finalize();
	
	return 0;
}

//测试
/*int main()
{
	string propath="/home/roo/Documents/pythoncpp/paperocr";
	cnn_ocr(propath);
	return 0;

}*/
