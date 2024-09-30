#include "WakeWordModel.h"
// #include "librosa.h"
#ifdef USE_LIBSNDFILE
#include "sndfile.hh"
#endif
#include <unordered_map>

WakeWordModel::WakeWordModel()
{
    // 训练参数
    m_max_epochs      = 500;
    m_batch           = 4;
    m_auto_stop_train = false;
    m_jitter          = 0.f;
    m_distort         = false;
    m_augment         = false;
    m_random          = true;

    // 音频处理参数
    m_sample_rate   = 16000;
    m_sample_time   = 750; // 1.5s
    m_mfcc_frame_shift = 160;
    m_mfcc_size     = 13;

    SetCacheSize(0);
    GetMFCC()->Reset(m_sample_rate, m_mfcc_frame_shift, m_mfcc_size);
}

void WakeWordModel::LoadNet()
{
    DarkNetSoftMaxModel::LoadNet();
    if (GetNet()->IsLoaded()) return;

    auto net     = GetNet();
    auto options = std::make_shared<std::vector<std::shared_ptr<DarkNetOptions>>>();
    auto option  = std::make_shared<DarkNetOptions>("net");
    option->SetOption("width", m_mfcc_size * 3);
    option->SetOption("height", (m_sample_rate * m_sample_time / 1000 - GetMFCC()->GetFrameSize() + m_mfcc_frame_shift ) / m_mfcc_frame_shift / 3);
    option->SetOption("channels", 1);
    options->push_back(option);

    option = std::make_shared<DarkNetOptions>("convolutional");
    option->SetOption("size", 8);
    option->SetOption("activation", "relu");
    option->SetOption("filters", 48);
    option->SetOption("stride", 2);
    options->push_back(option);

    option = std::make_shared<DarkNetOptions>("maxpool");
    options->push_back(option);

    option = std::make_shared<DarkNetOptions>("convolutional");
    option->SetOption("size", 5);
    option->SetOption("activation", "relu");
    option->SetOption("filters", 64);
    options->push_back(option);

    option = std::make_shared<DarkNetOptions>("maxpool");
    options->push_back(option);

    option = std::make_shared<DarkNetOptions>("connected");
    option->SetOption("output", 32);
    options->push_back(option);

    option = std::make_shared<DarkNetOptions>("dropout");
    options->push_back(option);

    option = std::make_shared<DarkNetOptions>("connected");
    option->SetOption("output", GetClassifySize());
    option->SetOption("activation", "linear");
    options->push_back(option);

    option = std::make_shared<DarkNetOptions>("softmax");
    options->push_back(option);

    net->Load(options);
}

void WakeWordModel::LoadClassifyIndex()
{
    CreateGetClassifyIndex("none");
    CreateGetClassifyIndex("开灯");
    CreateGetClassifyIndex("关灯");
    DarkNetSoftMaxModel::LoadClassifyIndex();
}

std::shared_ptr<DarkNetDataImage> WakeWordModel::AudioToImage(short *datas, int size)
{
    std::vector<float> audio(size);
    for (auto i = 0; i < size; i++) audio[i] = datas[i];
    return AudioToImage(audio);
}

std::shared_ptr<DarkNetDataImage> WakeWordModel::AudioToImage(std::vector<float> &audio)
{
    // 截取1.5s
    audio.resize(m_sample_rate * m_sample_time / 1000);
    // mfcc 提取音频特征
    // 归一化
    // auto mfcc = librosa::Feature::mfcc(audio, m_sample_rate, 512, m_mfcc_frame_shift, "hann", false, "", 2.0f, 40, 0, 8000, m_mfcc_size, true, 2);
    // CepstralMeanVarianceNormalization(mfcc);
    auto mfcc = GetMFCC()->Process(audio);
    // 构造图片 (150 / 3, 13 * 3) = 50, 49
    auto mfcc_width  = m_mfcc_size;
    auto mfcc_height = (int)mfcc.size();
    auto width       = GetNetWidth();
    auto height      = GetNetHeight();
    auto img         = std::make_shared<DarkNetDataImage>(width, height, 1);
    auto data        = img->GetData();
    for (auto i = 0; i < height; i++)
    {
        for (auto j = 0; j < width; j++)
        {
            data[i * width + j] = mfcc[(i * width + j) / mfcc_width][j % mfcc_width];
        }
    }
    return img;
}

