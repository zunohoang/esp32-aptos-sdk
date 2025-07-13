#include "AptosSDK.h"

AptosSDK::AptosSDK(const String& url) : nodeUrl(url), timeout(10000), debugMode(false) {
    // Initialize HTTP client
    http.setTimeout(timeout);
    http.setReuse(true);
}

AptosSDK::~AptosSDK() {
    http.end();
}

void AptosSDK::setNodeUrl(const String& url) {
    nodeUrl = url;
    logDebug("Node URL set to: " + url);
}

void AptosSDK::setTimeout(int timeoutMs) {
    timeout = timeoutMs;
    http.setTimeout(timeout);
    logDebug("Timeout set to: " + String(timeout) + "ms");
}

void AptosSDK::setDebugMode(bool enabled) {
    debugMode = enabled;
    logDebug("Debug mode: " + String(enabled ? "enabled" : "disabled"));
}

String AptosSDK::formatUrl(const String& endpoint) {
    if (endpoint.startsWith("/")) {
        return nodeUrl + endpoint;
    }
    return nodeUrl + "/" + endpoint;
}

void AptosSDK::logDebug(const String& message) {
    if (debugMode) {
        Serial.println("[AptosSDK] " + message);
    }
}

bool AptosSDK::makeHttpRequest(const String& endpoint, JsonDocument& response, 
                              const String& method, const String& payload) {
    String url = formatUrl(endpoint);
    logDebug("Making " + method + " request to: " + url);
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json");
    
    int httpCode;
    if (method == "GET") {
        httpCode = http.GET();
    } else if (method == "POST") {
        httpCode = http.POST(payload);
    } else {
        logDebug("Unsupported HTTP method: " + method);
        return false;
    }
    
    logDebug("HTTP response code: " + String(httpCode));
    
    if (httpCode > 0) {
        String responseBody = http.getString();
        logDebug("Response body length: " + String(responseBody.length()));
        
        DeserializationError error = deserializeJson(response, responseBody);
        if (error) {
            logDebug("JSON parsing failed: " + String(error.c_str()));
            http.end();
            return false;
        }
        
        http.end();
        return true;
    } else {
        logDebug("HTTP request failed with code: " + String(httpCode));
        http.end();
        return false;
    }
}

bool AptosSDK::getNodeInfo(JsonDocument& response) {
    return makeHttpRequest("", response, "GET");
}

bool AptosSDK::getLedgerInfo(JsonDocument& response) {
    return makeHttpRequest("", response, "GET");
}

bool AptosSDK::getBlockByHeight(uint64_t height, JsonDocument& response) {
    return makeHttpRequest("blocks/by_height/" + String((unsigned long)height), response, "GET");
}

bool AptosSDK::getBlockByVersion(uint64_t version, JsonDocument& response) {
    return makeHttpRequest("blocks/by_version/" + String((unsigned long)version), response, "GET");
}

bool AptosSDK::getAccount(const String& address, JsonDocument& response, uint64_t ledgerVersion) {
    if (!isValidAddress(address)) {
        logDebug("Invalid address: " + address);
        return false;
    }
    
    String normalizedAddr = normalizeAddress(address);
    String endpoint = "accounts/" + normalizedAddr;
    
    if (ledgerVersion > 0) {
        endpoint += "?ledger_version=" + String((unsigned long)ledgerVersion);
    }
    
    return makeHttpRequest(endpoint, response, "GET");
}

bool AptosSDK::getAccountBalance(const String& address, const String& assetType, JsonDocument& response, uint64_t ledgerVersion) {
    if (!isValidAddress(address)) {
        logDebug("Invalid address: " + address);
        return false;
    }
    
    String normalizedAddr = normalizeAddress(address);
    String endpoint = "accounts/" + normalizedAddr + "/balance/" + assetType;
    
    if (ledgerVersion > 0) {
        endpoint += "?ledger_version=" + String((unsigned long)ledgerVersion);
    }
    
    return makeHttpRequest(endpoint, response, "GET");
}

