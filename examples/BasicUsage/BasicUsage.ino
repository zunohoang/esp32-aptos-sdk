/*
 * Aptos SDK for ESP32 - Basic Usage Example
 * 
 * This example demonstrates how to:
 * 1. Connect to WiFi
 * 2. Initialize Aptos SDK
 * 3. Create/load an account
 * 4. Check account balance
 * 5. Transfer APT coins
 * 6. Query transaction status
 */

#include <WiFi.h>
#include <ArduinoJson.h>
#include "src/AptosSDK.h"
#include "src/AptosAccount.h"

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Aptos SDK instance
AptosSDK aptos(APTOS_TESTNET); // Use testnet for development

// Account instance
AptosAccount myAccount;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== Aptos SDK for ESP32 Example ===");
    
    // Connect to WiFi
    connectToWiFi();
    
    // Initialize SDK
    aptos.setDebugMode(true);
    aptos.setTimeout(15000); // 15 seconds timeout
    
    // Test connection
    if (aptos.ping()) {
        Serial.println("✓ Connected to Aptos network");
    } else {
        Serial.println("✗ Failed to connect to Aptos network");
        return;
    }
    
    // Create or load account
    setupAccount();
    
    // Check account balance
    checkBalance();
    
    // Example: Transfer coins (uncomment to test)
    // transferCoins();
}

void loop() {
    // Check network status every 30 seconds
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 30000) {
        if (aptos.ping()) {
            Serial.println("Network status: Connected");
        } else {
            Serial.println("Network status: Disconnected");
        }
        lastCheck = millis();
    }
    
    delay(1000);
}

void connectToWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.println("✓ WiFi connected!");
    Serial.println("IP address: " + WiFi.localIP().toString());
}

void setupAccount() {
    Serial.println("\n--- Account Setup ---");
    
    // Try to load existing account from EEPROM
    if (myAccount.loadFromEEPROM()) {
        Serial.println("✓ Account loaded from EEPROM");
        myAccount.print();
    } else {
        Serial.println("Creating new account...");
        
        // Create new random account
        if (myAccount.createRandom()) {
            Serial.println("✓ New account created");
            myAccount.print();
            
            // Save to EEPROM for next time
            if (myAccount.saveToEEPROM()) {
                Serial.println("✓ Account saved to EEPROM");
            } else {
                Serial.println("✗ Failed to save account to EEPROM");
            }
        } else {
            Serial.println("✗ Failed to create account");
            return;
        }
    }
}

void checkBalance() {
    Serial.println("\n--- Check Balance ---");
    
    uint64_t balance;
    if (aptos.getAccountBalanceSimple(myAccount.getAddress(), balance)) {
        Serial.println("Account: " + myAccount.getAddress());
        Serial.println("Balance: " + String((unsigned long)balance) + " APT");
    } else {
        Serial.println("✗ Failed to get balance (account might not exist on-chain)");
        Serial.println("Note: New accounts need to be funded first");
        
        // Try using new balance API
        JsonDocument balanceResponse;
        if (aptos.getAccountBalance(myAccount.getAddress(), "0x1::aptos_coin::AptosCoin", balanceResponse)) {
            Serial.println("Balance from new API:");
            String responseStr;
            serializeJsonPretty(balanceResponse, responseStr);
            Serial.println(responseStr);
        }
    }
}

void transferCoins() {
    Serial.println("\n--- Transfer Coins ---");
    
    // Example recipient address (replace with actual address)
    String recipient = "0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef";
    uint64_t amount = 1000000; // 0.01 APT (APT has 8 decimals)
    
    Serial.println("Transferring " + String((unsigned long)amount) + " to " + recipient);
    
    uint64_t txnHash;
    if (aptos.transferCoin(myAccount, recipient, amount, txnHash)) {
        Serial.println("✓ Transaction submitted!");
        Serial.println("Transaction hash: 0x" + String((unsigned long)txnHash, HEX));
        
        // Wait for transaction confirmation
        String hashStr = "0x" + String((unsigned long)txnHash, HEX);
        if (aptos.waitForTransaction(hashStr, 30)) {
            Serial.println("✓ Transaction confirmed!");
        } else {
            Serial.println("⚠ Transaction timeout (may still be processing)");
        }
    } else {
        Serial.println("✗ Transfer failed: " + aptos.getLastError());
    }
}

