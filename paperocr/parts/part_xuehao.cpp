#include<python2.7/Python.h>  
#include<string>
#include<iostream>

using namespace std;

/*	功能：python二维码识别的cpp调用
 *	输入：二维码图像
 *	输出：识别结果
 */
string xuehaoti(string pymodulepath,string qr_img_path)
{

	printf("二维码识别阶段:\n");
	Py_Initialize();
	PyRun_SimpleString("import sys");
	//PyRun_SimpleString("sys.path.append('./online')");
	PyRun_SimpleString(pymodulepath.c_str());

	PyObject *pMode = NULL;
	PyObject *pfunc = NULL;
	PyObject *pArg = NULL;
	PyObject *pRet = NULL;
	
	pMode = PyImport_ImportModule("zbar_xuehao_scan");
	pfunc = PyObject_GetAttrString(pMode, "qr_scan");
	pArg = Py_BuildValue("(s)", qr_img_path.c_str());  //传进去二维码图像文件
	pRet = PyEval_CallObject(pfunc, pArg);

	string qr_res="";
	int retstaus=PyArg_Parse(pRet,"s",&qr_res) //从返回值取出string类型的返回值

	Py_DECREF(pfunc);
	Py_Finalize();

	if(retstaus==0)
		return qr_res;	
	else
		return "error";
}

//功能测试区
int main(int argc, char const *argv[])
{
	cout<<"cpp调用返回值:"<<xuehao("./","../samples/qr.bmp")<<endl;
	return 0;
}
