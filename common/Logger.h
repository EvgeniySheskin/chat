#pragma once

#include "FileHandler.h"
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <future>
#include <memory>
#include <sstream>
#include <chrono>
#include <iomanip>
namespace chat
{
    /**
     * @brief Уровни логирования
     */
    enum class LogLevel
    {
        DEBUG,
        INFO,
        WARNING,
        L_ERROR
    };

    /**
     * @brief Класс логгера, работающий в отдельном потоке.
     * Принимает сообщения из основного потока, буферизует их и записывает в файл.
     */
    class Logger
    {
    public:
        explicit Logger(const std::string& log_filename);
        ~Logger();

        // Запускает фоновый поток логирования
        void start();
        // Останавливает фоновый поток
        void stop();

        // Метод для добавления сообщения в очередь
        void log(LogLevel level, const std::string& message);

        void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
        void info(const std::string& message) { log(LogLevel::INFO, message); }
        void warning(const std::string& message) { log(LogLevel::WARNING, message); }
        void error(const std::string& message) { log(LogLevel::L_ERROR, message); }

    private:
        void loggingLoop(); // Основной цикл фонового потока

        std::string m_log_filename;
        FileHandler m_file_handler;
        std::queue<std::string> m_log_queue;
        mutable std::mutex m_queue_mutex;
        std::condition_variable m_cv;
        std::thread m_logger_thread;
        std::atomic<bool> m_stop_flag;
        std::future<void> m_logger_future;
    };

    // Вспомогательная функция для форматирования времени
    inline std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        std::tm time_tm = {};
#ifdef _WIN32
        if (localtime_s(&time_tm, &time_t) != 0)
        {
            oss << "0000-00-00 00:00:00.000";
            return oss.str();
        }
#else
        if (localtime_r(&time_t, &time_tm) == nullptr)
        {
            oss << "0000-00-00 00:00:00.000";
            return oss.str();
        }
#endif

        char time_buffer[20]; // "YYYY-MM-DD HH:MM:SS"
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &time_tm);
        oss << time_buffer << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

    // Вспомогательная функция для преобразования уровня в строку
    inline std::string logLevelToString(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::L_ERROR:   return "L_ERROR";
        default:                return "UNKNOWN";
        }
    }
}