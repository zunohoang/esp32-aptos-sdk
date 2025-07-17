# AptosIoT SDK – The World's First IoT-to-Aptos Bridge

A comprehensive SDK for interacting with the Aptos blockchain from ESP32 devices using Arduino IDE.

## Features

- **Account Management**: Create, import, and manage Aptos accounts
- **Transaction Operations**: Send transactions, check status, estimate gas
- **Token Operations**: Transfer APT coins and custom tokens
- **NFT Support**: Create collections, mint tokens, transfer NFTs
- **Smart Contract Interaction**: Call view functions and execute contract functions
- **Event Querying**: Query blockchain events and transaction history
- **Secure Storage**: Save accounts to EEPROM with encryption
- **Network Support**: Mainnet, Testnet, and Devnet endpoints

## Installation

1. Download or clone this repository to your Arduino libraries folder
2. Install required dependencies:
   - ArduinoJson (>= 6.19.0)
   - WiFi library (included with ESP32 core)
   - HTTPClient library (included with ESP32 core)

## Quick Start

```cpp
#include <WiFi.h>
#include "src/AptosSDK.h"
#include "src/AptosAccount.h"

// Initialize SDK
AptosSDK aptos(APTOS_TESTNET);
AptosAccount myAccount;

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi
    WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    
    // Create account
    myAccount.createRandom();
    myAccount.print();
    
    // Check balance
    uint64_t balance;
    if (aptos.getAccountBalance(myAccount.getAddress(), balance)) {
        Serial.println("Balance: " + String(balance) + " APT");
    }
}
```

## API Reference

### AptosSDK Class

#### Configuration
```cpp
AptosSDK aptos(APTOS_TESTNET);        // Constructor
aptos.setNodeUrl("custom_url");        // Set custom node URL
aptos.setTimeout(15000);               // Set timeout in milliseconds
aptos.setDebugMode(true);              // Enable debug logging
```

#### Account Operations
```cpp
// Get account information (with optional ledger version)
JsonDocument account;
aptos.getAccount(address, account, ledgerVersion);

// Get account balance using new API
JsonDocument balance;
aptos.getAccountBalance(address, "0x1::aptos_coin::AptosCoin", balance);

// Get account balance (backward compatibility)
uint64_t simpleBalance;
aptos.getAccountBalanceSimple(address, simpleBalance);

// Get account resources with pagination
JsonDocument resources;
aptos.getAccountResources(address, resources, 0, "", 100);

// Get specific resource
JsonDocument coinStore;
aptos.getAccountResource(address, "0x1::coin::CoinStore<0x1::aptos_coin::AptosCoin>", coinStore);

// Get account modules
JsonDocument modules;
aptos.getAccountModules(address, modules);

// Get specific module
JsonDocument module;
aptos.getAccountModule(address, "MyModule", module);

// Get account events
JsonDocument events;
aptos.getAccountEvents(address, "withdraw_events", events, 25);
```

#### Transaction Operations
```cpp
// Get transaction by hash
JsonDocument txn;
aptos.getTransactionByHash(txnHash, txn);

// Get transaction by version
JsonDocument txn;
aptos.getTransactionByVersion(version, txn);

// Submit transaction
JsonDocument response;
aptos.submitTransaction(transaction, response);

// Simulate transaction with gas estimation
JsonDocument simulation;
aptos.simulateTransaction(transaction, simulation, true, true);

// Batch submit transactions
JsonArray transactions;
JsonDocument batchResponse;
aptos.batchSubmitTransactions(transactions, batchResponse);

// Wait for transaction confirmation
aptos.waitForTransaction(txnHash, 30);
aptos.waitForTransactionByVersion(version, 30);
```

#### Smart Contract Operations
```cpp
// Call view function
JsonDocument result;
aptos.viewFunction(moduleAddress, moduleName, functionName, typeArgs, args, result);

// Execute contract function
JsonDocument response;
aptos.callFunction(sender, moduleAddress, moduleName, functionName, typeArgs, args, response);
```

### AptosAccount Class

#### Account Creation
```cpp
AptosAccount account;

// Create random account
account.createRandom();

// Import from private key
account.fromPrivateKey("0x1234...");

// Load from EEPROM
account.loadFromEEPROM();
```

#### Account Information
```cpp
String address = account.getAddress();
String publicKey = account.getPublicKeyHex();
String privateKey = account.getPrivateKeyHex();
bool valid = account.isValid();
```

