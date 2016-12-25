#include "common.h"
#include <math.h>


using namespace cv;
using namespace std;


/* 功能：精确定位
 * 输入：粗定位图像，该区域标识
 * 输出：细定位的结果（相对于粗定位的位置）
 * 返回：精确定位到的结果区域的个数
 *
 */
int preciseloc(Mat roughimg,string areaflag,vector<SLocAnswer> &precise_boxes)
{
	//学号二维码精确定位
	if (areaflag=="xuehaoti")
	{
		//二值化
		Mat floodimg;
		roughimg.copyTo(floodimg);
		cvtColor(floodimg, floodimg, CV_RGB2GRAY);
		threshold(floodimg, floodimg, 180, 255, CV_THRESH_BINARY_INV);
		
		//形态学处理
		int Absolute_offset = 1;
		Mat element = getStructuringElement(MORPH_CROSS, Size(Absolute_offset * 2 + 1, Absolute_offset * 2 + 1), Point(Absolute_offset, Absolute_offset));
		morphologyEx(floodimg, floodimg, CV_MOP_CLOSE, element);
		//cvtColor(floodimg, floodimg, CV_GRAY2BGR);

		//轮廓验证
		vector<vector<Point> > contours;
		Mat demo;
		roughimg.copyTo(demo);
		findContours(floodimg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		drawContours(demo, contours, -1, Scalar(0, 0, 255), 1);

		//cout<<"lunkuo filter"<<endl;
		int downarea = 250;
		int uparea = floodimg.rows*floodimg.cols / 5;
		int maxarea = 0;
		int maxcontour = 0;
		vector<vector<Point> > approx;
		for (size_t i = 0; i < contours.size(); i++)
		{
			float contour_area = contourArea(contours[i]); //轮廓的面积的计算规则
			Rect box = boundingRect(Mat(contours[i]));
			float ratio = min(float(box.width) / box.height, float(box.height) / box.width);
			if (contour_area>downarea && contour_area < uparea && ratio>0.9)
			{
				maxarea = contour_area>maxarea ? contour_area : maxarea;
				maxcontour = i;
			}
		}

		//最大核验
		{
			float radius;
			Point2f center;
			Rect brect = boundingRect(Mat(contours[maxcontour]));
			cv::rectangle(demo, brect, Scalar(0, 255, 0), 2,8);

			float rect_area = brect.width*brect.height;
			float ratio = rect_area / maxarea;
			if (ratio > 1 &&ratio < 1.2)
			{
				//将这个轮廓的外接矩形保存
				Mat qr = roughimg(brect);
				copyMakeBorder(qr, qr, 5, 5, 5, 5, BORDER_CONSTANT, Scalar(255));
				//imshow("now arc", qr);
				//waitKey();

				//外框学号二维码纳入精确定位结果
				SLocAnswer part_xuehao;
				ostringstream s1;
				part_xuehao.what = areaflag;
				part_xuehao.where = brect;
				part_xuehao.pic = qr;
				precise_boxes.push_back(part_xuehao);

				imshow("xuehao_precise_loc", qr);
				waitKey();
			}
		}
	}

	//选择题精确定位
	if( areaflag=="xuanzeti" )
	{
		Mat pyr,demo,timg, gray;
		roughimg.copyTo(demo);

	    pyrDown(roughimg, pyr, Size(roughimg.cols/2, roughimg.rows/2));
	    pyrUp(pyr, timg, Size(roughimg.cols/2*2,roughimg.rows/2*2));
		Mat gray0(timg.size(), CV_8U);

		vector<vector<Point> > squares;
	    vector<vector<Point> > contours;

		int thresh = 50, N = 1;
	    for( int c = 0; c < 1; c++ )
	    {
	        int ch[] = {c, 0};
	        mixChannels(&timg, 1, &gray0, 1, ch, 1);

	        for( int l = 0; l < N; l++ )
	        {
	            if( l == 0 ) {
	                Canny(gray0, gray,0, thresh, 5);
					Mat element = getStructuringElement(MORPH_RECT, Size(2,2));
	                dilate(gray, gray, Mat(), Point(-1,-1));
					erode(gray, gray, element);
	            }
	            else
	            {
	                // apply threshold if l!=0:
	                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
	                gray = gray0 >= (l+1)*255/N;
	            }

				findContours(gray, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	            vector<Point> approx;
	            for( size_t i = 0; i < contours.size(); i++ )
	            {
	                approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
	                if( approx.size() == 4 &&fabs(contourArea(Mat(approx))) > 1000 &&isContourConvex(Mat(approx)) )
	                {
						//最小外接矩
						Rect box = boundingRect(approx);
						float ratio = min(float(box.width) / box.height, float(box.height) / box.width);
						if (ratio > 0.7){
							continue;
						}

						//拟合的四边形是否接近矩形
	                    float maxCosine = 0;
	                    for( int j = 2; j < 5; j++ ) {
	                        float cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
	                        maxCosine = MAX(maxCosine, cosine);
	                    }

						if (maxCosine < 0.2){
							squares.push_back(approx); 

							//精确定位在粗定位上进行了12像素的扩张(避免矩形扭曲带来的部分截断)
							Rect expandbox=Rect(box.tl() + Point(-6, -6), box.br()+ Point(6, 6));
							RNG rng = theRNG();
							Scalar newVal(rng(256), rng(256), rng(256));
							rectangle(demo, expandbox,Scalar(0,255,0), 2, CV_AA);
							rectangle(demo, box, newVal, 1, CV_AA);		

							//选择题框纳入精定位结果(这里是精确定位的结果，选择题答题域（细分粗域和细域）)
							SLocAnswer part_select;
							part_select.what = areaflag;
							part_select.where = expandbox;
							part_select.pic = roughimg(expandbox);
							precise_boxes.push_back(part_select);
						}	
	                }
	            }
	        }
	    }

	    //效果展示
		imshow("select_precise_loc", demo);
		waitKey();
	}
	
	//主观题精确定位
	if (areaflag=="zuguanti")
	{
	
		//二值化
		Mat floodimg;
		roughimg.copyTo(floodimg);
		cvtColor(floodimg, floodimg, CV_RGB2GRAY);
		threshold(floodimg, floodimg, 180, 255, CV_THRESH_BINARY_INV);
		
		//形态学处理
		int Absolute_offset = 1;
		Mat element = getStructuringElement(MORPH_CROSS, Size(Absolute_offset * 2 + 1, Absolute_offset * 2 + 1), Point(Absolute_offset, Absolute_offset));
		morphologyEx(floodimg, floodimg, CV_MOP_CLOSE, element);
		cvtColor(floodimg, floodimg, CV_GRAY2BGR);

		//漫水
		Mat imgbak;
		copyMakeBorder(roughimg, imgbak, 1, 1, 1, 1, BORDER_REPLICATE);
		Mat mask(roughimg.rows + 2, roughimg.cols + 2, CV_8UC1, Scalar::all(0));
		Mat now(roughimg.rows + 2, roughimg.cols + 2, CV_8UC3, Scalar::all(0));
	

		const Scalar& colorDiff = Scalar::all(50);
		int flag = 4 | (255 << 8);
		int downarea = 200; // img.cols*img.rows / 35;  //面积过滤条件
		int uparea = roughimg.cols*roughimg.rows / 5;

		vector<int> floodArea;
		vector<float> floodRatio;
		vector<Rect> floodRects;
		int num=0;
		RNG rng = theRNG();
		for (int y = 0; y < roughimg.rows; y=y+3)
		{
			for (int x = 0; x < roughimg.cols; x=x+3)
			{
				if (mask.at<uchar>(y + 1, x + 1) == 0)
				{
					Scalar newVal(rng(256), rng(256), rng(256));
  					Rect floodRect;
					//漫水区域面积
 					int area = floodFill(floodimg, mask, Point(x, y), newVal, &floodRect, colorDiff, colorDiff,flag); 
					//漫水区域的外接矩形形状
 					float  wrap_ratio = min(float(floodRect.width) / floodRect.height, float(floodRect.height) / floodRect.width);
					//漫水占比=漫水区域的面积/外接矩形的面积
 					float  occupation_ratio = float(area) / float(floodRect.area());

					imgbak.copyTo(now, mask);
 					
 					//过滤条件
  					if (area<downarea || area>uparea)
  						continue;
  
 					if (wrap_ratio < 0.7 || occupation_ratio < 0.7)
 						continue;
 
 					floodArea.push_back(area);
 					floodRatio.push_back(wrap_ratio);
 					floodRects.push_back(floodRect);
 
 					//imgbak.copyTo(now, mask);

 					//主观题纳入
					rectangle(roughimg, floodRect, Scalar(0, 0, 255), 2, 8);
 					SLocAnswer part_zuguan;
					ostringstream s1;
					s1 << areaflag<<"_" << num++;
					part_zuguan.what = s1.str();
					part_zuguan.where = floodRect;
					part_zuguan.pic = roughimg(floodRect);
					precise_boxes.push_back(part_zuguan);

					//主观题的定位展示
					imshow(part_zuguan.what, part_zuguan.pic);
					waitKey();

				}
			}
		}

		//在整个主观题的粗定位区域上显示精确定位的主观题位置
		if (0){
			imshow("demo", roughimg);
			waitKey();
		}
		
	}

	return 0;
}


//功能测试区
int main_preciseloc()
{
	string filename = "E:\\Code\\Git\\Code\\paperocr\\samples\\xuanzeti.bmp";
	Mat src = imread(filename);
	if (src.empty()){
		cout << "load fail" << endl;
		return 0;
	}

	vector<SLocAnswer> precisearea;
	preciseloc(src,"xuanzeti",precisearea);

	return 0;

	/*
	for (vector<SLocAnswer>::iterator itr=precisearea.begin();itr!=precisearea.end();itr++)
	{
		//resize(demo, demo, Size(), 0.5, 0.5);
		imshow("rougloc", itr->pic);
		waitKey();
	}
	*/

	return 0;
}
