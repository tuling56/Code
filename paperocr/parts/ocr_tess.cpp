/************************************************************************
* Copyright(c) 2015 tuling56
*
* File:	ocr_tess.cpp
* Brief: 调用tesseract进行识别的接口
* Reference:
* Status: 
* Date:	[5/6/2015 jmy]
************************************************************************/
#include "../common.h"


using namespace std;
using namespace cv;
using namespace tesseract;

#pragma  comment(lib,"libtesseract302d.lib")
#pragma  comment(lib,"liblept168d.lib")

//////////////////////////////////////////////////////////////////////////方案1
/*
 *	输入：整幅图像
 *  输出：整个识别结果，内部进行了单词的细分和统计
 *        在单词的置信概率上还要改进
 */
int ocranswer(Mat src, string & output, vector<string> &detect_words,vector<float> & detect_confidences)
{
	Mat image;
	if (src.channels() == 3)
		cvtColor(src, image, COLOR_RGB2GRAY);
	else
		src.copyTo(image);
	CV_Assert((image.type() == CV_8UC1) || (image.type() == CV_8UC1));

	//识别的时候又进行了边界扩展
	if (image.rows*image.cols < 5000)	{
		copyMakeBorder(image, image, image.rows / 2, image.rows / 2, image.cols / 2, image.cols / 2, BORDER_REPLICATE);
		copyMakeBorder(src, src, src.rows / 2, src.rows / 2, src.cols / 2, src.cols / 2, BORDER_REPLICATE);
	}

	//初始化工作
	float t = (float)getTickCount();
	tesseract::TessBaseAPI tess;
	const char *language = "eng";
	const char* datapath = "/usr/local/share/tessdata";
	int oemode = 3;
	int psmode = 8;
	if (tess.Init(datapath, language, (tesseract::OcrEngineMode)oemode)){
		cout << "OCRTesseract: Could not initialize tesseract." << endl;
		throw 1;
	}
	tesseract::PageSegMode pagesegmode = (tesseract::PageSegMode)psmode;
	tess.SetPageSegMode(pagesegmode);

	const char*char_whitelist = "0123456789ABCD";
	tess.SetVariable("tessedit_char_whitelist", char_whitelist);
	tess.SetVariable("save_best_choices", "T");
	if (0){
		cout << "OCRTesseract(" << oemode << "|" << psmode << ")" << endl;
		if (datapath != NULL)
			cout << "\t" << datapath << endl;
		if (language != NULL)
			cout << "\t" << language << endl;
		if (char_whitelist != NULL)
			cout << "\t" << char_whitelist << endl;
	}
	t = ((float)getTickCount() - t)/getTickFrequency();  
	cout<<"init cosume time:"<<t<<"(s)"<<endl;


	//整体识别阶段
	t = getTickCount();
	tess.SetImage((uchar*)image.data, image.size().width, image.size().height, image.channels(), image.step1());
	tess.Recognize(0);
	output = string(tess.GetUTF8Text());
	int conf = tess.MeanTextConf();
	string::iterator it;
	for (it = output.begin(); it!= output.end();)
	{
		if (*it == ' ' || *it == '\n')
			output.erase(it);
		else
			++it;
	}
	
	if (output.size()==0){
		cout << "whole OCR result：[]"<<endl;
		tess.Clear();
		cout << "---------------[END]------------------" << endl;
		return 0;
	}
	else{
		cout << "whole OCR result(confidence:"<<conf<<"):" << output << endl;
		cout << "details as follow:>>>" << endl;
	}

	//单区域多识别项问题
	vector<Rect> component_rects;
	vector<string> component_texts;
	vector<float> component_confidences;
	
	tesseract::ResultIterator* ri = tess.GetIterator();
	tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL; //默认识别的基础单位字符
	if (ri != 0) {
		do {
			const char* word = ri->GetUTF8Text(level);
			if (word == NULL)
				continue;
			float conf = ri->Confidence(level); 
			int x1, y1, x2, y2;
			ri->BoundingBox(level, &x1, &y1, &x2, &y2);
			component_texts.push_back(string(word));
			component_rects.push_back(Rect(x1, y1, x2 - x1, y2 - y1));
			component_confidences.push_back(conf);

			//delete[] word;
		} while (ri->Next(level));
	}
	delete ri;
	
	//识别结果分析和标注
	float scale_img = (float)(600.f / image.rows);
	float scale_font = (float)(abs(2 - scale_img)) / 1.2f;
	float min_confidence1 = 41.f, min_confidence2 = 60.f;

	//vector<string> detect_words;
	//vector<float> detect_confidences;
	for (int j = 0; j < (int)component_texts.size(); j++)
	{
		cout << "curcomponent = " << component_texts[j] << "\tcurconfidence = " << component_confidences[j] << endl;
		if (((component_texts[j].size() < 2) && (component_confidences[j] < min_confidence1))||
			((component_texts[j].size() < 4) && (component_confidences[j] < min_confidence2)))
				continue;
		detect_words.push_back(component_texts[j]);
		detect_confidences.push_back(component_confidences[j]);

		//标注单词位置
		rectangle(src, component_rects[j].tl(), component_rects[j].br(), Scalar(255, 0, 255), 1);
		Size word_size = getTextSize(component_texts[j], FONT_HERSHEY_SIMPLEX, (float)scale_font, (int)(3 * scale_font), NULL);

		//标注单词的识别结果
		//rectangle(src, component_rects[j].tl() - Point(3, word_size.height + 3), component_rects[j].tl() + Point(word_size.width, 0), Scalar(255, 0, 255), -1);
		Point2d textloc = component_rects[j].tl() - Point(1, 1);
		textloc.x = textloc.x > 0 ? textloc.x : 0;
		textloc.y = textloc.y > 0 ? textloc.y : 0;
		cout << "high confidence elem:" << component_texts[j];
		cout << " confidence:" << component_confidences[j] << endl;
		//putText(src, component_texts[j], textloc, FONT_HERSHEY_SIMPLEX, scale_font, Scalar(255, 0, 0), (int)(2 * scale_font));
	}
	tess.Clear();	
	
	t = ((float)getTickCount() - t) / getTickFrequency();
	cout << " ocr cost time:" << t;
	cout << "(s)" << endl;
	cout << "---------------[END]------------------" << endl;

	if (0){
		imshow("result", src);
		waitKey();
	}

	return 0;

}



