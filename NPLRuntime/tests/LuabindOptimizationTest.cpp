//-----------------------------------------------------------------------------
// LuabindOptimizationTest.cpp
// Verifies correctness, inline zero-malloc storage, overload resolution,
// policies, properties, table iterators, inheritance, coroutines, and
// multi-threaded lua_State safety of the refined luabind runtime.
// Pass --benchmark to run microbenchmarks.
//-----------------------------------------------------------------------------
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <luabind/luabind.hpp>
#include <luabind/class_info.hpp>
#include <luabind/copy_policy.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/raw_policy.hpp>
#include <luabind/return_reference_to_policy.hpp>

struct MockEngineEntity {
    int id = 42;
    float x = 1.0f, y = 2.0f, z = 3.0f;
    bool visible = true;
    int width = 100, height = 50;
    std::string name = "player_actor";
    std::string text = "Hello ParaEngine";
};

// Mirrors ParaScriptingGlobal.h's universal stack-object get_pointer(T const&) rule,
// where get_pointer(x) returns the interior/self address &x rather than a heap pointer.
namespace MockSelfPtrNS {

struct SelfPtrAttrHandle {
    MockEngineEntity* m_pAttr;
    std::uintptr_t m_pAttrClass;

    explicit SelfPtrAttrHandle(MockEngineEntity* p = nullptr, std::uintptr_t cls = 0xCAFEBABEULL) noexcept
      : m_pAttr(p), m_pAttrClass(cls) {}

    bool IsValid() const { return m_pAttr != nullptr && m_pAttrClass == 0xCAFEBABEULL; }
    int GetFieldNum() const {
        if (!IsValid()) return -1;
        return m_pAttr->id + 19;
    }
};

struct SelfPtrPolyBase {
    int baseTag = 7;
    virtual ~SelfPtrPolyBase() {}
    virtual int EvalTag() const { return baseTag; }
};

struct SelfPtrPolyDerived : SelfPtrPolyBase {
    int extraTag = 33;
    int EvalTag() const override { return baseTag + extraTag; }
};

template <typename T>
T* get_pointer(T const& obj) {
    return const_cast<T*>(&obj);
}

} // namespace MockSelfPtrNS

namespace luabind {
using MockSelfPtrNS::get_pointer;
}

namespace {

class ParaObjectHandle {
public:
    MockEngineEntity* m_pObj;
    std::uintptr_t m_weakGuard;

    ParaObjectHandle() noexcept : m_pObj(nullptr), m_weakGuard(0) {}
    explicit ParaObjectHandle(MockEngineEntity* p) noexcept : m_pObj(p), m_weakGuard(0x1234) {}

    bool IsValid() const { return m_pObj != nullptr; }
    int GetID() const { return m_pObj ? m_pObj->id : 0; }
    void SetPosition(float x, float y, float z) {
        if (m_pObj) { m_pObj->x = x; m_pObj->y = y; m_pObj->z = z; }
    }
    void GetPosition(float& x, float& y, float& z) const {
        if (m_pObj) { x = m_pObj->x; y = m_pObj->y; z = m_pObj->z; }
        else { x = y = z = 0.0f; }
    }
    bool GetVisible() const { return m_pObj ? m_pObj->visible : false; }
    void SetVisible(bool v) { if (m_pObj) m_pObj->visible = v; }
    int GetWidth() const { return m_pObj ? m_pObj->width : 0; }
    void SetWidth(int w) { if (m_pObj) m_pObj->width = w; }
    int GetHeight() const { return m_pObj ? m_pObj->height : 0; }
    void SetHeight(int h) { if (m_pObj) m_pObj->height = h; }
    const char* GetName() const { return m_pObj ? m_pObj->name.c_str() : ""; }
    void SetName(const char* s) { if (m_pObj && s) m_pObj->name = s; }
    std::string GetText() const { return m_pObj ? m_pObj->text : std::string(); }
    void SetText(const char* s) { if (m_pObj && s) m_pObj->text = s; }
    ParaObjectHandle& SelfRef() { return *this; }

    int PlayAnim(int animId) { return animId + 1; }
    int PlayAnim2(int animId, float speed) { return animId + static_cast<int>(speed * 10.0f); }
    int TargetBy(const ParaObjectHandle& o) const { return o.GetID() + 100; }
    int TargetByName(const char* s) const { return s ? static_cast<int>(std::strlen(s)) : -1; }

