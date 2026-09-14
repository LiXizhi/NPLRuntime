#pragma once
// Robot-only resource preparation. The shared ParaX importer is unchanged.
#include "json/json.h"
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <limits>
namespace ParaEngine { namespace RobotRenderAsset {
inline unsigned Count(const Json::Value& object, const char* field) { return object[field].size(); }
inline void Check(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(std::string("Robot render asset: ") + message);
}
inline uint32_t Read32(const std::string& s, size_t p) {
    Check(p <= s.size() && s.size()-p >= 4, "truncated binary");
    return uint32_t(uint8_t(s[p])) | (uint32_t(uint8_t(s[p+1]))<<8) | (uint32_t(uint8_t(s[p+2]))<<16) | (uint32_t(uint8_t(s[p+3]))<<24);
}
inline void Put32(std::string& s, uint32_t n) {
    for (int i=0;i<4;++i) s.push_back(char((n>>(8*i))&255));
}
inline unsigned Number(const Json::Value& v) { Check(v.isUInt() || (v.isInt() && v.asInt() >= 0), "expected unsigned integer"); return v.asUInt(); }
inline std::string DataURI(const std::string& uri) {
    size_t comma=uri.find(',');
    Check(uri.compare(0,5,"data:")==0 && comma!=std::string::npos && comma>=7 && uri.substr(comma-7,7)==";base64", "embed external buffers before loading");
    const std::string alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result; uint32_t value=0; int bits=0; bool padding=false;
    for(size_t i=comma+1;i<uri.size();++i) {
        char c=uri[i]; if(c=='=') { padding=true; continue; }
        Check(!padding,"invalid base64 padding"); size_t d=alphabet.find(c); Check(d!=std::string::npos,"invalid base64");
        value=(value<<6)|uint32_t(d); bits+=6;
        if(bits>=8) { bits-=8; result.push_back(char((value>>bits)&255)); }
    }
    return result;
}
class Preparation {
    Json::Value doc;
    std::string binary;
    std::map<unsigned,std::string> converted;
    Json::Value& At(const char* array,unsigned index) {
        Check(doc[array].isArray() && index<doc[array].size(),"invalid object index"); return doc[array][index];
    }
    unsigned View(const std::string& bytes) {
        while(binary.size()%4) binary.push_back(0);
        Check(binary.size()+bytes.size()<std::numeric_limits<uint32_t>::max(),"asset too large");
        Json::Value v; v["buffer"]=0u; v["byteOffset"]=unsigned(binary.size()); v["byteLength"]=unsigned(bytes.size());
        unsigned index=doc["bufferViews"].size(); doc["bufferViews"].append(v); binary+=bytes; return index;
    }
    std::string Dense(const Json::Value& a,unsigned width) {
        Check(!a.isMember("sparse"),"sparse coordinates require offline expansion");
        const Json::Value& v=At("bufferViews",Number(a["bufferView"])); Check(Number(v["buffer"])==0,"invalid buffer");
        size_t start=Number(v.get("byteOffset",0u)), length=Number(v["byteLength"]), offset=Number(a.get("byteOffset",0u));
        size_t stride=Number(v.get("byteStride",width)), count=Number(a["count"]);
        Check(stride>=width && start<=binary.size() && length<=binary.size()-start && offset<=length
            && count>0 && width<=length-offset && count-1<=(length-offset-width)/stride,"accessor outside buffer view");
        std::string result; result.reserve(count*width);
        for(size_t i=0;i<count;++i) result.append(binary,start+offset+i*stride,width);
        return result;
    }
    bool Once(unsigned index,const std::string& operation) {
        auto found=converted.find(index);
        if(found!=converted.end()) { Check(found->second==operation,"conflicting accessor coordinate roles"); return false; }
        converted[index]=operation; return true;
    }
    void Reflect(unsigned index,const char* type,const std::vector<unsigned>& signs) {
        std::string operation(type); for(auto i:signs) operation+=":"+std::to_string(i);
        if(!Once(index,operation)) return;
        Json::Value a=At("accessors",index);
        Check(a["type"].asString()==type && Number(a["componentType"])==5126 && !a.get("normalized",false).asBool(),"coordinates must contain floats");
        unsigned components=std::string(type)=="VEC3"?3:std::string(type)=="VEC4"?4:16;
        std::string data=Dense(a,components*4);
        for(size_t p=0;p<data.size();p+=components*4)
            for(auto i:signs) data[p+i*4+3]=char(uint8_t(data[p+i*4+3])^128);
        if(a.isMember("min") || a.isMember("max")) {
            Check(a["min"].size()==components && a["max"].size()==components,"invalid accessor bounds");
            for(auto i:signs) { double minimum=a["min"][i].asDouble(); a["min"][i]=-a["max"][i].asDouble(); a["max"][i]=-minimum; }
        }
        a["bufferView"]=View(data); a["byteOffset"]=0u; At("accessors",index)=a;
    }
    void Winding(Json::Value& p) {
        unsigned mode=Number(p.get("mode",4u)); if(mode<=3) return;
        Check(mode==4,"triangulate strips/fans before loading");
        if(!p.isMember("indices")) {
            unsigned count=Number(At("accessors",Number(p["attributes"]["POSITION"]))["count"]);
            Check(count%3==0,"invalid triangle count"); std::string data;
            for(unsigned i=0;i<count;i+=3) { Put32(data,i+2); Put32(data,i+1); Put32(data,i); }
            Json::Value a; a["bufferView"]=View(data); a["componentType"]=5125u; a["count"]=count; a["type"]="SCALAR";
            p["indices"]=doc["accessors"].size(); doc["accessors"].append(a); return;
        }
        unsigned index=Number(p["indices"]); if(!Once(index,"winding")) return;
        Json::Value a=At("accessors",index); unsigned component=Number(a["componentType"]);
        unsigned width=component==5121?1:component==5123?2:component==5125?4:0;
        Check(width && a["type"].asString()=="SCALAR" && Number(a["count"])%3==0,"invalid triangle indices");
        std::string data=Dense(a,width);
        for(size_t p=0;p<data.size();p+=width*3) for(unsigned j=0;j<width;++j) std::swap(data[p+j],data[p+2*width+j]);
        a["bufferView"]=View(data); a["byteOffset"]=0u; At("accessors",index)=a;
    }
    void Attributes(Json::Value& a,bool morph) {
        if(a.isMember("POSITION")) Reflect(Number(a["POSITION"]),"VEC3",{2});
        if(a.isMember("NORMAL")) Reflect(Number(a["NORMAL"]),"VEC3",{2});
        // Match Assimp MakeLeftHanded: reflect tangent Z and preserve its handedness scalar.
        if(a.isMember("TANGENT")) Reflect(Number(a["TANGENT"]),morph?"VEC3":"VEC4",std::vector<unsigned>{2});
    }
    void ArraySigns(Json::Value& object,const char* key,unsigned size,const std::vector<unsigned>& signs) {
        if(!object.isMember(key)) return;
        auto& values=object[key]; Check(values.isArray() && values.size()==size,"invalid node transform");
        for(unsigned i=0;i<size;++i) Check(values[i].isNumeric(),"invalid transform value");
        for(auto i:signs) values[i]=-values[i].asDouble();
    }
public:
    std::string Run(const std::string& bytes,const std::string& extension) {
        std::string json,embedded;
        if(extension=="glb") {
            Check(bytes.size()>=20 && bytes.compare(0,4,"glTF")==0 && Read32(bytes,4)==2 && Read32(bytes,8)==bytes.size(),"invalid GLB header");
            for(size_t p=12;p<bytes.size();) {
                uint32_t n=Read32(bytes,p),type=Read32(bytes,p+4); p+=8;
                Check(n%4==0 && n<=bytes.size()-p,"invalid GLB chunk");
                if(type==0x4e4f534a) { Check(json.empty() && p==20,"invalid JSON chunk"); json=bytes.substr(p,n); }
                else if(type==0x004e4942) { Check(embedded.empty(),"duplicate binary chunk"); embedded=bytes.substr(p,n); }
                else Check(false,"unsupported GLB chunk");
                p+=n;
            }
        } else { Check(extension=="gltf","requires glTF/GLB"); json=bytes; }
        Json::Reader reader;
        Check(reader.parse(json,doc,false) && doc.isObject() && doc["asset"]["version"].asString()=="2.0","invalid glTF 2 JSON");
        Check(!Count(doc,"extensionsRequired"),"required extensions need offline preparation");
        for(unsigned i=0;i<Count(doc,"nodes");++i) Check(!Count(doc["nodes"][i],"extensions"),"node extensions need offline preparation");
        for(unsigned i=0;i<Count(doc,"images");++i)
            if(doc["images"][i].isMember("uri")) Check(doc["images"][i]["uri"].asString().compare(0,5,"data:")==0,"embed external textures before loading");
        Check(doc["buffers"].isArray() && doc["buffers"].size()>0,"missing buffers");
        std::vector<unsigned> bases;
        for(unsigned i=0;i<doc["buffers"].size();++i) {
            auto& b=doc["buffers"][i]; std::string data=b.isMember("uri")?DataURI(b["uri"].asString()):embedded;
            Check(b.isMember("uri") || i==0,"missing embedded buffer"); unsigned length=Number(b["byteLength"]);
            Check(length<=data.size(),"truncated buffer"); while(binary.size()%4) binary.push_back(0);
            Check(binary.size()+length<std::numeric_limits<uint32_t>::max(),"asset too large");
            bases.push_back(unsigned(binary.size())); binary.append(data,0,length);
        }
        for(unsigned i=0;i<doc["bufferViews"].size();++i) {
            auto& v=doc["bufferViews"][i]; unsigned b=Number(v["buffer"]); Check(b<bases.size(),"invalid buffer index");
            unsigned offset=Number(v.get("byteOffset",0u)),length=Number(v["byteLength"]),total=Number(doc["buffers"][b]["byteLength"]);
            Check(offset<=total && length<=total-offset,"buffer view outside buffer"); v["buffer"]=0u; v["byteOffset"]=bases[b]+offset;
        }
        bool prepared=doc["asset"].get("generator","").asString()=="ParaRobot render preparation v2";
        if(!prepared) {
            // Reserve data used in roles that must not be reflected. A shared
            // accessor cannot be both a position and (for example) a scale.
            // Reject such exports instead of silently changing the other role.
            for(unsigned m=0;m<Count(doc,"meshes");++m) for(unsigned p=0;p<doc["meshes"][m]["primitives"].size();++p) {
                auto& primitive=doc["meshes"][m]["primitives"][p];
                const auto& attributes=primitive["attributes"];
                for(const auto& key:attributes.getMemberNames())
                    if(key!="POSITION" && key!="NORMAL" && key!="TANGENT") Once(Number(attributes[key]),"unchanged");
                if(Number(primitive.get("mode",4u))<=3 && primitive.isMember("indices")) Once(Number(primitive["indices"]),"unchanged");
            }
            for(unsigned i=0;i<Count(doc,"animations");++i) {
                const auto& animation=doc["animations"][i];
                for(unsigned c=0;c<animation["channels"].size();++c) {
                    const auto& channel=animation["channels"][c]; unsigned sampler=Number(channel["sampler"]);
                    Check(sampler<animation["samplers"].size(),"invalid animation sampler");
                    const auto& sampling=animation["samplers"][sampler]; Once(Number(sampling["input"]),"unchanged");
                    std::string path=channel["target"]["path"].asString();
                    if(path=="scale" || path=="weights") Once(Number(sampling["output"]),"unchanged");
                }
            }
            for(unsigned m=0;m<Count(doc,"meshes");++m) for(unsigned p=0;p<doc["meshes"][m]["primitives"].size();++p) {
                auto& primitive=doc["meshes"][m]["primitives"][p]; Check(!Count(primitive,"extensions"),"primitive extensions need offline preparation");
                Attributes(primitive["attributes"],false);
                for(unsigned t=0;t<Count(primitive,"targets");++t) Attributes(primitive["targets"][t],true);
                Winding(primitive);
            }
            for(unsigned i=0;i<Count(doc,"skins");++i)
                if(doc["skins"][i].isMember("inverseBindMatrices")) Reflect(Number(doc["skins"][i]["inverseBindMatrices"]),"MAT4",{2,6,8,9,11,14});
            for(unsigned i=0;i<Count(doc,"nodes");++i) {
                auto& node=doc["nodes"][i]; ArraySigns(node,"translation",3,{2}); ArraySigns(node,"rotation",4,{0,1}); ArraySigns(node,"matrix",16,{2,6,8,9,11,14});
            }
            for(unsigned i=0;i<Count(doc,"animations");++i) {
                auto& a=doc["animations"][i];
                for(unsigned c=0;c<a["channels"].size();++c) {
                    auto& channel=a["channels"][c]; unsigned sampler=Number(channel["sampler"]); Check(sampler<a["samplers"].size(),"invalid animation sampler");
                    unsigned output=Number(a["samplers"][sampler]["output"]); std::string path=channel["target"]["path"].asString();
                    if(path=="translation") Reflect(output,"VEC3",{2}); else if(path=="rotation") Reflect(output,"VEC4",{0,1});
                    else Check(path=="scale" || path=="weights","unsupported animation target");
                }
            }
        }
        doc["asset"]["generator"]="ParaRobot render preparation v2";
        doc["buffers"]=Json::Value(Json::arrayValue); Json::Value b; b["byteLength"]=unsigned(binary.size()); doc["buffers"].append(b);
        Json::FastWriter writer; json=writer.write(doc); while(json.size()%4) json.push_back(' '); while(binary.size()%4) binary.push_back(0);
        Check(json.size()+binary.size()+28<std::numeric_limits<uint32_t>::max(),"asset too large");
        std::string result("glTF"); Put32(result,2); Put32(result,unsigned(json.size()+binary.size()+28));
        Put32(result,unsigned(json.size())); Put32(result,0x4e4f534a); result+=json; Put32(result,unsigned(binary.size())); Put32(result,0x004e4942); result+=binary;
        return result;
    }
};
}
inline std::string PrepareRobotRenderAsset(const std::string& bytes,const std::string& extension) {
    return RobotRenderAsset::Preparation().Run(bytes,extension);
}
}
