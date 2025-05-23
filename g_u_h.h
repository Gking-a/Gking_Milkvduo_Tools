/**
 * Copyright © 2025-2025 Gking,All Rights Reserved.
 * https://github.com/Gking-a/Gking_Milkvduo_Tools
 * Mozilla Public License Version 2.0
 */
#pragma once
#include "g_u_version.h"
#ifndef __GKING_MIKLV_DUO_UTIL_INCLUDED
#define __GKING_MIKLV_DUO_UTIL_INCLUDED
#include <vector>
#include <signal.h>
#include <cvi_type.h>
#include <middleware_utils.h>
#include "cvi_tdl.h"
#include <cvi_comm.h>
#include <sample_utils.h>
#include "core/utils/vpss_helper.h"
#include "wiringx.h"
#include "opencv.hpp"
#include "numeric"
#include "utility"
typedef unsigned int uint;
typedef struct
{
    SAMPLE_TDL_MW_CONFIG_S *stMWConfig;
    SAMPLE_TDL_MW_CONTEXT *pstMWContext;
} CameraInitInfo;
extern CameraInitInfo *camconf;
typedef struct
{
    void *data;
    int channel;
    int width;
    int height;
    int datasize;
    cv::Mat cvimg;
} ImageData;
typedef struct
{
    int chan[2][3];
} L298NPort;

extern bool terminalFlag;
#ifdef __cplusplus
extern "C"
{
#endif
    int openCamara(SAMPLE_TDL_MW_CONFIG_S *stMWConfig);
    void setVPSS(SAMPLE_TDL_MW_CONFIG_S *stMWConfig, int count = 1, uint u32w = 1920, uint u32h = 1080);
    void defaultConfig(SAMPLE_TDL_MW_CONFIG_S *stMWConfig, int count = 1, uint u32w = 1920, uint u32h = 1080);
    SAMPLE_TDL_MW_CONTEXT *initContext(SAMPLE_TDL_MW_CONFIG_S *stMWConfig);
    CameraInitInfo *autoStartCamera(int width = 1920, int height = 1080, int count = 1);
    void terminalFlagfunc(int signo);
    void terminalService(CVI_S32 signo);

    class L298NController
    {
    public:
        L298NController();
        L298NController(char ap, char a1, char a2, char bp, char b3, char b4);
        ~L298NController();
        void setConfig(L298NPort port);
        /**
         * @param chan 0:A,chan1:B
         * @param speed thousand=max,minus=back,otherwise stop with max power
         */
        void setMode();
        void setspeed(int chan, int speed);
        L298NPort port;
    };
    int wiringSetup();
    /**
     * @param hl high:1,low:0
     * @return -1 if failed else 0
     */
    int wiringOpr(int gpioid, int hl);
    /**
     * waring:released the virAddress,keep the copy address
     * waring:the first image is null!keep refresh!
     */
    CVI_S32 getImageData(VIDEO_FRAME_INFO_S *frameInfo, ImageData *imageData, int channel = 0);
    
    
    typedef struct ImageDetectInfo{
        int referoffsety=0;
        int referoffsetx=0;
        bool detected=false;
    }ImageDetectInfo;
    ImageDetectInfo *detectBinaryMiddle(cv::Mat &img,void**);
    struct binaryDetectStruct{
        int starty,endy;
        int stride;
    };
    struct detectFollowStruct{
        cv::Scalar lower;
        cv::Scalar upper;
    };
    ImageDetectInfo *detectFollow(cv::Mat &bgr,void **);
#ifdef __cplusplus
}
#endif
#endif