bool AptosSDK::getAccountTransactions(const String& address, JsonDocument& response, 
                                     int limit, int start) {
    if (!isValidAddress(address)) {
        return false;
    }
    
    String normalizedAddr = normalizeAddress(address);
    String endpoint = "accounts/" + normalizedAddr + "/transactions?limit=" + 
                     String(limit) + "&start=" + String(start);
    return makeHttpRequest(endpoint, response, "GET");
}

bool AptosSDK::getAccountResources(const String& address, JsonDocument& response,
                                   uint64_t ledgerVersion, const String& start, int limit) {
    if (!isValidAddress(address)) {
        return false;
    }
    
    String normalizedAddr = normalizeAddress(address);
    String endpoint = "accounts/" + normalizedAddr + "/resources";
    
    String params = "";
    if (ledgerVersion > 0) {
        params += "ledger_version=" + String((unsigned long)ledgerVersion);
    }
    if (!start.isEmpty()) {
        if (!params.isEmpty()) params += "&";
        params += "start=" + start;
    }
    if (limit > 0) {
        if (!params.isEmpty()) params += "&";
        params += "limit=" + String(limit);
    }
    
    if (!params.isEmpty()) {
        endpoint += "?" + params;
    }
    
    return makeHttpRequest(endpoint, response, "GET");
}

bool AptosSDK::getAccountResource(const String& address, const String& resourceType,
                                 JsonDocument& response, uint64_t ledgerVersion) {
    if (!isValidAddress(address)) {
        return false;
    }
    
    String normalizedAddr = normalizeAddress(address);
    String endpoint = "accounts/" + normalizedAddr + "/resource/" + resourceType;
    
    if (ledgerVersion > 0) {
        endpoint += "?ledger_version=" + String((unsigned long)ledgerVersion);
    }
    
    return makeHttpRequest(endpoint, response, "GET");
}

bool AptosSDK::getAccountModules(const String& address, JsonDocument& response,
                                uint64_t ledgerVersion, const String& start, int limit) {
    if (!isValidAddress(address)) {
        return false;
    }
    
    String normalizedAddr = normalizeAddress(address);
    String endpoint = "accounts/" + normalizedAddr + "/modules";
    
    String params = "";
    if (ledgerVersion > 0) {
        params += "ledger_version=" + String((unsigned long)ledgerVersion);
    }
    if (!start.isEmpty()) {
        if (!params.isEmpty()) params += "&";
        params += "start=" + start;
    }
    if (limit > 0) {
        if (!params.isEmpty()) params += "&";
        params += "limit=" + String(limit);
    }
    
    if (!params.isEmpty()) {
        endpoint += "?" + params;
    }
    
    return makeHttpRequest(endpoint, response, "GET");
}

bool AptosSDK::getAccountModule(const String& address, const String& moduleName,
                               JsonDocument& response, uint64_t ledgerVersion) {
    if (!isValidAddress(address)) {
        return false;
    }
    
    String normalizedAddr = normalizeAddress(address);
    String endpoint = "accounts/" + normalizedAddr + "/module/" + moduleName;
    
    if (ledgerVersion > 0) {
        endpoint += "?ledger_version=" + String((unsigned long)ledgerVersion);
    }
    
    return makeHttpRequest(endpoint, response, "GET");
}

bool AptosSDK::getAccountEvents(const String& address, const String& eventHandle,
                               JsonDocument& response, int limit, int start) {
    if (!isValidAddress(address)) {
        return false;
    }
    
    String normalizedAddr = normalizeAddress(address);
    String endpoint = "accounts/" + normalizedAddr + "/events/" + eventHandle;
    
    String params = "limit=" + String(limit);
    if (start > 0) {
        params += "&start=" + String(start);
    }
    
    endpoint += "?" + params;
    return makeHttpRequest(endpoint, response, "GET");
}

bool AptosSDK::getTransactionByHash(const String& txnHash, JsonDocument& response) {
    return makeHttpRequest("transactions/by_hash/" + txnHash, response, "GET");
}

bool AptosSDK::getTransactionByVersion(uint64_t version, JsonDocument& response) {
    return makeHttpRequest("transactions/by_version/" + String((unsigned long)version), response, "GET");
}

