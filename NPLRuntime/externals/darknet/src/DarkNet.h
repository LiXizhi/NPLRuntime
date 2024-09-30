
#ifndef __DARKNET_H__
#define __DARKNET_H__

#include "models/ColorModel.h"
#include "models/Model.h"
#include "models/SoftMaxModel.h"
#include "models/YoloModel.h"
#include "utils/mfcc.h"

using DarkNetOptions = darknet::Options;
using DarkNetDataImage = darknet::DataImage;
using DarkNetModel = darknet::Model;
using DarkNetSoftMaxModel = darknet::SoftMaxModel;
using DarkNetMFCC = darknet::MFCC;
#endif
