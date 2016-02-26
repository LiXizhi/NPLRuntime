#pragma once
namespace ParaEngine
{
const char* PARAENGINE_TEMPLATE = "\
xof 0303txt 0032\
template Anim{\
 <00000002-0000-0000-0000-123456789000>\
 STRING name;\
 DWORD from;\
 DWORD to;\
 DWORD nextAnim;\
 FLOAT moveSpeed;\
 Vector minEntent;\
 Vector maxEntent;\
 FLOAT boundsRadius;\
}\
template AlphaKey {\
 <00000012-0000-0000-0000-123456789000>\
 DWORD nKeys;\
 array TimedFloatKeys keys[nKeys];\
}\
template Alpha{\
 <00000011-0000-0000-0000-123456789000>\
 [...]\
}\
template AlphaSet{\
 <00000010-0000-0000-0000-123456789000>\
 [...]\
}\
template Sequences{\
 <00000001-0000-0000-0000-123456789000>\
 [Anim <00000002-0000-0000-0000-123456789000>]\
}\
template ParaEngine{\
 <00000000-0000-0000-0000-123456789000>\
 [...]\
}\
template XSkinMeshHeader {\
 <3cf169ce-ff7c-44ab-93c0-f78f62d172e2>\
 WORD nMaxSkinWeightsPerVertex;\
 WORD nMaxSkinWeightsPerFace;\
 WORD nBones;\
}\
template VertexDuplicationIndices {\
 <b8d65549-d7c9-4995-89cf-53a9a8b031e3>\
 DWORD nIndices;\
 DWORD nOriginalVertices;\
 array DWORD indices[nIndices];\
}\
template SkinWeights {\
 <6f0d123b-bad2-4167-a0d0-80224f25fabb>\
 STRING transformNodeName;\
 DWORD nWeights;\
 array DWORD vertexIndices[nWeights];\
 array FLOAT weights[nWeights];\
 Matrix4x4 matrixOffset;\
}\
";
}