#ifndef __DARKNET_LAYER_H__
#define __DARKNET_LAYER_H__

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "utils/activations.h"
#include "utils/blas.h"
#include "utils/gemm.h"
#include "utils/im2col.h"
#include "utils/utils.h"

#include "cores/Options.h"
namespace darknet
{
    class Net;

    class Layer
    {
    public:
        Layer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options);

        virtual void Resize();
        virtual void Forward() {}
        virtual void Backward() {}
        virtual float Loss() { return m_cost; }
        virtual void Update(float learning_rate, float momentum, float decay, int batch);

        virtual bool LoadWeights(std::istream &is);
        virtual void SaveWeights(std::ostream &os);

        virtual std::vector<float> *GetPredicts() { return &m_outputs; }
        virtual void PrintPredicts(std::ostream *os) {}

        virtual int GetWorkspaceSize() { return 0; } // 计算所需缓存的空间大小

    public:
        int GetLayerIndex(Layer *layer);
        Layer *GetIndexLayer(int index);
        int GetNetWidth();
        int GetNetHeight();
        int GetNetChannels();
        int GetMaxTruthsSize();
        int GetMaxTruths();
        int GetTruthsSize();
        float *GetInTruths();
        float *GetInDatas();
        float *GetOutDatas();
        float *GetInDeltas();
        float *GetOutDeltas();
        float *GetWorkspace();

    protected:
        bool CheckNanAndInf();
        bool IsExistNanAndInf(float *datas, int size);
        std::ostream *GetInputOS(int input_index);

    protected:
        void ForwardAddBiases(float *im_datas, float *biases, int batch, int filter, int im_size);
        void BackwardAddBiases(float *biases_deltas, float *deltas, int batch, int filter, int im_size);
        void ForwardScale(float *im_datas, float *scales, int batch, int filter, int im_size);
        void BackwardScale(float *x_normalize, float *deltas, int batch, int filter, int im_size, float *scales_deltas);
        void InitBatchNormalize();
        void ForwardBatchNormalizeLayer();
        void BackwardBatchNormalizeLayer();

    protected:
        virtual void DebugForward() {}
        virtual void DebugBackward() {}
        virtual void DebugUpdate() {}

    protected:
        void DebugDatas(std::ostream *os, float *datas, int size, const std::string &data_name) { DebugDatas(os, datas, size, 1, data_name); }
        void DebugDatas(std::ostream *os, float *datas, int width, int height, const std::string &data_name);

    public:
        inline std::shared_ptr<Options> GetOptions() { return m_options; }
        inline Net *GetNet() { return m_net; }
        inline const std::string &GetName() { return m_name; }
        inline int GetType() { return m_type; }
        inline float *GetWeightsData() { return m_weights.data(); }
        inline int GetWeightsSize() { return m_weights.size(); }
        inline float *GetWeightsDeltasData() { return m_weights_deltas.data(); }
        inline int GetWeightsDeltasSize() { return m_weights_deltas.size(); }
        inline float *GetBiasesData() { return m_biases.data(); }
        inline int GetBiasesSize() { return m_biases.size(); }
        inline float *GetBiasesDeltasData() { return m_biases_deltas.data(); }
        inline int GetBiasesDeltasSize() { return m_biases_deltas.size(); }
        inline float *GetLossesData() { return m_losses.data(); }
        inline int GetLossesSize() { return m_losses.size(); }
        inline float GetCost() { return m_cost; }
        inline void SetBatch(int batch) { m_batch = batch; }
        inline int GetBatch() { return m_batch; }
        inline void SetInWidth(int in_width) { m_in_width = in_width; }
        inline void SetInHeight(int in_height) { m_in_height = in_height; }
        inline void SetInFilters(int in_filters) { m_in_filters = in_filters; }

        inline int GetInWidth() { return m_in_width; }
        inline int GetInHeight() { return m_in_height; }
        inline int GetInFilters() { return m_in_filters; }
        inline int GetInSize() { return m_in_width * m_in_height * m_in_filters; }
        inline int GetBatchInSize() { return m_batch * m_in_width * m_in_height * m_in_filters; }
        inline int GetOutWidth() { return m_out_width; }
        inline int GetOutHeight() { return m_out_height; }
        inline int GetOutFilters() { return m_out_filters; }
        inline int GetOutSize() { return m_out_width * m_out_height * m_out_filters; }
        inline int GetBatchOutSize() { return m_batch * m_out_width * m_out_height * m_out_filters; }

        inline int GetInImageSize() { return m_in_width * m_in_height; }
        inline int GetInImageCount() { return m_in_filters; }
        inline int GetOutImageSize() { return m_out_width * m_out_height; }
        inline int GetOutImageCount() { return m_out_filters; }

        inline void SetPrevLayer(Layer *prev_layer) { m_prev_layer = prev_layer; }
        inline Layer *GetPrevLayer() { return m_prev_layer; }
        inline void SetNextLayer(Layer *next_layer) { m_next_layer = next_layer; }
        inline Layer *GetNextLayer() { return m_next_layer; }

        inline void SetBatchNormalize(bool batch_normalize) { m_batch_normalize = batch_normalize; }
        inline bool IsBatchNormalize() { return m_batch_normalize && m_option_batch_normalize; }

    protected:
        inline bool IsNanOrInf(float data) { return std::isnan(data) || std::isinf(data); }

    protected:
        std::string m_name;
        int m_type;
        int m_batch;
        int m_in_width;
        int m_in_height;
        int m_in_filters;

        int m_out_width;
        int m_out_height;
        int m_out_filters;

