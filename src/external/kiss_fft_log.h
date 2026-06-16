/* Minimal kiss_fft_log.h — logging stubs so kiss_fft.c compiles without the full kissfft repo */
#ifndef KISS_FFT_LOG_H
#define KISS_FFT_LOG_H
#include <stdio.h>
#define KISS_FFT_LOG_MSG(sev, ...) ((void)0)
#define KISS_FFT_LOG_ERROR(...)    ((void)0)
#define KISS_FFT_LOG_WARNING(...)  ((void)0)
#define KISS_FFT_LOG_INFO(...)     ((void)0)
#define KISS_FFT_ERROR(msg)        ((void)0)
#endif
