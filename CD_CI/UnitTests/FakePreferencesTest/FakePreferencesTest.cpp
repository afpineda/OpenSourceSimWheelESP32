
/**
 * @file FakePreferencesTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-28
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */
#include "Preferences.h"

#include <iostream>
#include "cd_ci_assertions.hpp"
#include <cassert>
#include <cstring>

const char *k1 = "k1";
const char *k2 = "k2";
const char *k3 = "k3";
const char *k4 = "k4";

const char *strData = "1234567890";
#define strDataLen 11

int main()
{
    Preferences prefs;
    prefs.begin(k4);

    uint8_t v1;
    uint16_t v2;
    int32_t v4;
    char bytes[32];
    bool vb;
    size_t s;

    vb = prefs.isKey(k1);
    assert<bool>::equals("empty isKey 1", false, vb);
    vb = prefs.isKey(k2);
    assert<bool>::equals("empty isKey 2", false, vb);
    vb = prefs.isKey(k3);
    assert<bool>::equals("empty isKey 3", false, vb);
    vb = prefs.isKey(k4);
    assert<bool>::equals("empty isKey 4", false, vb);

    prefs.putUChar(k1, 0xAA);
    vb = prefs.isKey(k1);
    assert<bool>::equals("putUchar isKey", true, vb);
    v1 = prefs.getUChar(k1, 0xFF);
    assert<uint8_t>::equals("getUchar isKey", 0xAA, v1);

    s = prefs.putBytes(k2, strData, strDataLen);
    assert<size_t>::equals("putBytes return size", strDataLen, s);
    s = prefs.getBytesLength(k2);
    assert<size_t>::equals("getBytesLength", strDataLen, s);
    s = prefs.getBytes(k2, bytes, 30);
    assert<size_t>::equals("getBytes return size", strDataLen, s);
    vb = (std::strcmp(bytes, strData) == 0);
    assert(vb && "putBytes != getBytes");

    prefs.putInt(k3, 0xAAAAAAAA);
    vb = prefs.isKey(k3);
    assert<bool>::equals("putInt isKey", true, vb);
    v4 = prefs.getInt(k3, 0xFFFFFFFF);
    assert<int>::equals("getUchar isKey", 0xAAAAAAAA, v4);

    vb = prefs.isKey(k1);
    assert<bool>::equals("empty isKey 1", true, vb);
    vb = prefs.isKey(k2);
    assert<bool>::equals("empty isKey 2", true, vb);
    vb = prefs.isKey(k3);
    assert<bool>::equals("empty isKey 3", true, vb);
    vb = prefs.isKey(k4);
    assert<bool>::equals("empty isKey 4", false, vb);

    return 0;
}