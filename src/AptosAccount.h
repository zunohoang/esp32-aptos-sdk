#ifndef APTOS_ACCOUNT_H
#define APTOS_ACCOUNT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/sha256.h>
#include <EEPROM.h>

#define PRIVATE_KEY_SIZE 32
#define PUBLIC_KEY_SIZE 32
#define SIGNATURE_SIZE 64
#define ADDRESS_SIZE 32

class AptosAccount {
private:
    uint8_t privateKey[PRIVATE_KEY_SIZE];
    uint8_t publicKey[PUBLIC_KEY_SIZE];
    String address;
    bool isInitialized;
    
    // Cryptographic context
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    
    // Internal methods
    bool generateKeyPair();
    bool deriveAddress();
    bool initCrypto();
    void cleanupCrypto();
    
public:
    // Constructors
    AptosAccount();
    AptosAccount(const String& privateKeyHex);
    ~AptosAccount();
    
    // Key management
    bool createRandom();
    bool fromPrivateKey(const String& privateKeyHex);
    bool fromMnemonic(const String& mnemonic, int derivationIndex = 0);
    
    // Getters
    String getPrivateKeyHex() const;
    String getPublicKeyHex() const;
    String getAddress() const;
    bool isValid() const;
    
    // Signing
    bool signMessage(const uint8_t* message, size_t messageLen, uint8_t* signature);
    bool signTransaction(const JsonDocument& transaction, String& signedTxn);
    
    // Storage (EEPROM)
    bool saveToEEPROM(int offset = 0);
    bool loadFromEEPROM(int offset = 0);
    
    // Verification
    bool verifySignature(const uint8_t* message, size_t messageLen, 
                        const uint8_t* signature) const;
    
    // Utility
    void clear();
    void print() const;
};
