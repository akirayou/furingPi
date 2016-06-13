#include<iostream>
#include <boost/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <signal.h>
#include<unistd.h>
#include <X11/Xlib.h>
#include<opencv2/opencv.hpp>//debug view
using namespace std;
using namespace cv;
bool doEnd=false;

//header of wind.cc
void putWind(int w);
int getWind();
void wind(void);
void ledLoop(void);
extern int windDebug;
extern int ledDebug;
//end of header

static Mat ledView_d=Mat::zeros(1,1,CV_8UC3);
static boost::mutex viewMtl;
void getLedView(Mat &m){
  boost::mutex::scoped_lock lock(viewMtl);
  m=ledView_d.clone();
}
void setLedView(const Mat &m){
  boost::mutex::scoped_lock lock(viewMtl);
  ledView_d=m;
}

Mat windView =Mat::zeros(320/2,240/2,CV_8UC3);
static boost::mutex viewMt;
void getWindView(Mat &m){
  boost::mutex::scoped_lock lock(viewMt);
  m=windView.clone();
}
void setWindView(const Mat &m){
  boost::mutex::scoped_lock lock(viewMt);
  windView=m;
}
boost::lockfree::queue<int> windData(8);
void putWind(int w){
  windData.push(w);
}
int getWind(void){
  int w;
  if(windData.pop(w))return w;
  return -1;
}



static void ctrl_c_handler(int signum)
{
  doEnd=true;
}



int main(int argvc,char *argv[]){
  cerr << "start"<<endl;
  if(getenv("DEBUG")){
      windDebug=1;
      ledDebug=1;
  }else{
      windDebug=0;
      ledDebug=0;
  }
  if(windDebug || ledDebug){
    XInitThreads();
    if(windDebug)cv::namedWindow("Capture", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
    if(ledDebug)namedWindow("led", 0);//CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  }


  struct sigaction sa;
  sa.sa_handler = ctrl_c_handler;
 
  sigaction(SIGKILL, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);



  boost::thread thWind(&wind);  
  boost::thread thLed(&ledLoop);  
  cerr << "start threads"<<endl;
  while(!doEnd){
    Mat tmp;
    getWindView(tmp);
    if(windDebug)imshow("Capture",tmp);
    getLedView(tmp);
    if(ledDebug)imshow("led",tmp);
    int key=cv::waitKey(1);
    key-='0';
    if(key<10)putWind(key);
    usleep(1000*1000/15);
  }
  cerr << "end"<<endl;
  thWind.join();
  thLed.join();
  return 0;
}
