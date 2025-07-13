#ifndef APTOS_UTILS_H
#define APTOS_UTILS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <mbedtls/sha256.h>
#include <mbedtls/base64.h>

class AptosUtils {
public:
    // Hex conversion utilities
    static String bytesToHex(const uint8_t* bytes, size_t length);
    static bool hexToBytes(const String& hex, uint8_t* bytes, size_t maxLength);
    static String toHexString(uint64_t value);
    
    // Base64 utilities
    static String bytesToBase64(const uint8_t* bytes, size_t length);
    static bool base64ToBytes(const String& base64, uint8_t* bytes, size_t& length);
    
    // Address utilities
    static String padHexAddress(const String& address);
    static bool isValidHexString(const String& hex);
    static String removeHexPrefix(const String& hex);
    
    // Hash utilities
    static String sha256Hash(const String& input);
    static String sha256Hash(const uint8_t* data, size_t length);
    
    // BCS (Binary Canonical Serialization) utilities
    static String serializeU8(uint8_t value);
    static String serializeU64(uint64_t value);
    static String serializeString(const String& str);
    static String serializeAddress(const String& address);
    static String serializeVector(const JsonArray& array);
    
    // Transaction utilities
    static uint64_t getCurrentTimestamp();
    static String generateTransactionHash(const JsonDocument& transaction);
    
    // Network utilities
    static bool isValidUrl(const String& url);
    static String extractDomain(const String& url);
    
    // Memory utilities
    static void secureZero(uint8_t* data, size_t length);
    static void printHex(const uint8_t* data, size_t length, const String& label = "");
    
    // JSON utilities
    static bool parseJsonSafely(const String& jsonString, JsonDocument& doc);
    static String getJsonString(const JsonDocument& doc, const String& key, const String& defaultValue = "");
    static uint64_t getJsonUint64(const JsonDocument& doc, const String& key, uint64_t defaultValue = 0);
    
    // Validation utilities
    static bool isValidAmount(uint64_t amount);
    static bool isValidGasPrice(uint64_t gasPrice);
    static bool isValidSequenceNumber(uint64_t sequenceNumber);
    
private:
    static const char* hexChars;
    static char nibbleToHex(uint8_t nibble);
    static uint8_t hexToNibble(char hex);
};

#endif
