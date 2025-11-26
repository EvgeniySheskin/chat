#include "../common/Logger.h"
#include <sstream>
#include <chrono>
#include <iomanip>

namespace chat
{
    Logger::Logger(const std::string& log_filename)
        : m_log_filename(log_filename), m_file_handler(log_filename), m_stop_flag(false) {}

    Logger::~Logger()
    {
        stop();
        if (m_logger_future.valid())
        {
            m_logger_future.wait();
        }
    }

    void Logger::start()
    {
        if (m_logger_future.valid())
        {
            return;
        }
        m_stop_flag = false;
        m_file_handler.open();
        // «апускаем фоновую задачу с циклом логировани€
        m_logger_future = std::async(std::launch::async, &Logger::loggingLoop, this);
    }

    void Logger::stop()
    {
        if (m_stop_flag.exchange(true))
        {
            return;
        }
        m_cv.notify_all();
        if (m_logger_future.valid())
        {
            m_logger_future.wait();
        }
        // «аписываем оставшиес€ сообщени€ перед закрытием
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        while (!m_log_queue.empty())
        {
            std::string log_entry = m_log_queue.front();
            m_log_queue.pop();
            m_file_handler.write(log_entry);
        }
    }

    void Logger::log(LogLevel level, const std::string& message)
    {
        std::string formatted_message = "[" + getCurrentTimestamp() + "] [" + logLevelToString(level) + "] " + message + "\n";
        {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            m_log_queue.push(formatted_message);
        }
        m_cv.notify_one();
    }

    void Logger::loggingLoop()
    {
        while (!m_stop_flag.load())
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            m_cv.wait(lock, [this] { return !m_log_queue.empty() || m_stop_flag.load(); });

            if (m_stop_flag.load() && m_log_queue.empty())
            {
                break;
            }

            while (!m_log_queue.empty())
            {
                std::string log_entry = m_log_queue.front();
                m_log_queue.pop();
                lock.unlock();
                m_file_handler.write(log_entry);
                lock.lock();
            }
        }
    }
}