        std::vector<float> m_outputs;        // 输出特征图数据
        std::vector<float> m_deltas;         // 增量
        std::vector<float> m_losses;         // 损失
        std::vector<float> m_weights;        // 权重 size * size * in_filters * out_filters
        std::vector<float> m_weights_deltas; // 权重增量
        std::vector<float> m_biases;         // 偏执参数
        std::vector<float> m_biases_deltas;  // 偏置参数增量

        // batch normalize
        bool m_batch_normalize;
        bool m_option_batch_normalize;
        std::vector<float> m_batch_normalize_scales;
        std::vector<float> m_batch_normalize_scales_deltas;
        std::vector<float> m_batch_normalize_means;
        std::vector<float> m_batch_normalize_means_deltas;
        std::vector<float> m_batch_normalize_variances;
        std::vector<float> m_batch_normalize_variances_deltas;
        std::vector<float> m_batch_normalize_rolling_means;
        std::vector<float> m_batch_normalize_rolling_variances;
        std::vector<float> m_batch_normalize_x;
        std::vector<float> m_batch_normalize_x_normalize;

        float m_cost; // 损失 代价

        std::shared_ptr<Options> m_options;
        Net *m_net;
        Layer *m_prev_layer;
        Layer *m_next_layer;

    public:
        static const int s_none          = 0;
        static const int s_convolutional = 1;
        static const int s_connected     = 2;
        static const int s_softmax       = 3;
        static const int s_yolo          = 4;
        static const int s_maxpool       = 5;
        static const int s_route         = 6;
        static const int s_shortcut      = 7;
        static const int s_upsample      = 8;
        static const int s_dropout       = 9;
        static const int s_normalization = 10;
        static const int s_attention     = 11;
        static const int s_transformer   = 12;

        static int GetLayerTypeByName(const std::string &layer_name)
        {
            if (layer_name == "conv" || layer_name == "convolutional") return s_convolutional;
            if (layer_name == "conn" || layer_name == "connected") return s_connected;
            if (layer_name == "soft" || layer_name == "softmax") return s_softmax;
            if (layer_name == "yolo") return s_yolo;
            if (layer_name == "max" || layer_name == "maxpool") return s_maxpool;
            if (layer_name == "route") return s_route;
            if (layer_name == "shortcut") return s_shortcut;
            if (layer_name == "upsample") return s_upsample;
            if (layer_name == "dropout") return s_dropout;
            if (layer_name == "normalization") return s_normalization;
            if (layer_name == "attention") return s_attention;
            if (layer_name == "transformer") return s_transformer;
            return s_none;
            // if (strcmp(type, "[shortcut]")==0) return SHORTCUT;
            // if (strcmp(type, "[scale_channels]") == 0) return SCALE_CHANNELS;
            // if (strcmp(type, "[sam]") == 0) return SAM;
            // if (strcmp(type, "[crop]")==0) return CROP;
            // if (strcmp(type, "[cost]")==0) return COST;
            // if (strcmp(type, "[detection]")==0) return DETECTION;
            // if (strcmp(type, "[region]")==0) return REGION;
            // if (strcmp(type, "[yolo]") == 0) return YOLO;
            // if (strcmp(type, "[Gaussian_yolo]") == 0) return GAUSSIAN_YOLO;
            // if (strcmp(type, "[local]")==0) return LOCAL;
            // if (strcmp(type, "[conv]")==0
            //         || strcmp(type, "[convolutional]")==0) return CONVOLUTIONAL;
            // if (strcmp(type, "[activation]")==0) return ACTIVE;
            // if (strcmp(type, "[net]")==0
            //         || strcmp(type, "[network]")==0) return NETWORK;
            // if (strcmp(type, "[crnn]")==0) return CRNN;
            // if (strcmp(type, "[gru]")==0) return GRU;
            // if (strcmp(type, "[lstm]")==0) return LSTM;
            // if (strcmp(type, "[conv_lstm]") == 0) return CONV_LSTM;
            // if (strcmp(type, "[history]") == 0) return HISTORY;
            // if (strcmp(type, "[rnn]")==0) return RNN;
            // if (strcmp(type, "[conn]")==0
            //         || strcmp(type, "[connected]")==0) return CONNECTED;
            // if (strcmp(type, "[max]")==0
            //         || strcmp(type, "[maxpool]")==0) return MAXPOOL;
            // if (strcmp(type, "[local_avg]") == 0
            //     || strcmp(type, "[local_avgpool]") == 0) return LOCAL_AVGPOOL;
            // if (strcmp(type, "[reorg3d]")==0) return REORG;
            // if (strcmp(type, "[reorg]") == 0) return REORG_OLD;
            // if (strcmp(type, "[avg]")==0
            //         || strcmp(type, "[avgpool]")==0) return AVGPOOL;
            // if (strcmp(type, "[dropout]")==0) return DROPOUT;
            // if (strcmp(type, "[lrn]")==0
            //         || strcmp(type, "[normalization]")==0) return NORMALIZATION;
            // if (strcmp(type, "[batchnorm]")==0) return BATCHNORM;
            // if (strcmp(type, "[soft]")==0
            //         || strcmp(type, "[softmax]")==0) return SOFTMAX;
            // if (strcmp(type, "[contrastive]") == 0) return CONTRASTIVE;
            // if (strcmp(type, "[route]")==0) return ROUTE;
            // if (strcmp(type, "[upsample]") == 0) return UPSAMPLE;
            // if (strcmp(type, "[empty]") == 0
            //     || strcmp(type, "[silence]") == 0) return EMPTY;
            // if (strcmp(type, "[implicit]") == 0) return IMPLICIT;
            // return BLANK;
        }
    };
}
#endif