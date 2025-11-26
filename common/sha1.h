#ifndef SHA1_HASHER_H
#define SHA1_HASHER_H

#include <string>
#include <cstdint> 

namespace chat 
{

    /**
     * @brief Класс для вычисления хеша SHA-1.
     */
    class sha1 
    {
    public:
        /**
         * @brief Вычисляет SHA-1 хеш для строки.
         * @param input Входная строка.
         * @return Хеш в шестнадцатеричном представлении (строка длиной 40 символов).
         */
        static std::string hash(const std::string& input);

    private:
        // Размер блока данных в байтах (512 бит)
        static const size_t BLOCK_SIZE = 64;
        // Размер хеша в байтах (160 бит)
        static const size_t HASH_SIZE = 20;

        // Внутренние типы
        using HashState = uint32_t[5];
        using MessageBlock = uint8_t[BLOCK_SIZE];

        // Вспомогательные функции
        static uint32_t leftRotate(uint32_t value, unsigned int shift);
        static void processBlock(const MessageBlock& block, HashState& state);
        static std::string toHex(const uint8_t* data, size_t len);

        // Константы для раундов
        static const uint32_t K[4];
    };

} 

#endif