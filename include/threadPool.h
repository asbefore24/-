#ifndef THREADPOOL
#define THREADPOOL
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>

using namespace std;

class threadPool {
private:
    vector<thread> threads;
    queue<function<void()>> tasks;
    mutex mtx;
    condition_variable condition;
    bool isStop;
    once_flag flag;

private:
    threadPool() noexcept = default;

    ~threadPool() {
        lock_guard<mutex> lock(mtx);
        isStop = true;

        condition.notify_all();
        for (auto& iter: threads) {
            iter.join();
        }
    }

public:
    static threadPool& GetInstance() {
        static threadPool threadpool;
        return threadpool;
    }

    void init(int num = thread::hardware_concurrency() / 2) noexcept {
        call_once(flag, [this, &num]() {
            for (int i = 0; i < num; i++) {
                threads.emplace_back([this]() {
                    while (true) {
                        unique_lock<mutex> lock(mtx);
                        condition.wait(lock, [this]() {
                            return !tasks.empty() || isStop;
                        });
                        if (isStop && tasks.empty()) {
                            return;
                        }
                        auto task = tasks.front();
                        tasks.pop();
                        lock.unlock();
                        task();
                    }
                });
            }
        });
    }

    template<class F, class... Args>
    void enqueueTask(F &&f, Args&&... args) {
        function<void()> task = bind(std::forward<F>(f), std::forward<Args>(args)...);
        lock_guard<mutex> lock(mtx);
        tasks.emplace(task);
        condition.notify_one();
    }
};

#endif