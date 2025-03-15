/**
 * @file Preferences.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-28
 * @brief Fake Arduino's Preferences class for testing
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

#include <cinttypes>
#include <map>
#include <vector>
#include <string>

class Preferences
{
private:
    inline static std::map<std::string, uint8_t> _map1;
    inline static std::map<std::string, uint16_t> _map2;
    inline static std::map<std::string, int32_t> _map4;
    inline static std::map<std::string, std::vector<uint8_t>> _mapBytes;
    bool _readOnly;

public:
    Preferences() { _readOnly = false; }
    ~Preferences() {}

    bool begin(const char *name, bool readOnly = false, const char *partition_label = NULL)
    {
        _readOnly = readOnly;
        return true;
    }

    void end() {};

    bool clear();
    bool remove(const char *key);

    size_t putUChar(const char *key, uint8_t value);
    size_t putUShort(const char *key, uint16_t value);
    size_t putInt(const char *key, int32_t value);
    size_t putBool(const char *key, bool value);
    size_t putBytes(const char *key, const void *value, size_t len);

    bool isKey(const char *key);
    uint8_t getUChar(const char *key, uint8_t defaultValue = 0);
    uint16_t getUShort(const char *key, uint16_t defaultValue = 0);
    int32_t getInt(const char *key, int32_t defaultValue = 0);
    bool getBool(const char *key, bool defaultValue = false);

    size_t getBytesLength(const char *key);
    size_t getBytes(const char *key, void *buf, size_t maxLen);
};
