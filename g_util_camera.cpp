/**
 * Copyright © 2025-2025 Gking,All Rights Reserved.
 * https://github.com/Gking-a/Gking_Milkvduo_Tools
 * Mozilla Public License Version 2.0
 */
#include "g_u_h.h"
CameraInitInfo *camconf{};
bool terminalFlag = false;
ImageData getImageData(VIDEO_FRAME_INFO_S *frameInfo, int channel = 0)
{
    ImageData idata{};
    getImageData(frameInfo, &idata, channel);
    return idata;
}
CVI_S32 getImageData(VIDEO_FRAME_INFO_S *frameInfo, ImageData *imgd, int channel)
{
    CVI_S32 s32Ret = CVI_VPSS_GetChnFrame(0, channel, frameInfo, 2000);
    if (s32Ret != CVI_SUCCESS)
    {
        printf("CVI_VPSS_GetChnFrame chn0 failed with %#x\n", s32Ret);
        return s32Ret;
    }
    int image_pix = frameInfo->stVFrame.u32Width * frameInfo->stVFrame.u32Height;
    int image_size = image_pix * 3;
    int datasize = 0;
    for (int i = 0; i < 3; i++)
    {
        if (frameInfo->stVFrame.u32Length[i] == 0)
            continue;
        datasize += frameInfo->stVFrame.u32Length[i];
        frameInfo->stVFrame.pu8VirAddr[i] = (CVI_U8 *)CVI_SYS_Mmap(frameInfo->stVFrame.u64PhyAddr[i], image_pix);
    }
    unsigned char *ptr = (unsigned char *)calloc(image_size / 2, 1);
    for (int i = 0; i < 3; i++)
    {
        if (frameInfo->stVFrame.u32Length[i] == 0)
            continue;
        memcpy(&ptr[image_pix * i], (const CVI_VOID *)frameInfo->stVFrame.pu8VirAddr[i], frameInfo->stVFrame.u32Length[i]);
        CVI_SYS_Munmap(frameInfo->stVFrame.pu8VirAddr[i], frameInfo->stVFrame.u32Length[i]);
    }
    if(GU_ROTATE180){
        std::reverse(ptr, ptr + frameInfo->stVFrame.u32Length[0]);
        struct UV { uint8_t v, u; };  // NV21 格式中 V 在前
        UV* uv_elements = reinterpret_cast<UV*>(ptr + frameInfo->stVFrame.u32Length[0]);
        std::reverse(uv_elements, uv_elements + (frameInfo->stVFrame.u32Length[1] / sizeof(UV)));
    }
    imgd->data = ptr;
    imgd->channel = 2;
    imgd->width = frameInfo->stVFrame.u32Width;
    imgd->height = frameInfo->stVFrame.u32Height;
    imgd->datasize = datasize;
    return CVI_SUCCESS;
}
int openCamara(SAMPLE_TDL_MW_CONFIG_S *stMWConfig)
{
    CVI_S32 s32Ret = SAMPLE_TDL_Get_VI_Config(&stMWConfig->stViConfig);
    if (s32Ret != CVI_SUCCESS || stMWConfig->stViConfig.s32WorkingViNum <= 0)
    {
        printf("Failed to get senor infomation from ini file (/mnt/data/sensor_cfg.ini).\n");
        return -1;
    }
    return 0;
}
void setVPSS(SAMPLE_TDL_MW_CONFIG_S *stMWConfig, int count, uint u32w, uint u32h)
{
    // Setup VPSS Grp0
    stMWConfig->stVPSSPoolConfig.u32VpssGrpCount = 1;
#ifndef CV186X
    stMWConfig->stVPSSPoolConfig.stVpssMode.aenInput[0] = VPSS_INPUT_MEM;
    stMWConfig->stVPSSPoolConfig.stVpssMode.enMode = VPSS_MODE_DUAL;
    stMWConfig->stVPSSPoolConfig.stVpssMode.ViPipe[0] = 0;
    stMWConfig->stVPSSPoolConfig.stVpssMode.aenInput[1] = VPSS_INPUT_ISP;
    stMWConfig->stVPSSPoolConfig.stVpssMode.ViPipe[1] = 0;
#endif
    SAMPLE_TDL_VPSS_CONFIG_S *pstVpssConfig = &stMWConfig->stVPSSPoolConfig.astVpssConfig[0];
    pstVpssConfig->bBindVI = true;
    // PIXEL_FORMAT_NV21
    // PIXEL_FORMAT_RGB_888
    VPSS_GRP_DEFAULT_HELPER2(&pstVpssConfig->stVpssGrpAttr, 1920,
                             1080, PIXEL_FORMAT_NV21, 1);
    pstVpssConfig->u32ChnCount = count;
    pstVpssConfig->u32ChnBindVI = 0;
    for (int i = 0; i < count; i++)
    {
        VPSS_CHN_DEFAULT_HELPER(&pstVpssConfig->astVpssChnAttr[i], u32w,
                                u32h, PIXEL_FORMAT_NV21, true);
    }
    SAMPLE_TDL_Get_Input_Config(&stMWConfig->stVencConfig.stChnInputCfg);
    stMWConfig->stVencConfig.u32FrameWidth = u32w;
    stMWConfig->stVencConfig.u32FrameHeight = u32h;
}
void defaultConfig(SAMPLE_TDL_MW_CONFIG_S *stMWConfig, int count, uint u32w, uint u32h)
{
    stMWConfig->stVBPoolConfig.u32VBPoolCount += count;
    for (int i = 0; i < count; i++)
    {
        // VBPool 0 for VPSS Grp0 Chn0
        stMWConfig->stVBPoolConfig.astVBPoolSetup[i].enFormat = PIXEL_FORMAT_NV21;
        stMWConfig->stVBPoolConfig.astVBPoolSetup[i].u32BlkCount = 2;
        stMWConfig->stVBPoolConfig.astVBPoolSetup[i].u32Height = u32h;
        stMWConfig->stVBPoolConfig.astVBPoolSetup[i].u32Width = u32w;
        stMWConfig->stVBPoolConfig.astVBPoolSetup[i].bBind = true;
        stMWConfig->stVBPoolConfig.astVBPoolSetup[i].u32VpssChnBinding = i;
        stMWConfig->stVBPoolConfig.astVBPoolSetup[i].u32VpssGrpBinding = (VPSS_GRP)0;
    }
}
SAMPLE_TDL_MW_CONTEXT *initContext(SAMPLE_TDL_MW_CONFIG_S *stMWConfig)
{
    SAMPLE_TDL_MW_CONTEXT *stMWContext = (SAMPLE_TDL_MW_CONTEXT *)calloc(1, sizeof(SAMPLE_TDL_MW_CONTEXT));
    CVI_S32 s32Ret = SAMPLE_TDL_Init_WM(stMWConfig, stMWContext);
    if (s32Ret != CVI_SUCCESS)
    {
        printf("init middleware failed! ret=%x\n", s32Ret);
        return nullptr;
    }
    return stMWContext;
}
CameraInitInfo *autoStartCamera(int width, int height, int count)
{
    printf("user config w=%d,h=%d", width, height);
    SAMPLE_TDL_MW_CONFIG_S *stMWConfig = (SAMPLE_TDL_MW_CONFIG_S *)calloc(1, sizeof(SAMPLE_TDL_MW_CONFIG_S));
    openCamara(stMWConfig);
    defaultConfig(stMWConfig, count, width, height);
    setVPSS(stMWConfig, count, width, height);
    SAMPLE_TDL_MW_CONTEXT *stmc = initContext(stMWConfig);
    CameraInitInfo *res = (CameraInitInfo *)calloc(1, sizeof(CameraInitInfo));
    res->pstMWContext = stmc;
    res->stMWConfig = stMWConfig;
    camconf = res;
    return res;
}

void terminalFlagfunc(int signo)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    printf("handle signal flag, signo: %d\n", signo);
    terminalFlag = true;
    int led = 25;
    pinMode(led, PINMODE_OUTPUT);
    wiringOpr(led, 0);
}
void terminalService(CVI_S32 signo)
{
    printf("handle signal, signo: %d\n", signo);
    terminalFlag = true;
    SAMPLE_TDL_Destroy_MW(camconf->pstMWContext);
}