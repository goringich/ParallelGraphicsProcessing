#include <string>
#include <iostream>
#include <chrono>
#include <algorithm>

class LogDuration {
public:
    LogDuration(std::string id, bool enabled = true)
        : id_(id.empty() ? "Unnamed operation" : std::move(id)), enabled_(enabled) {
        std::replace(id_.begin(), id_.end(), '\n', '_');
    }

    ~LogDuration() {
        if (enabled_) {
            const auto end_time = std::chrono::steady_clock::now();
            const auto dur = end_time - start_time_;
            std::cout << id_ << ": ";
            std::cout << "operation time"
                      << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count()
                      << " ms" << std::endl;
        }
    }

    LogDuration(const LogDuration&) = delete;
    LogDuration& operator=(const LogDuration&) = delete;

private:
    const std::string id_;
    const bool enabled_;
    const std::chrono::steady_clock::time_point start_time_ = std::chrono::steady_clock::now();
};
