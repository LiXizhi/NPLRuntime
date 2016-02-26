#pragma once

class Mutex {
    CRITICAL_SECTION cs;
public:
    Mutex() { InitializeCriticalSection(&cs); }
    ~Mutex() { DeleteCriticalSection(&cs); }

    void m_lock() { EnterCriticalSection(&cs); }
    void m_unlock() { LeaveCriticalSection(&cs); }
};


class Lock {
    Mutex &m;
    BOOL bLocked;

public:
    Lock(Mutex *pMutex)
        : m(*pMutex), bLocked(TRUE)
    {
        m.m_lock();
    }

    Lock(Mutex &input_m) 
        : m(input_m), bLocked(TRUE)
    {
        m.m_lock();
    }
    ~Lock() { if (bLocked) unlock(); }

    void lock() {
        if (!bLocked) {
            bLocked = TRUE;
            m.m_lock();
        }
    }
    void unlock() {
        if (bLocked) {
            bLocked = FALSE;
            m.m_unlock();
        }
    }

    BOOL islocked() const { return bLocked; }
};

