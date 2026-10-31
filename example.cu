// comment /32 does not count
int lane = threadIdx.x & 31;
#if __CUDA_ARCH__ >= 800
asm("mov.u32 %0, %laneid;");
#endif