//////////////////////////////////////////////////////////////////////////方案2
/*序列图像识别
 *   输入：序列图像
 *
 */
int ocranswer_seqs(vector<Mat> srcs, vector<string> & outputs, vector<vector<string> > &detect_chars, vector<vector<float> > & detect_confidences)
{
	//初始化工作
	float t = (float)getTickCount();
	tesseract::TessBaseAPI tess;
	const char *language = "eng";
	const char* datapath = "/usr/local/share";
	int oemode = 3;
	int psmode = 8;
	if (tess.Init(datapath, language, (tesseract::OcrEngineMode)oemode)){
		cout << "OCRTesseract: Could not initialize tesseract." << endl;
		throw 1;
	}
	tesseract::PageSegMode pagesegmode = (tesseract::PageSegMode)psmode;
	tess.SetPageSegMode(pagesegmode);

	const char*char_whitelist = "0123456789ABCD";
	tess.SetVariable("tessedit_char_whitelist", char_whitelist);
	tess.SetVariable("save_best_choices", "T");
	if (0){
		cout << "OCRTesseract(" << oemode << "|" << psmode << ")" << endl;
		if (datapath != NULL)
			cout << "\t" << datapath << endl;
		if (language != NULL)
			cout << "\t" << language << endl;
		if (char_whitelist != NULL)
			cout << "\t" << char_whitelist << endl;
	}
	t = ((float)getTickCount() - t) / getTickFrequency();
	cout << "Init consume time:" << t << "(s)" << endl;

	for (vector<Mat>::iterator it = srcs.begin(); it != srcs.end();it++)
	{
		Mat src=*it;
		Mat image;
		if (src.channels() == 3)
			cvtColor(src, image, COLOR_RGB2GRAY);
		else
			src.copyTo(image);
		CV_Assert((image.type() == CV_8UC1) || (image.type() == CV_8UC1));

		//识别的时候又进行了边界扩展
		if (image.rows*image.cols < 5000)	{
			copyMakeBorder(image, image, image.rows / 2, image.rows / 2, image.cols / 2, image.cols / 2, BORDER_REPLICATE);
			copyMakeBorder(src, src, src.rows / 2, src.rows / 2, src.cols / 2, src.cols / 2, BORDER_REPLICATE);
		}
		
		//识别阶段
		t = getTickCount();
		tess.SetImage((uchar*)image.data, image.size().width, image.size().height, image.channels(), image.step1());
		tess.Recognize(0);
		string output = string(tess.GetUTF8Text());
		string::iterator itres;
		for (itres = output.begin(); itres != output.end();)
		{
			if (*itres == ' ' || *itres == '\n')
				output.erase(itres);
			else
				++it;
		}

		if (output.size() == 0){
			cout << "whole OCR result：[]" << endl;
			cout << "---------------[END]------------------" << endl;
			continue;
		}
		
		cout << "whole OCR result:" << output << endl;
		cout << "details as follow:>>>" << endl;
		
		vector<Rect> component_rects;
		vector<string> component_texts;
		vector<float> component_confidences;
		tesseract::ResultIterator* ri = tess.GetIterator();
		tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL; //默认识别的基础单位是单词
		if (ri != 0) {
			do {
				const char* word = ri->GetUTF8Text(level);
				if (word == NULL)
					continue;
				float conf = ri->Confidence(level);
				int x1, y1, x2, y2;
				ri->BoundingBox(level, &x1, &y1, &x2, &y2);
				component_texts.push_back(string(word));
				component_rects.push_back(Rect(x1, y1, x2 - x1, y2 - y1));
				component_confidences.push_back(conf);

				//delete[] word;
			} while (ri->Next(level));
		}
		delete ri;

		//识别结果分析和标注
		float scale_img = (float)(600.f / image.rows);
		float scale_font = (float)(abs(2 - scale_img)) / 1.2f;
		float min_confidence1 = 41.f, min_confidence2 = 60.f;

		vector<string> detect_char;
		vector<float> detect_confidence;
		for (int j = 0; j < (int)component_texts.size(); j++)
		{
			cout << "curcomponent = " << component_texts[j] << "\tcurconfidence = " << component_confidences[j] << endl;
			if (((component_texts[j].size() < 2) && (component_confidences[j] < min_confidence1)) ||
				((component_texts[j].size() < 4) && (component_confidences[j] < min_confidence2)))
				continue;
			detect_char.push_back(component_texts[j]);
			detect_confidence.push_back(component_confidences[j]);

			//标注单词位置
			rectangle(src, component_rects[j].tl(), component_rects[j].br(), Scalar(255, 0, 255), 1);
			Size word_size = getTextSize(component_texts[j], FONT_HERSHEY_SIMPLEX, (float)scale_font, (int)(3 * scale_font), NULL);

			//标注单词的识别结果
			//rectangle(src, component_rects[j].tl() - Point(3, word_size.height + 3), component_rects[j].tl() + Point(word_size.width, 0), Scalar(255, 0, 255), -1);
			Point2d textloc = component_rects[j].tl() - Point(1, 1);
			textloc.x = textloc.x > 0 ? textloc.x : 0;
			textloc.y = textloc.y > 0 ? textloc.y : 0;
			cout << "high confidence elem:" << component_texts[j] << "\tconfidence:" << component_confidences[j] << endl;
			//putText(src, component_texts[j], textloc, FONT_HERSHEY_SIMPLEX, scale_font, Scalar(255, 0, 0), (int)(2 * scale_font));
		}
		
		
		//信息录入
		outputs.push_back(output);
		detect_chars.push_back(detect_char);
		detect_confidences.push_back(detect_confidence);

		t = ((float)getTickCount() - t) / getTickFrequency();
		cout << "cost time:" << t << "(s)" << endl;
		cout << "---------------[END]------------------" << endl;
		
		if (0){
			imshow("result", src);
			waitKey();
		}

	}

	//清理
	tess.Clear();

	return 0;
}




