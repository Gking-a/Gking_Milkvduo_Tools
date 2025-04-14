#include "g_u_h.h"
#include "wiringx.h"
int wiringSetup(){
    if(wiringXSetup("milkv_duo", NULL) == -1) {
        wiringXGC();
        return -1;
    }
    return 0;
}
int wiringOpr(int gpioid,int hl){
    if(wiringXValidGPIO(gpioid) != 0) {
        printf("Invalid GPIO %d\n", gpioid);
        return -1;
    }
    
    digitalWrite(gpioid, digital_value_t(hl));
    return 0;
}