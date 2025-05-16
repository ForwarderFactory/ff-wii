#include <iostream>
#include <thread>
#include <chrono>
#include <gccore.h>

void init() {
    VIDEO_Init();
    PAD_Init();

    const auto rmode = VIDEO_GetPreferredMode(NULL);
    const auto xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
}

int main() {
    init();

    std::cout << "Hello world!" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(5));
}