#### Transaction Signing
```cpp
String signedTxn;
account.signTransaction(transaction, signedTxn);
```

### AptosTransaction Class

#### Transaction Building
```cpp
AptosTransaction txn;
txn.setSender(senderAddress)
   .setSequenceNumber(sequenceNum)
   .setMaxGasAmount(2000)
   .setGasUnitPrice(100)
   .coinTransfer(recipient, amount);

if (txn.build()) {
    // Transaction ready for signing
}
```

#### Pre-built Transaction Types
```cpp
// Coin transfer
txn.coinTransfer(recipient, amount);

// Create account
txn.createAccount(authKey);

// Create NFT collection
txn.createCollection(name, description, uri, maximum, mutateSettings);

// Mint token
txn.createToken(collection, name, description, supply, uri, mutateSettings);
```

## Examples

### Basic Usage
See `examples/BasicUsage/BasicUsage.ino` for a complete example showing:
- WiFi connection
- Account creation/loading
- Balance checking
- Coin transfers

### Advanced Usage
See `examples/AdvancedUsage/AdvancedUsage.ino` for examples of:
- NFT collection creation
- Token minting
- Event querying
- Smart contract interaction

## Network Endpoints

The SDK supports three networks:

- **Mainnet**: `APTOS_MAINNET`
- **Testnet**: `APTOS_TESTNET` (recommended for development)
- **Devnet**: `APTOS_DEVNET`

## Error Handling

The SDK provides comprehensive error handling:

```cpp
if (!aptos.transferCoin(sender, recipient, amount, txnHash)) {
    Serial.println("Error: " + aptos.getLastError());
}
```

Error codes are defined in the `AptosError` enum:
- `APTOS_SUCCESS`
- `APTOS_ERROR_NETWORK`
- `APTOS_ERROR_JSON`
- `APTOS_ERROR_INVALID_ADDRESS`
- `APTOS_ERROR_INVALID_SIGNATURE`
- `APTOS_ERROR_INSUFFICIENT_FUNDS`
- `APTOS_ERROR_TRANSACTION_FAILED`
- `APTOS_ERROR_TIMEOUT`

## Security Considerations

1. **Private Key Storage**: Private keys are stored in EEPROM. Consider additional encryption for production use.
2. **Network Security**: Always use HTTPS endpoints for mainnet transactions.
3. **Gas Limits**: Set appropriate gas limits to prevent unexpected fees.
4. **Input Validation**: The SDK validates addresses and amounts, but always verify user inputs.

## Memory Usage

The SDK is optimized for ESP32 constraints:
- Minimal memory footprint
- Efficient JSON parsing
- Reusable HTTP connections
- Optional debug logging

## Troubleshooting

### Common Issues

1. **WiFi Connection**: Ensure strong WiFi signal and correct credentials
2. **Account Not Found**: New accounts must be funded before they appear on-chain
3. **Transaction Failures**: Check gas limits and account balance
4. **JSON Parsing Errors**: Increase JSON document size if needed

### Debug Mode

Enable debug mode for detailed logging:
```cpp
aptos.setDebugMode(true);
```

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Submit a pull request

## License

This project is licensed under the MIT License.

## Support

For support and questions:
- Create an issue on GitHub
- Check the examples for usage patterns
- Enable debug mode for troubleshooting

### New API Features (v1.1.0)

#### Table Operations
```cpp
// Get table item
JsonDocument tableRequest;
tableRequest["key_type"] = "address";
tableRequest["value_type"] = "u64";
tableRequest["key"] = "0x1";

JsonDocument tableItem;
aptos.getTableItem(tableHandle, tableRequest, tableItem);

// Get raw table item
JsonDocument rawItem;
aptos.getRawTableItem(tableHandle, tableRequest, rawItem);
```

#### Enhanced Event Operations
```cpp
// Get events by event key
JsonDocument events;
aptos.getEventsByEventKey(eventKey, events, 25);

// Get events with pagination
aptos.getEventsByEventHandle(address, eventHandle, events, 25, 10);
```

#### Health and Monitoring
```cpp
// Check node health
JsonDocument health;
aptos.getHealth(health);

// Get API specification
JsonDocument spec;
aptos.getSpec(spec);

// Get OpenAPI specification
JsonDocument openApiSpec;
aptos.getOpenApiSpec(openApiSpec);
```
