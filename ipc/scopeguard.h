#ifndef _SCOPEGUARD_H_
#define _SCOPEGUARD_H_


// the class help to dealing with the aftermaeh
// just like shared_ptr/unique_ptr
template <typename FUNC>
class ScopeGuard
{
public:
    ScopeGuard(FUNC f):m_f(std::move(f)), m_active(true){}
    ScopeGuard(ScopeGuard && rhs) :
        m_f(std::move(rhs.m_f)),
        m_active(rhs.m_active) {
        rhs.dismiss();
    }

    ~ScopeGuard()
    {
        if(m_active) m_f();
    }

    ScopeGuard() = delete;
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    void dismiss() { m_active = false; }

private:
    FUNC m_f;
    bool m_active;

};

template <typename FUNC>
ScopeGuard<FUNC> makeScopeGuard(FUNC f)
{
    return ScopeGuard<FUNC>(std::move(f));
}



#endif //_SCOPEGUARD_H_
