#ifndef __YOLO_LAYER_H__
#define __YOLO_LAYER_H__
#include "Layer.h"

class YoloLayer : public Layer
{
public:
    YoloLayer(Net *net, Layer *prev_layer, Options *options);

    virtual void Forward();
    virtual void Backward();

    virtual std::vector<float> *GetPredicts();
    virtual void PrintPredicts(std::ostream *ofs);

protected:
    void ParseIouLoss(const std::string iou_loss, int &iou_loss_type);
    int GetMaskIndex(int mask);
    int GetPredictClassId(float *out_datas, int class_index, int classes, int stride);

protected:
    inline float *GetAnchorsData() { return m_anchors.data(); }
    inline int GetAnchorsSize() { return m_anchors.size(); }

protected:
    int m_classes;                       // 分类数
    int m_boxes;                         // 单个网格预测框数量

    float m_object_normalizer;           // 对象归一化参数
    float m_iou_normalizer;              // iou归一化参数
    float m_iou_thresh;                  // iou阈值 大于此值为对象否则为背景
    float m_predict_thresh;              // 预测阈值
    std::vector<float> m_anchors;        // 锚点
    std::vector<int> m_masks;            // 锚点索引
    std::unordered_set<int> m_is_deltas; // 是否更新增量
    int m_iou_loss_type;                 // 损失类型
};

#endif