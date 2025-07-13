#ifndef APTOS_SDK_H
#define APTOS_SDK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <mbedtls/sha256.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include "AptosAccount.h"
#include "AptosTransaction.h"
#include "AptosUtils.h"

// Network endpoints
#define APTOS_MAINNET "https://fullnode.mainnet.aptoslabs.com/v1"
#define APTOS_TESTNET "https://fullnode.testnet.aptoslabs.com/v1"
#define APTOS_DEVNET "https://fullnode.devnet.aptoslabs.com/v1"

// Error codes
enum AptosError {
    APTOS_SUCCESS = 0,
    APTOS_ERROR_NETWORK = -1,
    APTOS_ERROR_JSON = -2,
    APTOS_ERROR_INVALID_ADDRESS = -3,
    APTOS_ERROR_INVALID_SIGNATURE = -4,
    APTOS_ERROR_INSUFFICIENT_FUNDS = -5,
    APTOS_ERROR_TRANSACTION_FAILED = -6,
    APTOS_ERROR_TIMEOUT = -7
};

class AptosSDK {
private:
    String nodeUrl;
    HTTPClient http;
    int timeout;
    bool debugMode;
    
    // Internal helper methods
    bool makeHttpRequest(const String& endpoint, JsonDocument& response, 
                        const String& method = "GET", const String& payload = "");
    String formatUrl(const String& endpoint);
    void logDebug(const String& message);
    
public:
    // Constructor & Destructor
    AptosSDK(const String& url = APTOS_TESTNET);
    ~AptosSDK();
    
    // Configuration
    void setNodeUrl(const String& url);
    void setTimeout(int timeoutMs);
    void setDebugMode(bool enabled);
    
    // Node Information
    bool getNodeInfo(JsonDocument& response);
    bool getLedgerInfo(JsonDocument& response);
    bool getBlockByHeight(uint64_t height, JsonDocument& response);
    bool getBlockByVersion(uint64_t version, JsonDocument& response);
    
    // Account Operations
    bool getAccount(const String& address, JsonDocument& response, uint64_t ledgerVersion = 0);
    bool getAccountBalance(const String& address, const String& assetType, JsonDocument& response, uint64_t ledgerVersion = 0);
    bool getAccountResources(const String& address, JsonDocument& response, 
                            uint64_t ledgerVersion = 0, const String& start = "", int limit = 0);
    bool getAccountResource(const String& address, const String& resourceType, 
                           JsonDocument& response, uint64_t ledgerVersion = 0);
    bool getAccountModules(const String& address, JsonDocument& response, 
                          uint64_t ledgerVersion = 0, const String& start = "", int limit = 0);
    bool getAccountModule(const String& address, const String& moduleName, 
                         JsonDocument& response, uint64_t ledgerVersion = 0);
    bool getAccountTransactions(const String& address, JsonDocument& response, 
                               int limit = 25, int start = 0);
    bool getAccountEvents(const String& address, const String& eventHandle, 
                         JsonDocument& response, int limit = 25, int start = 0);
    
    // Transaction Operations
    bool getTransactionByHash(const String& txnHash, JsonDocument& response);
    bool getTransactionByVersion(uint64_t version, JsonDocument& response);
    bool getTransactions(JsonDocument& response, int limit = 25, int start = 0);
    bool submitTransaction(const JsonDocument& transaction, JsonDocument& response);
    bool simulateTransaction(const JsonDocument& transaction, JsonDocument& response, 
                            bool estimateGas = false, bool estimateMaxGas = false);
    bool batchSubmitTransactions(const JsonArray& transactions, JsonDocument& response);
    bool waitForTransaction(const String& txnHash, int maxWaitTime = 30);
    bool waitForTransactionByVersion(uint64_t version, int maxWaitTime = 30);
    
    // Transfer Operations
    bool transferCoin(AptosAccount& sender, const String& recipient, 
                     uint64_t amount, uint64_t& txnHash,
                     const String& coinType = "0x1::aptos_coin::AptosCoin");
    bool transferToken(AptosAccount& sender, const String& recipient,
                      const String& creator, const String& collection,
                      const String& tokenName, uint64_t amount, uint64_t& txnHash);
    
    // Smart Contract Operations
    bool callFunction(AptosAccount& sender, const String& moduleAddress,
                     const String& moduleName, const String& functionName,
                     const JsonArray& typeArgs, const JsonArray& args,
                     JsonDocument& response);
    bool viewFunction(const String& moduleAddress, const String& moduleName,
                     const String& functionName, const JsonArray& typeArgs,
                     const JsonArray& args, JsonDocument& response);
    
    // Event Operations
    bool getEventsByEventHandle(const String& address, const String& eventHandle,
                               JsonDocument& response, int limit = 25, int start = 0);
    bool getEventsByCreationNumber(const String& address, uint64_t creationNumber,
                                  JsonDocument& response, int limit = 25, int start = 0);
    bool getEventsByEventKey(const String& eventKey, JsonDocument& response, 
                            int limit = 25, int start = 0);
    
    // Table Operations  
    bool getTableItem(const String& tableHandle, const JsonDocument& tableItemRequest,
                     JsonDocument& response, uint64_t ledgerVersion = 0);
    bool getRawTableItem(const String& tableHandle, const JsonDocument& tableItemRequest,
                        JsonDocument& response, uint64_t ledgerVersion = 0);
    
    // State and Health Operations
    bool getHealth(JsonDocument& response);
    bool getSpec(JsonDocument& response);
    bool getOpenApiSpec(JsonDocument& response);
    
    // Gas and Fee Operations
    bool estimateGasPrice(uint64_t& gasPrice);
    bool estimateTransactionGas(const JsonDocument& transaction, uint64_t& gasUsed);
    
    // Utility Methods
    bool isValidAddress(const String& address);
    String normalizeAddress(const String& address);
    bool isConnected();
    String getLastError();
    
    // Helper methods for backward compatibility
    bool getAccountBalanceSimple(const String& address, uint64_t& balance, 
                                const String& coinType = "0x1::aptos_coin::AptosCoin");
    
    // Network status
    bool ping();
    bool getChainId(uint8_t& chainId);
};

#endif
