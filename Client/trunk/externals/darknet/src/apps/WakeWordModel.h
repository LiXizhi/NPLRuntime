#ifndef __WAKE_WORD_MODEL_H__
#define __WAKE_WORD_MODEL_H__

#include "DarkNet.h"

class WakeWordModel : public DarkNetSoftMaxModel
{
public:
    WakeWordModel();
    virtual void LoadNet();
    virtual void LoadClassifyIndex();
#ifdef USE_LIBSNDFILE
    virtual std::shared_ptr<DarkNetDataImage> LoadInputImage(const std::string &input_path);
#else
    virtual std::shared_ptr<DarkNetDataImage> LoadInputImage(const std::string &input_path) { return nullptr; }
#endif

public:
    int Predict(short *datas, int size, float *predict_probability) { return DarkNetSoftMaxModel::Predict(AudioToImage(datas, size), predict_probability); }
    int Predict(const std::string &img_path, float *predict_probability) { return DarkNetSoftMaxModel::Predict(img_path, predict_probability); }

public:
    inline void SetSampleTime(int sample_time) { m_sample_time = sample_time; }
    inline int GetSampleTime() { return m_sample_time; }
    inline int GetSampleRate() { return m_sample_rate; }
    inline DarkNetMFCC *GetMFCC() { return &m_mfcc; }

#ifdef USE_LIBSNDFILE
public:
    void GenerateNoise(std::string out_wav_dircectory, std::string in_wav_path, std::function<void(std::string)> callback = nullptr);
    void GerneateVoice(std::string out_wav_dircectory, std::string yes_wav_path, std::string no_wav_path, std::function<void(std::string)> callback = nullptr);
#endif

protected:
    std::shared_ptr<DarkNetDataImage> AudioToImage(short *datas, int size);
    std::shared_ptr<DarkNetDataImage> AudioToImage(std::vector<float> &audio);
    void CepstralMeanVarianceNormalization(std::vector<std::vector<float>> &mfccCoefficients);

protected:
    int m_sample_rate;
    int m_sample_time;
    int m_mfcc_frame_shift;
    int m_mfcc_size;

    DarkNetMFCC m_mfcc;
};

#endif