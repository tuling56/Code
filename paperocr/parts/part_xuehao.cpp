#include<Python.h>  
//#include<python2.7/Python.h>  
#include<string>
#include<iostream>

using namespace std;

/*	功能：python二维码识别的cpp调用
 *	输入：二维码识别的python脚本所在目录，二维码图像位置
 *	输出：识别结果
 */
string xuehaotiProcess(string pymodulepath,string qr_img_path)
{

	printf("二维码识别阶段:\n");
	Py_Initialize();
	PyRun_SimpleString("import sys");
    
    string importstr="sys.path.append('"+pymodulepath+"')";	
    PyRun_SimpleString(importstr.c_str());

	PyObject *pMode = NULL;
	PyObject *pfunc = NULL;
	PyObject *pArg = NULL;
	PyObject *pRet = NULL;
	
	pMode = PyImport_ImportModule("zbar_xuehao_scan");
	pfunc = PyObject_GetAttrString(pMode, "qr_scan");
	pArg = Py_BuildValue("(s)", qr_img_path.c_str());  //传进去二维码图像文件
	pRet = PyEval_CallObject(pfunc, pArg);

	char* qr_res=NULL;
	int retstaus = PyArg_Parse(pRet, "s", &qr_res); //从返回值取出string类型的返回值
    //printf("识别结果:%s\n",qr_res); 

	Py_DECREF(pfunc);
	Py_Finalize();

	if(retstaus!=0){
        string res=qr_res;
		return res ;	
    }
	else
		return "error";
}

//功能测试区
/*
int main(int argc, char const *argv[])
{
	string modulepath= "/home/roo/Documents/pythoncpp/pythonocr_git/parts";
	string xuehaopic = "/home/roo/Documents/pythoncpp/pythonocr_git/samples/qr.bmp";
    cout<<"cpp调用返回值:"<<xuehaotiProcess(modulepath,xuehaopic)<<endl;
	
	return 0;
}
*/