#ifndef __DARKNET_UTILS_MFCC_H__
#define __DARKNET_UTILS_MFCC_H__

#include <algorithm>
#include <cmath>
#include <complex>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace darknet
{
    class MFCC
    {
    public:
        MFCC() { Reset(); }

        void Reset(int sample_rate = 16000, int frame_shift = 160, int cepstral_coeffs = 13, int filters = 40);
        std::vector<std::vector<float>> Process(const std::vector<float> &samples);
        std::vector<float> ProcessFrame(std::vector<float> &frames);
        // std::vector<float> process(const std::string &wavFile);
        // std::vector<float> processFrame(const std::vector<float> &samples);
        // std::vector<float> processFrame(const std::string &wavFile);

    public:
        inline int GetFrameSize() { return m_frame_size; }
        inline std::vector<std::vector<float>> &GetFilterBank() { return m_filterbank; }

    protected:
        void InitFilterBank(int filters, int fft, int fft_bins, int sample_rate);
        void InitWindow(int window_size, std::string window_type);
        void InitDCT2AndOrthoMatrix(int filters);

        // 预加重(Pre-emphasis)
        void PreEmphasis(std::vector<float> &frames, float emphasis_coeff);
        void ApplyWindow(std::vector<float> &frames, std::vector<float> &window);
        void FFT(std::vector<std::complex<float>> &x);
        void ComputePowerSpectrum(std::vector<float> &power_spectrum, const std::vector<float> &frames, int fft, int fft_bins);
        void ApplyLogMelFilterBank(std::vector<float> &spectrogram, const std::vector<float> &power_spectrum, const std::vector<std::vector<float>> &filterbank, int filters, int fft_bins, float floor = 1.0);
        void ApplyDCT2AndOrthoMatrix(std::vector<float> &mfcc, const std::vector<float> &spectrogram, const std::vector<std::vector<float>> &dct_matrix, const std::vector<std::vector<float>> &ortho_matrix, int filters);
        void CepstralMeanVarianceNormalization(std::vector<std::vector<float>> &mfccCoefficients);
        std::vector<float> LinSpace(int size, float start, float end);

        static void Print(const std::string &name, std::vector<float> x, std::ostream &os = std::cout)
        {
            os << name << ": ";
            for (auto &val : x)
            {
                os << val << " ";
            }
            os << std::endl;
        }

        static void Print(const std::string &name, std::vector<std::vector<float>> x, std::ostream &os = std::cout)
        {
            os << name << ": ";
            for (int i = 0; i < x.size(); ++i)
            {
                Print(std::to_string(i), x[i], os);
            }
            os << std::endl;
        }

    protected:
        // Hertz to Mel conversion
        inline float hz2mel(float freq) { return 2595.0 * std::log10(1.0 + freq / 700.0); }
        // Mel to Hertz conversion
        inline float mel2hz(float mel) { return 700.0 * (std::pow(10.0, mel / 2595.0) - 1); }

    protected:
        int m_sample_rate;                              // 采样率s
        int m_fft;                                      // 快速傅里叶变换（Fast Fourier Transform，FFT）大小
        int m_filters;                                  // 滤波器数量
        int m_cepstral_coeffs;                          // 倒谱系数
        float m_emphasis_coeff;                         // 预加重系数
        std::vector<float> m_window;                    // 窗口 用于减少频谱泄漏和旁瓣效应
        std::vector<std::vector<float>> m_dct_matrix;   // 离散余弦变换(Discrete Cosine Transform) 矩阵
        std::vector<std::vector<float>> m_ortho_matrix; // 正交矩阵调整输出
        std::vector<std::vector<float>> m_filterbank;   // 梅尔滤波器

        int m_frame_size;  // 帧大小
        int m_frame_shift; // 帧步长

        // 中间结果
        int m_fft_bins;                      // FFT的频谱数量
        std::vector<float> m_power_spectrum; // 功率谱
        std::vector<float> m_spectrogram;    // 频谱
        std::vector<float> m_mfcc;
    };
}
#endif