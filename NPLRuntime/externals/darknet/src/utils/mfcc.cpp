#include "mfcc.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
namespace darknet
{
    void MFCC::Reset(int sample_rate, int frame_shift, int cepstral_coeffs, int filters)
    {
        m_fft             = sample_rate <= 20000 ? 512 : 2048;
        m_fft_bins        = m_fft / 2 + 1;
        m_emphasis_coeff  = 0.97; // 0.97 or 0.95
        m_sample_rate     = sample_rate;
        m_filters         = filters;
        m_cepstral_coeffs = cepstral_coeffs;
        m_frame_size      = m_fft;
        m_frame_shift     = frame_shift;

        InitFilterBank(m_filters, m_fft, m_fft_bins, m_sample_rate);
        InitWindow(m_frame_size, "hamming");
        InitDCT2AndOrthoMatrix(m_filters);
    }

    void MFCC::InitWindow(int window_size, std::string window_type)
    {
        m_window.resize(window_size);

        if (window_type == "hamming")
        {
            for (int i = 0; i < window_size; i++)
            {
                m_window[i] = 0.54 - 0.46 * std::cos((2 * M_PI * i) / (window_size - 1));
            }
        }
        else if (window_type == "hanning")
        {
            for (int i = 0; i < window_size; i++)
            {
                m_window[i] = 0.5 * (1 - std::cos((2 * M_PI * i) / (window_size - 1)));
            }
        }
        else if (window_type == "blackman")
        {
            for (int i = 0; i < window_size; i++)
            {
                m_window[i] = 0.42 - 0.5 * std::cos((2 * M_PI * i) / (window_size - 1)) + 0.08 * std::cos((4 * M_PI * i) / (window_size - 1));
            }
        }
        else if (window_type == "bartlett")
        {
            auto half_window_size = window_size / 2;
            for (int i = 0; i < window_size; i++)
            {
                if (i < half_window_size)
                {
                    m_window[i] = static_cast<float>(i) / half_window_size; // 从0到N/2的线性增加
                }
                else
                {
                    m_window[i] = static_cast<float>(window_size - i - 1) / half_window_size; // 从N/2到N的线性减少
                }
            }
        }
        else // if (window_type == "rectangular")
        {
            for (int i = 0; i < window_size; i++)
            {
                m_window[i] = 1;
            }
        }
    }

    // 生成 DCT二维 矩阵
    // filters 梅尔滤波器个数
    void MFCC::InitDCT2AndOrthoMatrix(int filters)
    {
        m_dct_matrix.assign(filters, LinSpace(filters, 0, filters - 1));
        std::vector<std::vector<float>> transpose_matrix(filters, std::vector<float>(filters));

        for (int i = 0; i < filters; ++i)
        {
            for (int j = 0; j < filters; ++j)
            {
                transpose_matrix[j][i] = m_dct_matrix[i][j];
            }
        }

        // 计算系数矩阵
        for (int i = 0; i < filters; ++i)
        {
            for (int j = 0; j < filters; ++j)
            {
                m_dct_matrix[i][j] = 2 * std::cos(M_PI * transpose_matrix[i][j] / filters * (m_dct_matrix[i][j] + 0.5));
            }
        }

        // 初始化正交矩阵, 归一化 dct 矩阵结果
        m_ortho_matrix.assign(filters, std::vector<float>(filters, 0.0f));

        std::vector<float> ortho(filters, std::sqrtf(0.5f / filters));
        ortho[0] = std::sqrtf(0.25f / filters);

        for (int i = 0; i < filters; ++i)
        {
            m_ortho_matrix[i][i] = ortho[i];
        }
    }

