/**
 * @file Preferences_fake.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-28
 * @brief Fake Arduino's Preferences class for testing
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "Preferences.h"
#include <cstring>
#include <algorithm>

bool Preferences::clear()
{
    _map1.clear();
    _map2.clear();
    _map4.clear();
    for (auto item : _mapBytes)
        item.second.clear();
    _mapBytes.clear();
    return true;
}

bool Preferences::remove(const char *key)
{
    std::string sKey(key);
    _map1.erase(_map1.find(sKey));
    _map2.erase(_map2.find(sKey));
    _map4.erase(_map4.find(sKey));
    _mapBytes.erase(_mapBytes.find(sKey));
    return true;
}

size_t Preferences::putUChar(const char *key, uint8_t value)
{
    std::string sKey(key);
    if (_readOnly)
        return 0;
    _map1[sKey] = value;
    return 1;
}

size_t Preferences::putUShort(const char *key, uint16_t value)
{
    std::string sKey(key);
    if (_readOnly)
        return 0;
    _map2[sKey] = value;
    return 2;
}

size_t Preferences::putInt(const char *key, int32_t value)
{
    std::string sKey(key);
    if (_readOnly)
        return 0;
    _map4[sKey] = value;
    return 4;
}

size_t Preferences::putBool(const char *key, bool value)
{
    std::string sKey(key);
    if (_readOnly)
        return 0;
    _map1[sKey] = (value) ? 1 : 0;
    return 1;
}

size_t Preferences::putBytes(const char *key, const void *value, size_t len)
{
    if (_readOnly)
        return 0;
    std::vector<uint8_t> v;
    std::string sKey(key);
    v.reserve(len);
    for (size_t i = 0; i < len; i++)
        v.push_back(((const uint8_t *)value)[i]);
    _mapBytes[sKey] = v;
    return len;
}

bool Preferences::isKey(const char *key)
{
    std::string sKey(key);
    if (_map1.find(sKey) == _map1.end())
    {
        if (_map2.find(sKey) == _map2.end())
        {
            if (_map4.find(sKey) == _map4.end())
            {
                return (_mapBytes.find(sKey) != _mapBytes.end());
            }
        }
    }
    return true;
}

uint8_t Preferences::getUChar(const char *key, uint8_t defaultValue)
{
    std::string sKey(key);
    auto iter = _map1.find(sKey);
    if (iter != _map1.end())
    {
        return iter->second;
    }
    else
        return defaultValue;
}

uint16_t Preferences::getUShort(const char *key, uint16_t defaultValue)
{
    std::string sKey(key);
    auto iter = _map2.find(sKey);
    if (iter != _map2.end())
    {
        return iter->second;
    }
    else
        return defaultValue;
}

int32_t Preferences::getInt(const char *key, int32_t defaultValue)
{
    std::string sKey(key);
    auto iter = _map4.find(sKey);
    if (iter != _map4.end())
    {
        return iter->second;
    }
    else
        return defaultValue;
}

bool Preferences::getBool(const char *key, bool defaultValue)
{
    std::string sKey(key);
    auto iter = _map1.find(sKey);
    if (iter != _map1.end())
    {
        return (iter->second) != 0;
    }
    else
        return defaultValue;
}

size_t Preferences::getBytesLength(const char *key)
{
    std::string sKey(key);
    auto iter = _mapBytes.find(sKey);
    if (iter != _mapBytes.end())
        return iter->second.size();
    else
        return 0;
}

size_t Preferences::getBytes(const char *key, void *buf, size_t maxLen)
{
    std::string sKey(key);
    auto iter = _mapBytes.find(sKey);
    if (iter != _mapBytes.end())
    {
        uint8_t *dest = (uint8_t *)buf;
        size_t n = (iter->second.size() < maxLen) ? iter->second.size() : maxLen;
        for (size_t i = 0; i < n; i++)
            dest[i] = iter->second[i];
        return n;
    }
    else
        return 0;
}
