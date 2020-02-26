#ifndef  _SINGLETON_H_
#define  _SINGLETON_H_

template <typename TYPE>
class singleton
{
public:
    static TYPE* instance() {
        static TYPE obj;
        return &obj;
    }

protected:
    singleton() {}
    ~singleton() {}

private:
    singleton(singleton&) {}
    singleton(const singleton&) {}
    singleton& operator= (singleton&) {}
    singleton& operator= (const singleton&) {}

};

class NonCopyable
{
protected:
    NonCopyable() {}
    ~NonCopyable() {}

private:  
    NonCopyable(NonCopyable&) = delete;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator= (NonCopyable&) = delete;
    NonCopyable& operator= (const NonCopyable&) = delete;
};


#endif  //_SINGLETON_H_