//////////////////////////////////////////////////////////////////////////方案3
/* step1:引擎初始化
*
*/
int initOCR(tesseract::TessBaseAPI &tess)
{
	float t = (float)getTickCount();

	//tesseract::TessBaseAPI tess;
	const char *language = "eng";
	const char* datapath = "/usr/local/share";
	int oemode = 3;
	int psmode = 8;
	if (tess.Init(datapath, language, (tesseract::OcrEngineMode)oemode)){
		cout << "OCRTesseract: Could not initialize tesseract." << endl;
		throw 1;
	}
	tesseract::PageSegMode pagesegmode = (tesseract::PageSegMode)psmode;
	tess.SetPageSegMode(pagesegmode);

	const char*char_whitelist = "0123456789ABCD";
	tess.SetVariable("tessedit_char_whitelist", char_whitelist);
	tess.SetVariable("save_best_choices", "T");
	if (0){
		cout << "OCRTesseract(" << oemode << "|" << psmode << ")" << endl;
		if (datapath != NULL)
			cout << "\t" << datapath << endl;
		if (language != NULL)
			cout << "\t" << language << endl;
		if (char_whitelist != NULL)
			cout << "\t" << char_whitelist << endl;
	}
	t = ((float)getTickCount() - t) / getTickFrequency();
	cout << "init cost time:" << t << "(s)" << endl;

	return 0;

}

