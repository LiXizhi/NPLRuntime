#include "NormalizationLayer.h"

NormalizationLayer::NormalizationLayer(Net *net, Layer *prev_layer, Options *options) : Layer(net, prev_layer, options)
{
    m_name = "normalization";
    m_type = Layer::s_normalization;

    m_normalization_type = s_normalization_default;
    if (m_normalization_type == s_normalization_channel_shared)
    {
        m_alpha = options->GetOption("alpha", 0.0001f);
        m_beta  = options->GetOption("beta", 0.75f);
        m_kappa = options->GetOption("kappa", 1.0f);
        m_size  = options->GetOption("size", 5);
    }

    m_out_width   = m_in_width;
    m_out_height  = m_in_height;
    m_out_filters = m_in_filters;
}

void NormalizationLayer::Resize()
{
    auto batch_out_size = GetBatchOutSize();
    if (batch_out_size == m_outputs.size()) return;

    if (m_normalization_type == s_normalization_channel_shared)
    {
        m_outputs.assign(batch_out_size, 0.f);
        m_deltas.assign(batch_out_size, 0.f);
        m_squared.assign(batch_out_size, 0.f);
        m_norms.assign(batch_out_size, 0.f);
    }
    else if (m_normalization_type == s_normalization_spatial)
    {
        m_outputs.assign(batch_out_size, 0.f);
        m_deltas.assign(batch_out_size, 0.f);
    }
    else
    {
        m_outputs.assign(batch_out_size, 0.f);
        m_deltas.assign(batch_out_size, 0.f);
        m_weights.assign(m_out_filters, 0.f);
        m_weights_deltas.assign(m_out_filters, 0.f);
        m_biases.assign(m_out_filters, 0.f);
        m_biases_deltas.assign(m_out_filters, 0.f);

        m_means.assign(m_batch * m_out_width * m_out_height, 0.f);
        m_variances.assign(m_batch * m_out_width * m_out_height, 0.f);
    }
}

void NormalizationLayer::Forward()
{
    if (m_normalization_type == s_normalization_channel_shared)
    {
        auto w              = m_out_width;
        auto h              = m_out_height;
        auto c              = m_out_filters;
        auto batch_out_size = GetBatchOutSize();
        auto in_datas       = GetInDatas();
        auto out_datas      = GetOutDatas();
        scal_cpu(m_out_filters, 0, m_squared.data(), 1);

        for (int b = 0; b < m_batch; ++b)
        {
            float *squared = m_squared.data() + w * h * c * b;
            float *norms   = m_norms.data() + w * h * c * b;
            float *input   = in_datas + w * h * c * b;
            pow_cpu(w * h * c, 2, input, 1, squared, 1);

            const_cpu(w * h, m_kappa, norms, 1);
            for (int k = 0; k < m_size / 2; ++k)
            {
                axpy_cpu(w * h, m_alpha, squared + w * h * k, 1, norms, 1);
            }

            for (int k = 1; k < c; ++k)
            {
                copy_cpu(w * h, norms + w * h * (k - 1), 1, norms + w * h * k, 1);
                int prev = k - ((m_size - 1) / 2) - 1;
                int next = k + (m_size / 2);
                if (prev >= 0) axpy_cpu(w * h, -m_alpha, squared + w * h * prev, 1, norms + w * h * k, 1);
                if (next < c) axpy_cpu(w * h, m_alpha, squared + w * h * next, 1, norms + w * h * k, 1);
            }
        }
        pow_cpu(w * h * c * m_batch, -m_beta, m_norms.data(), 1, out_datas, 1);
        mul_cpu(w * h * c * m_batch, in_datas, 1, out_datas, 1);
    }
    else if (m_normalization_type == s_normalization_spatial)
    {
        auto in_datas       = GetInDatas();
        auto out_datas      = GetOutDatas();
        auto batch_out_size = GetBatchOutSize();
        copy_cpu(batch_out_size, in_datas, 1, out_datas, 1);

        auto spatial = m_out_width * m_out_height;
        auto filters = m_out_filters;
        auto x       = out_datas;
        for (int b = 0; b < m_batch; ++b)
        {
            for (int i = 0; i < spatial; ++i)
            {
                float sum = 0;
                for (int f = 0; f < filters; ++f)
                {
                    int index = b * filters * spatial + f * spatial + i;
                    sum += x[index] * x[index];
                }
                sum = sqrtf(sum);
                for (int f = 0; f < filters; ++f)
                {
                    int index = b * filters * spatial + f * spatial + i;
                    x[index] /= sum;
                }
            }
        }
    }
    else
    {
        auto eps       = 1e-5f;
        auto in_datas  = GetInDatas();
        auto out_datas = GetOutDatas();
        auto weights   = GetWeightsData();
        auto biases    = GetBiasesData();
        auto spatial   = m_out_width * m_out_height;
        auto filters   = m_out_filters;
        for (int b = 0; b < m_batch; b++)
        {
            for (int i = 0; i < spatial; i++)
            {
                // 计算均值
                float mean = 0.f;
                for (int f = 0; f < filters; ++f)
                {
                    auto index = b * filters * spatial + f * spatial + i;
                    mean += in_datas[index];
                }
                mean = mean / filters;

                // 计算方差
                float variance = 0.f;
                for (int f = 0; f < filters; ++f)
                {
                    auto index = b * filters * spatial + f * spatial + i;
                    auto x     = in_datas[index] - mean;
                    variance   = x * x;
                }
                variance = variance / filters;

                // 归一化
                float r_variance = 1.f / sqrtf(variance + eps);
                for (int f = 0; f < filters; ++f)
                {
                    auto index       = b * filters * spatial + f * spatial + i;
                    auto x           = (in_datas[index] - mean) * r_variance;
                    out_datas[index] = x * weights[f] + biases[f];
                }
                m_means[b * spatial + i]     = mean;
                m_variances[b * spatial + i] = variance;
            }
        }
    }
}

