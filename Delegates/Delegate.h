#pragma once

#include <cstring>
#include <functional>
#include <mutex>
#include <vector>

template <typename... Args> class Delegate {
public:
  using DelegateHandle = std::size_t;

private:
  std::vector<std::pair<DelegateHandle, std::function<void(Args...)>>>
      functions;
  mutable std::mutex mutex;
  DelegateHandle nextHandle = 1;

public:
  Delegate() = default;
  Delegate(const Delegate &other) = delete;
  Delegate &operator=(const Delegate &other) = delete;
  Delegate(Delegate &&) noexcept = default;
  Delegate &operator=(Delegate &&) noexcept = default;
  ~Delegate() = default;

  DelegateHandle operator+=(const std::function<void(Args...)> &func) {
    std::lock_guard<std::mutex> lock(mutex);
    functions.push_back({nextHandle++, func});
    return functions.size() - 1; // Return the index as a handle
  }

  void operator-=(DelegateHandle handle) {
    std::lock_guard<std::mutex> lock(mutex);
    functions.erase(std::remove_if(functions,
                                   [handle](const auto &pair) {
                                     return pair.first == handle;
                                   }),
                    functions.end());
  }

  std::vector<DelegateHandle> operator+=(const Delegate &other) {
    std::vector<DelegateHandle> handles;
    if (this == &other)
      return handles; // Avoid self-assignment
    std::scoped_lock lock(mutex, other.mutex);
    for (const auto &pair : other.functions) {
      functions.push_back(pair); // Add each function from the other delegate
      handles.push_back(pair.first);
    }
    return handles;
  }

  void Clear() {
    std::lock_guard<std::mutex> lock(mutex);
    functions.clear();
  }

  void operator()(Args... args) {
    std::vector<std::function<void(Args...)>> funcs;
    {
      std::lock_guard<std::mutex> lock(mutex);
      funcs = functions;
    }

    for (const auto &func : funcs) {
      func(args...);
    }
  }
};