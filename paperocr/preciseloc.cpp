#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <math.h>
#include <string.h>
#include "online.h"

using namespace cv;
using namespace std;


// helper function: finds a cosine of angle between vectors from pt0->pt1 and from pt0->pt2
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
 * 输出：细定位的结果
 * 返回：精确定位到的结果区域的个数
 *
 */
int preciseloc(Mat roughimg,string areaflag,vector<SRPart> &precise_boxes)
{
	if (areaflag=="xuehao"){
		cout <<"学号区暂不检测" << endl;
		//调用python接口返回学号
		return -1;
	}

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
							
							SRPart partdetail;
							ostringstream s1;
							s1 << areaflag<<"_" << num++;
							partdetail.what = s1.str();
							partdetail.where = expandbox;
							precise_boxes.push_back(partdetail);

							RNG rng = theRNG();
							Scalar newVal(rng(256), rng(256), rng(256));
							rectangle(demo, expandbox,Scalar(0,255,0), 2, CV_AA);
							rectangle(demo, box, newVal, 1, CV_AA);
						
						}	
	                }
	            }
	        }
	    }

	    //效果展示
	    if (0)	{
			imshow(areaflag, demo);
			//waitKey();
			//destroyAllWindows();
		}


	}
	
	if (areaflag=="zuguanti")
	{

	}

}