bool AptosSDK::getTransactions(JsonDocument& response, int limit, int start) {
    String endpoint = "transactions?limit=" + String(limit) + "&start=" + String(start);
    return makeHttpRequest(endpoint, response, "GET");
}

bool AptosSDK::submitTransaction(const JsonDocument& transaction, JsonDocument& response) {
    String payload;
    serializeJson(transaction, payload);
    logDebug("Submitting transaction: " + payload);
    return makeHttpRequest("transactions", response, "POST", payload);
}

bool AptosSDK::simulateTransaction(const JsonDocument& transaction, JsonDocument& response,
                                   bool estimateGas, bool estimateMaxGas) {
    String payload;
    serializeJson(transaction, payload);
    
    String endpoint = "transactions/simulate";
    String params = "";
    
    if (estimateGas) {
        params += "estimate_gas_unit_price=true";
    }
    if (estimateMaxGas) {
        if (!params.isEmpty()) params += "&";
        params += "estimate_max_gas_amount=true";
    }
    
    if (!params.isEmpty()) {
        endpoint += "?" + params;
    }
    
    return makeHttpRequest(endpoint, response, "POST", payload);
}

bool AptosSDK::batchSubmitTransactions(const JsonArray& transactions, JsonDocument& response) {
    String payload;
    serializeJson(transactions, payload);
    return makeHttpRequest("transactions/batch", response, "POST", payload);
}

bool AptosSDK::waitForTransactionByVersion(uint64_t version, int maxWaitTime) {
    unsigned long startTime = millis();
    JsonDocument response;
    
    while ((millis() - startTime) < (maxWaitTime * 1000)) {
        if (getTransactionByVersion(version, response)) {
            if (response.containsKey("success") && response["success"].as<bool>()) {
                return true;
            }
        }
        delay(1000);
    }
    
    logDebug("Transaction wait timeout for version: " + String((unsigned long)version));
    return false;
}

bool AptosSDK::waitForTransaction(const String& txnHash, int maxWaitTime) {
    unsigned long startTime = millis();
    JsonDocument response;
    
    while ((millis() - startTime) < (maxWaitTime * 1000)) {
        if (getTransactionByHash(txnHash, response)) {
            if (response.containsKey("success") && response["success"].as<bool>()) {
                return true;
            }
        }
        delay(1000); // Wait 1 second before retry
    }
    
    logDebug("Transaction wait timeout for hash: " + txnHash);
    return false;
}

bool AptosSDK::viewFunction(const String& moduleAddress, const String& moduleName,
                           const String& functionName, const JsonArray& typeArgs,
                           const JsonArray& args, JsonDocument& response) {
    JsonDocument payload;
    payload["function"] = moduleAddress + "::" + moduleName + "::" + functionName;
    payload["type_arguments"] = typeArgs;
    payload["arguments"] = args;
    
    String payloadStr;
    serializeJson(payload, payloadStr);
    return makeHttpRequest("view", response, "POST", payloadStr);
}

bool AptosSDK::getEventsByEventHandle(const String& address, const String& eventHandle,
                                     JsonDocument& response, int limit) {
    String normalizedAddr = normalizeAddress(address);
    String endpoint = "accounts/" + normalizedAddr + "/events/" + eventHandle + 
                     "?limit=" + String(limit);
    return makeHttpRequest(endpoint, response, "GET");
}

bool AptosSDK::estimateGasPrice(uint64_t& gasPrice) {
    JsonDocument response;
    if (makeHttpRequest("estimate_gas_price", response, "GET")) {
        gasPrice = response["gas_estimate"].as<uint64_t>();
        return true;
    }
    return false;
}

