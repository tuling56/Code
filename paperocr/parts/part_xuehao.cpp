#include "../common.h"
#include <zbar.h>


using namespace std;
using namespace cv;
using namespace zbar;


/* 功能：直接在opencv中使用zbar库进行二维码识别
 * 输入：二维码灰度图像
 * 输出：二维码类型和识别结果
 */
string xuehaotiProcess(Mat imageGray)
{
	if (imageGray.channels() == 3)
		cvtColor(imageGray, imageGray,CV_BGR2GRAY);

	int width = imageGray.cols;
	int height = imageGray.rows;
	uchar *raw = (uchar *)imageGray.data;

	ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

	Image imageZbar(width, height, "Y800", raw, width * height);
	scanner.scan(imageZbar);								//扫描条码    
	Image::SymbolIterator symbol = imageZbar.symbol_begin();
	if (imageZbar.symbol_begin() == imageZbar.symbol_end()){
		cout << "查询条码失败，请检查图片！" << endl;
	}

	string qrres;
	for (; symbol != imageZbar.symbol_end(); ++symbol){
		//cout << "类型：" << endl << symbol->get_type_name() << endl;
		qrres = symbol->get_data();
		//cout << "	学号：" << qrres<< endl;
	}

	imageZbar.set_data(NULL, 0);
	
	return qrres;
}

//功能测试区
/*
int main(int argc, char const *argv[])
{
	Mat src = imread("samples/qr.bmp", 0);
	if (src.empty()){
		cout << "load fail" << endl;
		return 0;
	}
	cout << "返回值：" << xuehaotiProcess(src) << endl;;
	
	return 0;
}
*/