    int RawStackAdd(int a, lua_State* L) const {
        return a + GetID() + lua_gettop(L);
    }
};

class ParaCharacterHandle {
public:
    MockEngineEntity* m_pObj;
    explicit ParaCharacterHandle(MockEngineEntity* p = nullptr) noexcept : m_pObj(p) {}
    bool IsValid() const { return m_pObj != nullptr; }
    int GetAnimID() const { return m_pObj ? m_pObj->id : 0; }
};

struct PolyBase {
    int baseVal = 10;
    virtual ~PolyBase() {}
    virtual int Compute() const { return baseVal; }
};

struct PolyDerived : PolyBase {
    int derivedVal = 25;
    int Compute() const override { return baseVal + derivedVal; }
    int OnlyDerived() const { return derivedVal * 2; }
};

static MockEngineEntity g_entity;

static uint32_t GetBlockId(uint16_t x, int32_t y, uint16_t z) {
    return (static_cast<uint32_t>(x) * 73856093u) ^
           (static_cast<uint32_t>(y) * 19349663u) ^
           (static_cast<uint32_t>(z) * 83492791u);
}

static ParaObjectHandle GetPlayer() { return ParaObjectHandle(&g_entity); }
static ParaCharacterHandle GetChar() { return ParaCharacterHandle(&g_entity); }
static MockSelfPtrNS::SelfPtrAttrHandle GetSelfPtrAttr() {
    return MockSelfPtrNS::SelfPtrAttrHandle(&g_entity, 0xCAFEBABEULL);
}
static MockSelfPtrNS::SelfPtrPolyDerived GetSelfPtrPoly() {
    return MockSelfPtrNS::SelfPtrPolyDerived();
}
static int ClobberNativeStack(int depth) {
    volatile std::uintptr_t scrub[64];
    for (int i = 0; i < 64; ++i)
        scrub[i] = 0x3dULL + static_cast<std::uintptr_t>(i + depth);
    if (depth > 0)
        return ClobberNativeStack(depth - 1) + static_cast<int>(scrub[0] & 1);
    return static_cast<int>(scrub[63] & 1);
}

static int Activate1(const char* file) { return static_cast<int>(std::strlen(file)); }
static int Activate2(const char* file, const char* msg) { return static_cast<int>(std::strlen(file) + std::strlen(msg)); }
static int Activate3(const char* file, const luabind::object& tbl, int channel) {
    (void)file;
    if (luabind::type(tbl) == LUA_TTABLE) {
        int sum = channel;
        if (luabind::type(tbl["cmd"]) == LUA_TSTRING)
            sum += static_cast<int>(std::strlen(luabind::object_cast<const char*>(tbl["cmd"])));
        if (luabind::type(tbl["count"]) == LUA_TNUMBER)
            sum += static_cast<int>(luabind::object_cast<double>(tbl["count"]));
        if (luabind::type(tbl[1]) == LUA_TNUMBER)
            sum += luabind::object_cast<int>(tbl[1]);
        return sum;
    }
    return -1;
}

static int SumTableViaIterator(const luabind::object& tbl) {
    int total = 0;
    for (luabind::iterator itCur(tbl), itEnd; itCur != itEnd; ++itCur) {
        const luabind::object& key = itCur.key();
        const luabind::object& val = *itCur;
        if (luabind::type(key) == LUA_TSTRING) {
            const char* k = luabind::object_cast<const char*>(key);
            total += static_cast<int>(k[0]);
        } else if (luabind::type(key) == LUA_TNUMBER) {
            total += luabind::object_cast<int>(key);
        }
        if (luabind::type(val) == LUA_TNUMBER) {
            total += luabind::object_cast<int>(val);
        } else if (luabind::type(val) == LUA_TSTRING) {
            total += static_cast<int>(std::strlen(luabind::object_cast<const char*>(val)));
        }
    }
    return total;
}

static void ThrowRuntimeErr() {
    throw std::runtime_error("boom_from_cpp");
}

// Mirrors CNPL::load / CNPL::load1: declared void in C++, but pushes exported module onto L via object::interpreter()
static void VoidPushExportedModule(const luabind::object& filePath) {
    lua_State* L = filePath.interpreter();
    lua_newtable(L);
    lua_pushliteral(L, "loaded");
    lua_pushboolean(L, 1);
    lua_rawset(L, -3);
}

static void RegisterTestModule(lua_State* L) {
    using namespace luabind;
    luabind::open(L);
    luabind::bind_class_info(L);

    module(L)
    [
        namespace_("MockScene")
        [
            class_<ParaObjectHandle>("ParaObject")
                .def(constructor<>())
                .property("visible", &ParaObjectHandle::GetVisible, &ParaObjectHandle::SetVisible)
                .property("width", &ParaObjectHandle::GetWidth, &ParaObjectHandle::SetWidth)
                .property("height", &ParaObjectHandle::GetHeight, &ParaObjectHandle::SetHeight)
                .property("name", &ParaObjectHandle::GetName, &ParaObjectHandle::SetName)
                .property("text", &ParaObjectHandle::GetText, &ParaObjectHandle::SetText)
                .property("id", &ParaObjectHandle::GetID)
                .def("IsValid", &ParaObjectHandle::IsValid)
                .def("GetID", &ParaObjectHandle::GetID)
                .def("SetPosition", &ParaObjectHandle::SetPosition)
                .def("GetPosition", &ParaObjectHandle::GetPosition, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4))
                .def("SelfRef", &ParaObjectHandle::SelfRef, return_reference_to(_1))
                .def("PlayAnim", &ParaObjectHandle::PlayAnim)
                .def("PlayAnim", &ParaObjectHandle::PlayAnim2)
                .def("TargetBy", &ParaObjectHandle::TargetBy)
                .def("TargetBy", &ParaObjectHandle::TargetByName)
                .def("RawStackAdd", &ParaObjectHandle::RawStackAdd, raw(_3)),

            class_<ParaCharacterHandle>("ParaCharacter")
                .def(constructor<>())
                .def("IsValid", &ParaCharacterHandle::IsValid)
                .def("GetAnimID", &ParaCharacterHandle::GetAnimID),

            class_<PolyBase>("PolyBase")
                .def(constructor<>())
                .def_readwrite("baseVal", &PolyBase::baseVal)
                .def("Compute", &PolyBase::Compute),

            class_<PolyDerived, PolyBase>("PolyDerived")
                .def(constructor<>())
                .def_readwrite("derivedVal", &PolyDerived::derivedVal)
                .def("OnlyDerived", &PolyDerived::OnlyDerived),

            class_<MockSelfPtrNS::SelfPtrAttrHandle>("SelfPtrAttrHandle")
                .def("IsValid", &MockSelfPtrNS::SelfPtrAttrHandle::IsValid)
                .def("GetFieldNum", &MockSelfPtrNS::SelfPtrAttrHandle::GetFieldNum),

            class_<MockSelfPtrNS::SelfPtrPolyBase>("SelfPtrPolyBase")
                .def("EvalTag", &MockSelfPtrNS::SelfPtrPolyBase::EvalTag),

            class_<MockSelfPtrNS::SelfPtrPolyDerived, MockSelfPtrNS::SelfPtrPolyBase>("SelfPtrPolyDerived"),

            def("GetBlockId", &GetBlockId),
            def("GetPlayer", &GetPlayer),
            def("GetChar", &GetChar),
            def("GetSelfPtrAttr", &GetSelfPtrAttr),
            def("GetSelfPtrPoly", &GetSelfPtrPoly),
            def("ClobberNativeStack", &ClobberNativeStack),
            def("activate", &Activate1),
            def("activate", &Activate2),
            def("activate", &Activate3),
            def("SumTable", &SumTableViaIterator),
            def("ThrowRuntimeErr", &ThrowRuntimeErr),
            def("VoidPushExportedModule", &VoidPushExportedModule)
        ]
    ];
}

