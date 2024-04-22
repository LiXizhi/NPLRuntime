#ifndef __COROUTINE_THREAD_H__
#define __COROUTINE_THREAD_H__

#ifdef EMSCRIPTEN_SINGLE_THREAD

#include <coroutine>
#include <functional>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <iostream>

#define CO_ASYNC CoroutineThread::Coroutine
#define CO_SLEEP(co_thread, ms) { co_thread->AsyncSleep(ms); co_yield; }
#define CO_AWAIT(expr) { auto _v_corountine = (expr); while (!_v_corountine.IsDone()) { co_await _v_corountine; _v_corountine.Resume();} }
#define CO_RETURN co_return

class CoroutineThread
{
public:
    class Coroutine
    {
    public:
        class promise_type
        {
        public:
            Coroutine get_return_object() { return Coroutine(std::coroutine_handle<promise_type>::from_promise(*this)); }
            static std::suspend_always initial_suspend() noexcept { return {}; }
            static std::suspend_always final_suspend() noexcept { return {}; }
            static void unhandled_exception() { throw; }
            void return_void() {}
        };
    public:
        Coroutine() = default;
        Coroutine(const Coroutine&) = delete;
        Coroutine &operator=(const Coroutine&) = delete;
        ~Coroutine() { if (m_coroutine != nullptr) m_coroutine.destroy(); }
        Coroutine(std::coroutine_handle<promise_type> coroutine) : m_coroutine(coroutine) {}
        Coroutine(Coroutine&& other) { StdMove(&other); }
        Coroutine& operator=(Coroutine &&other) { StdMove(&other); return *this; }

        void Resume() { if (m_coroutine != nullptr) m_coroutine.resume(); }
        bool IsDone() { if (m_coroutine != nullptr) return m_coroutine.done(); return true; }

        bool await_ready() { return IsDone(); }
        void await_suspend(std::coroutine_handle<> h) {}
        void await_resume() {}

    private:
        inline void StdMove(Coroutine* other) { if (other == this) return; m_coroutine = other->m_coroutine; other->m_coroutine = nullptr; }
    protected:
        std::coroutine_handle<promise_type> m_coroutine;
    };

public:
    static CoroutineThread* StartCoroutineThread(std::function<CO_ASYNC(CoroutineThread*)> thread_func, void* thread_data)
    {
        return CreateCoroutineThread(thread_func, thread_data)->Start();
    }

    static void Tick()
    {
        auto coroutine_threads = GetCoroutineThreads();
        auto it = coroutine_threads->begin();
        while (it != coroutine_threads->end())
        {
            auto coroutine_thread = it->second;
            if (coroutine_thread->IsFinished())
            {
                // it = coroutine_threads->erase(it);
                it++;
            }
            else
            {
                coroutine_thread->Rusume();
                it++;
            }
        }
    }
public:
    CoroutineThread(std::function<CO_ASYNC(CoroutineThread*)> thread_func = nullptr, void* thread_data = nullptr)
    {
        m_thread_data = thread_data;
        m_thread_func = thread_func;
        m_state = CoroutineThreadState_Inited;
        m_sleep_timestmap = 0;
    }

    ~CoroutineThread()
    {
        GetCoroutineThreads()->erase(this);
    }

    bool joinable() { return false; }
    void join() {}
    void detach() {}
    template<typename TimeDuration>
    bool timed_join(TimeDuration const& rel_time) {return true;}
    
    inline void* GetThreadData() { return m_thread_data; }
    inline bool IsRunning() { return m_state == CoroutineThreadState_Running; }
    inline bool IsFinished() { return m_state == CoroutineThreadState_Finished; }

    CO_ASYNC Sleep(unsigned long long timestmap)
    {
        m_sleep_timestmap = GetTimeStamp() + timestmap; 
        co_await std::suspend_never{};
    }

    CoroutineThread* Start() 
    { 
        if (m_state != CoroutineThreadState_Inited) return this;

        m_state = CoroutineThreadState_Running; 
        m_coroutine = StartCoroutine(); 

        return this;
    }

    void Stop() 
    {
        if (m_state == CoroutineThreadState_Inited) 
        {
            m_state = CoroutineThreadState_Finished;
        }
        else if (m_state == CoroutineThreadState_Running)
        {
            m_state = CoroutineThreadState_Stoped; 
        }
    }

    void AsyncSleep(unsigned long long timestmap) { m_sleep_timestmap = GetTimeStamp() + timestmap; }

    void Rusume() 
    { 
        if (m_state == CoroutineThreadState_Inited || m_state == CoroutineThreadState_Finished) return;
        if (m_sleep_timestmap == 0) return m_coroutine.Resume();
        unsigned long long cur_timestmap = GetTimeStamp();
        if (cur_timestmap < m_sleep_timestmap) return;
        m_sleep_timestmap = 0;
        m_coroutine.Resume();
    }
private:

    CO_ASYNC StartCoroutine()
    {
        if (m_thread_func != nullptr)
        {
            CO_AWAIT(m_thread_func(this));
        }
        m_state = CoroutineThreadState_Finished;
    }
private:
    static unsigned long long GetTimeStamp()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    static std::shared_ptr<CoroutineThread> CreateCoroutineThread(std::function<CO_ASYNC(CoroutineThread*)> thread_func, void* thread_data)
    {
        std::shared_ptr<CoroutineThread> coroutine_thread = std::make_shared<CoroutineThread>(thread_func, thread_data);
        GetCoroutineThreads()->insert_or_assign(coroutine_thread.get(), coroutine_thread);
        return coroutine_thread;
    }
    static std::unordered_map<void*, std::shared_ptr<CoroutineThread>>* GetCoroutineThreads()
    {
        static std::unordered_map<void*, std::shared_ptr<CoroutineThread>> s_coroutine_threads;
        return &s_coroutine_threads;
    } 

    static const unsigned int CoroutineThreadState_Inited = 0;
    static const unsigned int CoroutineThreadState_Running = 1;
    static const unsigned int CoroutineThreadState_Stoped = 2;
    static const unsigned int CoroutineThreadState_Finished = 3;
protected:
    CO_ASYNC m_coroutine;
    void* m_thread_data;
    std::function<CO_ASYNC(CoroutineThread*)> m_thread_func;
    unsigned int m_state;
    unsigned long long m_sleep_timestmap;
};

#endif

#endif