void queryTransaction() {
    Serial.println("\n--- Query Transaction ---");
    
    // Example transaction hash
    String txnHash = "0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef";
    
    JsonDocument response;
    if (aptos.getTransactionByHash(txnHash, response)) {
        Serial.println("Transaction details:");
        String responseStr;
        serializeJsonPretty(response, responseStr);
        Serial.println(responseStr);
    } else {
        Serial.println("✗ Transaction not found");
    }
}

void smartContractExample() {
    Serial.println("\n--- Smart Contract Call ---");
    
    // Example: Call a view function
    String moduleAddress = "0x1";
    String moduleName = "coin";
    String functionName = "balance";
    
    JsonDocument typeArgs;
    JsonArray typeArgsArray = typeArgs.to<JsonArray>();
    typeArgsArray.add("0x1::aptos_coin::AptosCoin");
    
    JsonDocument args;
    JsonArray argsArray = args.to<JsonArray>();
    argsArray.add(myAccount.getAddress());
    
    JsonDocument response;
    if (aptos.viewFunction(moduleAddress, moduleName, functionName, 
                          typeArgsArray, argsArray, response)) {
        Serial.println("View function result:");
        String responseStr;
        serializeJsonPretty(response, responseStr);
        Serial.println(responseStr);
    } else {
        Serial.println("✗ View function call failed");
    }
}

void printNetworkInfo() {
    Serial.println("\n--- Network Information ---");
    
    // Check health
    JsonDocument health;
    if (aptos.getHealth(health)) {
        Serial.println("Node health: OK");
    } else {
        Serial.println("Node health: Failed");
    }
    
    JsonDocument nodeInfo;
    if (aptos.getNodeInfo(nodeInfo)) {
        Serial.println("Node version: " + AptosUtils::getJsonString(nodeInfo, "node_role"));
        Serial.println("Chain ID: " + String(AptosUtils::getJsonUint64(nodeInfo, "chain_id")));
    }
    
    JsonDocument ledgerInfo;
    if (aptos.getLedgerInfo(ledgerInfo)) {
        Serial.println("Ledger version: " + String(AptosUtils::getJsonUint64(ledgerInfo, "ledger_version")));
        Serial.println("Block height: " + String(AptosUtils::getJsonUint64(ledgerInfo, "block_height")));
    }
}

void advancedAccountOperations() {
    Serial.println("\n--- Advanced Account Operations ---");
    
    // Get account with specific ledger version
    JsonDocument account;
    if (aptos.getAccount(myAccount.getAddress(), account, 0)) {
        Serial.println("Account sequence number: " + 
                      String(AptosUtils::getJsonUint64(account, "sequence_number")));
    }
    
    // Get specific resource
    JsonDocument coinStore;
    if (aptos.getAccountResource(myAccount.getAddress(), 
                                "0x1::coin::CoinStore<0x1::aptos_coin::AptosCoin>", 
                                coinStore)) {
        Serial.println("Coin store resource found");
        String responseStr;
        serializeJsonPretty(coinStore, responseStr);
        Serial.println(responseStr);
    }
    
    // Get account events
    JsonDocument events;
    if (aptos.getAccountEvents(myAccount.getAddress(), 
                              "0x1::coin::CoinStore<0x1::aptos_coin::AptosCoin>/withdraw_events",
                              events, 10, 0)) {
        Serial.println("Withdraw events:");
        String eventsStr;
        serializeJsonPretty(events, eventsStr);
        Serial.println(eventsStr);
    }
}