static void EvalLuaOrDie(lua_State* L, const char* script) {
    if (luaL_dostring(L, script) != 0) {
        std::fprintf(stderr, "[LuabindOptimizationTest] Lua error: %s\n", lua_tostring(L, -1));
        std::exit(1);
    }
}

} // namespace

int main(int argc, char** argv) {
    // Compile-time guarantee that 64-bit pointer_holder and compact_value_holder fit inline
    // inside object_rep without overflowing to std::malloc.
    static_assert(
        sizeof(luabind::detail::pointer_holder<ParaObjectHandle*>) <= luabind::detail::object_rep::kInlineBufferSize,
        "pointer_holder must fit in object_rep inline buffer on 64-bit platforms");
    static_assert(
        sizeof(luabind::detail::compact_value_holder<ParaObjectHandle>) <= luabind::detail::object_rep::kInlineBufferSize,
        "compact_value_holder<ParaObjectHandle> must fit in object_rep inline buffer");
    static_assert(
        sizeof(luabind::detail::compact_pointer_holder<MockSelfPtrNS::SelfPtrAttrHandle>) <= luabind::detail::object_rep::kInlineBufferSize,
        "compact_pointer_holder<16B wrapper> must fit in object_rep inline buffer with zero malloc");

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    RegisterTestModule(L);

    EvalLuaOrDie(L, R"LUA(
        local p = MockScene.GetPlayer()
        assert(p:IsValid() == true)
        assert(p:GetID() == 42)
        assert(p.id == 42)

        local okRo = pcall(function() p.id = 7 end)
        assert(not okRo)

        assert(p.visible == true)
        p.visible = false
        assert(p.visible == false)
        p.visible = true

        p.width = 640
        p.height = 480
        assert(p.width == 640 and p.height == 480)
        p.name = "hero_npc"
        assert(p.name == "hero_npc")
        p.text = "Updated UI Text"
        assert(p.text == "Updated UI Text")

        p:SetPosition(10.5, -20.25, 30.75)
        local x, y, z = p:GetPosition()
        assert(x == 10.5 and y == -20.25 and z == 30.75)

        local same = p:SelfRef()
        assert(same == p)
        assert(p:RawStackAdd(8) == 52)

        assert(p:PlayAnim(5) == 6)
        assert(p:PlayAnim(5, 2.5) == 30)
        assert(p:TargetBy(p) == 142)
        assert(p:TargetBy("hello") == 5)
        assert(MockScene.activate("a.lua") == 5)
        assert(MockScene.activate("a.lua", "msg") == 8)
        assert(MockScene.activate("a.lua", { cmd = "run", count = 7, [1] = 10 }, 3) == 23)

        local s = MockScene.SumTable({ [1] = 10, [2] = 20, alpha = "abcd" })
        assert(s == 134)

        local ch = MockScene.GetChar()
        assert(ch:IsValid() and ch:GetAnimID() == 42)

        -- Regression test for ParaScriptingGlobal.h self-referential get_pointer(T const&)
        local att = MockScene.GetSelfPtrAttr()
        local spoly = MockScene.GetSelfPtrPoly()
        MockScene.ClobberNativeStack(8)
        for i = 1, 500 do
            local tmp = MockScene.GetSelfPtrAttr()
            assert(tmp:IsValid() and tmp:GetFieldNum() == 61)
        end
        collectgarbage("collect")
        MockScene.ClobberNativeStack(8)
        assert(att:IsValid() == true)
        assert(att:GetFieldNum() == 61)
        assert(spoly:EvalTag() == 40)

        local exported = MockScene.VoidPushExportedModule("(gl)script/mock_mod.lua")
        assert(type(exported) == "table" and exported.loaded == true)

        p.customField = 999
        p.customField2 = 888
        assert(p.customField == 999 and p.customField2 == 888)
        ch.customCharTag = "fast_upgraded"
        assert(ch.customCharTag == "fast_upgraded" and ch:GetAnimID() == 42)

        local info = class_info(p)
        assert(info.name == "ParaObject")
        local function contains(list, value)
            for _, item in ipairs(list) do
                if item == value then return true end
            end
            return false
        end
        assert(contains(info.attributes, "visible"))
        assert(contains(class_names(), "ParaObject"))

        local d = MockScene.PolyDerived()
        assert(d:Compute() == 35)
        assert(d:OnlyDerived() == 50)
        d.baseVal = 100
        assert(d:Compute() == 125)

        local co = coroutine.create(function()
            local cp = MockScene.GetPlayer()
            return cp:GetID() + MockScene.GetBlockId(1, 2, 3)
        end)
        local ok, res = coroutine.resume(co)
        assert(ok and res == 42 + MockScene.GetBlockId(1, 2, 3))

        local ok1, err1 = pcall(function() p:SetPosition("bad", 1, 2) end)
        assert(not ok1 and type(err1) == "string")
        local ok2, err2 = pcall(function() MockScene.ThrowRuntimeErr() end)
        assert(not ok2 and string.find(err2, "boom_from_cpp") ~= nil)

        collectgarbage("collect")
    )LUA");

    lua_close(L);

    // Multi-threaded independent lua_State lifecycle check
    std::vector<std::thread> workers;
    for (int t = 0; t < 4; ++t) {
        workers.emplace_back([]() {
            for (int iter = 0; iter < 4; ++iter) {
                lua_State* WL = luaL_newstate();
                luaL_openlibs(WL);
                RegisterTestModule(WL);
                EvalLuaOrDie(WL, R"LUA(
                    local ch = MockScene.GetChar()
                    local acc = 0
                    for i = 1, 500 do
                        acc = acc + MockScene.GetBlockId(i, 64, i) + ch:GetAnimID()
                    end
                    assert(acc > 0)
                )LUA");
                lua_close(WL);
            }
        });
    }
    for (auto& th : workers) th.join();

    bool runBench = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--benchmark") == 0)
            runBench = true;
    }

    if (runBench) {
        lua_State* BL = luaL_newstate();
        luaL_openlibs(BL);
        RegisterTestModule(BL);
        auto t0 = std::chrono::steady_clock::now();
        EvalLuaOrDie(BL, "local ch = MockScene.GetChar(); local s = 0; for i = 1, 2000000 do s = s + ch:GetAnimID() end; assert(s > 0)");
        auto t1 = std::chrono::steady_clock::now();
        double ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / 2000000.0;
        std::printf("[LuabindOptimizationTest] ch:GetAnimID() = %.2f ns/call\n", ns);
        lua_close(BL);
    }

    std::printf("[LuabindOptimizationTest] PASS\n");
    return 0;
}
