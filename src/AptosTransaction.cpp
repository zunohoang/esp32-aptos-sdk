#include "AptosTransaction.h"

AptosTransaction::AptosTransaction() : isBuilt(false) {
    transaction.clear();
}

AptosTransaction::AptosTransaction(const JsonDocument& txn) : isBuilt(true) {
    transaction.set(txn);
}

AptosTransaction::~AptosTransaction() {
    // Destructor
}

AptosTransaction& AptosTransaction::setSender(const String& sender) {
    transaction["sender"] = AptosUtils::padHexAddress(sender);
    return *this;
}

AptosTransaction& AptosTransaction::setSequenceNumber(uint64_t sequenceNumber) {
    transaction["sequence_number"] = String(sequenceNumber);
    return *this;
}

AptosTransaction& AptosTransaction::setMaxGasAmount(uint64_t maxGasAmount) {
    transaction["max_gas_amount"] = String(maxGasAmount);
    return *this;
}

AptosTransaction& AptosTransaction::setGasUnitPrice(uint64_t gasUnitPrice) {
    transaction["gas_unit_price"] = String(gasUnitPrice);
    return *this;
}

AptosTransaction& AptosTransaction::setExpirationTimestamp(uint64_t expirationTimestamp) {
    transaction["expiration_timestamp_secs"] = String(expirationTimestamp);
    return *this;
}

AptosTransaction& AptosTransaction::setChainId(uint8_t chainId) {
    // Chain ID is typically included in the signing process
    return *this;
}

AptosTransaction& AptosTransaction::entryFunction(const String& moduleAddress, const String& moduleName,
                                                 const String& functionName, const JsonArray& typeArgs,
                                                 const JsonArray& args) {
    buildEntryFunctionPayload(moduleAddress, moduleName, functionName, typeArgs, args);
    return *this;
}

AptosTransaction& AptosTransaction::script(const String& code, const JsonArray& typeArgs, const JsonArray& args) {
    buildScriptPayload(code, typeArgs, args);
    return *this;
}

AptosTransaction& AptosTransaction::coinTransfer(const String& recipient, uint64_t amount,
                                                const String& coinType) {
    JsonArray typeArgs = transaction.createNestedArray();
    typeArgs.add(coinType);
    
    JsonArray args = transaction.createNestedArray();
    args.add(AptosUtils::padHexAddress(recipient));
    args.add(String(amount));
    
    return entryFunction("0x1", "aptos_account", "transfer_coins", typeArgs, args);
}

AptosTransaction& AptosTransaction::tokenTransfer(const String& recipient, const String& creator,
                                                 const String& collection, const String& tokenName,
                                                 uint64_t amount) {
    JsonArray typeArgs = transaction.createNestedArray();
    
    JsonArray args = transaction.createNestedArray();
    args.add(AptosUtils::padHexAddress(creator));
    args.add(collection);
    args.add(tokenName);
    args.add(AptosUtils::padHexAddress(recipient));
    args.add(String(amount));
    
    return entryFunction("0x3", "token", "direct_transfer_script", typeArgs, args);
}

AptosTransaction& AptosTransaction::createAccount(const String& authKey) {
    JsonArray typeArgs = transaction.createNestedArray();
    
    JsonArray args = transaction.createNestedArray();
    args.add(AptosUtils::padHexAddress(authKey));
    
    return entryFunction("0x1", "aptos_account", "create_account", typeArgs, args);
}

AptosTransaction& AptosTransaction::createCollection(const String& name, const String& description,
                                                    const String& uri, uint64_t maximum,
                                                    const JsonArray& mutateSettings) {
    JsonArray typeArgs = transaction.createNestedArray();
    
    JsonArray args = transaction.createNestedArray();
    args.add(name);
    args.add(description);
    args.add(uri);
    args.add(String(maximum));
    
    // Add mutate settings
    for (JsonVariant setting : mutateSettings) {
        args.add(setting);
    }
    
    return entryFunction("0x3", "token", "create_collection_script", typeArgs, args);
}

AptosTransaction& AptosTransaction::createToken(const String& collection, const String& name,
                                               const String& description, uint64_t supply,
                                               const String& uri, const JsonArray& mutateSettings) {
    JsonArray typeArgs = transaction.createNestedArray();
    
    JsonArray args = transaction.createNestedArray();
    args.add(collection);
    args.add(name);
    args.add(description);
    args.add(String(supply));
    args.add(uri);
    
    // Add mutate settings
    for (JsonVariant setting : mutateSettings) {
        args.add(setting);
    }
    
    return entryFunction("0x3", "token", "create_token_script", typeArgs, args);
}

AptosTransaction& AptosTransaction::publishModule(const String& moduleCode) {
    JsonObject payload = transaction.createNestedObject("payload");
    payload["type"] = "module_bundle_payload";
    JsonArray modules = payload.createNestedArray("modules");
    
    JsonObject module = modules.createNestedObject();
    module["bytecode"] = moduleCode;
    
    return *this;
}

