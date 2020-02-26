#include <iostream>
#include <string>

#include "singleton.h"

class Single: public singleton<Single>
{
public:
    int getnum(){
        return m_num;
    }

protected:
    Single(int num=10):m_num(num) {}
    ~Single() {}

    friend class singleton<Single>;

private:
    int m_num;
};


class NonCopy: public NonCopyable
{
public:
    NonCopy(int num):m_num(num) {}
    ~NonCopy() {}

    int getnum() {
        return m_num;
    }

private:
    int m_num;
};


int main(int argc, char* argv[])
{
    Single* A = Single::instance();
    std::cout << "Num:"  <<  A->getnum() << std::endl; 
    //Single B(*A); 
    
    NonCopy N(10);
    std::cout << "Num:"  <<  N.getnum() << std::endl; 
    
    //NonCopy N1(N);
    //
    //NonCopy N2(10);
    //N2 = N;

    return 0;
}
