#ifndef APTOS_TRANSACTION_H
#define APTOS_TRANSACTION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "AptosUtils.h"

// Transaction types
enum TransactionType {
    USER_TRANSACTION,
    GENESIS_TRANSACTION,
    BLOCK_METADATA_TRANSACTION,
    STATE_CHECKPOINT_TRANSACTION
};

// Payload types
enum PayloadType {
    ENTRY_FUNCTION_PAYLOAD,
    SCRIPT_PAYLOAD,
    MODULE_BUNDLE_PAYLOAD
};

class AptosTransaction {
private:
    JsonDocument transaction;
    bool isBuilt;
    String lastError;
    
    // Internal builders
    bool buildEntryFunctionPayload(const String& moduleAddress, const String& moduleName,
                                  const String& functionName, const JsonArray& typeArgs,
                                  const JsonArray& args);
    bool buildScriptPayload(const String& code, const JsonArray& typeArgs, const JsonArray& args);
    
public:
    // Constructors
    AptosTransaction();
    AptosTransaction(const JsonDocument& txn);
    ~AptosTransaction();
    
    // Transaction builder methods
    AptosTransaction& setSender(const String& sender);
    AptosTransaction& setSequenceNumber(uint64_t sequenceNumber);
    AptosTransaction& setMaxGasAmount(uint64_t maxGasAmount);
    AptosTransaction& setGasUnitPrice(uint64_t gasUnitPrice);
    AptosTransaction& setExpirationTimestamp(uint64_t expirationTimestamp);
    AptosTransaction& setChainId(uint8_t chainId);
    
    // Payload builders
    AptosTransaction& entryFunction(const String& moduleAddress, const String& moduleName,
                                   const String& functionName, const JsonArray& typeArgs,
                                   const JsonArray& args);
    AptosTransaction& script(const String& code, const JsonArray& typeArgs, const JsonArray& args);
    
    // Pre-built transaction types
    AptosTransaction& coinTransfer(const String& recipient, uint64_t amount,
                                  const String& coinType = "0x1::aptos_coin::AptosCoin");
    AptosTransaction& tokenTransfer(const String& recipient, const String& creator,
                                   const String& collection, const String& tokenName,
                                   uint64_t amount);
    AptosTransaction& createAccount(const String& authKey);
    AptosTransaction& createCollection(const String& name, const String& description,
                                      const String& uri, uint64_t maximum,
                                      const JsonArray& mutateSettings);
    AptosTransaction& createToken(const String& collection, const String& name,
                                 const String& description, uint64_t supply,
                                 const String& uri, const JsonArray& mutateSettings);
    
    // Smart contract interactions
    AptosTransaction& publishModule(const String& moduleCode);
    AptosTransaction& callContract(const String& contractAddress, const String& moduleName,
                                  const String& functionName, const JsonArray& args);
    
    // Transaction finalization
    bool build();
    JsonDocument& getTransaction();
    String serialize() const;
    bool isValid() const;
    String getError() const;
    
    // Transaction information
    String getSender() const;
    uint64_t getSequenceNumber() const;
    uint64_t getMaxGasAmount() const;
    uint64_t getGasUnitPrice() const;
    uint64_t getExpirationTimestamp() const;
    uint8_t getChainId() const;
    
    // Utility methods
    void reset();
    void print() const;
    String getHash() const;
    
    // Static helper methods
    static AptosTransaction createCoinTransfer(const String& sender, const String& recipient,
                                              uint64_t amount, uint64_t sequenceNumber,
                                              uint64_t maxGas = 2000, uint64_t gasPrice = 100);
    static AptosTransaction createAccountTransaction(const String& sender, const String& authKey,
                                                    uint64_t sequenceNumber,
                                                    uint64_t maxGas = 2000, uint64_t gasPrice = 100);
    
    // Default values
    static const uint64_t DEFAULT_MAX_GAS = 2000;
    static const uint64_t DEFAULT_GAS_PRICE = 100;
    static const uint64_t DEFAULT_EXPIRATION_OFFSET = 600; // 10 minutes
};

#endif
