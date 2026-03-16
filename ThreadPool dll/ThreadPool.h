#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#ifdef _WIN32
#ifdef ThreadPool_EXPORTS
#define THREADPOOL_API __declspec(dllexport)
#else
#define THREADPOOL_API __declspec(dllimport)
#endif
#else
#ifdef ThreadPool_EXPORTS
#define THREADPOOL_API __attribute__((visibility("default")))
#else
#define THREADPOOL_API
#endif
#endif

class THREADPOOL_API ThreadPool {
public:
  ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
  ~ThreadPool();

  template <typename Func, typename... Args>
  auto Submit(Func &&func, Args &&...args)
      -> std::future<decltype(func(args...))> {
    using return_type = decltype(func(args...));
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    std::future<return_type> result = task->get_future();

    auto wrapper = [task]() { (*task)(); };

    EnqueueReal(std::move(wrapper));
    return result;
  }

private:
  void EnqueueReal(std::function<void()> task);

  struct WorkQueue {
    std::deque<std::function<void()>> tasks;
    std::mutex mtx;
  };

  std::vector<std::unique_ptr<WorkQueue>> queues;
  std::vector<std::thread> workers;

  std::atomic<bool> terminate;
  std::atomic<size_t> submit_index;

  // 用于在没有任务时休眠的工作机制
  std::mutex sleep_mtx;
  std::condition_variable sleep_cv;

  void WorkerRoutine(size_t index);

public:
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool &operator=(ThreadPool &&) = delete;
};
