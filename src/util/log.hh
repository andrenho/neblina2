#ifndef LOG_HH
#define LOG_HH

#include <string>

#include <format>
#include <mutex>
#include <iostream>
#include <utility>

std::string logging_color = "0";
std::string service_name;
bool        logging_verbose = false;

static std::mutex mutex_;

template <typename... Args>
void DBG(std::format_string<Args...> fmt, Args&&... a) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!logging_verbose)
        return;
    std::cout << std::format("\e[{}m{:13}: ", logging_color, service_name) + std::format(fmt, std::forward<Args>(a)...) + "\e[0m\n";
}

template <typename... Args>
void LOG(std::format_string<Args...> fmt, Args&&... a) {
    std::unique_lock<std::mutex> lock(mutex_);
    std::cout << std::format("\e[{}m{:13}: ", logging_color, service_name) + std::format(fmt, std::forward<Args>(a)...) + "\e[0m\n";
}

template <typename... Args>
void ERR(std::format_string<Args...> fmt, Args&&... a) {
    std::unique_lock<std::mutex> lock(mutex_);
    std::cerr << service_name + ": " + std::format(fmt, std::forward<Args>(a)...) + "\n";
}

#endif //LOG_HH
