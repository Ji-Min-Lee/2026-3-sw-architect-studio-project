#ifndef FFTLIB_H
#define FFTLIB_H

#include <cstddef>
#include <vector>

std::vector<double> processFFT(const float *pcmData, size_t windowSize);

#endif // FFTLIB_H