void NormalizationLayer::Backward()
{
    if (m_prev_layer == nullptr) return;

    if (m_normalization_type == s_normalization_channel_shared)
    {
        auto w          = m_out_width;
        auto h          = m_out_height;
        auto c          = m_out_filters;
        auto in_deltas  = GetInDeltas();
        auto out_deltas = GetOutDeltas();
        pow_cpu(w * h * c * m_batch, -m_beta, m_norms.data(), 1, out_deltas, 1);
        mul_cpu(w * h * c * m_batch, in_deltas, 1, out_deltas, 1);
    }
    else if (m_normalization_type == s_normalization_spatial)
    {
        auto batch_out_size = GetBatchOutSize();
        auto in_deltas      = GetInDeltas();
        auto out_deltas     = GetOutDeltas();
        if (out_deltas == nullptr) return;
        axpy_cpu(batch_out_size, 1, in_deltas, 1, out_deltas, 1);
    }
    else
    {
        auto eps      = 1e-5f;
        auto in_datas = GetInDatas();
        auto weights  = GetWeightsData();
        auto spatial  = m_out_width * m_out_height;
        auto filters  = m_out_filters;

        auto dweights   = GetWeightsDeltasData();
        auto dbias      = GetBiasesDeltasData();
        auto in_deltas  = GetInDeltas();
        auto out_deltas = GetOutDeltas();
        for (int b = 0; b < m_batch; b++)
        {
            for (int i = 0; i < spatial; i++)
            {
                auto mean       = m_means[b * spatial + i];
                auto variance   = m_variances[b * spatial + i];
                auto r_variance = 1.0f / sqrtf(variance + eps);

                float dnorm_mean      = 0.0f;
                float dnorm_norm_mean = 0.0f;
                for (int f = 0; f < filters; f++)
                {
                    auto index    = b * filters * spatial + f * spatial + i;
                    float norm_i  = (in_datas[index] - mean) * r_variance; // 归一化输入值
                    float dnorm_i = weights[f] * in_deltas[index];         // 梯度输入值
                    dnorm_mean += dnorm_i;                                 // 梯度均值
                    dnorm_norm_mean += dnorm_i * norm_i;                   // 梯度归一化
                }
                dnorm_mean      = dnorm_mean / filters;
                dnorm_norm_mean = dnorm_norm_mean / filters;

                // now iterate again and accumulate all the gradients
                for (int f = 0; i < filters; f++)
                {
                    auto index    = b * filters * spatial + f * spatial + i; // 数据索引
                    float norm_i  = (in_datas[index] - mean) * r_variance;   // 归一化输入值
                    float dnorm_i = weights[f] * in_deltas[index];           // 梯度输入值
                    // gradient contribution to bias
                    dbias[f] += in_deltas[index]; // out_datas[index] = x * weights[f] + biases[f];  x = norm_i;
                    // gradient contribution to weight
                    dweights[f] += norm_i * in_deltas[index]; // out_datas[index] = x * weights[f] + biases[f];  x = norm_i;
                    // gradient contribution to input
                    float dval = 0.0f;
                    dval += dnorm_i;                  // term 1  dx
                    dval -= dnorm_mean;               // term 2  dx -= mean_dx
                    dval -= norm_i * dnorm_norm_mean; // term 3  dx -= dx * x * x
                    dval *= r_variance;               // final scale dx *= 1 / sqrt(x * x)
                    out_deltas[index] += dval;
                }
            }
        }
    }
}
