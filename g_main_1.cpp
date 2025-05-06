/**
 * Copyright © 2025-2025 Gking,All Rights Reserved.
 * https://github.com/Gking-a/Gking_Milkvduo_Tools
 * Mozilla Public License Version 2.0
 */
#define PWM_PERIOD 1000
#define PWM_ENABLE_MIN 700
#define MAX_SPEED 999
extern int hardcode_milkv256_gp_pwm[]; 
#define __GKING_MILKVDUO_GPIO_VERSION__ hardcode_milkv256_gp_pwm
#include <stdio.h>
#include <unistd.h>
#include <g_u_h.h>
#define SPEED_KEEP_SECOND 3
#include <chrono>
long long lasttime=0;
// 获取当前时间戳（毫秒级）
long long get_timestamp_ms() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
}

L298NController ctl(8, 15, 18, 9, 16, 17);
int x2vlinear__bu(ImageDetectInfo* info,int lr){
    int x=info->referoffsetx;
    int maxspeed=1000;
    if(!info->detected){
        ctl.setspeed(0,0);
        ctl.setspeed(1,0);
        return 0;
    }
    if(!info->detected){
        ctl.setspeed(0,0);
        ctl.setspeed(1,0);
        return 0;
    }
    if (x == 0)
    {
        ctl.setspeed(0,maxspeed);
        ctl.setspeed(1,maxspeed);
    }
    else if(x<0){
        ctl.setspeed(1,maxspeed);
        ctl.setspeed(0,maxspeed*2*(x+160)/320);
    }
    else if(x>0){
        ctl.setspeed(0,maxspeed);
        ctl.setspeed(1,maxspeed*2*(160-x)/320);
    }
    return 0;
}
inline int _enablegap(int x){
    printf("gap pure speed=%d\n",x);
    double gap=static_cast<double>(PWM_ENABLE_MIN)/PWM_PERIOD;
    return x==0?x:(int)(x*(1-gap)+(x>0?PWM_ENABLE_MIN:-PWM_ENABLE_MIN));
}
inline int _x2vinverseratiohelper(int x,int halfWidth,int max){
    double xd=abs(x);
    return (int)((xd/halfWidth-0.5)*2*max);
}
int x2vinverseratio(ImageDetectInfo* info,int halfWidth,int halfHeight,int lr){
    int x=info->referoffsetx;
    if(x==0){
        return lr?1000:-1000;
    }else if(x>0){
        return lr?1000:_x2vinverseratiohelper(x,halfWidth,MAX_SPEED);
    }else{
        return lr?_x2vinverseratiohelper(x,halfWidth,MAX_SPEED):1000;
    }
}
int x2vinverseratiogap(ImageDetectInfo* info,int halfWidth,int halfHeight,int lr){
    int x=info->referoffsetx;
    if(x==0){
        return lr?1000:-1000;
    }else if(x>0){
        return _enablegap( lr?1000:_x2vinverseratiohelper(x,halfWidth,MAX_SPEED));
    }else{
        return _enablegap( lr?_x2vinverseratiohelper(x,halfWidth,MAX_SPEED):1000);
    }
}
int x2vlinear(ImageDetectInfo* info,int halfWidth,int halfHeight,int lr){
    int x=info->referoffsetx;
    if(!info->detected){
        printf("not detected!\n");
        return 0;
    }
    printf("offsetx=%d\n",info->referoffsetx);
    if (x == 0)
    {
        return MAX_SPEED;
    }
    else if(x<0){
        return lr?MAX_SPEED:MAX_SPEED*2*(x+halfWidth/2)/halfWidth;
    }
    else if(x>0){
        return lr?MAX_SPEED*2*(halfWidth/2-x)/halfWidth:MAX_SPEED;
    }
    return 0;
}

