#include "online.h"
#include <math.h>


using namespace cv;
using namespace std;

// 计算两条线段的夹角:pt0->pt1 and from pt0->pt2
static double angle( Point pt1, Point pt2, Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}


/* 功能：精确定位
 * 输入：粗定位图像，该区域标识
 * 输出：细定位的结果（相对于粗定位的位置）
 * 返回：精确定位到的结果区域的个数
 *
 */
int preciseloc(Mat roughimg,string areaflag,vector<SLocAnswer> &precise_boxes)
{
	//根据学号的特点进行优化
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
		drawContours(demo, contours, -1, Scalar(255, 255, 0), 1);

		//cout<<"lunkuo filter"<<endl;
		int downarea = 250;
		int uparea = floodimg.rows*floodimg.cols / 3;
		int num = 0;
		vector<vector<Point> > approx;
		for (size_t i = 0; i < contours.size(); i++)
		{
			float contour_area = contourArea(contours[i]);
			//approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
			//if (fabs(contourArea(Mat(contours[i]))) > 10 && isContourConvex(Mat(contours[i])))
			{
				float radius;
				Point2f center;
				Rect brect=boundingRect(Mat(contours[i]));
			
				float rect_area = brect.width*brect.height;
				float ratio = rect_area / contour_area;
				if (ratio > 1.2&&ratio<2)
				{
					//将这个轮廓的外接矩形保存
					Mat qr=roughimg(brect);
					imshow("now arc",qr);
					waitKey();

					//外框学号二维码纳入精确定位结果
					SLocAnswer part_xuehao;
					ostringstream s1;
					s1 << areaflag << "_" << num++;
					part_xuehao.what = s1.str();
					part_xuehao.where = brect;
					precise_boxes.push_back(part_xuehao);
				}
				
			}
		}
	}

	//选择题的精确定位已解决
	if( areaflag=="xuanzeti" )
	{

		Mat pyr,demo,timg, gray;
		roughimg.copyTo(demo);

	    pyrDown(roughimg, pyr, Size(roughimg.cols/2, roughimg.rows/2));
	    pyrUp(pyr, timg, Size(roughimg.cols/2*2,roughimg.rows/2*2));
		Mat gray0(timg.size(), CV_8U);

		vector<vector<Point> > squares;
	    vector<vector<Point> > contours;

		int num = 1;
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
	                    double maxCosine = 0;
	                    for( int j = 2; j < 5; j++ ) {
	                        double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
	                        maxCosine = MAX(maxCosine, cosine);
	                    }

						if (maxCosine < 0.2){
							squares.push_back(approx); 

							//精确定位在粗定位上进行了12像素的扩张
							Rect expandbox=Rect(box.tl() + Point(-6, -6), box.br()+ Point(6, 6));

							RNG rng = theRNG();
							Scalar newVal(rng(256), rng(256), rng(256));
							rectangle(demo, expandbox,Scalar(0,255,0), 2, CV_AA);
							rectangle(demo, box, newVal, 1, CV_AA);		

							//选择题框纳入精定位结果
							SLocAnswer partselect;
							ostringstream s1;
							s1 << areaflag << "_" << num++;
							partselect.what = s1.str();
							partselect.where = expandbox;
							precise_boxes.push_back(partselect);
						}	
	                }
	            }
	        }
	    }

	    //效果展示
	    if (1)	{
			imshow(areaflag, demo);
			waitKey();
			destroyAllWindows();
		}

	}
	
	//要根据主观题的特点进行优化
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
		for (int y = 0; y < roughimg.rows; y++)
		{
			for (int x = 0; x < roughimg.cols; x++)
			{
				if (mask.at<uchar>(y + 1, x + 1) == 0)
				{
					Scalar newVal(rng(256), rng(256), rng(256));
  					Rect floodRect;
 					int area = floodFill(floodimg, mask, Point(x, y), newVal, &floodRect, colorDiff, colorDiff,flag);  				//漫水区域面积
 					float  wrap_ratio = min(float(floodRect.width) / floodRect.height, float(floodRect.height) / floodRect.width);  //漫水区域的外接矩形形状
 					float  occupation_ratio = float(area) / float(floodRect.area());												//漫水占比=漫水区域的面积/外接矩形的面积
 					
 					//过滤条件
  					if (area<downarea || area>uparea)
  						continue;
  
 					if (wrap_ratio < 0.7 || occupation_ratio < 0.7)
 						continue;
 
 					floodArea.push_back(area);
 					floodRatio.push_back(wrap_ratio);
 					floodRects.push_back(floodRect);
 
 					imgbak.copyTo(now, mask);

 					//主观题纳入
 					SLocAnswer part_zuguan;
					ostringstream s1;
					s1 << areaflag<<"_" << num++;
					part_zuguan.what = s1.str();
					part_zuguan.where = floodRect;
					precise_boxes.push_back(part_zuguan);

				}
			}
		}
	}

	return 0;
}


//功能测试区
int main()
{
	string filename = "C:\\Users\\xl\\Desktop\\Code\\paperocr\\samples\\xuehaoti.bmp";
	Mat src = imread(filename);
	if (src.empty()){
		cout << "load fail" << endl;
		return 0;
	}

	vector<SLocAnswer> precisearea;
	preciseloc(src,"xuehaoti",precisearea);

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
