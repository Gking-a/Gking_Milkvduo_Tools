#pragma once
#ifndef __GKING_MIKLV_DUO_UTIL_INCLUDED
#define __GKING_MIKLV_DUO_UTIL_INCLUDED
#include <signal.h>
#include <cvi_type.h>
#include <stdio.h>
#include <middleware_utils.h>
#include "cvi_tdl.h"
#include <cvi_comm.h>
#include <sample_utils.h>
#include "core/utils/vpss_helper.h"
#include "wiringx.h"
typedef unsigned int uint;
typedef struct {
    SAMPLE_TDL_MW_CONFIG_S* stMWConfig;
    SAMPLE_TDL_MW_CONTEXT *pstMWContext;
  } CameraInitInfo;
extern CameraInitInfo* camconf;
#ifdef __cplusplus
extern "C"
{
#endif

int openCamara(SAMPLE_TDL_MW_CONFIG_S *stMWConfig);
void setVPSS(SAMPLE_TDL_MW_CONFIG_S *stMWConfig, int count = 1, uint u32w = 1920, uint u32h = 1080);
void defaultConfig(SAMPLE_TDL_MW_CONFIG_S *stMWConfig, int count = 1, uint u32w = 1920, uint u32h = 1080);
SAMPLE_TDL_MW_CONTEXT *initContext(SAMPLE_TDL_MW_CONFIG_S *stMWConfig);
CameraInitInfo *autoStartCamera(int count = 1, int width = 1920, int height = 1080);
void terminalService(CVI_S32 signo);
int wiringSetup();
/**
 * @param hl high:1,low:0
 * @return -1 if failed else 0
 */
int wiringOpr(int gpioid,int hl);

#ifdef __cplusplus
}
#endif
#endif