    // 初始化梅尔滤波器组
    // filters 滤波器个数
    // fft 傅里叶变换的采样点个数
    // sample_rate 采样率
    void MFCC::InitFilterBank(int filters, int fft, int fft_bins, int sample_rate)
    {
        float f_min       = 0.0;                         // 最小频率偏移
        float f_sp        = 200.f / 3.f;                 // 频率间距
        float min_log_hz  = 1000.0;                      // 最小赫兹频率
        float min_log_mel = (min_log_hz - f_min) / f_sp; // 最小梅尔频率
        float logstep     = std::logf(6.4f) / 27.f;      // 对数步长

        auto hz_to_mel = [=](int hz) -> float
        {
            return (hz >= min_log_hz) ? (min_log_mel + std::logf(hz / min_log_hz) / logstep) : ((hz - f_min) / f_sp);
        };
        auto mel_to_hz = [=](float &mel) -> float
        {
            return (mel > min_log_mel) ? (std::exp((mel - min_log_mel) * logstep) * min_log_hz) : ((mel * f_sp) + f_min);
        };

        // 计算梅尔频率范围
        float min_mel = hz_to_mel(0.0);
        float max_mel = hz_to_mel(sample_rate / 2.0);
        std::vector<float> mels(filters + 2);
        for (int i = 0; i < filters + 2; ++i)
        {
            mels[i] = min_mel + i * (max_mel - min_mel) / (filters + 1);
        }

        // 计算滤波器的中心频率
        std::vector<float> mel_hzs(filters + 2);
        for (int i = 0; i < filters + 2; ++i)
        {
            mel_hzs[i] = mel_to_hz(mels[i]);
        }

        // 计算频率差
        std::vector<float> fdiff(filters + 1);
        for (int i = 0; i <= filters; ++i)
        {
            fdiff[i] = mel_hzs[i + 1] - mel_hzs[i];
        }

        // 计算FFT频域的频率点
        std::vector<float> fft_hzs(fft_bins);
        for (int i = 0; i < fft_bins; i++)
        {
            fft_hzs[i] = static_cast<float>(sample_rate) * i / fft;
        }

        // 计算梅尔频率的复制矩阵
        std::vector<std::vector<float>> mel_f_replicated(fft_bins, mel_hzs);
        // 转置复制矩阵
        std::vector<std::vector<float>> mel_f_transposed(filters + 2, std::vector<float>(fft_bins, 0.0));
        for (int i = 0; i < fft_bins; ++i)
        {
            for (int j = 0; j < filters + 2; ++j)
            {
                mel_f_transposed[j][i] = mel_f_replicated[i][j];
            }
        }

        // 计算FFT频率点的复制矩阵
        std::vector<std::vector<float>> fft_freqs_replicated(filters + 2, fft_hzs);

        // 计算斜率矩阵
        std::vector<std::vector<float>> ramps(filters + 2, std::vector<float>(fft_bins, 0.0));
        for (int i = 0; i < filters + 2; ++i)
        {
            for (int j = 0; j < fft_bins; ++j)
            {
                ramps[i][j] = mel_f_transposed[i][j] - fft_hzs[j];
            }
        }

        // 计算下限和上限
        std::vector<std::vector<float>> lower(filters, std::vector<float>(fft_bins, 0.0));
        std::vector<std::vector<float>> upper(filters, std::vector<float>(fft_bins, 0.0));
        for (int i = 0; i < filters; ++i)
        {
            for (int j = 0; j < fft_bins; ++j)
            {
                lower[i][j] = -ramps[i][j] / fdiff[i];
                upper[i][j] = ramps[i + 2][j] / fdiff[i + 1];
            }
        }

        // 计算权重
        std::vector<std::vector<float>> weights(filters, std::vector<float>(fft_bins, 0.0));
        for (int i = 0; i < filters; ++i)
        {
            for (int j = 0; j < fft_bins; ++j)
            {
                if (lower[i][j] < upper[i][j])
                {
                    weights[i][j] = lower[i][j];
                }
                else
                {
                    weights[i][j] = upper[i][j];
                }
                weights[i][j] = std::max(weights[i][j], 0.0f);
            }
        }

        // 计算归一化系数
        std::vector<float> enorm(filters);
        for (int i = 0; i < filters; ++i)
        {
            enorm[i] = 2.0 / (mel_hzs[i + 2] - mel_hzs[i]);
        }

        // 构建滤波器
        m_filterbank.resize(filters, std::vector<float>(fft_bins, 0.0));
        for (int i = 0; i < filters; ++i)
        {
            for (int j = 0; j < fft_bins; ++j)
            {
                m_filterbank[i][j] = weights[i][j] * enorm[i];
            }
        }
    }

    std::vector<std::vector<float>> MFCC::Process(const std::vector<float> &samples)
    {
        std::vector<std::vector<float>> mfccs;
        std::vector<float> frames(m_frame_size, 0.f);
        auto frames_count = (samples.size() - m_frame_size + m_frame_shift) / m_frame_shift;

        for (int i = 0; i < frames_count; i++)
        {
            frames.assign(samples.begin() + i * m_frame_shift, samples.begin() + i * m_frame_shift + m_frame_size);
            mfccs.push_back(ProcessFrame(frames));
        }

        CepstralMeanVarianceNormalization(mfccs);
        return mfccs;
    }

    std::vector<float> MFCC::ProcessFrame(std::vector<float> &frames)
    {
        PreEmphasis(frames, m_emphasis_coeff);
        ApplyWindow(frames, m_window);
        ComputePowerSpectrum(m_power_spectrum, frames, m_fft, m_fft_bins);
        ApplyLogMelFilterBank(m_spectrogram, m_power_spectrum, m_filterbank, m_filters, m_fft_bins);
        ApplyDCT2AndOrthoMatrix(m_mfcc, m_spectrogram, m_dct_matrix, m_ortho_matrix, m_filters);
        m_mfcc.resize(m_cepstral_coeffs);
        return m_mfcc;
    }

    void MFCC::PreEmphasis(std::vector<float> &frames, float emphasis_coeff)
    {
        for (int i = 1; i < frames.size(); ++i)
        {
            frames[i] = frames[i] - emphasis_coeff * frames[i - 1];
        }
    }

    void MFCC::ApplyWindow(std::vector<float> &frames, std::vector<float> &window)
    {
        for (int i = 0; i < frames.size(); ++i)
        {
            frames[i] *= window[i];
        }
    }

