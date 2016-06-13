#include<opencv2/opencv.hpp>
#include<iostream>
#include<vector>
#include<algorithm>
#include <boost/thread.hpp>
using namespace std;
using namespace cv;

extern bool doEnd;
//wind data manupilation
void putWind(int w);
int getWind(void);


static float getCenter(vector<int>& sx){
  const int hlen=3;
  int mx=distance(sx.begin(),max_element(sx.begin(),sx.end()));
  mx=max(hlen,min((int)(sx.size())-1-hlen,mx));
  float center=0;
  int count=0;
  for(int i=mx-hlen;i<=mx+hlen;i++){
    center+= i* sx[i];
    count+=sx[i];
  }
  return center/count;
}
void getWindView(Mat &m);
void setWindView(const Mat &m);


int windDebug=0;
void wind(void){
  int dev=0;
  system("sudo v4l2-ctl  -cpower_line_frequency=0 -cexposure_auto=1 -cexposure_absolute=3");
  VideoCapture cap(dev);
  cap.set(CV_CAP_PROP_FRAME_WIDTH, 320/2);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240/2);
  if(!cap.isOpened()){
    cerr << "can not open camera"<<endl;
    return ;
  }
  int count=0;
  vector<int> sx(320/2),sy(240/2);
  Mat frame;
  Point_<float> center,centerMid,centerOld,diff(0,0);
  double thresh=0.5;
  int ignoreCount=0;
  while(1) {

    cap >> frame;
    for(int i=0;i<sx.size();i++)sx[i]=0;
    for(int i=0;i<sy.size();i++)sy[i]=0;
    for(int y = 0; y < frame.rows; ++y){
      for(int x = 0; x < frame.cols; ++x){
	int a = frame.data[ y * frame.step + x * frame.elemSize() + 1 ] ;
	sx[x]+=a;
	sy[y]+=a;
      }
    }

    Point_<float> tmp;
    tmp.x=getCenter(sx);
    tmp.y=getCenter(sy);
    if(count==0){
      centerMid=centerOld=center=tmp;
    }
    if(windDebug){
      rectangle(frame,Point(center.x+1,center.y+1),Point(center.x-1,center.y-1),Scalar(0,0,255),-1);
      rectangle(frame,Point(centerOld.x+1,centerOld.y+1),Point(centerOld.x-1,centerOld.y-1),Scalar(255,0,0),-1);
      rectangle(frame,Point(centerMid.x+1,centerMid.y+1),Point(centerMid.x-1,centerMid.y-1),Scalar(255,255,255),-1);
    }
    center = center*0.9+tmp*0.1;
    centerMid = centerMid*0.9+center*0.1;
    diff=  (center-centerOld);
    
    if(ignoreCount)ignoreCount--;
    if(thresh*4<norm(diff) ){//detect wind
      if(ignoreCount==0){
	//Put in wind daata when qeue size is enough small
	
	double arg=(atan2(diff.y,diff.x)/(M_PI)+1.0)  *8 ;
	if(windDebug){
	  cerr <<  (int)arg ;
	  cerr << diff <<endl;
	}
	putWind((int)arg);
	ignoreCount=40;
      }

    }    
    {
      static int stableCount=0;
      if(thresh<norm(center-centerMid))stableCount=10;
      else if(stableCount)stableCount--;
      if(stableCount!=0){
	//centerOld = centerOld*0.99+0.01*center;// only little update in wind
      }else{
	centerOld = centerOld*0.97+0.03*center ;// update  in no wind
	//cerr << thresh<<endl;
      }    
    }
    setWindView(frame);
    //if(windDebug &&count%2==0)cv::imshow("Capture", frame); // 表示
    count++;
  }
}

