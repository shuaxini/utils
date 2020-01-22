#include "threadPool.h"
#include <iostream>

void function(int slp)
{
    std::cout << "hello, function " << std::this_thread::get_id()  << std::endl;;
    if(slp > 0) {
        std::cout << "---function sleep " << slp << " seconds." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(slp));
    }
}

int main(int argc, char* argv[])
{
    std::cout << "threadPool" << std::endl;

    threadPool executor(50);
    for(int i=0; i<100; ++i){
        std::cout << "i=" << i << std::endl;
        std::future<void> ff = executor.commit(function, i);
    }

    return 0;
}
