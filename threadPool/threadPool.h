#ifndef  __THREADPOOL_H__
#define  __THREADPOOL_H__

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <future>
#include <functional>
#include <stdexcept>
#include <condition_variable>


#define  MAX_THREAD_MAX  256

class threadPool {
    public:
        threadPool(unsigned int num = 4):stoped(false)
        {
            idleThreadNum = num < 1 ? 1 : num;
            for(int i=0; i<idleThreadNum; ++i){
                pool.emplace_back(
                    [this]
                    {
                        while(!this->stoped){
                            std::function<void()> task;
                            {
                                std::unique_lock<std::mutex> lock(this->m_lock);
                                this->cv_task.wait(lock, [this]{return this->stoped.load() || !this->tasks.empty();});
                                if(this->stoped && this->tasks.empty())
                                    return;
                                task = std::move(this->tasks.front());
                                this->tasks.pop();
                            }
                            idleThreadNum--;
                            task();
                            idleThreadNum++;
                        }
                    }
                );
            }
        }

        ~threadPool()
        {
            stoped.store(true);
            cv_task.notify_all();
            for(auto& thread : pool){
                if(thread.joinable())
                    thread.join();
            }
        }

        template <typename F, typename... Args>
        auto commit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
        {
            if(stoped.load())
                throw std::runtime_error("commit on ThreadPool is stoped.");

            using RetType = decltype(f(args...));
            auto task = std::make_shared<std::packaged_task<RetType()> >(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );
            std::future<RetType> future = task->get_future();
            {
                std::lock_guard<std::mutex> lock(m_lock);
                tasks.emplace(
                        [task]
                        {
                            (*task)();
                        }
                        );
            }
            cv_task.notify_one();

            return future;
        }

        int idleCount(){ return idleThreadNum; };

    private:
        using Task = std::function<void()>;
        // thread pool
        std::vector<std::thread> pool;
        // tasks queue
        std::queue<Task> tasks;
        // sync
        std::mutex m_lock;
        //
        std::condition_variable cv_task;
        // if stop and exit
        std::atomic<bool>  stoped; 
        // free threads num
        std::atomic<int>  idleThreadNum;
};



#endif  //__THREADPOOL_H__