bool AptosSDK::isValidAddress(const String& address) {
    // Remove 0x prefix if present
    String addr = address;
    if (addr.startsWith("0x") || addr.startsWith("0X")) {
        addr = addr.substring(2);
    }
    
    // Check length (should be 1-64 hex characters)
    if (addr.length() == 0 || addr.length() > 64) {
        return false;
    }
    
    // Check if all characters are hex
    for (int i = 0; i < addr.length(); i++) {
        char c = addr.charAt(i);
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    
    return true;
}

String AptosSDK::normalizeAddress(const String& address) {
    String addr = address;
    
    // Remove 0x prefix if present
    if (addr.startsWith("0x") || addr.startsWith("0X")) {
        addr = addr.substring(2);
    }
    
    // Convert to lowercase
    addr.toLowerCase();
    
    // Pad with leading zeros to make it 64 characters
    while (addr.length() < 64) {
        addr = "0" + addr;
    }
    
    return "0x" + addr;
}

bool AptosSDK::isConnected() {
    JsonDocument response;
    return getNodeInfo(response);
}

bool AptosSDK::ping() {
    return isConnected();
}

bool AptosSDK::getChainId(uint8_t& chainId) {
    JsonDocument response;
    if (getLedgerInfo(response)) {
        chainId = response["chain_id"].as<uint8_t>();
        return true;
    }
    return false;
}

bool AptosSDK::transferCoin(AptosAccount& sender, const String& recipient, 
                           uint64_t amount, uint64_t& txnHash,
                           const String& coinType) {
    // Get sender's account info for sequence number
    JsonDocument accountInfo;
    if (!getAccount(sender.getAddress(), accountInfo)) {
        logDebug("Failed to get sender account info");
        return false;
    }
    
    uint64_t sequenceNumber = AptosUtils::getJsonUint64(accountInfo, "sequence_number", 0);
    
    // Create transaction
    AptosTransaction txn = AptosTransaction::createCoinTransfer(
        sender.getAddress(), recipient, amount, sequenceNumber
    );
    
    if (!txn.build()) {
        logDebug("Failed to build transaction: " + txn.getError());
        return false;
    }
    
    // Sign transaction
    String signedTxn;
    if (!sender.signTransaction(txn.getTransaction(), signedTxn)) {
        logDebug("Failed to sign transaction");
        return false;
    }
    
    // Submit transaction
    JsonDocument signedDoc;
    if (!AptosUtils::parseJsonSafely(signedTxn, signedDoc)) {
        logDebug("Failed to parse signed transaction");
        return false;
    }
    
    JsonDocument response;
    if (!submitTransaction(signedDoc, response)) {
        logDebug("Failed to submit transaction");
        return false;
    }
    
    // Extract transaction hash
    String hashStr = AptosUtils::getJsonString(response, "hash", "");
    if (hashStr.isEmpty()) {
        logDebug("No transaction hash in response");
        return false;
    }
    
    txnHash = strtoull(hashStr.c_str(), nullptr, 16);
    return true;
}

bool AptosSDK::transferToken(AptosAccount& sender, const String& recipient,
                            const String& creator, const String& collection,
                            const String& tokenName, uint64_t amount, uint64_t& txnHash) {
    // Get sender's account info for sequence number
    JsonDocument accountInfo;
    if (!getAccount(sender.getAddress(), accountInfo)) {
        return false;
    }
    
    uint64_t sequenceNumber = AptosUtils::getJsonUint64(accountInfo, "sequence_number", 0);
    
    // Create token transfer transaction
    AptosTransaction txn;
    txn.setSender(sender.getAddress())
       .setSequenceNumber(sequenceNumber)
       .tokenTransfer(recipient, creator, collection, tokenName, amount);
    
    if (!txn.build()) {
        return false;
    }
    
    // Sign and submit
    String signedTxn;
    if (!sender.signTransaction(txn.getTransaction(), signedTxn)) {
        return false;
    }
    
    JsonDocument signedDoc;
    if (!AptosUtils::parseJsonSafely(signedTxn, signedDoc)) {
        return false;
    }
    
    JsonDocument response;
    if (!submitTransaction(signedDoc, response)) {
        return false;
    }
    
    String hashStr = AptosUtils::getJsonString(response, "hash", "");
    if (!hashStr.isEmpty()) {
        txnHash = strtoull(hashStr.c_str(), nullptr, 16);
        return true;
    }
    
    return false;
}

bool AptosSDK::callFunction(AptosAccount& sender, const String& moduleAddress,
                           const String& moduleName, const String& functionName,
                           const JsonArray& typeArgs, const JsonArray& args,
                           JsonDocument& response) {
    // Get sender's account info for sequence number
    JsonDocument accountInfo;
    if (!getAccount(sender.getAddress(), accountInfo)) {
        return false;
    }
    
    uint64_t sequenceNumber = AptosUtils::getJsonUint64(accountInfo, "sequence_number", 0);
    
    // Create function call transaction
    AptosTransaction txn;
    txn.setSender(sender.getAddress())
       .setSequenceNumber(sequenceNumber)
       .entryFunction(moduleAddress, moduleName, functionName, typeArgs, args);
    
    if (!txn.build()) {
        return false;
    }
    
    // Sign and submit
    String signedTxn;
    if (!sender.signTransaction(txn.getTransaction(), signedTxn)) {
        return false;
    }
    
    JsonDocument signedDoc;
    if (!AptosUtils::parseJsonSafely(signedTxn, signedDoc)) {
        return false;
    }
    
    return submitTransaction(signedDoc, response);
}

bool AptosSDK::getEventsByCreationNumber(const String& address, uint64_t creationNumber,
                                        JsonDocument& response, int limit) {
    String normalizedAddr = normalizeAddress(address);
    String endpoint = "accounts/" + normalizedAddr + "/events/" + String((unsigned long)creationNumber) + 
                     "?limit=" + String(limit);
    return makeHttpRequest(endpoint, response, "GET");
}

bool AptosSDK::estimateTransactionGas(const JsonDocument& transaction, uint64_t& gasUsed) {
    JsonDocument response;
    if (simulateTransaction(transaction, response)) {
        if (response.is<JsonArray>() && response.size() > 0) {
            gasUsed = AptosUtils::getJsonUint64(response[0], "gas_used", 0);
            return true;
        }
    }
    return false;
}

String AptosSDK::getLastError() {
    // Return the last HTTP error or other error
    return "HTTP Error Code: " + String(http.errorToString(http.getLastError()).c_str());
}

bool AptosSDK::getEventsByEventKey(const String& eventKey, JsonDocument& response,
                                  int limit, int start) {
    String endpoint = "events/" + eventKey;
    String params = "limit=" + String(limit);
    if (start > 0) {
        params += "&start=" + String(start);
    }
    endpoint += "?" + params;
    return makeHttpRequest(endpoint, response, "GET");
}

bool AptosSDK::getTableItem(const String& tableHandle, const JsonDocument& tableItemRequest,
                           JsonDocument& response, uint64_t ledgerVersion) {
    String endpoint = "tables/" + tableHandle + "/item";
    if (ledgerVersion > 0) {
        endpoint += "?ledger_version=" + String((unsigned long)ledgerVersion);
    }
    
    String payload;
    serializeJson(tableItemRequest, payload);
    return makeHttpRequest(endpoint, response, "POST", payload);
}

bool AptosSDK::getRawTableItem(const String& tableHandle, const JsonDocument& tableItemRequest,
                              JsonDocument& response, uint64_t ledgerVersion) {
    String endpoint = "tables/" + tableHandle + "/raw_item";
    if (ledgerVersion > 0) {
        endpoint += "?ledger_version=" + String((unsigned long)ledgerVersion);
    }
    
    String payload;
    serializeJson(tableItemRequest, payload);
    return makeHttpRequest(endpoint, response, "POST", payload);
}

bool AptosSDK::getHealth(JsonDocument& response) {
    return makeHttpRequest("-/healthy", response, "GET");
}

bool AptosSDK::getSpec(JsonDocument& response) {
    return makeHttpRequest("spec", response, "GET");
}

bool AptosSDK::getOpenApiSpec(JsonDocument& response) {
    return makeHttpRequest("spec.yaml", response, "GET");
}

bool AptosSDK::getAccountBalanceSimple(const String& address, uint64_t& balance, const String& coinType) {
    JsonDocument response;
    if (!getAccountResources(address, response)) {
        return false;
    }
    
    // Find coin store resource
    for (JsonVariant resource : response.as<JsonArray>()) {
        String resourceType = resource["type"].as<String>();
        if (resourceType.indexOf("0x1::coin::CoinStore<" + coinType + ">") != -1) {
            balance = resource["data"]["coin"]["value"].as<uint64_t>();
            return true;
        }
    }
    
    balance = 0;
    return false;
}
