#include "Layer.h"
#include "cores/Net.h"
namespace darknet
{
    Layer::Layer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options)
    {
        m_name        = "layer";
        m_batch       = 1;
        m_type        = s_none;
        m_in_width    = 0;
        m_in_height   = 0;
        m_in_filters  = 0;
        m_out_width   = 0;
        m_out_height  = 0;
        m_out_filters = 0;
        m_batch       = 1;
        m_cost        = 0;

        m_options    = options;
        m_net        = nullptr;
        m_prev_layer = nullptr;
        m_next_layer = nullptr;

        if (prev_layer != nullptr)
        {
            m_prev_layer = prev_layer;
            m_prev_layer->SetNextLayer(this);
            m_in_width   = m_prev_layer->GetOutWidth();
            m_in_height  = m_prev_layer->GetOutHeight();
            m_in_filters = m_prev_layer->GetOutFilters();
        }

        if (net != nullptr)
        {
            m_net = net;
            if (prev_layer == nullptr)
            {
                m_in_width   = m_net->GetWidth();
                m_in_height  = m_net->GetHeight();
                m_in_filters = m_net->GetChannels();
            }
        }

        m_batch_normalize        = false;
        m_option_batch_normalize = options->GetOption("batch_normalize", false);
    }

    void Layer::InitBatchNormalize()
    {
        if (!IsBatchNormalize()) return;

        auto filter         = GetOutFilters();
        auto batch_out_size = GetBatchOutSize();

        m_batch_normalize_scales.assign(filter, 1.0f);
        m_batch_normalize_scales_deltas.assign(filter, 0.f);

        m_batch_normalize_means.assign(filter, 0.f);
        m_batch_normalize_means_deltas.assign(filter, 0.f);

        m_batch_normalize_variances.assign(filter, 0.f);
        m_batch_normalize_variances_deltas.assign(filter, 0.f);

        m_batch_normalize_rolling_means.assign(filter, 0.f);
        m_batch_normalize_rolling_variances.assign(filter, 0.f);

        m_batch_normalize_x.assign(batch_out_size, 0.f);
        m_batch_normalize_x_normalize.assign(batch_out_size, 0.f);
    }

    void Layer::Resize()
    {
        auto batch_out_size = GetBatchOutSize();
        if (batch_out_size != m_outputs.size()) m_outputs.assign(batch_out_size, 0.0f);
        if (batch_out_size != m_deltas.size()) m_deltas.assign(batch_out_size, 0.0f);

        if (IsBatchNormalize() && batch_out_size != m_batch_normalize_x.size())
        {
            m_batch_normalize_x.assign(batch_out_size, 0.f);
            m_batch_normalize_x_normalize.assign(batch_out_size, 0.f);
        }

        if (m_next_layer == nullptr && batch_out_size != m_losses.size()) m_losses.assign(batch_out_size, 0.0f);
    }

    int Layer::GetLayerIndex(Layer *layer)
    {
        int layer_index = 0;
        auto prev_layer = layer == nullptr ? m_prev_layer : layer->m_prev_layer;
        while (prev_layer != nullptr)
        {
            prev_layer = prev_layer->GetPrevLayer();
            layer_index++;
        }
        return layer_index;
    }

    Layer *Layer::GetIndexLayer(int index)
    {
        return GetNet()->GetLayer(index);
    }

    int Layer::GetNetWidth()
    {
        return GetNet()->GetWidth();
    }

    int Layer::GetNetHeight()
    {
        return GetNet()->GetHeight();
    }

    int Layer::GetMaxTruthsSize()
    {
        return GetNet()->GetMaxTruthsSize();
    }

    int Layer::GetMaxTruths()
    {
        return GetNet()->GetMaxTruths();
    }

    int Layer::GetTruthsSize()
    {
        return GetNet()->GetTruthsSize();
    }

    int Layer::GetNetChannels()
    {
        return GetNet()->GetChannels();
    }

    float *Layer::GetWorkspace()
    {
        return GetNet()->GetWorkspace();
    }

    float *Layer::GetInTruths()
    {
        return GetNet()->GetInTruths();
    }

    float *Layer::GetInDatas()
    {
        return m_prev_layer == nullptr ? GetNet()->GetInDatas() : m_prev_layer->GetOutDatas();
    }

    float *Layer::GetOutDatas()
    {
        return m_outputs.data();
    }

    float *Layer::GetInDeltas()
    {
        return m_deltas.data();
    }

    float *Layer::GetOutDeltas()
    {
        return m_prev_layer == nullptr ? nullptr : m_prev_layer->GetInDeltas();
    }

    bool Layer::CheckNanAndInf()
    {
        auto in_datas      = GetInDatas();
        auto in_datas_size = GetBatchInSize();
        if (IsExistNanAndInf(in_datas, in_datas_size)) return false;
        return true;
    }

    bool Layer::IsExistNanAndInf(float *datas, int size)
    {
        for (int i = 0; i < size; i++)
        {
            auto data = datas[i];
            if (std::isnan(data) || std::isinf(data))
            {
                return true;
            }
        }
        return false;
    }

    std::ostream *Layer::GetInputOS(int input_index)
    {
        auto in_oss = GetNet()->GetInOSS();
        if (in_oss == nullptr) return nullptr;
        return input_index < 0 || input_index >= in_oss->size() ? nullptr : in_oss->at(input_index);
    }

    void Layer::ForwardAddBiases(float *im_datas, float *biases, int batch, int filter, int im_size)
    {
        for (int b = 0; b < batch; ++b)
        {
            for (int i = 0; i < filter; ++i)
            {
                for (int j = 0; j < im_size; ++j)
                {
                    im_datas[(b * filter + i) * im_size + j] += biases[i];
                }
            }
        }
    }

    void Layer::BackwardAddBiases(float *biases_deltas, float *deltas, int batch, int filter, int im_size)
    {
        for (int b = 0; b < batch; ++b)
        {
            for (int i = 0; i < filter; ++i)
            {
                float sum      = 0.f;
                float *i_delta = deltas + im_size * (i + b * filter);
                for (int j = 0; j < im_size; ++j)
                {
                    sum += *i_delta++;
                }
                biases_deltas[i] += sum;
            }
        }
    }

    void Layer::ForwardScale(float *im_datas, float *scales, int batch, int filter, int im_size)
    {
        for (int b = 0; b < batch; ++b)
        {
            for (int i = 0; i < filter; ++i)
            {
                for (int j = 0; j < im_size; ++j)
                {
                    im_datas[(b * filter + i) * im_size + j] *= scales[i];
                }
            }
        }
    }
    void Layer::BackwardScale(float *x_normalize, float *deltas, int batch, int filter, int im_size, float *scales_deltas)
    {
        for (int f = 0; f < filter; ++f)
        {
            float sum = 0;
            for (int b = 0; b < batch; ++b)
            {
                for (int i = 0; i < im_size; ++i)
                {
                    int index = i + im_size * (f + filter * b);
                    sum += deltas[index] * x_normalize[index];
                }
            }
            scales_deltas[f] += sum;
        }
    }

    void Layer::ForwardBatchNormalizeLayer()
    {
        auto out_datas         = GetOutDatas();
        auto batch_out_size    = GetBatchOutSize();
        auto biases            = m_biases.data();
        auto scales            = m_batch_normalize_scales.data();
        auto means             = m_batch_normalize_means.data();
        auto rolling_means     = m_batch_normalize_rolling_means.data();
        auto variances         = m_batch_normalize_variances.data();
        auto rolling_variances = m_batch_normalize_rolling_variances.data();
        auto x                 = m_batch_normalize_x.data();
        auto x_normalize       = m_batch_normalize_x_normalize.data();
        copy_cpu(batch_out_size, out_datas, 1, x, 1);
        if (GetNet()->GetInTruths() == nullptr)
        {
            normalize_cpu(out_datas, rolling_means, rolling_variances, m_batch, m_out_filters, m_out_width * m_out_height);
        }
        else
        {
            mean_cpu(out_datas, m_batch, m_out_filters, m_out_width * m_out_height, means);

            variance_cpu(out_datas, means, m_batch, m_out_filters, m_out_width * m_out_height, variances);

            scal_cpu(m_out_filters, 0.99f, rolling_means, 1);
            axpy_cpu(m_out_filters, 0.01f, means, 1, rolling_means, 1);
            scal_cpu(m_out_filters, 0.99f, rolling_variances, 1);
            axpy_cpu(m_out_filters, 0.01f, variances, 1, rolling_variances, 1);

            normalize_cpu(out_datas, means, variances, m_batch, m_out_filters, m_out_width * m_out_height);
            copy_cpu(batch_out_size, out_datas, 1, x_normalize, 1);
        }
        ForwardScale(out_datas, scales, m_batch, m_out_filters, m_out_width * m_out_height);
        ForwardAddBiases(out_datas, biases, m_batch, m_out_filters, m_out_width * m_out_height);
    }

    void Layer::BackwardBatchNormalizeLayer()
    {
        auto means             = m_batch_normalize_means.data();
        auto rolling_means     = m_batch_normalize_rolling_means.data();
        auto variances         = m_batch_normalize_variances.data();
        auto rolling_variances = m_batch_normalize_rolling_variances.data();

        if (GetNet()->GetInTruths() == nullptr)
        {
            means     = rolling_means;
            variances = rolling_variances;
        }

        auto deltas           = m_deltas.data();
        auto biases_deltas    = m_biases_deltas.data();
        auto scales           = m_batch_normalize_scales.data();
        auto scales_deltas    = m_batch_normalize_scales_deltas.data();
        auto means_deltas     = m_batch_normalize_means_deltas.data();
        auto variances_deltas = m_batch_normalize_variances_deltas.data();
        auto x                = m_batch_normalize_x.data();
        auto x_normalize      = m_batch_normalize_x_normalize.data();
        BackwardAddBiases(biases_deltas, deltas, m_batch, m_out_filters, m_out_width * m_out_height);
        BackwardScale(x_normalize, deltas, m_batch, m_out_filters, m_out_width * m_out_height, scales_deltas);
        ForwardScale(deltas, scales, m_batch, m_out_filters, m_out_width * m_out_height);

        mean_delta_cpu(deltas, variances, m_batch, m_out_filters, m_out_width * m_out_height, means_deltas);
        variance_delta_cpu(x, deltas, means, variances, m_batch, m_out_filters, m_out_width * m_out_height, variances_deltas);
        normalize_delta_cpu(x, means, variances, means_deltas, variances_deltas, m_batch, m_out_filters, m_out_width * m_out_height, deltas);
    }

    bool Layer::LoadWeights(std::istream &is)
    {
        int biases_size = GetBiasesSize();
        if (biases_size > 0)
        {
            is.read((char *)(&biases_size), sizeof(biases_size));
            if (biases_size != GetBiasesSize()) return false;
            is.read((char *)(GetBiasesData()), biases_size * sizeof(float));
        }
        int weights_size = GetWeightsSize();
        if (weights_size > 0)
        {
            is.read((char *)(&weights_size), sizeof(weights_size));
            if (weights_size != GetWeightsSize()) return false;
            is.read((char *)(GetWeightsData()), weights_size * sizeof(float));
        }
        if (m_batch_normalize)
        {
            int nscales = m_batch_normalize_scales.size();
            if (nscales > 0)
            {
                is.read((char *)(&nscales), sizeof(nscales));
                if (nscales != m_batch_normalize_scales.size()) return false;
                is.read((char *)(m_batch_normalize_scales.data()), nscales * sizeof(float));
            }
            int nrolling_means = m_batch_normalize_rolling_means.size();
            if (nrolling_means > 0)
            {
                is.read((char *)(&nrolling_means), sizeof(nrolling_means));
                if (nrolling_means != m_batch_normalize_rolling_means.size()) return false;
                is.read((char *)(m_batch_normalize_rolling_means.data()), nrolling_means * sizeof(float));
            }
            int nrolling_variances = m_batch_normalize_rolling_variances.size();
            if (nrolling_variances > 0)
            {
                is.read((char *)(&nrolling_variances), sizeof(nrolling_variances));
                if (nrolling_variances != m_batch_normalize_rolling_variances.size()) return false;
                is.read((char *)(m_batch_normalize_rolling_variances.data()), nrolling_variances * sizeof(float));
            }
        }
        return true;
    }

    void Layer::SaveWeights(std::ostream &os)
    {
        int biases_size = GetBiasesSize();
        if (biases_size > 0)
        {
            os.write((const char *)(&biases_size), sizeof(biases_size));
            os.write((char *)GetBiasesData(), sizeof(float) * biases_size);
        }

        int weights_size = GetWeightsSize();
        if (weights_size > 0)
        {
            os.write((const char *)(&weights_size), sizeof(weights_size));
            os.write((char *)GetWeightsData(), sizeof(float) * weights_size);
        }

        if (m_batch_normalize)
        {
            int nscales = m_batch_normalize_scales.size();
            if (nscales > 0)
            {
                os.write((const char *)(&nscales), sizeof(nscales));
                os.write((char *)(m_batch_normalize_scales.data()), sizeof(float) * nscales);
            }
            int nrolling_means = m_batch_normalize_rolling_means.size();
            if (nrolling_means > 0)
            {
                os.write((const char *)(&nrolling_means), sizeof(nrolling_means));
                os.write((char *)(m_batch_normalize_rolling_means.data()), sizeof(float) * nrolling_means);
            }
            int nrolling_variances = m_batch_normalize_rolling_variances.size();
            if (nrolling_variances > 0)
            {
                os.write((const char *)(&nrolling_variances), sizeof(nrolling_variances));
                os.write((char *)(m_batch_normalize_rolling_variances.data()), sizeof(float) * nrolling_variances);
            }
        }
    }

    // learning_rate 学习速率
    // momentum 推进速度
    // decay    权重衰减
    // batch    批大小 样本个数
    void Layer::Update(float learning_rate, float momentum, float decay, int batch)
    {
        auto nbiases       = GetBiasesSize();
        auto biases        = GetBiasesData();
        auto biases_deltas = GetBiasesDeltasData();
        if (nbiases > 0)
        {
            axpy_cpu(nbiases, learning_rate / batch, biases_deltas, 1, biases, 1);
            scal_cpu(nbiases, momentum, biases_deltas, 1);
        }

        auto nweights       = GetWeightsSize();
        auto weights        = GetWeightsData();
        auto weights_deltas = GetWeightsDeltasData();
        if (nweights > 0)
        {
            axpy_cpu(nweights, -decay * batch, weights, 1, weights_deltas, 1);
            axpy_cpu(nweights, learning_rate / batch, weights_deltas, 1, weights, 1);
            scal_cpu(nweights, momentum, weights_deltas, 1);
        }

        auto nscales       = m_batch_normalize_scales.size();
        auto scales        = m_batch_normalize_scales.data();
        auto scales_deltas = m_batch_normalize_scales_deltas.data();
        if (nscales > 0)
        {
            axpy_cpu(nscales, learning_rate / batch, scales_deltas, 1, scales, 1);
            scal_cpu(nscales, momentum, scales_deltas, 1);
        }

        DebugUpdate();
    }

    void Layer::DebugDatas(std::ostream *os, float *datas, int width, int height, const std::string &data_name)
    {
        if (os == nullptr) return;
        *os << data_name << ":" << std::endl;

        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < width; i++)
            {
                auto v = datas[j * width + i];
                if (std::isnan(v) || std::isinf(v))
                {
                    *os << std::setw(10) << "nan|inf ";
                }
                else
                {
                    *os << std::setw(10) << std::setprecision(6) << std::fixed << v << " ";
                }
            }
            *os << std::endl;
        }
    }
}