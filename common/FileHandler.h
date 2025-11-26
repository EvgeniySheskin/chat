#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <filesystem>

namespace chat
{
    /**
     * @brief Класс для безопасной работы с файлами.
     * Инкапсулирует операции открытия, записи и закрытия файла с использованием мьютекса.
     */
    class FileHandler
    {
    public:
        explicit FileHandler(const std::string& filename);
        ~FileHandler();

        bool open();
        void close();
        bool write(const std::string& data);
        bool isOpen() const;

    private:
        std::string m_filename;
        std::ofstream m_file_stream;
        mutable std::mutex m_file_mutex;
        std::filesystem::path m_filepath;
    };
}