/* step2:识别模块
*
*/
int tess_ocr(tesseract::TessBaseAPI &tess, Mat src,string &output,int &conf,vector<string> &detect_words, vector<float> & detect_confidences)
{
	//识别阶段
	float t = (float)getTickCount();
	tess.SetImage((uchar*)src.data, src.size().width, src.size().height, src.channels(), src.step1());
	//tess.Recognize(0);
	output = string(tess.GetUTF8Text());
	conf = tess.MeanTextConf();

	string::iterator it;
	for (it = output.begin(); it != output.end();){
		if (*it == ' ' || *it == '\n')
			output.erase(it);
		else
			++it;
	}

	if (output.size() == 0){
		cout << "整体的识别结果是：[空]" << endl;
		//tess.Clear();
		cout << "---------------[END]------------------" << endl;
		return 0;
	}
	else{
		cout << "whole OCR result(confidence:" << conf << "):" << output << endl;
		cout << "detail as follow：>>>" << endl;
	}

	//整体识别的细分（）
	vector<Rect> component_rects;
	vector<string> component_texts;
	vector<float> component_confidences;
	tesseract::ResultIterator* ri = tess.GetIterator();
	tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL; //默认识别的基础单位是字符
	if (ri != 0) 
	{
		do {
			const char* word = ri->GetUTF8Text(level);
			if (word == NULL||string(word)==" ")
				continue;
			float conf = ri->Confidence(level);
			int x1, y1, x2, y2;
			ri->BoundingBox(level, &x1, &y1, &x2, &y2);
			component_texts.push_back(string(word));
			component_rects.push_back(Rect(x1, y1, x2 - x1, y2 - y1));
			component_confidences.push_back(conf);

			//delete[] word;
		} while (ri->Next(level));
	}
	delete ri;

	//识别结果分析和标注
	float scale_img = (float)(600.f / src.rows);
	float scale_font = (float)(abs(2 - scale_img)) / 1.2f;
	float min_confidence1 = 41.f, min_confidence2 = 60.f;

	//vector<string> detect_words;
	//vector<float> detect_confidences;
	for (int j = 0; j < (int)component_texts.size(); j++)
	{
		cout << "curcomponent = " << component_texts[j] << "\tcurconfidence = " << component_confidences[j] << endl;
		if (((component_texts[j].size() < 2) && (component_confidences[j] < min_confidence1)) ||
			((component_texts[j].size() < 4) && (component_confidences[j] < min_confidence2)))
			continue;
		detect_words.push_back(component_texts[j]);
		detect_confidences.push_back(component_confidences[j]);

		//标注字符位置
		rectangle(src, component_rects[j].tl(), component_rects[j].br(), Scalar(255, 0, 255), 1);
		Size word_size = getTextSize(component_texts[j], FONT_HERSHEY_SIMPLEX, (float)scale_font, (int)(3 * scale_font), NULL);

		//标注字符识别结果
		//rectangle(src, component_rects[j].tl() - Point(3, word_size.height + 3), component_rects[j].tl() + Point(word_size.width, 0), Scalar(255, 0, 255), -1);
		Point2d textloc = component_rects[j].tl() - Point(1, 1);
		textloc.x = textloc.x > 0 ? textloc.x : 0;
		textloc.y = textloc.y > 0 ? textloc.y : 0;
		cout << "high confidence elem:" << component_texts[j] << "\tconfidence:" << component_confidences[j] << endl;
		//putText(src, component_texts[j], textloc, FONT_HERSHEY_SIMPLEX, scale_font, Scalar(255, 0, 0), (int)(2 * scale_font));
	}
	t = ((float)getTickCount() - t) / getTickFrequency();
	cout << "cost time:" << t << "(s)" << endl;
	cout << "---------------[END]------------------" << endl;

	if (0){
		imshow("result", src);
		waitKey();
	}

	return 0;
}


/* step3:关闭引擎
 *	
 */
int closeOCR(tesseract::TessBaseAPI &tess)
{
	tess.Clear();
	tess.End();
	return 0;
}

//功能测试区
int main_ocr_tess()
{
	return 0;

}