#include<iostream>
#include<iomanip>
#include<opencv2/opencv.hpp>//debug view
#include <stdint.h>
#include <cmath>
#include<stdio.h>
#include<unistd.h>
using namespace cv;

extern "C"{
#include<ws2811.h>

}
extern bool doEnd;
using namespace std;
int getWind();

int ledDebug=0;

#define TARGET_FREQ                              WS2811_TARGET_FREQ
#define GPIO_PIN                                 18
#define DMA                                      5

#define WIDTH                                    (19*3)
#define HEIGHT                                   5
#define LED_COUNT                                (WIDTH * HEIGHT+1)
ws2811_t ledstring;
static Mat ledView=Mat::zeros(HEIGHT*3,WIDTH*3,CV_8UC3);
void getLedView(Mat &m);
void setLedView(const Mat &m);


static int posTr[WIDTH*HEIGHT]={0};
void initPos(){
  int x=0,y=0;
  int xd=1,yd=1;
  for(int  l=1;l<WIDTH*HEIGHT+1;l++){
    cerr << x<<","<<y<<endl;
    posTr[y*WIDTH+x]=l;

    x+=xd;
    if((l-1)%19==18){
      xd*=-1;
      x+=xd;
      y+=yd;
      if(y<0 || HEIGHT<=y){
	yd*=-1;
	y+=yd;
	xd*=-1;
	x+=xd;
      }
    }
  }

  for(y=0;y<HEIGHT;y++){
      for(x=0;x<WIDTH;x++){
	cerr <<setw(3)<< posTr[y*WIDTH+x] <<" ";
      }
      cerr << endl;
  }
  
  
}

void matrix_render(Mat img){
  int x, y;
  int c=0;
  if(posTr[0]==0)initPos();
  for (x = 0; x < WIDTH; x++){
    for (y = 0; y < HEIGHT; y++){
      unsigned char *ip=&(img.at<unsigned char>(y,x*3));
      uint32_t col=
	((uint32_t)(ip[0])<<16)  +//r
	((uint32_t)(ip[1])<<8)  +//g
	( uint32_t)(ip[2])  ;//b

      
      ledstring.channel[0].leds[posTr[y*WIDTH+x]] = col;

      if(ledDebug){
	int vx=x*3+1;
	int vy=y*3+1;
	unsigned char *vp=&(ledView.at<unsigned char>(vy,vx*3));
	vp[0]= 0xff & (col);
	vp[1]= 0xff & (col>>8);
	vp[2]= 0xff & (col >>16);
      }
    }
    setLedView(ledView);
  }
  ledstring.channel[0].leds[LED_COUNT-1] =ledstring.channel[0].leds[LED_COUNT-2];// LED for singal Buffer 
}


void updateIdle(Mat &idle){
  for(int x=0;x<WIDTH;x++){
    for(int y=0;y<HEIGHT;y++){
      float *p=(float *)&(idle.at<float>(y,x*3));
      float & h=  p[0];
      float & s=  p[1];
      float & v=  p[2];
      
      
      v-=0.0002*(rand()%50);
      if(v<0){
	if(rand()%32==0 && (x+y)%3==0){
	  v=0.1;
	  s=1;
	  h=(rand()%3600)/10.0;
	}else{
	  v=0;
	}
      }
    }
  }
}

void overwrite(Mat &out,Mat &back,Mat &fore){
  for(int y=0;y<HEIGHT;y++){
    for(int x=0;x<WIDTH;x++){
      
      float *po=&(out.at<float>(y,x*3));
      float *pb=&(back.at<float>(y,x*3));
      float *pf=&(fore.at<float>(y,x*3));
      float *in=(pb[2]<pf[2])?pf:pb;
      po[0]=in[0];
      po[1]=in[1];
      po[2]=in[2];
    }
  }
}
void kickWave(Mat &wave,int direc){//1-8 down 9-16 up
  int by,ey;
  if(direc<=8){
    by=0;
    ey=0;
  }else{
    by=HEIGHT-1-0;
    ey=HEIGHT-1;
  }

  for(int y=by;y<=ey;y++){
    for(int x=0;x<WIDTH;x++){
      float * p=&(wave.at<float>(y,x*3));
      p[0]=110;
      p[1]=0.8;
      p[2]=0.3;
    }
  }
}

void updateWave(Mat &wave,int direc){
  int by,ey;
  int d;
  if(direc<=8){  //0-8 down 9-15 up
    by=HEIGHT-1;
    ey=0;
    d=-1;
  }else{
    by=0;
    ey=HEIGHT-1;
    d=1;
  }
  float rate=0.9;
  for(int x=0;x<WIDTH;x++){ (&(wave.at<float>(ey,x*3)))[2]*=rate;}
  for(int y=by;y!=ey;y+=d){
    for(int x=0;x<WIDTH;x++){
      float * pf=&(wave.at<float>(y+d,x*3));
      float * p=&(wave.at<float>(y,x*3));
      if( p[2]+0.1<pf[2] ){
	p[0]=pf[0];
	p[1]=pf[1];
	p[2]=pf[2];
      }else{
	p[2]=p[2]*rate;
      }
    }
  }

}
void ledLoop(void){
    
  //if(ledDebug)imshow("led",ledView);
  
  ledstring.freq = TARGET_FREQ;
  ledstring.dmanum = DMA;
  ledstring.channel[0].gpionum = GPIO_PIN;
  ledstring.channel[0].count = LED_COUNT;
  ledstring.channel[0].invert = 0;
  ledstring.channel[0].leds = 0;
  ledstring.channel[0].brightness = 100;
  ledstring.channel[1].gpionum = 0;
  ledstring.channel[1].count = 0;
  ledstring.channel[1].invert = 0;
  ledstring.channel[1].brightness = 0;
  ledstring.channel[1].leds = 0;
  if (ws2811_init(&ledstring)){
    cerr << "can not init LED"<<endl;
    return;
  }
  Mat hsv= Mat::zeros(HEIGHT,WIDTH,CV_32FC3);
  Mat wave= Mat::zeros(HEIGHT,WIDTH,CV_32FC3);
  Mat idle= Mat::zeros(HEIGHT,WIDTH,CV_32FC3);
  Mat rgb= Mat::zeros(HEIGHT,WIDTH,CV_32FC3);
  Mat rgbu=Mat::zeros(HEIGHT,WIDTH,CV_8UC3);

  int eventCount=0;
  int eventNo=0;
  int direc=1;
  cerr << "led start"<<endl;
  while(!doEnd){
    int a=getWind();
    if(0<=a){
      eventNo=a+1;      
      cout <<"WIND_EVENT"<< eventNo <<endl;
      eventCount=0;
      direc=a;
      kickWave(wave,direc);
    }
    if(eventNo){
      eventCount++;
      if(eventCount>=15*1.5){
	eventCount=0;
	eventNo=0;
      }
    }
    updateWave(wave,direc);
    updateIdle(idle);
    overwrite(hsv,idle,wave);


    cvtColor(hsv,rgb,CV_HSV2BGR);
    rgb.convertTo(rgbu, CV_8U, 255);
   
    matrix_render(rgbu);ws2811_render(&ledstring);
    usleep(1000*1000/15);
  }
  ws2811_fini(&ledstring);
}
