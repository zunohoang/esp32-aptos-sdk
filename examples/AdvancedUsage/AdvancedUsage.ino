/*
 * Advanced Aptos SDK Example - Smart Contract Interaction
 * 
 * This example shows how to:
 * 1. Deploy a smart contract module
 * 2. Call contract functions
 * 3. Query contract state
 * 4. Handle events
 */

#include <WiFi.h>
#include "src/AptosSDK.h"
#include "src/AptosAccount.h"
#include "src/AptosTransaction.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

AptosSDK aptos(APTOS_TESTNET);
AptosAccount myAccount;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== Advanced Aptos SDK Example ===");
    
    // Connect to WiFi
    connectToWiFi();
    
    // Setup SDK and account
    aptos.setDebugMode(true);
    setupAccount();
    
    // Examples
    createCollectionExample();
    mintTokenExample();
    queryEventsExample();
}

void loop() {
    delay(10000);
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
}

void setupAccount() {
    if (!myAccount.loadFromEEPROM()) {
        myAccount.createRandom();
        myAccount.saveToEEPROM();
    }
    myAccount.print();
}

void createCollectionExample() {
    Serial.println("\n--- Creating NFT Collection ---");
    
    // Get account sequence number
    JsonDocument accountInfo;
    if (!aptos.getAccount(myAccount.getAddress(), accountInfo)) {
        Serial.println("Account not found on-chain. Please fund it first.");
        return;
    }
    
    uint64_t sequenceNumber = AptosUtils::getJsonUint64(accountInfo, "sequence_number", 0);
    
    // Create collection transaction
    AptosTransaction txn;
    JsonArray mutateSettings = txn.getTransaction().createNestedArray();
    mutateSettings.add(false); // description
    mutateSettings.add(false); // uri
    mutateSettings.add(false); // maximum
    
    txn.setSender(myAccount.getAddress())
       .setSequenceNumber(sequenceNumber)
       .createCollection(
           "My ESP32 Collection",
           "A collection created from ESP32",
           "https://example.com/collection.json",
           1000,
           mutateSettings
       );
    
    if (txn.build()) {
        String signedTxn;
        if (myAccount.signTransaction(txn.getTransaction(), signedTxn)) {
            JsonDocument signedDoc;
            if (AptosUtils::parseJsonSafely(signedTxn, signedDoc)) {
                JsonDocument response;
                if (aptos.submitTransaction(signedDoc, response)) {
                    String hash = AptosUtils::getJsonString(response, "hash");
                    Serial.println("Collection creation submitted: " + hash);
                    
                    if (aptos.waitForTransaction(hash, 30)) {
                        Serial.println("✓ Collection created successfully!");
                    }
                } else {
                    Serial.println("✗ Failed to submit transaction");
                }
            }
        }
    }
}

void mintTokenExample() {
    Serial.println("\n--- Minting NFT Token ---");
    
    // Get account sequence number
    JsonDocument accountInfo;
    if (!aptos.getAccount(myAccount.getAddress(), accountInfo)) {
        return;
    }
    
    uint64_t sequenceNumber = AptosUtils::getJsonUint64(accountInfo, "sequence_number", 0);
    
    // Create token transaction
    AptosTransaction txn;
    JsonArray mutateSettings = txn.getTransaction().createNestedArray();
    mutateSettings.add(false); // description
    mutateSettings.add(false); // uri
    mutateSettings.add(false); // royalty
    mutateSettings.add(false); // properties
    
    txn.setSender(myAccount.getAddress())
       .setSequenceNumber(sequenceNumber)
       .createToken(
           "My ESP32 Collection",
           "ESP32 Token #1",
           "First token minted from ESP32",
           1,
           "https://example.com/token1.json",
           mutateSettings
       );
    
    if (txn.build()) {
        String signedTxn;
        if (myAccount.signTransaction(txn.getTransaction(), signedTxn)) {
            JsonDocument signedDoc;
            if (AptosUtils::parseJsonSafely(signedTxn, signedDoc)) {
                JsonDocument response;
                if (aptos.submitTransaction(signedDoc, response)) {
                    String hash = AptosUtils::getJsonString(response, "hash");
                    Serial.println("Token minting submitted: " + hash);
                    
                    if (aptos.waitForTransaction(hash, 30)) {
                        Serial.println("✓ Token minted successfully!");
                    }
                }
            }
        }
    }
}

void queryEventsExample() {
    Serial.println("\n--- Querying Events ---");
    
    // Query token creation events
    JsonDocument events;
    if (aptos.getEventsByEventHandle(
        myAccount.getAddress(),
        "0x3::token::TokenStore/deposit_events",
        events,
        10
    )) {
        Serial.println("Token events found:");
        String eventsStr;
        serializeJsonPretty(events, eventsStr);
        Serial.println(eventsStr);
    } else {
        Serial.println("No events found or error occurred");
    }
}

void customContractExample() {
    Serial.println("\n--- Custom Contract Interaction ---");
    
    // Example: Call a custom smart contract function
    String moduleAddress = myAccount.getAddress();
    String moduleName = "my_module";
    String functionName = "my_function";
    
    JsonDocument typeArgs;
    JsonArray typeArgsArray = typeArgs.to<JsonArray>();
    
    JsonDocument args;
    JsonArray argsArray = args.to<JsonArray>();
    argsArray.add("hello world");
    argsArray.add(42);
    
    JsonDocument response;
    if (aptos.callFunction(
        myAccount,
        moduleAddress,
        moduleName,
        functionName,
        typeArgsArray,
        argsArray,
        response
    )) {
        Serial.println("Contract function called successfully!");
        String responseStr;
        serializeJsonPretty(response, responseStr);
        Serial.println(responseStr);
    } else {
        Serial.println("Contract function call failed");
    }
}