AptosTransaction& AptosTransaction::callContract(const String& contractAddress, const String& moduleName,
                                                const String& functionName, const JsonArray& args) {
    JsonArray typeArgs = transaction.createNestedArray();
    return entryFunction(contractAddress, moduleName, functionName, typeArgs, args);
}

bool AptosTransaction::buildEntryFunctionPayload(const String& moduleAddress, const String& moduleName,
                                                const String& functionName, const JsonArray& typeArgs,
                                                const JsonArray& args) {
    JsonObject payload = transaction.createNestedObject("payload");
    payload["type"] = "entry_function_payload";
    payload["function"] = AptosUtils::padHexAddress(moduleAddress) + "::" + moduleName + "::" + functionName;
    payload["type_arguments"] = typeArgs;
    payload["arguments"] = args;
    
    return true;
}

bool AptosTransaction::buildScriptPayload(const String& code, const JsonArray& typeArgs, const JsonArray& args) {
    JsonObject payload = transaction.createNestedObject("payload");
    payload["type"] = "script_payload";
    payload["code"]["bytecode"] = code;
    payload["type_arguments"] = typeArgs;
    payload["arguments"] = args;
    
    return true;
}

bool AptosTransaction::build() {
    // Set default values if not provided
    if (!transaction.containsKey("max_gas_amount")) {
        setMaxGasAmount(DEFAULT_MAX_GAS);
    }
    
    if (!transaction.containsKey("gas_unit_price")) {
        setGasUnitPrice(DEFAULT_GAS_PRICE);
    }
    
    if (!transaction.containsKey("expiration_timestamp_secs")) {
        uint64_t currentTime = AptosUtils::getCurrentTimestamp();
        setExpirationTimestamp(currentTime + DEFAULT_EXPIRATION_OFFSET);
    }
    
    // Validate required fields
    if (!transaction.containsKey("sender")) {
        lastError = "Sender is required";
        return false;
    }
    
    if (!transaction.containsKey("sequence_number")) {
        lastError = "Sequence number is required";
        return false;
    }
    
    if (!transaction.containsKey("payload")) {
        lastError = "Payload is required";
        return false;
    }
    
    isBuilt = true;
    return true;
}

JsonDocument& AptosTransaction::getTransaction() {
    return transaction;
}

String AptosTransaction::serialize() const {
    String result;
    serializeJson(transaction, result);
    return result;
}

bool AptosTransaction::isValid() const {
    return isBuilt && !lastError.length();
}

String AptosTransaction::getError() const {
    return lastError;
}

String AptosTransaction::getSender() const {
    return AptosUtils::getJsonString(transaction, "sender");
}

uint64_t AptosTransaction::getSequenceNumber() const {
    return AptosUtils::getJsonUint64(transaction, "sequence_number");
}

uint64_t AptosTransaction::getMaxGasAmount() const {
    return AptosUtils::getJsonUint64(transaction, "max_gas_amount");
}

uint64_t AptosTransaction::getGasUnitPrice() const {
    return AptosUtils::getJsonUint64(transaction, "gas_unit_price");
}

uint64_t AptosTransaction::getExpirationTimestamp() const {
    return AptosUtils::getJsonUint64(transaction, "expiration_timestamp_secs");
}

uint8_t AptosTransaction::getChainId() const {
    return 1; // Default to mainnet
}

void AptosTransaction::reset() {
    transaction.clear();
    isBuilt = false;
    lastError = "";
}

void AptosTransaction::print() const {
    Serial.println("=== Aptos Transaction ===");
    String txnStr;
    serializeJsonPretty(transaction, txnStr);
    Serial.println(txnStr);
    Serial.println("========================");
}

String AptosTransaction::getHash() const {
    return AptosUtils::generateTransactionHash(transaction);
}

AptosTransaction AptosTransaction::createCoinTransfer(const String& sender, const String& recipient,
                                                     uint64_t amount, uint64_t sequenceNumber,
                                                     uint64_t maxGas, uint64_t gasPrice) {
    AptosTransaction txn;
    JsonArray args = txn.transaction.createNestedArray();
    return txn.setSender(sender)
              .setSequenceNumber(sequenceNumber)
              .setMaxGasAmount(maxGas)
              .setGasUnitPrice(gasPrice)
              .coinTransfer(recipient, amount);
}

AptosTransaction AptosTransaction::createAccountTransaction(const String& sender, const String& authKey,
                                                           uint64_t sequenceNumber,
                                                           uint64_t maxGas, uint64_t gasPrice) {
    AptosTransaction txn;
    return txn.setSender(sender)
              .setSequenceNumber(sequenceNumber)
              .setMaxGasAmount(maxGas)
              .setGasUnitPrice(gasPrice)
              .createAccount(authKey);
}
