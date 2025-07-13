#include "AptosUtils.h"

const char* AptosUtils::hexChars = "0123456789abcdef";

String AptosUtils::bytesToHex(const uint8_t* bytes, size_t length) {
    String result;
    result.reserve(length * 2);
    
    for (size_t i = 0; i < length; i++) {
        result += nibbleToHex(bytes[i] >> 4);
        result += nibbleToHex(bytes[i] & 0x0F);
    }
    
    return result;
}

bool AptosUtils::hexToBytes(const String& hex, uint8_t* bytes, size_t maxLength) {
    String cleanHex = removeHexPrefix(hex);
    
    if (cleanHex.length() % 2 != 0) {
        return false; // Invalid hex length
    }
    
    size_t byteLength = cleanHex.length() / 2;
    if (byteLength > maxLength) {
        return false; // Buffer too small
    }
    
    for (size_t i = 0; i < byteLength; i++) {
        uint8_t high = hexToNibble(cleanHex.charAt(i * 2));
        uint8_t low = hexToNibble(cleanHex.charAt(i * 2 + 1));
        
        if (high == 0xFF || low == 0xFF) {
            return false; // Invalid hex character
        }
        
        bytes[i] = (high << 4) | low;
    }
    
    return true;
}

String AptosUtils::toHexString(uint64_t value) {
    String result = "0x";
    
    if (value == 0) {
        result += "0";
        return result;
    }
    
    // Convert to hex without leading zeros
    char buffer[17];
    sprintf(buffer, "%llx", value);
    result += buffer;
    
    return result;
}

String AptosUtils::bytesToBase64(const uint8_t* bytes, size_t length) {
    size_t outputLen = 0;
    
    // Calculate required buffer size
    mbedtls_base64_encode(nullptr, 0, &outputLen, bytes, length);
    
    char* buffer = new char[outputLen + 1];
    int ret = mbedtls_base64_encode((unsigned char*)buffer, outputLen, &outputLen, bytes, length);
    
    String result;
    if (ret == 0) {
        buffer[outputLen] = '\0';
        result = String(buffer);
    }
    
    delete[] buffer;
    return result;
}

bool AptosUtils::base64ToBytes(const String& base64, uint8_t* bytes, size_t& length) {
    size_t outputLen = 0;
    
    int ret = mbedtls_base64_decode(bytes, length, &outputLen, 
                                   (const unsigned char*)base64.c_str(), base64.length());
    
    if (ret == 0) {
        length = outputLen;
        return true;
    }
    
    return false;
}

String AptosUtils::padHexAddress(const String& address) {
    String addr = removeHexPrefix(address);
    addr.toLowerCase();
    
    // Pad to 64 characters
    while (addr.length() < 64) {
        addr = "0" + addr;
    }
    
    return "0x" + addr;
}

bool AptosUtils::isValidHexString(const String& hex) {
    String cleanHex = removeHexPrefix(hex);
    
    for (int i = 0; i < cleanHex.length(); i++) {
        char c = cleanHex.charAt(i);
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    
    return true;
}

String AptosUtils::removeHexPrefix(const String& hex) {
    if (hex.startsWith("0x") || hex.startsWith("0X")) {
        return hex.substring(2);
    }
    return hex;
}

String AptosUtils::sha256Hash(const String& input) {
    return sha256Hash((const uint8_t*)input.c_str(), input.length());
}

String AptosUtils::sha256Hash(const uint8_t* data, size_t length) {
    uint8_t hash[32];
    
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0); // 0 = SHA256
    mbedtls_sha256_update(&ctx, data, length);
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);
    
    return bytesToHex(hash, 32);
}

String AptosUtils::serializeU8(uint8_t value) {
    return bytesToHex(&value, 1);
}

String AptosUtils::serializeU64(uint64_t value) {
    uint8_t bytes[8];
    
    // Little-endian encoding
    for (int i = 0; i < 8; i++) {
        bytes[i] = (value >> (i * 8)) & 0xFF;
    }
    
    return bytesToHex(bytes, 8);
}

String AptosUtils::serializeString(const String& str) {
    String result = serializeU64(str.length()); // Length prefix
    
    for (int i = 0; i < str.length(); i++) {
        result += serializeU8((uint8_t)str.charAt(i));
    }
    
    return result;
}