int x2vlineargap(ImageDetectInfo* info,int halfWidth,int halfHeight,int lr){
    int x=info->referoffsetx;
    if(!info->detected){
        printf("not detected!\n");
        return 0;
    }
    printf("offsetx%d\n",info->referoffsetx);
    printf("halfWidth=%d\n",halfWidth);
    if (x == 0)
    {
        return MAX_SPEED;
    }
    else if(x<0){
        return _enablegap( lr?MAX_SPEED:MAX_SPEED*2*(x+halfWidth/2)/halfWidth);
    }
    else if(x>0){
        return _enablegap( lr?MAX_SPEED*2*(halfWidth/2-x)/halfWidth:MAX_SPEED);
    }
    return 0;
}
using namespace cv;
inline void img2control(void (*imgpreprocess)(ImageData&,Mat&),ImageDetectInfo* (*imgdetect)(Mat&,void**),int (*velmapping)(ImageDetectInfo*,int,int,int),void** args){
    VIDEO_FRAME_INFO_S stFrame{};
    CVI_S32 s32Ret;
    while (!terminalFlag)
    {
        ImageData imageData{};
        s32Ret = getImageData(&stFrame, &imageData);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VPSS_ReleaseChnFrame(0, 0, &stFrame);
            printf("Send Output Frame NG, ret=%x\n", s32Ret);
            continue;
        }
        using namespace cv;
        Mat img;
        imgpreprocess(imageData,img);
        ImageDetectInfo* info=imgdetect(img,args);
        if(!info->detected&&SPEED_KEEP_SECOND!=0){
            long long now=get_timestamp_ms();
            if(lasttime==0){
                lasttime=now;
            }
            else if((now-lasttime)/1000<SPEED_KEEP_SECOND){
                
            }else{
                lasttime=0;
                ctl.setspeed(0,0);
                ctl.setspeed(1,0);
            }
        }else{
            lasttime=0;
            ctl.setspeed(0,velmapping(info,imageData.width/2,imageData.height/2,0));
            ctl.setspeed(1,velmapping(info,imageData.width/2,imageData.height/2,1));
        }
        delete info;
        free(imageData.data);
        CVI_VPSS_ReleaseChnFrame(0, 0, &stFrame);
        printf("release getImageData\n");
        if (s32Ret != CVI_SUCCESS)
        {
            terminalService(SIGTERM);
            printf("terminal finish\n");
            break;
        }
    }
}

void nv2gray(ImageData& imageData,Mat& outer){
    Mat gray(imageData.height, imageData.width, CV_8UC1, imageData.data);
    cv::Rect roi(0, 60, 640, 360);
    outer=gray(roi);
}
void nv2hsv(ImageData& data,Mat& hsv){
    Mat nv21(data.height*3/2,data.width,CV_8UC1,data.data);
    cv::cvtColor(nv21,hsv,COLOR_YUV2BGR_NV21);
    cv::cvtColor(hsv, hsv, COLOR_BGR2HSV);
    cv::Rect roi(0, 60, 640, 360);
    hsv=hsv(roi);
}
extern void helpmethodtogp(int pin);
int main(int argc, char **argv)
{
    wiringSetup();
    ctl.setMode();
    ctl.setspeed(0, 0);
    ctl.setspeed(1, 0);
    signal(SIGINT, terminalFlagfunc);
    signal(SIGTERM, terminalFlagfunc);
    camconf = autoStartCamera(640, 480);
    /*以下是寻迹
    binaryDetectStruct bFstruct={
        300,360,1
    };
    auto *pBFS=&bFstruct;
    img2control(nv2gray,detectBinaryMiddle,x2vlineargap,(void**)&pBFS);
    */
   /*以下是避障
    detectFollowStruct dFstruct={
        cv::Scalar(0,0,0),
        cv::Scalar(255,255,50),
    };
    auto *pdFS=&dFstruct;
    // img2control(nv2hsv,detectFollow,x2vlineargap,(void**)&pdFS);
    img2control(nv2hsv,detectFollow,x2vinverseratiogap,(void**)&pdFS);
    */
    terminalService(SIGTERM);
    return 0;
}