void WakeWordModel::CepstralMeanVarianceNormalization(std::vector<std::vector<float>> &mfccCoefficients)
{
    size_t numFrames = mfccCoefficients.size();
    size_t numCoeffs = mfccCoefficients[0].size();

    // 计算全局均值
    std::vector<double> mean(numCoeffs, 0.0);
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
    std::vector<double> variance(numCoeffs, 0.0);
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

#ifdef USE_LIBSNDFILE
std::shared_ptr<DarkNetDataImage> WakeWordModel::LoadInputImage(const std::string &input_path)
{
    SndfileHandle wavHandle(input_path);
    if ((wavHandle.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16 && (wavHandle.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_FLOAT)
    {
        std::cerr << "Unsupported audio format, use 16 bit or 32 float PCM Wave" << std::endl;
        return nullptr;
    }
    // Check sampling rate
    if (wavHandle.samplerate() != m_sample_rate)
    {
        std::cerr << "Sampling rate mismatch: Found " << wavHandle.samplerate() << " instead of " << m_sample_rate << std::endl;
        return nullptr;
    }

    // Check sampling rate
    if (wavHandle.channels() != 1)
    {
        std::cerr << wavHandle.channels() << " channel files are unsupported. Use mono." << std::endl;
        return nullptr;
    }

    // 提取音频数据
    auto audio_size = m_sample_rate * m_sample_time / 1000;
    std::vector<float> audio(audio_size);
    if ((wavHandle.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16)
    {
        std::vector<short> buffer(audio_size);
        auto size  = wavHandle.read(buffer.data(), buffer.size());
        auto start = audio_size == size ? 0 : darknet::random_int(0, audio_size - size);
        for (auto i = 0; i < size; i++) audio[start + i] = buffer[i];
    }
    else
    {
        std::vector<float> buffer(audio_size);
        auto size  = wavHandle.read(buffer.data(), buffer.size());
        auto start = audio_size == size ? 0 : darknet::random_int(0, audio_size - size);
        for (auto i = 0; i < size; i++) audio[start + i] = buffer[i];
    }

    // 噪音混入 TODO
    return AudioToImage(audio);
}

void WakeWordModel::GenerateNoise(std::string out_wav_prefix, std::string in_wav_path, std::function<void(std::string)> callback)
{
    static int s_out_wav_size      = 0;
    static const int s_sample_time = m_sample_time;
    static const int s_sample_rate = m_sample_rate;
    static const int s_channels    = 1;
    static const int s_audio_size  = s_sample_rate * s_sample_time / 1000;

    SndfileHandle in_wav(in_wav_path);
    if ((in_wav.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16 || in_wav.channels() != s_channels || in_wav.samplerate() != s_sample_rate) return;

    while (true)
    {
        std::vector<short> in_audio(s_audio_size);
        auto in_size = in_wav.read(in_audio.data(), s_audio_size);
        if (in_size < s_audio_size) break;
        std::string out_wav_path = out_wav_prefix + std::to_string(s_out_wav_size++) + ".wav";
        SndfileHandle out_yes_wav(out_wav_path, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, s_channels, s_sample_rate);
        out_yes_wav.write(in_audio.data(), s_audio_size);
        if (callback) callback(out_wav_path);
    }
}

void WakeWordModel::GerneateVoice(std::string out_wav_prefix, std::string yes_wav_path, std::string no_wav_path, std::function<void(std::string)> callback)
{
    static std::unordered_map<std::string, int> s_out_wav_sizes;
    static const int s_sample_time = m_sample_time;
    static const int s_sample_rate = m_sample_rate;
    static const int s_channels    = 1;
    static const int s_audio_size  = s_sample_rate * s_sample_time / 1000;

    SndfileHandle yes_wav(yes_wav_path);
    SndfileHandle no_wav(no_wav_path);
    if ((yes_wav.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16 || yes_wav.channels() != s_channels || yes_wav.samplerate() != s_sample_rate) return;
    if ((no_wav.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16 || no_wav.channels() != s_channels || no_wav.samplerate() != s_sample_rate) return;

    auto s_out_wav_size = s_out_wav_sizes[out_wav_prefix];
    std::vector<short> yes_audio(s_audio_size);
    auto yes_size = yes_wav.read(yes_audio.data(), yes_audio.size());

    while (true)
    {
        std::vector<short> out_audio(s_audio_size);
        std::vector<short> no_audio(s_audio_size);
        auto no_size = no_wav.read(no_audio.data(), no_audio.size());

        if (no_size < s_audio_size) break;
        auto yes_start = s_audio_size == yes_size ? 0 : darknet::random_int(0, s_audio_size - yes_size);
        auto yes_end   = yes_start + yes_size;
        for (int i = 0; i < s_audio_size; i++)
        {
            out_audio[i] = no_audio[i] * 0.1f + ((i < yes_start || i >= yes_end) ? 0.f : yes_audio[i]) * 0.9f;
        }
        std::string out_wav_path = out_wav_prefix + std::to_string(s_out_wav_size++) + ".wav";
        SndfileHandle out_wav(out_wav_path, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, s_channels, s_sample_rate);
        out_wav.write(out_audio.data(), s_audio_size);
        if (callback) callback(out_wav_path);
    }

    s_out_wav_sizes[out_wav_prefix] = s_out_wav_size;
}
#endif
