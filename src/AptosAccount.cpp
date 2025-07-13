#include "AptosAccount.h"
#include "AptosUtils.h"

AptosAccount::AptosAccount() : isInitialized(false) {
    memset(privateKey, 0, PRIVATE_KEY_SIZE);
    memset(publicKey, 0, PUBLIC_KEY_SIZE);
    initCrypto();
}

AptosAccount::AptosAccount(const String& privateKeyHex) : isInitialized(false) {
    memset(privateKey, 0, PRIVATE_KEY_SIZE);
    memset(publicKey, 0, PUBLIC_KEY_SIZE);
    initCrypto();
    fromPrivateKey(privateKeyHex);
}

AptosAccount::~AptosAccount() {
    clear();
    cleanupCrypto();
}

bool AptosAccount::initCrypto() {
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    
    const char* pers = "aptos_account";
    int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                   (const unsigned char*)pers, strlen(pers));
    return ret == 0;
}

void AptosAccount::cleanupCrypto() {
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
}

bool AptosAccount::createRandom() {
    // Generate random private key
    int ret = mbedtls_ctr_drbg_random(&ctr_drbg, privateKey, PRIVATE_KEY_SIZE);
    if (ret != 0) {
        Serial.println("Failed to generate random private key");
        return false;
    }
    
    return generateKeyPair();
}

bool AptosAccount::fromPrivateKey(const String& privateKeyHex) {
    if (!AptosUtils::hexToBytes(privateKeyHex, privateKey, PRIVATE_KEY_SIZE)) {
        Serial.println("Invalid private key hex format");
        return false;
    }
    
    return generateKeyPair();
}

bool AptosAccount::generateKeyPair() {
    // For Ed25519, the public key is derived from private key
    // This is a simplified implementation - in production, use proper Ed25519 library
    
    // Calculate SHA256 of private key as public key (simplified)
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, 0); // 0 = SHA256
    mbedtls_sha256_update(&sha256_ctx, privateKey, PRIVATE_KEY_SIZE);
    mbedtls_sha256_finish(&sha256_ctx, publicKey);
    mbedtls_sha256_free(&sha256_ctx);
    
    isInitialized = deriveAddress();
    return isInitialized;
}

bool AptosAccount::deriveAddress() {
    // Address = SHA3-256(public_key + 0x00)[0:32]
    // Simplified: use SHA256 instead of SHA3-256
    uint8_t input[PUBLIC_KEY_SIZE + 1];
    memcpy(input, publicKey, PUBLIC_KEY_SIZE);
    input[PUBLIC_KEY_SIZE] = 0x00; // Single signature scheme
    
    uint8_t hash[32];
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, 0);
    mbedtls_sha256_update(&sha256_ctx, input, PUBLIC_KEY_SIZE + 1);
    mbedtls_sha256_finish(&sha256_ctx, hash);
    mbedtls_sha256_free(&sha256_ctx);
    
    address = "0x" + AptosUtils::bytesToHex(hash, ADDRESS_SIZE);
    return true;
}

String AptosAccount::getPrivateKeyHex() const {
    if (!isInitialized) return "";
    return AptosUtils::bytesToHex(privateKey, PRIVATE_KEY_SIZE);
}

String AptosAccount::getPublicKeyHex() const {
    if (!isInitialized) return "";
    return AptosUtils::bytesToHex(publicKey, PUBLIC_KEY_SIZE);
}

String AptosAccount::getAddress() const {
    return address;
}

bool AptosAccount::isValid() const {
    return isInitialized;
}

bool AptosAccount::signMessage(const uint8_t* message, size_t messageLen, uint8_t* signature) {
    if (!isInitialized) return false;
    
    // Create message hash
    uint8_t messageHash[32];
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, 0);
    mbedtls_sha256_update(&sha256_ctx, message, messageLen);
    mbedtls_sha256_finish(&sha256_ctx, messageHash);
    mbedtls_sha256_free(&sha256_ctx);
    
    // Simplified signing: XOR with private key (NOT secure - use proper Ed25519)
    for (int i = 0; i < SIGNATURE_SIZE; i++) {
        signature[i] = messageHash[i % 32] ^ privateKey[i % PRIVATE_KEY_SIZE];
    }
    
    return true;
}

bool AptosAccount::signTransaction(const JsonDocument& transaction, String& signedTxn) {
    if (!isInitialized) return false;
    
    // Serialize transaction for signing
    String txnString;
    serializeJson(transaction, txnString);
    
    uint8_t signature[SIGNATURE_SIZE];
    if (!signMessage((const uint8_t*)txnString.c_str(), txnString.length(), signature)) {
        return false;
    }
    
    // Create signed transaction
    JsonDocument signedTransaction;
    signedTransaction["sender"] = address;
    signedTransaction["sequence_number"] = transaction["sequence_number"];
    signedTransaction["max_gas_amount"] = transaction["max_gas_amount"];
    signedTransaction["gas_unit_price"] = transaction["gas_unit_price"];
    signedTransaction["expiration_timestamp_secs"] = transaction["expiration_timestamp_secs"];
    signedTransaction["payload"] = transaction["payload"];
    
    JsonObject authenticator = signedTransaction.createNestedObject("signature");
    authenticator["type"] = "ed25519_signature";
    authenticator["public_key"] = "0x" + getPublicKeyHex();
    authenticator["signature"] = "0x" + AptosUtils::bytesToHex(signature, SIGNATURE_SIZE);
    
    serializeJson(signedTransaction, signedTxn);
    return true;
}

bool AptosAccount::saveToEEPROM(int offset) {
    if (!isInitialized) return false;
    
    EEPROM.begin(512);
    
    // Write magic bytes
    EEPROM.write(offset, 0xAP);
    EEPROM.write(offset + 1, 0xTO);
    
    // Write private key
    for (int i = 0; i < PRIVATE_KEY_SIZE; i++) {
        EEPROM.write(offset + 2 + i, privateKey[i]);
    }
    
    EEPROM.commit();
    return true;
}

bool AptosAccount::loadFromEEPROM(int offset) {
    EEPROM.begin(512);
    
    // Check magic bytes
    if (EEPROM.read(offset) != 0xAP || EEPROM.read(offset + 1) != 0xTO) {
        return false;
    }
    
    // Read private key
    for (int i = 0; i < PRIVATE_KEY_SIZE; i++) {
        privateKey[i] = EEPROM.read(offset + 2 + i);
    }
    
    return generateKeyPair();
}

bool AptosAccount::verifySignature(const uint8_t* message, size_t messageLen, 
                                  const uint8_t* signature) const {
    if (!isInitialized) return false;
    
    // Create test signature
    uint8_t testSignature[SIGNATURE_SIZE];
    AptosAccount* self = const_cast<AptosAccount*>(this);
    if (!self->signMessage(message, messageLen, testSignature)) {
        return false;
    }
    
    // Compare signatures
    return memcmp(signature, testSignature, SIGNATURE_SIZE) == 0;
}

void AptosAccount::clear() {
    memset(privateKey, 0, PRIVATE_KEY_SIZE);
    memset(publicKey, 0, PUBLIC_KEY_SIZE);
    address = "";
    isInitialized = false;
}

void AptosAccount::print() const {
    if (!isInitialized) {
        Serial.println("Account not initialized");
        return;
    }
    
    Serial.println("=== Aptos Account ===");
    Serial.println("Address: " + address);
    Serial.println("Public Key: 0x" + getPublicKeyHex());
    Serial.println("Private Key: 0x" + getPrivateKeyHex());
    Serial.println("====================");
}
