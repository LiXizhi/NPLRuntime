#include "YoloLayer.h"

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <sstream>
namespace darknet
{
    static const int s_iou_loss_iou  = 0;
    static const int s_iou_loss_giou = 1;
    static const int s_iou_loss_mse  = 2;
    static const int s_iou_loss_diou = 3;
    static const int s_iou_loss_ciou = 4;
    static inline float fix_nan_inf(float val) { return (std::isnan(val) || std::isinf(val)) ? 0.f : val; }
    static inline float clip_value(float val, const float max_val) { return val > max_val ? max_val : (val < -max_val ? -max_val : val); }

    // batch * m_boxes * (4 + 1 + m_classes) * m_out_width * m_out_height 当前测试数据起始地址
    // box * (4 + 1 + m_classes) * m_out_width * m_out_height 当前预测边框盒起始地址 x, y, w, h, objectness m_classes
    // entry * m_width * m_height 起始地址 x, y, w, h, objectness m_classes
    // y * width + x 单元格地址
    static inline int GetEntryIndex(int batch, int box, int entry, int x, int y, int width, int height, int boxes, int classes) { return ((batch * boxes + box) * (4 + 1 + classes) + entry) * width * height + y * width + x; }
    static inline int GetGridIndex(int batch, int box, int x, int y, int width, int height, int boxes) { return batch * boxes * width * height + box * width * height + y * width + x; }
    static inline int GetTruthClassId(float *truths, int batch, int index, int truths_size, int batch_truths_size) { return *(truths + batch * batch_truths_size + index * truths_size + 4); }

    typedef struct Box
    {
        float m_x;
        float m_y;
        float m_w;
        float m_h;
    } Box;

    // box.h
    typedef struct BoxAbs
    {
        float m_left;
        float m_right;
        float m_top;
        float m_bottom;
    } BoxAbs;

    typedef struct DeltaXrep
    {
        float m_dt;
        float m_db;
        float m_dl;
        float m_dr;
    } DeltaXrep;

    typedef struct Ious
    {
        float m_iou;
        float m_giou;
        float m_diou;
        float m_ciou;
        DeltaXrep m_dx_iou;
        DeltaXrep m_dx_giou;
    } Ious;

    typedef struct YoloTrainArgs
    {
        // input ouput args
        Layer *m_layer;
        std::ostream *m_os;
        std::unordered_set<int> *m_is_deltas;
        float *m_datas;
        float *m_deltas;
        float *m_truths;
        float *m_anchors;
        int *m_masks;
        int m_layer_width;
        int m_layer_height;
        int m_net_width;
        int m_net_height;
        int m_stride;
        int m_classes;
        int m_boxes;
        int m_max_truths;
        int m_truths_size;
        int m_max_truths_size;

        // config args
        float m_iou_normalizer;
        float m_object_normalizer;
        int m_iou_loss_type;

        // calculate args
        float m_total_iou;
        float m_total_giou;
        float m_total_diou;
        float m_total_ciou;
        float m_total_iou_loss;
        float m_total_giou_loss;
        float m_total_diou_loss;
        float m_total_ciou_loss;
        float m_total_classify;
        float m_total_objectness;
        int m_delta_count;

        // train args
        int m_batch;
        int m_box;
        int m_x;
        int m_y;
        int m_truth_index;
    } YoloTrainArgs;

    float BoxIouKind(const Box &a, const Box &b, int iou_kind);
    float BoxIou(const Box &a, const Box &b);
    float BoxGIou(const Box &a, const Box &b);
    float BoxDIou(const Box &a, const Box &b);
    float BoxCIou(const Box &a, const Box &b);
    float BoxIntersection(const Box &a, const Box &b);
    float BoxUnion(const Box &a, const Box &b);
    float BoxOverlap(float x1, float w1, float x2, float w2);
    BoxAbs BoxC(const Box &a, const Box &b);
    BoxAbs ToBoxAbs(const Box &a);
    DeltaXrep DeltaXrepBoxIou(const Box &pred, const Box &truth, int iou_loss);
    Box GetPredictBox(float *out_datas, float *anchors, int mask, int index, int i, int j, int lw, int lh, int w, int h, int stride);
    Box GetTruthBox(float *truths, int batch, int index, int truth_size, int batch_truth_size);
    void DeltaClass(float *out_datas, float *deltas, int index, int class_id, int classes, int stride, float *avg_cat, std::ostream *os);
    Ious DeltaBox(Box truth, float *out_datas, float *anchors, int mask, int index, int i, int j, int lw, int lh, int w, int h, float *deltas, int stride, float iou_normalizer, int iou_loss, std::ostream *os);
    void DeltaClassAndBox(YoloTrainArgs *args);