String AptosUtils::serializeAddress(const String& address) {
    String normalized = padHexAddress(address);
    String cleanHex = removeHexPrefix(normalized);
    
    uint8_t bytes[32];
    if (hexToBytes(cleanHex, bytes, 32)) {
        return bytesToHex(bytes, 32);
    }
    
    return "";
}

String AptosUtils::serializeVector(const JsonArray& array) {
    String result = serializeU64(array.size()); // Length prefix
    
    for (JsonVariant item : array) {
        if (item.is<String>()) {
            result += serializeString(item.as<String>());
        } else if (item.is<int>()) {
            result += serializeU64(item.as<uint64_t>());
        }
        // Add more types as needed
    }
    
    return result;
}

uint64_t AptosUtils::getCurrentTimestamp() {
    // Return current time in seconds since epoch
    // For ESP32, you might need to use NTP or RTC
    return millis() / 1000 + 1640995200; // Approximate timestamp
}

String AptosUtils::generateTransactionHash(const JsonDocument& transaction) {
    String txnString;
    serializeJson(transaction, txnString);
    return sha256Hash(txnString);
}

bool AptosUtils::isValidUrl(const String& url) {
    return url.startsWith("http://") || url.startsWith("https://");
}

String AptosUtils::extractDomain(const String& url) {
    String domain = url;
    
    // Remove protocol
    if (domain.startsWith("https://")) {
        domain = domain.substring(8);
    } else if (domain.startsWith("http://")) {
        domain = domain.substring(7);
    }
    
    // Remove path
    int slashIndex = domain.indexOf('/');
    if (slashIndex != -1) {
        domain = domain.substring(0, slashIndex);
    }
    
    return domain;
}

void AptosUtils::secureZero(uint8_t* data, size_t length) {
    volatile uint8_t* ptr = data;
    for (size_t i = 0; i < length; i++) {
        ptr[i] = 0;
    }
}

void AptosUtils::printHex(const uint8_t* data, size_t length, const String& label) {
    if (!label.isEmpty()) {
        Serial.print(label + ": ");
    }
    
    for (size_t i = 0; i < length; i++) {
        if (data[i] < 16) Serial.print("0");
        Serial.print(data[i], HEX);
        if (i < length - 1) Serial.print(" ");
    }
    Serial.println();
}

bool AptosUtils::parseJsonSafely(const String& jsonString, JsonDocument& doc) {
    DeserializationError error = deserializeJson(doc, jsonString);
    return error == DeserializationError::Ok;
}

String AptosUtils::getJsonString(const JsonDocument& doc, const String& key, const String& defaultValue) {
    if (doc.containsKey(key) && doc[key].is<String>()) {
        return doc[key].as<String>();
    }
    return defaultValue;
}

uint64_t AptosUtils::getJsonUint64(const JsonDocument& doc, const String& key, uint64_t defaultValue) {
    if (doc.containsKey(key)) {
        if (doc[key].is<String>()) {
            return strtoull(doc[key].as<String>().c_str(), nullptr, 10);
        } else if (doc[key].is<uint64_t>()) {
            return doc[key].as<uint64_t>();
        }
    }
    return defaultValue;
}

bool AptosUtils::isValidAmount(uint64_t amount) {
    return amount > 0 && amount <= UINT64_MAX;
}

bool AptosUtils::isValidGasPrice(uint64_t gasPrice) {
    return gasPrice > 0 && gasPrice <= 1000000; // Max reasonable gas price
}

bool AptosUtils::isValidSequenceNumber(uint64_t sequenceNumber) {
    return sequenceNumber <= UINT64_MAX;
}

char AptosUtils::nibbleToHex(uint8_t nibble) {
    return hexChars[nibble & 0x0F];
}

uint8_t AptosUtils::hexToNibble(char hex) {
    if (hex >= '0' && hex <= '9') {
        return hex - '0';
    } else if (hex >= 'a' && hex <= 'f') {
        return hex - 'a' + 10;
    } else if (hex >= 'A' && hex <= 'F') {
        return hex - 'A' + 10;
    }
    return 0xFF; // Invalid character
}
