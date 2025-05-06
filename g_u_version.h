#define GU_MILKV_DUO_256 1
#define GU_ROTATE180 1
// void run_bmd(){
//     VIDEO_FRAME_INFO_S stFrame{};
//     CVI_S32 s32Ret;
//     while (!terminalFlag)
//     {
//         ImageData imageData{};
//         s32Ret = getImageData(&stFrame, &imageData);
//         if (s32Ret != CVI_SUCCESS)
//         {
//             CVI_VPSS_ReleaseChnFrame(0, 0, &stFrame);
//             printf("Send Output Frame NG, ret=%x\n", s32Ret);
//             continue;
//         }
//         using namespace cv;
//         Mat gray(imageData.height, imageData.width, CV_8UC1, imageData.data);
//         cv::Rect roi(0, 60, 640, 360);
//         Mat cutimg=gray(roi);
//         ImageDetectInfo* info=detectBinaryMiddle(cutimg);
//         if(info->detected){
//             printf("detected,the offset is %d\n",info->referoffsetx);
//             x2vlinear(info);
//         }
//         else{
//             printf("not detected\n");
//         }
//         delete info;
//         free(imageData.data);
//         CVI_VPSS_ReleaseChnFrame(0, 0, &stFrame);
//         printf("release getImageData\n");
//         if (s32Ret != CVI_SUCCESS)
//         {
//             terminalService(SIGTERM);
//             printf("terminal finish\n");
//             break;
//         }
//     }
// }
// void* ledon(void *arg){
//     int led=25;
//     pinMode(led,PINMODE_OUTPUT);
//     wiringOpr(led,1);
//     sleep(1);
//     return nullptr;
// }