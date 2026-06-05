#include "FftLib.h"

#include <cmath>
#include <complex>
#include <iostream>

typedef std::complex<double> Complex;

static void fftCore(std::vector<Complex> &buffer)
{
    const size_t N = buffer.size();
    if (N <= 1) return;

    std::vector<Complex> even(N / 2);
    std::vector<Complex> odd(N / 2);
    for (size_t i = 0; i < N / 2; ++i) {
        even[i] = buffer[2 * i];
        odd[i] = buffer[2 * i + 1];
    }

    fftCore(even);
    fftCore(odd);

    const double pi = std::acos(-1.0);
    for (size_t k = 0; k < N / 2; ++k) {
        Complex t = std::polar(1.0, -2.0 * pi * static_cast<double>(k) / static_cast<double>(N)) * odd[k];
        buffer[k] = even[k] + t;
        buffer[k + N / 2] = even[k] - t;
    }
}

static void applyHannWindow(std::vector<Complex> &buffer)
{
    const size_t N = buffer.size();
    if (N == 0) return;

    for (size_t i = 0; i < N; ++i) {
        const double w = 0.5 * (1.0 - std::cos(2.0 * std::acos(-1.0) * static_cast<double>(i) / static_cast<double>(N - 1)));
        buffer[i] *= w;
    }
}

std::vector<double> processFFT(const float *pcmData, size_t windowSize)
{
    if ((windowSize & (windowSize - 1)) != 0 || windowSize == 0) {
        std::cerr << "Error: Window size must be a power of 2!" << std::endl;
        return {};
    }

    std::vector<Complex> fftBuffer(windowSize);
    for (size_t i = 0; i < windowSize; ++i) {
        fftBuffer[i] = Complex(static_cast<double>(pcmData[i]), 0.0);
    }

    applyHannWindow(fftBuffer);
    fftCore(fftBuffer);

    const size_t halfSize = windowSize / 2;
    std::vector<double> magnitudes(halfSize);
    for (size_t i = 0; i < halfSize; ++i) {
        magnitudes[i] = std::abs(fftBuffer[i]);
    }

    return magnitudes;
}
