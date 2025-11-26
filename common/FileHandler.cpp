#include "../common/FileHandler.h"
#include <iostream>
#include <filesystem>

namespace chat
{
    FileHandler::FileHandler(const std::string& filename)
        : m_filename(filename), m_filepath(filename) {}

    FileHandler::~FileHandler()
    {
        close();
    }

    bool FileHandler::open()
    {
        std::lock_guard<std::mutex> lock(m_file_mutex);
        if (m_file_stream.is_open())
        {
            return true;
        }
        m_file_stream.open(m_filepath, std::ios::app);
        if (!m_file_stream.is_open())
        {
            std::cerr << "Failed to open log file: " << m_filepath << std::endl;
            return false;
        }
        return true;
    }

    void FileHandler::close()
    {
        std::lock_guard<std::mutex> lock(m_file_mutex);
        if (m_file_stream.is_open())
        {
            m_file_stream.close();
        }
    }

    bool FileHandler::write(const std::string& data)
    {
        std::lock_guard<std::mutex> lock(m_file_mutex);
        if (m_file_stream.is_open())
        {
            m_file_stream << data << std::flush;
            return m_file_stream.good();
        }
        return false;
    }

    bool FileHandler::isOpen() const
    {
        std::lock_guard<std::mutex> lock(m_file_mutex);
        return m_file_stream.is_open();
    }
}