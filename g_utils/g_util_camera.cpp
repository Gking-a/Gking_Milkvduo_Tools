#include "g_u_h.h"
CameraInitInfo* camconf{};
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
void setVPSS(SAMPLE_TDL_MW_CONFIG_S *stMWConfig, int count , uint u32w , uint u32h )
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
    VPSS_GRP_DEFAULT_HELPER2(&pstVpssConfig->stVpssGrpAttr, u32w,
                             u32h, VI_PIXEL_FORMAT, 1);
    pstVpssConfig->u32ChnCount = count;
    pstVpssConfig->u32ChnBindVI = 0;
    for (int i = 0; i < count; i++)
    {
        VPSS_CHN_DEFAULT_HELPER(&pstVpssConfig->astVpssChnAttr[i], u32w,
                                u32h, VI_PIXEL_FORMAT, true);
    }
    SAMPLE_TDL_Get_Input_Config(&stMWConfig->stVencConfig.stChnInputCfg);
    stMWConfig->stVencConfig.u32FrameWidth = u32w;
    stMWConfig->stVencConfig.u32FrameHeight = u32h;
}
void defaultConfig(SAMPLE_TDL_MW_CONFIG_S *stMWConfig, int count , uint u32w , uint u32h )
{
    stMWConfig->stVBPoolConfig.u32VBPoolCount += count;
    for (int i = 0; i < count; i++)
    {
        // VBPool 0 for VPSS Grp0 Chn0
        stMWConfig->stVBPoolConfig.astVBPoolSetup[i].enFormat = VI_PIXEL_FORMAT;
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
CameraInitInfo *autoStartCamera(int count , int width , int height )
{
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
void terminalService(CVI_S32 signo)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    printf("handle signal, signo: %d\n", signo);
    if (SIGINT == signo || SIGTERM == signo)
    {
        SAMPLE_TDL_Destroy_MW(camconf->pstMWContext);
    }
}