    YoloLayer::YoloLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options) : Layer(net, prev_layer, options)
    {
        m_name = "yolo";
        m_type = s_yolo;

        options->ParseOption("mask", "", m_masks);
        options->ParseOption("anchors", "", m_anchors);
        ParseIouLoss(options->GetOption("iou_loss", "mse"), m_iou_loss_type);

        m_classes           = options->GetOption("classes", 20);
        m_iou_thresh        = options->GetOption("iou_thresh", 0.7f);
        m_iou_normalizer    = options->GetOption("iou_normalizer", 1.0f);
        m_object_normalizer = options->GetOption("obj_normalizer", 1.0f);
        m_predict_thresh    = options->GetOption("predict_thresh", 0.7f);

        m_boxes       = m_masks.size();
        m_out_width   = m_in_width;
        m_out_height  = m_in_height;
        m_out_filters = m_boxes * (m_classes + 4 + 1); // 每个格子预测m_boxes个边界框  每个框包含 5 个参数  (x, y, w, h, objectness) 预测 classes 个类概率

        m_outputs.assign(GetBatchOutSize(), 0.f);
        m_deltas.assign(GetBatchOutSize(), 0.f);
    }

    void YoloLayer::ParseIouLoss(const std::string iou_loss, int &iou_loss_type)
    {
        if (iou_loss == "mse")
            iou_loss_type = s_iou_loss_mse;
        else if (iou_loss == "giou")
            iou_loss_type = s_iou_loss_giou;
        else if (iou_loss == "diou")
            iou_loss_type = s_iou_loss_diou;
        else if (iou_loss == "ciou")
            iou_loss_type = s_iou_loss_ciou;
        else
            iou_loss_type = s_iou_loss_iou;
    }

    int YoloLayer::GetMaskIndex(int mask)
    {
        auto size = m_masks.size();
        for (int i = 0; i < size; i++)
        {
            if (mask == m_masks[i]) return i;
        }
        return -1;
    }

    int YoloLayer::GetPredictClassId(float *out_datas, int class_index, int classes, int stride)
    {
        float max_prob = 0;
        float max_id   = -1;
        for (int i = 0; i < classes; ++i)
        {
            auto class_prob = fix_nan_inf(out_datas[class_index + stride * i]);
            if (max_prob < class_prob)
            {
                max_prob = class_prob;
                max_id   = i;
            }
        }
        return max_id;
    }

    void YoloLayer::Forward()
    {
        auto in_datas            = GetInDatas();
        auto out_datas           = GetOutDatas();
        auto im_size             = GetOutImageSize();
        auto logistic_activation = get_activation_by_name("logistic");
        auto max_truths          = GetMaxTruths();
        auto truths_size         = GetTruthsSize();
        auto max_truths_size     = GetMaxTruthsSize();
        memcpy(out_datas, in_datas, GetBatchOutSize() * sizeof(float));

        for (int batch = 0; batch < m_batch; batch++)
        {
            for (int box = 0; box < m_boxes; box++)
            {
                int bbox_index = GetEntryIndex(batch, box, 0, 0, 0, m_out_width, m_out_height, m_boxes, m_classes); // x, y, w, h
                int obj_index  = GetEntryIndex(batch, box, 4, 0, 0, m_out_width, m_out_height, m_boxes, m_classes); // objectness
                activate_array(out_datas + bbox_index, 2 * im_size, logistic_activation);                           // x, y, logistic  w, h 不激活
                activate_array(out_datas + obj_index, (1 + m_classes) * im_size, logistic_activation);              // objectness + classes
            }
        }

        if (GetInTruths() == nullptr) return;

        // 初始化相关数据
        auto deltas       = GetInDeltas();
        auto anchors      = GetAnchorsData();
        auto anchors_size = GetAnchorsSize();
        auto net_width    = GetNetWidth();
        auto net_height   = GetNetHeight();
        auto in_truths    = GetInTruths();
        const int stride  = m_out_width * m_out_height;
        m_is_deltas.clear();
        // memset(deltas, 0, GetBatchOutSize() * sizeof(float));

        YoloTrainArgs args       = {0};
        args.m_layer             = this;
        args.m_is_deltas         = &m_is_deltas;
        args.m_datas             = out_datas;
        args.m_deltas            = deltas;
        args.m_truths            = in_truths;
        args.m_anchors           = anchors;
        args.m_masks             = m_masks.data();
        args.m_layer_width       = m_out_width;
        args.m_layer_height      = m_out_height;
        args.m_net_width         = net_width;
        args.m_net_height        = net_height;
        args.m_stride            = stride;
        args.m_classes           = m_classes;
        args.m_boxes             = m_boxes;
        args.m_max_truths        = max_truths;
        args.m_truths_size       = truths_size;
        args.m_max_truths_size   = max_truths_size;
        args.m_iou_normalizer    = m_iou_normalizer;
        args.m_object_normalizer = m_object_normalizer;
        args.m_iou_loss_type     = m_iou_loss_type;

        for (int batch = 0; batch < m_batch; batch++)
        {
            args.m_batch = batch;
            args.m_os    = this->GetInputOS(batch);
            // 所有单元格预测边框并进行增量计算
            for (int j = 0; j < m_out_height; j++)
            {
                for (int i = 0; i < m_out_width; i++)
                {
                    for (int box = 0; box < m_boxes; box++)
                    {
                        args.m_x   = i;
                        args.m_y   = j;
                        args.m_box = box;

                        const int class_index      = GetEntryIndex(batch, box, 5, i, j, m_out_width, m_out_height, m_boxes, m_classes); // (i, j)格子的分类起始地址
                        const int obj_index        = GetEntryIndex(batch, box, 4, i, j, m_out_width, m_out_height, m_boxes, m_classes); // (i, j)格子的对象起始地址
                        const int box_index        = GetEntryIndex(batch, box, 0, i, j, m_out_width, m_out_height, m_boxes, m_classes); // (i, j)格子的边框盒起始地址
                        const float objectness     = fix_nan_inf(out_datas[obj_index]);                                                 // 是否为目标物体
                        const int predict_class_id = GetPredictClassId(out_datas, class_index, m_classes, stride);

                        // 获取预测盒子
                        auto predict         = GetPredictBox(out_datas, anchors, m_masks[box], box_index, i, j, m_out_width, m_out_height, net_width, net_height, stride);
                        float best_iou       = 0.f;
                        int best_truth_index = 0;

                        for (int truth_index = 0; truth_index < max_truths; truth_index++)
                        {
                            auto truth = GetTruthBox(in_truths, batch, truth_index, truths_size, max_truths_size);
                            // 全为0表示结束
                            if (truth.m_x == 0 && truth.m_y == 0 && truth.m_w == 0 && truth.m_h == 0) break;
                            // 数据验证应放在数据加载处
                            if (truth.m_w <= 0 || truth.m_h <= 0 || truth.m_x < 0 || truth.m_y < 0 || truth.m_x > 1 || truth.m_y > 1)
                            {
                                printf("\n Wrong label: truth.x = %f, truth.y = %f, truth.w = %f, truth.h = %f \n", truth.m_x, truth.m_y, truth.m_w, truth.m_h);
                                continue;
                            }

                            float iou = BoxIou(predict, truth);
                            if (iou > best_iou)
                            {
                                best_iou         = iou;
                                best_truth_index = truth_index;
                            }
                        }

                        // 最好iou大于对象忽略阈值则进行有对象阈值计算或置0忽略  此逻辑提高objectness的准确率  控制m_iou_thresh值, 否则objectness概率会全部趋近于0
                        if (best_iou > m_iou_thresh)
                        {
                            int truth_class_id = GetTruthClassId(in_truths, batch, best_truth_index, truths_size, max_truths_size);
                            if (truth_class_id == predict_class_id)
                            {
                                deltas[obj_index] = m_object_normalizer * (best_iou - objectness);
                            }
                            else
                            {
                                deltas[obj_index] = 0.f;
                            }
                            DeltaClass(out_datas, deltas, class_index, truth_class_id, m_classes, stride, nullptr, nullptr);
                        }
                        else
                        {
                            deltas[obj_index] = m_object_normalizer * (0 - objectness); // 无对象概率损失 无对象真实值为0 有对象真实值为1
                        }
                    }
                }
            }
            // 真实值强制预测与增量计算
            for (int truth_index = 0; truth_index < max_truths; truth_index++)
            {
                auto truth = GetTruthBox(in_truths, batch, truth_index, truths_size, max_truths_size);
                // 全为0表示结束
                if (truth.m_x == 0 && truth.m_y == 0 && truth.m_w == 0 && truth.m_h == 0) break;
                // 数据验证应放在数据加载处
                if (truth.m_w <= 0 || truth.m_h <= 0 || truth.m_x < 0 || truth.m_y < 0 || truth.m_x > 1 || truth.m_y > 1)
                {
                    printf("\n Wrong label: truth.x = %f, truth.y = %f, truth.w = %f, truth.h = %f\n", truth.m_x, truth.m_y, truth.m_w, truth.m_h);
                    continue;
                }

                args.m_x           = truth.m_x * m_out_width;
                args.m_y           = truth.m_y * m_out_height;
                args.m_truth_index = truth_index;

                // 找出最合适的预测框
                float best_iou   = 0;
                int best_mask    = 0;
                auto truth_shift = truth;
                int max_mask     = anchors_size / 2;
                truth_shift.m_x  = 0;
                truth_shift.m_y  = 0;
                for (int mask = 0; mask < max_mask; ++mask)
                {
                    Box pred  = {0};
                    pred.m_w  = anchors[2 * mask] / net_width;
                    pred.m_h  = anchors[2 * mask + 1] / net_height;
                    float iou = BoxIou(pred, truth_shift);
                    if (iou > best_iou)
                    {
                        best_iou  = iou;
                        best_mask = mask;
                    }
                }

                // 判断预测框是否在mask中, 即是否是本层预测
                int box = GetMaskIndex(best_mask);
                if (box >= 0)
                {
                    args.m_box = box;
                    DeltaClassAndBox(&args);
                }
            }
        }

        if (args.m_delta_count == 0) args.m_delta_count = 1;
        m_cost = std::pow(mag_array(deltas, GetBatchOutSize()), 2) / m_batch;
        std::cout << "v3 iou loss type: " << (m_iou_loss_type == s_iou_loss_mse ? "mse" : (m_iou_loss_type == s_iou_loss_giou ? "giou" : "iou")) << ", ";
        std::cout << "objectness: " << args.m_total_objectness / args.m_delta_count << ", ";
        std::cout << "classify: " << args.m_total_classify / args.m_delta_count << ", ";
        std::cout << "avg iou: " << args.m_total_iou / args.m_delta_count << ", ";
        std::cout << "total loss = " << m_cost << std::endl;
    }

    void YoloLayer::Backward()
    {
        auto prev_layer = GetPrevLayer();
        if (prev_layer == nullptr) return;
        auto logistic_activation = get_activation_by_name("logistic");
        auto out_datas           = GetOutDatas();
        auto deltas              = GetInDeltas();
        auto im_size             = GetOutImageSize();
        for (int batch = 0; batch < m_batch; batch++)
        {
            for (int box = 0; box < m_boxes; box++)
            {
                int bbox_index = GetEntryIndex(batch, box, 0, 0, 0, m_out_width, m_out_height, m_boxes, m_classes);         // x, y, w, h
                int obj_index  = GetEntryIndex(batch, box, 4, 0, 0, m_out_width, m_out_height, m_boxes, m_classes);         // objectness
                gradient_array(out_datas + bbox_index, 2 * im_size, logistic_activation, deltas + bbox_index);              // x, y, logistic  w, h 不激活
                gradient_array(out_datas + obj_index, (1 + m_classes) * im_size, logistic_activation, deltas + bbox_index); // objectness + classes
            }
        }
        axpy_cpu(GetBatchInSize(), 1, GetInDeltas(), 1, prev_layer->GetInDeltas(), 1);
    }

    void YoloLayer::PrintPredicts(std::ostream *os)
    {
        if (os == nullptr) return;

        auto max_truths      = GetMaxTruths();
        auto truths_size     = GetTruthsSize();
        auto max_truths_size = GetMaxTruthsSize();
        auto out_datas       = GetOutDatas();
        auto in_truths       = GetInTruths();
        auto stride          = m_out_width * m_out_height;
        for (int batch = 0; batch < m_batch; batch++)
        {
            *os << "data index = " << batch << std::endl;

            // 打印真实数据
            *os << "truths => " << std::endl;
            for (int truth_index = 0; truth_index < max_truths; truth_index++)
            {
                auto truth   = GetTruthBox(in_truths, batch, truth_index, truths_size, max_truths_size);
                auto classid = GetTruthClassId(in_truths, batch, truth_index, truths_size, max_truths_size);
                int grid_x   = truth.m_x * m_out_width;
                int grid_y   = truth.m_y * m_out_height;
                if (truth.m_x == 0 && truth.m_y == 0 && truth.m_w == 0 && truth.m_h == 0) break; // 全为0表示结束
                *os << "truth index = " << truth_index << " grid_x = " << grid_x << " grid_y = " << grid_y << std::endl;
                *os << "x = " << truth.m_x << " y = " << truth.m_y << " w = " << truth.m_w << " h = " << truth.m_h << " classid = " << classid << std::endl;
                float best_iou = 0;
                int best_mask  = 0;
                for (int mask = 0; mask < m_masks.size(); ++mask)
                {
                    auto box_index = GetEntryIndex(batch, mask, 0, grid_x, grid_y, m_out_width, m_out_height, m_boxes, m_classes);
                    auto predict   = GetPredictBox(out_datas, GetAnchorsData(), m_masks[mask], box_index, grid_x, grid_y, m_out_width, m_out_height, GetNetWidth(), GetNetHeight(), m_out_width * m_out_height);
                    auto iou       = fix_nan_inf(BoxIou(predict, truth));
                    if (iou > best_iou)
                    {
                        best_iou  = iou;
                        best_mask = mask;
                    }
                }
                auto box_index = GetEntryIndex(batch, best_mask, 0, grid_x, grid_y, m_out_width, m_out_height, m_boxes, m_classes);
                auto obj_index = GetEntryIndex(batch, best_mask, 4, grid_x, grid_y, m_out_width, m_out_height, m_boxes, m_classes);
                auto predict   = GetPredictBox(out_datas, GetAnchorsData(), m_masks[best_mask], box_index, grid_x, grid_y, m_out_width, m_out_height, GetNetWidth(), GetNetHeight(), m_out_width * m_out_height);
                *os << "best iou predict: mask = " << best_mask << " iou = " << best_iou << " obj = " << out_datas[obj_index] << std::endl;
                *os << "x = " << predict.m_x << " y = " << predict.m_y << " w = " << predict.m_w << " h = " << predict.m_h << std::endl;
            }

            *os << "predicts => " << std::endl;
            float max_objectness  = 0;
            int count             = 0;
            float max_class_value = 0.f;
            int max_class_index   = 0;
            for (int i = 0; i < stride; ++i)
            {
                int row = i / m_out_width;
                int col = i % m_out_width;
                for (int n = 0; n < m_boxes; ++n)
                {
                    int obj_index    = GetEntryIndex(batch, n, 4, col, row, m_out_width, m_out_height, m_boxes, m_classes);
                    float objectness = fix_nan_inf(out_datas[obj_index]);
                    if (max_objectness < objectness) max_objectness = objectness;
                    if (objectness <= m_predict_thresh) continue;
                    int box_index   = GetEntryIndex(batch, n, 0, col, row, m_out_width, m_out_height, m_boxes, m_classes);
                    int class_index = GetEntryIndex(batch, n, 5, col, row, m_out_width, m_out_height, m_boxes, m_classes);
                    for (int c = 0; c < m_classes; ++c)
                    {
                        auto class_value = fix_nan_inf(out_datas[class_index + stride * c]);
                        if (class_value > max_class_value)
                        {
                            max_class_value = class_value;
                            max_class_index = c;
                        }
                    }
                    auto bbox = GetPredictBox(out_datas, GetAnchorsData(), m_masks[n], box_index, col, row, m_out_width, m_out_height, GetNetWidth(), GetNetHeight(), m_out_width * m_out_height);
                    *os << "id = " << count << " grid_x = " << col << " grid_y = " << row << " objectness = " << objectness << std::endl;
                    *os << "x = " << bbox.m_x << " y = " << bbox.m_y << " w = " << bbox.m_w << " h = " << bbox.m_h << " classid = " << max_class_index << " class prob = " << max_class_value << std::endl;
                    count++;
                }
            }
            if (count == 0) *os << "max_objectness = " << max_objectness << std::endl;
        }
    }

    std::vector<float> *YoloLayer::GetPredicts()
    {
        static std::vector<float> s_predicts;
        s_predicts.clear();

        auto out_datas       = GetOutDatas();
        auto out_size        = m_out_width * m_out_height;
        float max_objectness = 0;
        int count            = 0;
        for (int i = 0; i < out_size; ++i)
        {
            int row = i / m_out_width;
            int col = i % m_out_width;
            for (int n = 0; n < m_boxes; ++n)
            {
                int obj_index    = GetEntryIndex(0, n, 4, col, row, m_out_width, m_out_height, m_boxes, m_classes);
                float objectness = fix_nan_inf(out_datas[obj_index]);
                if (max_objectness < objectness) max_objectness = objectness;
                if (objectness <= m_predict_thresh) continue;
                int box_index = GetEntryIndex(0, n, 0, col, row, m_out_width, m_out_height, m_boxes, m_classes);
                auto bbox     = GetPredictBox(out_datas, GetAnchorsData(), m_masks[n], box_index, col, row, m_out_width, m_out_height, GetNetWidth(), GetNetHeight(), m_out_width * m_out_height);
                s_predicts.push_back(bbox.m_x);
                s_predicts.push_back(bbox.m_y);
                s_predicts.push_back(bbox.m_w);
                s_predicts.push_back(bbox.m_h);
                std::cout << "id = " << count << " grid_x = " << col << " grid_y = " << row << " x = " << bbox.m_x << " y = " << bbox.m_y << " w = " << bbox.m_w << " h = " << bbox.m_h << " objectness =" << objectness << std::endl;
                count++;

                int class_index       = GetEntryIndex(0, n, 4 + 1, col, row, m_out_width, m_out_height, m_boxes, m_classes);
                int stride            = m_out_width * m_out_height;
                float max_class_value = 0.f;
                int max_class_index   = 0;
                for (int c = 0; c < m_classes; ++c)
                {
                    auto class_value = fix_nan_inf(out_datas[class_index + stride * c]);
                    if (class_value > max_class_value)
                    {
                        max_class_value = class_value;
                        max_class_index = c;
                    }
                }
                s_predicts.push_back(max_class_index);
                s_predicts.push_back(max_class_value);
            }
        }
        if (count == 0)
        {
            std::cout << "max_objectness = " << max_objectness << std::endl;
        }
        return &s_predicts;
    }

    Box GetPredictBox(float *out_datas, float *anchors, int mask, int index, int i, int j, int lw, int lh, int w, int h, int stride)
    {
        Box b = {0};
        b.m_x = (i + out_datas[index + 0 * stride]) / lw;
        b.m_y = (j + out_datas[index + 1 * stride]) / lh;
        b.m_w = std::exp(out_datas[index + 2 * stride]) * anchors[2 * mask] / w;
        b.m_h = std::exp(out_datas[index + 3 * stride]) * anchors[2 * mask + 1] / h;
        return b;
    }

    Box GetTruthBox(float *truths, int batch, int index, int truth_size, int batch_truth_size)
    {
        Box b  = {0};
        truths = truths + batch * batch_truth_size + index * truth_size;
        b.m_x  = *truths++;
        b.m_y  = *truths++;
        b.m_w  = *truths++;
        b.m_h  = *truths++;
        return b;
    }

    void DeltaClass(float *out_datas, float *deltas, int index, int class_id, int classes, int stride, float *avg_cat, std::ostream *os)
    {
        for (int n = 0; n < classes; ++n)
        {
            auto class_data            = fix_nan_inf(out_datas[index + stride * n]);
            deltas[index + stride * n] = (n == class_id ? 1 : 0) - class_data;
            if (n == class_id && avg_cat) *avg_cat += out_datas[index + stride * n];

            if (os != nullptr && n == class_id)
            {
                *os << "class_id = " << class_id << " predict = " << class_data << " delta = " << deltas[index + stride * n] << std::endl;
            }
        }
    }

    Ious DeltaBox(Box truth, float *out_datas, float *anchors, int mask, int index, int i, int j, int lw, int lh, int w, int h, float *deltas, int stride, float iou_normalizer, int iou_loss, std::ostream *os)
    {
        Ious all_ious   = {0};
        Box predict     = GetPredictBox(out_datas, anchors, mask, index, i, j, lw, lh, w, h, stride);
        all_ious.m_iou  = fix_nan_inf(BoxIou(predict, truth));
        all_ious.m_giou = fix_nan_inf(BoxGIou(predict, truth));
        all_ious.m_diou = fix_nan_inf(BoxDIou(predict, truth));
        all_ious.m_ciou = fix_nan_inf(BoxCIou(predict, truth));

        if (os != nullptr)
        {
            *os << "mask = " << mask << " x = " << i << " y = " << j << " lw = " << lw << " lh = " << lh << " w = " << w << " h = " << h << std::endl;
            *os << "predict box: x " << predict.m_x << " y " << predict.m_y << " w " << predict.m_w << " h " << predict.m_h << std::endl;
            *os << "truth box: x " << truth.m_x << " y " << truth.m_y << " w " << truth.m_w << " h " << truth.m_h << std::endl;
        }

        // avoid nan in dx_box_iou
        if (predict.m_w == 0) { predict.m_w = 1.0; }
        if (predict.m_h == 0) { predict.m_h = 1.0; }
        if (iou_loss == s_iou_loss_mse) // old loss
        {
            float scale = 2 - truth.m_w * truth.m_h;
            float tx    = (truth.m_x * lw - i);
            float ty    = (truth.m_y * lh - j);
            float tw    = std::log(truth.m_w * w / anchors[2 * mask]);
            float th    = std::log(truth.m_h * h / anchors[2 * mask + 1]);

            deltas[index + 0 * stride] = fix_nan_inf(scale * (tx - out_datas[index + 0 * stride]) * iou_normalizer);
            deltas[index + 1 * stride] = fix_nan_inf(scale * (ty - out_datas[index + 1 * stride]) * iou_normalizer);
            deltas[index + 2 * stride] = fix_nan_inf(scale * (tw - out_datas[index + 2 * stride]) * iou_normalizer);
            deltas[index + 3 * stride] = fix_nan_inf(scale * (th - out_datas[index + 3 * stride]) * iou_normalizer);

            if (os != nullptr)
            {
                *os << "delta x = " << deltas[index + 0 * stride] << " y = " << deltas[index + 1 * stride] << " w = " << deltas[index + 2 * stride] << " h = " << deltas[index + 3 * stride] << std::endl;
                *os << "delta index x = " << index + 0 * stride << " y = " << index + 1 * stride << " w = " << index + 2 * stride << " h = " << index + 3 * stride << std::endl;
                *os << "iou = " << all_ious.m_iou << std::endl;
            }
        }
        else
        {
            // https://github.com/generalized-iou/g-darknet
            // https://arxiv.org/abs/1902.09630v2
            // https://giou.stanford.edu/
            all_ious.m_dx_iou = DeltaXrepBoxIou(predict, truth, iou_loss);

            // jacobian^t (transpose)
            float dx = all_ious.m_dx_iou.m_dt;
            float dy = all_ious.m_dx_iou.m_db;
            float dw = all_ious.m_dx_iou.m_dl;
            float dh = all_ious.m_dx_iou.m_dr;

            dw *= std::exp(out_datas[index + 2 * stride]);
            dh *= std::exp(out_datas[index + 3 * stride]);

            // normalize iou weight
            dx *= iou_normalizer;
            dy *= iou_normalizer;
            dw *= iou_normalizer;
            dh *= iou_normalizer;

            dx = fix_nan_inf(dx);
            dy = fix_nan_inf(dy);
            dw = fix_nan_inf(dw);
            dh = fix_nan_inf(dh);

            // const float max_delta = FLT_MAX;
            // if (max_delta != FLT_MAX)
            // {
            //     dx = clip_value(dx, max_delta);
            //     dy = clip_value(dy, max_delta);
            //     dw = clip_value(dw, max_delta);
            //     dh = clip_value(dh, max_delta);
            // }

            // accumulate delta
            deltas[index + 0 * stride] = dx;
            deltas[index + 1 * stride] = dy;
            deltas[index + 2 * stride] = dw;
            deltas[index + 3 * stride] = dh;
        }

        return all_ious;
    }

    void DeltaClassAndBox(YoloTrainArgs *args)
    {
        // 指定那个单元格进行delta
        auto grid_index = GetGridIndex(args->m_batch, args->m_box, args->m_x, args->m_y, args->m_layer_width, args->m_layer_height, args->m_boxes);
        if (args->m_is_deltas->find(grid_index) != args->m_is_deltas->end()) return;

        const int box_index   = GetEntryIndex(args->m_batch, args->m_box, 0, args->m_x, args->m_y, args->m_layer_width, args->m_layer_height, args->m_boxes, args->m_classes);
        const int obj_index   = GetEntryIndex(args->m_batch, args->m_box, 4, args->m_x, args->m_y, args->m_layer_width, args->m_layer_height, args->m_boxes, args->m_classes);
        const int class_index = GetEntryIndex(args->m_batch, args->m_box, 5, args->m_x, args->m_y, args->m_layer_width, args->m_layer_height, args->m_boxes, args->m_classes);

        // objectness delta
        auto objectness           = fix_nan_inf(args->m_datas[obj_index]);
        args->m_deltas[obj_index] = args->m_object_normalizer * (1 - objectness);
        args->m_total_objectness  = args->m_total_objectness + objectness;
        if (args->m_os != nullptr) *args->m_os << "objectness = " << objectness << " delta objectness = " << args->m_object_normalizer * (1 - objectness) << std::endl;
        // class delta
        auto class_id = GetTruthClassId(args->m_truths, args->m_batch, args->m_truth_index, args->m_truths_size, args->m_max_truths_size);
        DeltaClass(args->m_datas, args->m_deltas, class_index, class_id, args->m_classes, args->m_stride, &args->m_total_classify, args->m_os);

        // box delta
        auto truth = GetTruthBox(args->m_truths, args->m_batch, args->m_truth_index, args->m_truths_size, args->m_max_truths_size);
        auto mask  = args->m_masks[args->m_box];
        auto ious  = DeltaBox(truth, args->m_datas, args->m_anchors, mask, box_index, args->m_x, args->m_y, args->m_layer_width, args->m_layer_height, args->m_net_width, args->m_net_height, args->m_deltas, args->m_stride, args->m_iou_normalizer, args->m_iou_loss_type, args->m_os);

        args->m_total_iou += ious.m_iou;
        args->m_total_iou_loss += 1 - ious.m_iou;
        args->m_total_giou += ious.m_giou;
        args->m_total_giou_loss += 1 - ious.m_giou;
        args->m_total_diou += ious.m_diou;
        args->m_total_diou_loss += 1 - ious.m_diou;
        args->m_total_ciou += ious.m_ciou;
        args->m_total_ciou_loss += 1 - ious.m_ciou;
        args->m_delta_count++;

        args->m_is_deltas->insert(grid_index);
    }

    float BoxIouKind(const Box &a, const Box &b, int iou_kind)
    {
        // IOU, GIOU, MSE, DIOU, CIOU
        if (iou_kind == s_iou_loss_iou) return BoxIou(a, b);
        if (iou_kind == s_iou_loss_giou) return BoxGIou(a, b);
        if (iou_kind == s_iou_loss_diou) return BoxDIou(a, b);
        if (iou_kind == s_iou_loss_ciou) return BoxCIou(a, b);
        return BoxIou(a, b);
    }

    float BoxIou(const Box &a, const Box &b)
    {
        float I = BoxIntersection(a, b);
        float U = BoxUnion(a, b);
        if (I == 0 || U == 0) return 0;
        return I / U;
    }

    float BoxGIou(const Box &a, const Box &b)
    {
        auto ba   = BoxC(a, b);
        float w   = ba.m_right - ba.m_left;
        float h   = ba.m_bottom - ba.m_top;
        float c   = w * h;
        float iou = BoxIou(a, b);
        if (c == 0) return iou;
        float u         = BoxUnion(a, b);
        float giou_term = (c - u) / c;
        return iou - giou_term;
    }

    float BoxDIou(const Box &a, const Box &b)
    {
        auto ba   = BoxC(a, b);
        float w   = ba.m_right - ba.m_left;
        float h   = ba.m_bottom - ba.m_top;
        float c   = w * w + h * h;
        float iou = BoxIou(a, b);
        if (c == 0) return iou;
        float d         = (a.m_x - b.m_x) * (a.m_x - b.m_x) + (a.m_y - b.m_y) * (a.m_y - b.m_y);
        float u         = std::pow(d / c, 0.6f);
        float diou_term = u;
        return iou - diou_term;
    }

    float BoxCIou(const Box &a, const Box &b)
    {
        auto ba   = BoxC(a, b);
        float w   = ba.m_right - ba.m_left;
        float h   = ba.m_bottom - ba.m_top;
        float c   = w * w + h * h;
        float iou = BoxIou(a, b);
        if (c == 0) return iou;

        float u         = (a.m_x - b.m_x) * (a.m_x - b.m_x) + (a.m_y - b.m_y) * (a.m_y - b.m_y);
        float d         = u / c;
        float ar_gt     = b.m_w / b.m_h;
        float ar_pred   = a.m_w / a.m_h;
        float ar_loss   = 4 / (M_PI * M_PI) * (std::atan(ar_gt) - std::atan(ar_pred)) * (std::atan(ar_gt) - std::atan(ar_pred));
        float alpha     = ar_loss / (1 - iou + ar_loss + 0.000001);
        float ciou_term = d + alpha * ar_loss;
        return iou - ciou_term;
    }

    float BoxIntersection(const Box &a, const Box &b)
    {
        float w = BoxOverlap(a.m_x, a.m_w, b.m_x, b.m_w);
        float h = BoxOverlap(a.m_y, a.m_h, b.m_y, b.m_h);
        if (w < 0 || h < 0) return 0;
        float area = w * h;
        return area;
    }
    float BoxUnion(const Box &a, const Box &b)
    {
        float i = BoxIntersection(a, b);
        float u = a.m_w * a.m_h + b.m_w * b.m_h - i;
        return u;
    }

    float BoxOverlap(float x1, float w1, float x2, float w2)
    {
        float l1    = x1 - w1 / 2;
        float l2    = x2 - w2 / 2;
        float left  = l1 > l2 ? l1 : l2;
        float r1    = x1 + w1 / 2;
        float r2    = x2 + w2 / 2;
        float right = r1 < r2 ? r1 : r2;
        return right - left;
    }

    BoxAbs BoxC(const Box &a, const Box &b)
    {
        BoxAbs ba   = {0};
        ba.m_top    = std::fmin(a.m_y - a.m_h / 2, b.m_y - b.m_h / 2);
        ba.m_bottom = fmax(a.m_y + a.m_h / 2, b.m_y + b.m_h / 2);
        ba.m_left   = fmin(a.m_x - a.m_w / 2, b.m_x - b.m_w / 2);
        ba.m_right  = fmax(a.m_x + a.m_w / 2, b.m_x + b.m_w / 2);
        return ba;
    }

    BoxAbs ToBoxAbs(const Box &a)
    {
        BoxAbs tblr   = {0};
        float t       = a.m_y - (a.m_h / 2);
        float b       = a.m_y + (a.m_h / 2);
        float l       = a.m_x - (a.m_w / 2);
        float r       = a.m_x + (a.m_w / 2);
        tblr.m_top    = t;
        tblr.m_bottom = b;
        tblr.m_left   = l;
        tblr.m_right  = r;
        return tblr;
    }

    DeltaXrep DeltaXrepBoxIou(const Box &pred, const Box &truth, int iou_loss)
    {
        BoxAbs pred_tblr = ToBoxAbs(pred);
        float pred_t     = std::fmin(pred_tblr.m_top, pred_tblr.m_bottom);
        float pred_b     = std::fmax(pred_tblr.m_top, pred_tblr.m_bottom);
        float pred_l     = std::fmin(pred_tblr.m_left, pred_tblr.m_right);
        float pred_r     = std::fmax(pred_tblr.m_left, pred_tblr.m_right);

        BoxAbs truth_tblr = ToBoxAbs(truth);
        DeltaXrep ddx     = {0};
        float X           = (pred_b - pred_t) * (pred_r - pred_l);
        float Xhat        = (truth_tblr.m_bottom - truth_tblr.m_top) * (truth_tblr.m_right - truth_tblr.m_left);
        float Ih          = std::fmin(pred_b, truth_tblr.m_bottom) - std::fmax(pred_t, truth_tblr.m_top);
        float Iw          = std::fmin(pred_r, truth_tblr.m_right) - std::fmax(pred_l, truth_tblr.m_left);
        float I           = Iw * Ih;
        float U           = X + Xhat - I;
        float S           = (pred.m_x - truth.m_x) * (pred.m_x - truth.m_x) + (pred.m_y - truth.m_y) * (pred.m_y - truth.m_y);
        float giou_Cw     = std::fmax(pred_r, truth_tblr.m_right) - std::fmin(pred_l, truth_tblr.m_left);
        float giou_Ch     = std::fmax(pred_b, truth_tblr.m_bottom) - std::fmin(pred_t, truth_tblr.m_top);
        float giou_C      = giou_Cw * giou_Ch;

        // Partial Derivatives, derivatives
        float dX_wrt_t = -1 * (pred_r - pred_l);
        float dX_wrt_b = pred_r - pred_l;
        float dX_wrt_l = -1 * (pred_b - pred_t);
        float dX_wrt_r = pred_b - pred_t;

        // gradient of I min/max in IoU calc (prediction)
        float dI_wrt_t = pred_t > truth_tblr.m_top ? (-1 * Iw) : 0;
        float dI_wrt_b = pred_b < truth_tblr.m_bottom ? Iw : 0;
        float dI_wrt_l = pred_l > truth_tblr.m_left ? (-1 * Ih) : 0;
        float dI_wrt_r = pred_r < truth_tblr.m_right ? Ih : 0;
        // derivative of U with regard to x
        float dU_wrt_t = dX_wrt_t - dI_wrt_t;
        float dU_wrt_b = dX_wrt_b - dI_wrt_b;
        float dU_wrt_l = dX_wrt_l - dI_wrt_l;
        float dU_wrt_r = dX_wrt_r - dI_wrt_r;
        // gradient of C min/max in IoU calc (prediction)
        float dC_wrt_t = pred_t < truth_tblr.m_top ? (-1 * giou_Cw) : 0;
        float dC_wrt_b = pred_b > truth_tblr.m_bottom ? giou_Cw : 0;
        float dC_wrt_l = pred_l < truth_tblr.m_left ? (-1 * giou_Ch) : 0;
        float dC_wrt_r = pred_r > truth_tblr.m_right ? giou_Ch : 0;

        float p_dt = 0;
        float p_db = 0;
        float p_dl = 0;
        float p_dr = 0;
        if (U > 0)
        {
            p_dt = ((U * dI_wrt_t) - (I * dU_wrt_t)) / (U * U);
            p_db = ((U * dI_wrt_b) - (I * dU_wrt_b)) / (U * U);
            p_dl = ((U * dI_wrt_l) - (I * dU_wrt_l)) / (U * U);
            p_dr = ((U * dI_wrt_r) - (I * dU_wrt_r)) / (U * U);
        }
        // apply grad from prediction min/max for correct corner selection
        p_dt = pred_tblr.m_top < pred_tblr.m_bottom ? p_dt : p_db;
        p_db = pred_tblr.m_top < pred_tblr.m_bottom ? p_db : p_dt;
        p_dl = pred_tblr.m_left < pred_tblr.m_right ? p_dl : p_dr;
        p_dr = pred_tblr.m_left < pred_tblr.m_right ? p_dr : p_dl;

        if (iou_loss == s_iou_loss_giou)
        {
            if (giou_C > 0)
            {
                // apply "C" term from gIOU
                p_dt += ((giou_C * dU_wrt_t) - (U * dC_wrt_t)) / (giou_C * giou_C);
                p_db += ((giou_C * dU_wrt_b) - (U * dC_wrt_b)) / (giou_C * giou_C);
                p_dl += ((giou_C * dU_wrt_l) - (U * dC_wrt_l)) / (giou_C * giou_C);
                p_dr += ((giou_C * dU_wrt_r) - (U * dC_wrt_r)) / (giou_C * giou_C);
            }
            if (Iw <= 0 || Ih <= 0)
            {
                p_dt = ((giou_C * dU_wrt_t) - (U * dC_wrt_t)) / (giou_C * giou_C);
                p_db = ((giou_C * dU_wrt_b) - (U * dC_wrt_b)) / (giou_C * giou_C);
                p_dl = ((giou_C * dU_wrt_l) - (U * dC_wrt_l)) / (giou_C * giou_C);
                p_dr = ((giou_C * dU_wrt_r) - (U * dC_wrt_r)) / (giou_C * giou_C);
            }
        }

        float Ct = std::fmin(pred.m_y - pred.m_h / 2, truth.m_y - truth.m_h / 2);
        float Cb = std::fmax(pred.m_y + pred.m_h / 2, truth.m_y + truth.m_h / 2);
        float Cl = std::fmin(pred.m_x - pred.m_w / 2, truth.m_x - truth.m_w / 2);
        float Cr = std::fmax(pred.m_x + pred.m_w / 2, truth.m_x + truth.m_w / 2);
        float Cw = Cr - Cl;
        float Ch = Cb - Ct;
        float C  = Cw * Cw + Ch * Ch;

        float dCt_dx = 0;
        float dCt_dy = pred_t < truth_tblr.m_top ? 1 : 0;
        float dCt_dw = 0;
        float dCt_dh = pred_t < truth_tblr.m_top ? -0.5 : 0;

        float dCb_dx = 0;
        float dCb_dy = pred_b > truth_tblr.m_bottom ? 1 : 0;
        float dCb_dw = 0;
        float dCb_dh = pred_b > truth_tblr.m_bottom ? 0.5 : 0;

        float dCl_dx = pred_l < truth_tblr.m_left ? 1 : 0;
        float dCl_dy = 0;
        float dCl_dw = pred_l < truth_tblr.m_left ? -0.5 : 0;
        float dCl_dh = 0;

        float dCr_dx = pred_r > truth_tblr.m_right ? 1 : 0;
        float dCr_dy = 0;
        float dCr_dw = pred_r > truth_tblr.m_right ? 0.5 : 0;
        float dCr_dh = 0;

        float dCw_dx = dCr_dx - dCl_dx;
        float dCw_dy = dCr_dy - dCl_dy;
        float dCw_dw = dCr_dw - dCl_dw;
        float dCw_dh = dCr_dh - dCl_dh;

        float dCh_dx = dCb_dx - dCt_dx;
        float dCh_dy = dCb_dy - dCt_dy;
        float dCh_dw = dCb_dw - dCt_dw;
        float dCh_dh = dCb_dh - dCt_dh;

        // Final IOU loss (prediction) (negative of IOU gradient, we want the negative loss)
        float p_dx = 0;
        float p_dy = 0;
        float p_dw = 0;
        float p_dh = 0;

        p_dx = p_dl + p_dr; // p_dx, p_dy, p_dw and p_dh are the gradient of IoU or GIoU.
        p_dy = p_dt + p_db;
        p_dw = (p_dr - p_dl); // For dw and dh, we do not divided by 2.
        p_dh = (p_db - p_dt);

        // https://github.com/Zzh-tju/DIoU-darknet
        // https://arxiv.org/abs/1911.08287
        if (iou_loss == s_iou_loss_diou)
        {
            if (C > 0)
            {
                p_dx += (2 * (truth.m_x - pred.m_x) * C - (2 * Cw * dCw_dx + 2 * Ch * dCh_dx) * S) / (C * C);
                p_dy += (2 * (truth.m_y - pred.m_y) * C - (2 * Cw * dCw_dy + 2 * Ch * dCh_dy) * S) / (C * C);
                p_dw += (2 * Cw * dCw_dw + 2 * Ch * dCh_dw) * S / (C * C);
                p_dh += (2 * Cw * dCw_dh + 2 * Ch * dCh_dh) * S / (C * C);
            }
            if (Iw <= 0 || Ih <= 0)
            {
                p_dx = (2 * (truth.m_x - pred.m_x) * C - (2 * Cw * dCw_dx + 2 * Ch * dCh_dx) * S) / (C * C);
                p_dy = (2 * (truth.m_y - pred.m_y) * C - (2 * Cw * dCw_dy + 2 * Ch * dCh_dy) * S) / (C * C);
                p_dw = (2 * Cw * dCw_dw + 2 * Ch * dCh_dw) * S / (C * C);
                p_dh = (2 * Cw * dCw_dh + 2 * Ch * dCh_dh) * S / (C * C);
            }
        }
        // The following codes are calculating the gradient of ciou.

        if (iou_loss == s_iou_loss_ciou)
        {
            float ar_gt   = truth.m_w / truth.m_h;
            float ar_pred = pred.m_w / pred.m_h;
            float ar_loss = 4 / (M_PI * M_PI) * (std::atan(ar_gt) - std::atan(ar_pred)) * (std::atan(ar_gt) - std::atan(ar_pred));
            float alpha   = ar_loss / (1 - I / U + ar_loss + 0.000001);
            float ar_dw   = 8 / (M_PI * M_PI) * (std::atan(ar_gt) - std::atan(ar_pred)) * pred.m_h;
            float ar_dh   = -8 / (M_PI * M_PI) * (std::atan(ar_gt) - std::atan(ar_pred)) * pred.m_w;
            if (C > 0)
            {
                // dar*
                p_dx += (2 * (truth.m_x - pred.m_x) * C - (2 * Cw * dCw_dx + 2 * Ch * dCh_dx) * S) / (C * C);
                p_dy += (2 * (truth.m_y - pred.m_y) * C - (2 * Cw * dCw_dy + 2 * Ch * dCh_dy) * S) / (C * C);
                p_dw += (2 * Cw * dCw_dw + 2 * Ch * dCh_dw) * S / (C * C) + alpha * ar_dw;
                p_dh += (2 * Cw * dCw_dh + 2 * Ch * dCh_dh) * S / (C * C) + alpha * ar_dh;
            }
            if (Iw <= 0 || Ih <= 0)
            {
                p_dx = (2 * (truth.m_x - pred.m_x) * C - (2 * Cw * dCw_dx + 2 * Ch * dCh_dx) * S) / (C * C);
                p_dy = (2 * (truth.m_y - pred.m_y) * C - (2 * Cw * dCw_dy + 2 * Ch * dCh_dy) * S) / (C * C);
                p_dw = (2 * Cw * dCw_dw + 2 * Ch * dCh_dw) * S / (C * C) + alpha * ar_dw;
                p_dh = (2 * Cw * dCw_dh + 2 * Ch * dCh_dh) * S / (C * C) + alpha * ar_dh;
            }
        }

        ddx.m_dt = p_dx; // We follow the original code released from GDarknet. So in yolo_layer.c, dt, db, dl, dr are already dx, dy, dw, dh.
        ddx.m_db = p_dy;
        ddx.m_dl = p_dw;
        ddx.m_dr = p_dh;

        return ddx;
    }
}