    void MFCC::FFT(std::vector<std::complex<float>> &x)
    {
        auto N = x.size();
        if (N <= 1) return;

        // 分离偶数和奇数索引元素
        auto half_N = N / 2;
        std::vector<std::complex<float>> even(half_N);
        std::vector<std::complex<float>> odd(half_N);
        for (int i = 0; i < half_N; ++i)
        {
            even[i] = x[i * 2];
            odd[i]  = x[i * 2 + 1];
        }

        // 对偶数和奇数部分进行递归FFT
        FFT(even);
        FFT(odd);

        // 合并结果
        for (int i = 0; i < half_N; ++i)
        {
            auto angle    = -2 * M_PI * i / N;
            auto t        = std::complex<float>(std::cos(angle), std::sin(angle)) * odd[i];
            x[i]          = even[i] + t;
            x[i + half_N] = even[i] - t;
        }
    }

    // 计算功率谱
    void MFCC::ComputePowerSpectrum(std::vector<float> &power_spectrum, const std::vector<float> &frames, int fft, int fft_bins)
    {
        std::vector<std::complex<float>> fft_x(fft);
        fft_x.assign(frames.begin(), frames.end());

        FFT(fft_x);

        power_spectrum.resize(fft_bins);
        for (int i = 0; i < fft_bins; i++)
        {
            power_spectrum[i] = std::pow(std::abs(fft_x[i]), 2);
        }
    }

    // 应用 Log Mel 滤波器组
    void MFCC::ApplyLogMelFilterBank(std::vector<float> &spectrogram, const std::vector<float> &power_spectrum, const std::vector<std::vector<float>> &filterbank, int filters, int fft_bins, float floor)
    {
        spectrogram.assign(filters, 0.0);

        // 应用mel滤波
        for (int m = 0; m < filters; ++m)
        {
            for (int k = 0; k < fft_bins; ++k)
            {
                spectrogram[m] += power_spectrum[k] * filterbank[m][k];
            }
        }

        // 计算对数谱
        for (size_t i = 0; i < filters; ++i)
        {
            spectrogram[i] = 10.0f * std::log10(std::max(spectrogram[i], 1e-10f));
        }

        // 找到最大值
        float max_coeff = *std::max_element(spectrogram.begin(), spectrogram.end());

        // 裁剪对数谱
        for (size_t i = 0; i < filters; ++i)
        {
            spectrogram[i] = std::max(spectrogram[i], max_coeff - 80.0f);
        }
    }

    // 应用离散余弦变换 Computing discrete cosine transform
    void MFCC::ApplyDCT2AndOrthoMatrix(std::vector<float> &mfcc, const std::vector<float> &spectrogram, const std::vector<std::vector<float>> &dct_matrix, const std::vector<std::vector<float>> &ortho_matrix, int filters)
    {
        std::vector dct_matrix_results(filters, 0.f);
        for (int i = 0; i < filters; i++)
        {
            for (int j = 0; j < filters; j++)
            {
                dct_matrix_results[i] += spectrogram[j] * dct_matrix[i][j];
            }
        }

        mfcc.assign(filters, 0);
        for (int i = 0; i < m_filters; ++i)
        {
            for (int j = 0; j < m_filters; ++j)
            {
                mfcc[i] += dct_matrix_results[j] * ortho_matrix[j][i]; // (1, N) * (N, M) = (1, M)
            }
        }
    }

    void MFCC::CepstralMeanVarianceNormalization(std::vector<std::vector<float>> &mfccCoefficients)
    {
        size_t numFrames = mfccCoefficients.size();
        size_t numCoeffs = mfccCoefficients[0].size();

        // 计算全局均值
        std::vector<float> mean(numCoeffs, 0.0);
        for (const auto &frame : mfccCoefficients)
        {
            for (size_t i = 0; i < numCoeffs; ++i)
            {
                mean[i] += frame[i];
            }
        }
        for (auto &m : mean)
        {
            m /= numFrames;
        }

        // 计算全局方差
        std::vector<float> variance(numCoeffs, 0.0);
        for (const auto &frame : mfccCoefficients)
        {
            for (size_t i = 0; i < numCoeffs; ++i)
            {
                variance[i] += (frame[i] - mean[i]) * (frame[i] - mean[i]);
            }
        }
        for (auto &v : variance)
        {
            v /= numFrames;
            v = std::sqrt(v); // 计算标准差
        }

        // 归一化
        for (auto &frame : mfccCoefficients)
        {
            for (size_t i = 0; i < numCoeffs; ++i)
            {
                frame[i] = (frame[i] - mean[i]) / (variance[i] + 1e-8); // 避免除以0
            }
        }
    }

    std::vector<float> MFCC::LinSpace(int size, float start, float end)
    {
        std::vector<float> result(size);
        float step = (end - start) / (size - 1);
        for (int i = 0; i < size; ++i)
        {
            result[i] = start + i * step;
        }
        return result;
    }
}