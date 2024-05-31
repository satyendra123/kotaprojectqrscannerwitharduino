//EXAMPLE-1 ye work kar rha hai. isme maine ek esp32 ko connect kiya hai ENC ke sath. aur scanner ke rx and tx pin ko esp32 ke tx0 aur rx0 par connect kiya hai
#include <SPI.h>
#include <EthernetENC.h>
#include <HardwareSerial.h>

#define relayPin 4
HardwareSerial SerialPort(0);

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 2, 88);

EthernetServer server(6000);
EthernetClient client;

String clientData = "";
String data = "";

boolean IsClientConnected = false;

unsigned long currentMillis, previousMillis, reconnectMillis;
const unsigned long healthPacketInterval = 3000; // 3 seconds
const unsigned long reconnectInterval = 5000;    // 5 seconds
const unsigned long resetInterval = 60000;

String remove_all_repeating_substrings(String input_value)
{
  int length = input_value.length();

  for (int i = 1; i <= length / 2; i++)
  {
    String substr = input_value.substring(0, i);
    int repeat_count = length / i;

    String repeated = "";
    for (int j = 0; j < repeat_count; j++)
    {
      repeated += substr;
    }

    if (repeated == input_value)
    {
      return substr;
    }
  }

  return input_value;
}

String extractData(String inputString)
{
  if ((inputString.startsWith("QR") || inputString.startsWith("TG")) && inputString.endsWith("END"))
  {
    String extractedData = inputString.substring(2, inputString.length() - 3);
    return extractedData;
  }
  else
  {
    return inputString;
  }
}

bool isNumeric(String str)
{
  for (size_t i = 0; i < str.length(); i++)
  {
    if (!isDigit(str.charAt(i)))
    {
      return false;
    }
  }
  return true;
}

void setup()
{
  Ethernet.init(5); 
  Serial.begin(115200);
  SerialPort.begin(115200,SERIAL_8N1,3,1);
  Ethernet.begin(mac, ip);
  server.begin();
  while (!Serial)
  {
    ; // wait for Serial port to connect
  }

  Serial.print("Machine Gate IP: ");
  Serial.println(ip);

  Ethernet.begin(mac, ip);
  server.begin();

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  
  IsClientConnected = false;

  currentMillis = 0;
  previousMillis = 0;
  reconnectMillis = 0;
}

void loop()
{
  if (!IsClientConnected)
  {
    if (!client.connected())
    {
      client.stop(); // Ensure client is disconnected
    }

    EthernetClient newClient = server.available();
    if (newClient)
    {
      client = newClient;
      IsClientConnected = true;
      client.flush();
      Serial.println("Client Connected");
      client.println("Connected to ESP32");
    }
  }

  if (IsClientConnected)
  {
    if (client.available() > 0)
    {
      char thisChar = client.read();
      if (thisChar == '|')
      {
        clientData = "";
      }
      else if (thisChar == '%')
      {
        Serial.println(clientData);
        if (clientData.equals("OPENEN"))
        {
          Serial.println("Barrier is opening");
          digitalWrite(relayPin, LOW);
          delay(300);
          digitalWrite(relayPin, HIGH);
        }
      }
      else
      {
        clientData += thisChar;
      }
    }

   if(SerialPort.available())
    {
      data = SerialPort.readString();
      Serial.println(data);
      String result = remove_all_repeating_substrings(data);
      String entryData = extractData(result);

      if (isNumeric(entryData))
      {
        String sendTagData = "|ENTG-" + entryData + "%";
        client.println(sendTagData);
        Serial.println(sendTagData);
      }
      else
      {
        String sendEntryQRData = "|ENQR-" + entryData + "%";
        client.println(sendEntryQRData);
        Serial.println(sendEntryQRData);
      }
    }

    currentMillis = millis();
    if (currentMillis - previousMillis >= healthPacketInterval)
    {
      previousMillis = currentMillis;
      client.println("|HLT%");
    }

    if (!client.connected())
    {
      Serial.println("Client Disconnected");
      IsClientConnected = false;
      reconnectMillis = millis();
    }

    if ((!client.connected()) && (Ethernet.linkStatus() == LinkOFF))
    {
      Serial.println("Attempting to reconnect...");
      delay(100);
      server.begin();
    }
  }
}



/*
 ye same code jo hai mere esp32 based qr scanner ke liye hai
#include <Arduino.h>
#include <SPI.h>
#include <EthernetLarge.h>
#include <HardwareSerial.h>
#include "esp_task_wdt.h"

#define relayPin 13
HardwareSerial SerialPort(2);

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 151);

EthernetServer server(7000);
EthernetClient client;

String clientData = "";
String data = "";

boolean IsClientConnected = false;

unsigned long currentMillis, previousMillis, reconnectMillis;
const unsigned long healthPacketInterval = 3000; // 3 seconds
const unsigned long reconnectInterval = 5000;    // 5 seconds
const unsigned long resetInterval = 60000;

String remove_all_repeating_substrings(String input_value)
{
  int length = input_value.length();

  for (int i = 1; i <= length / 2; i++)
  {
    String substr = input_value.substring(0, i);
    int repeat_count = length / i;

    String repeated = "";
    for (int j = 0; j < repeat_count; j++)
    {
      repeated += substr;
    }

    if (repeated == input_value)
    {
      return substr;
    }
  }

  return input_value;
}

String extractData(String inputString)
{
  if ((inputString.startsWith("QR") || inputString.startsWith("TG")) && inputString.endsWith("END"))
  {
    String extractedData = inputString.substring(2, inputString.length() - 3);
    return extractedData;
  }
  else
  {
    return inputString;
  }
}

bool isNumeric(String str)
{
  for (size_t i = 0; i < str.length(); i++)
  {
    if (!isDigit(str.charAt(i)))
    {
      return false;
    }
  }
  return true;
}

void setup()
{
  Ethernet.init(5); 
  esp_task_wdt_init(4, true);
  Serial.begin(115200);
  SerialPort.begin(115200,SERIAL_8N1,16,17);
  Ethernet.begin(mac, ip);
  server.begin();
  while (!Serial)
  {
    ; // wait for Serial port to connect
  }

  Serial.print("Machine Gate IP: ");
  Serial.println(ip);

  Ethernet.begin(mac, ip);
  server.begin();

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  
  IsClientConnected = false;

  currentMillis = 0;
  previousMillis = 0;
  reconnectMillis = 0;
}

void loop()
{
  esp_task_wdt_reset();

  if (!IsClientConnected)
  {
    if (!client.connected())
    {
      client.stop(); // Ensure client is disconnected
    }

    EthernetClient newClient = server.available();
    if (newClient)
    {
      client = newClient;
      IsClientConnected = true;
      client.flush();
      Serial.println("Client Connected");
      client.println("Connected to ESP32");
    }
  }

  if (IsClientConnected)
  {
    if (client.available() > 0)
    {
      char thisChar = client.read();
      if (thisChar == '|')
      {
        clientData = "";
      }
      else if (thisChar == '%')
      {
        Serial.println(clientData);
        if (clientData.equals("OPENEN"))
        {
          Serial.println("Barrier is opening");
          digitalWrite(relayPin, LOW);
          delay(300);
          digitalWrite(relayPin, HIGH);
        }
      }
      else
      {
        clientData += thisChar;
      }
    }

   if(SerialPort.available())
    {
      data = SerialPort.readString();
      Serial.println(data);
      String result = remove_all_repeating_substrings(data);
      String entryData = extractData(result);

      if (isNumeric(entryData))
      {
        String sendTagData = "|ENTG-" + entryData + "%";
        client.println(sendTagData);
        Serial.println(sendTagData);
      }
      else
      {
        String sendEntryQRData = "|ENQR-" + entryData + "%";
        client.println(sendEntryQRData);
        Serial.println(sendEntryQRData);
      }
    }

    currentMillis = millis();
    if (currentMillis - previousMillis >= healthPacketInterval)
    {
      previousMillis = currentMillis;
      client.println("|HLT%");
    }

    if (!client.connected())
    {
      Serial.println("Client Disconnected");
      IsClientConnected = false;
      reconnectMillis = millis();
    }

    if ((!client.connected()) && (Ethernet.linkStatus() == LinkOFF))
    {
      Serial.println("Attempting to reconnect...");
      delay(100);
      esp_task_wdt_init(15, true);
      server.begin();
    }
  }
}
*/






//EXAMPLE-2 isko humne arduino se banaya hai.
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#include <avr/wdt.h> // Include the watchdog library

#define relayPin 9

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 151);

EthernetServer server(7000);
EthernetClient client;

String clientData = "";
String data="";

boolean IsClientConnected = false;

unsigned long currentMillis, previousMillis, reconnectMillis;
const unsigned long healthPacketInterval = 3000; // 3 seconds
const unsigned long reconnectInterval = 5000;   // 5 seconds
const unsigned long resetInterval = 60000;

String remove_all_repeating_substrings(String input_value) {
  int length = input_value.length();

  for (int i = 1; i <= length / 2; i++) {
    String substr = input_value.substring(0, i);
    int repeat_count = length / i;

    String repeated = "";
    for (int j = 0; j < repeat_count; j++) {
      repeated += substr;
    }

    if (repeated == input_value) {
      return substr;
    }
  }

  return input_value;
}

String extractData(String inputString) {
  if ((inputString.startsWith("QR") || inputString.startsWith("TG")) && inputString.endsWith("END")) {
    String extractedData = inputString.substring(2, inputString.length() - 3);
    return extractedData;
  } else {
    return inputString;
  }
}

bool isNumeric(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    if (!isDigit(str.charAt(i))) {
      return false;
    }
  }
  return true;
}

void setup() {
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(115200);

  while (!Serial) {
    ; // wait for the serial port to connect, needed for native USB port only
  }

  Serial.print("Machine Gate IP: ");
  Serial.println(Ethernet.localIP());

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  // Enable the watchdog timer to reset the Arduino upon power interruption
  wdt_enable(WDTO_4S); // 4-second timeout

  IsClientConnected = false;

  currentMillis = 0;
  previousMillis = 0;
  reconnectMillis = 0;
}

void loop() {
  // Reset the watchdog timer to prevent reset upon normal operation
  wdt_reset();

  if (!IsClientConnected) {
    if (!client.connected()) {
      client.stop(); // Ensure client is disconnected
    }
    
    EthernetClient newClient = server.available();
    if (newClient) {
      client = newClient;
      IsClientConnected = true;
      client.flush();
      Serial.println("Client Connected");
      client.println("Connected to Arduino");
    }
  }

  if (IsClientConnected) {
    if (client.available() > 0) {
      char thisChar = client.read();
      if (thisChar == '|') {
        clientData = "";
      } else if (thisChar == '%') {
        Serial.println(clientData);
        if (clientData.equals("OPENEN")) {
          Serial.println("Barrier is opening");
          digitalWrite(relayPin, LOW);
          delay(300);
          digitalWrite(relayPin, HIGH);
        }
      } else {
        clientData += thisChar;
      }
    }
    if (Serial.available()) {
      data = Serial.readStringUntil('\n');
      Serial.println(data);
      String result = remove_all_repeating_substrings(data);
      String entryData = extractData(result);

      if (isNumeric(entryData)) {
        String sendTagData = "|ENTG-" + entryData + "%";
        client.println(sendTagData);
        Serial.println(sendTagData);
      } else {
        String sendEntryQRData = "|ENQR-" + entryData + "%";
        client.println(sendEntryQRData);
        Serial.println(sendEntryQRData);
      }
    }
    // Check and send the health packet every 3 seconds
    currentMillis = millis();
    if (currentMillis - previousMillis >= healthPacketInterval) {
      previousMillis = currentMillis;
      client.println("|HLT%");
    }

    // Check if the client is still connected, if not, attempt to reconnect
    if (!client.connected()) {
      Serial.println("Client Disconnected");
      IsClientConnected = false;
      reconnectMillis = millis();
    }

    // Reconnect if the reconnect interval has passed and Ethernet link is down
    if ((!client.connected()) && (Ethernet.linkStatus() == LinkOFF)) {
      Serial.println("Attempting to reconnect...");
      wdt_enable(WDTO_15MS); // Trigger a reset
      delay(100); // Wait for reset to occur
      server.begin(); // Restart the server
    }
  }
}

/*
 ye same code jo hai mere esp32 based qr scanner ke liye hai
#include <Arduino.h>
#include <SPI.h>
#include <EthernetLarge.h>
#include <HardwareSerial.h>
#include "esp_task_wdt.h"

#define relayPin 13
HardwareSerial SerialPort(2);

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 151);

EthernetServer server(7000);
EthernetClient client;

String clientData = "";
String data = "";

boolean IsClientConnected = false;

unsigned long currentMillis, previousMillis, reconnectMillis;
const unsigned long healthPacketInterval = 3000; // 3 seconds
const unsigned long reconnectInterval = 5000;    // 5 seconds
const unsigned long resetInterval = 60000;

String remove_all_repeating_substrings(String input_value)
{
  int length = input_value.length();

  for (int i = 1; i <= length / 2; i++)
  {
    String substr = input_value.substring(0, i);
    int repeat_count = length / i;

    String repeated = "";
    for (int j = 0; j < repeat_count; j++)
    {
      repeated += substr;
    }

    if (repeated == input_value)
    {
      return substr;
    }
  }

  return input_value;
}

String extractData(String inputString)
{
  if ((inputString.startsWith("QR") || inputString.startsWith("TG")) && inputString.endsWith("END"))
  {
    String extractedData = inputString.substring(2, inputString.length() - 3);
    return extractedData;
  }
  else
  {
    return inputString;
  }
}

bool isNumeric(String str)
{
  for (size_t i = 0; i < str.length(); i++)
  {
    if (!isDigit(str.charAt(i)))
    {
      return false;
    }
  }
  return true;
}

void setup()
{
  Ethernet.init(5); 
  esp_task_wdt_init(4, true);
  Serial.begin(115200);
  SerialPort.begin(115200,SERIAL_8N1,16,17);
  Ethernet.begin(mac, ip);
  server.begin();
  while (!Serial)
  {
    ; // wait for Serial port to connect
  }

  Serial.print("Machine Gate IP: ");
  Serial.println(ip);

  Ethernet.begin(mac, ip);
  server.begin();

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  
  IsClientConnected = false;

  currentMillis = 0;
  previousMillis = 0;
  reconnectMillis = 0;
}

void loop()
{
  esp_task_wdt_reset();

  if (!IsClientConnected)
  {
    if (!client.connected())
    {
      client.stop(); // Ensure client is disconnected
    }

    EthernetClient newClient = server.available();
    if (newClient)
    {
      client = newClient;
      IsClientConnected = true;
      client.flush();
      Serial.println("Client Connected");
      client.println("Connected to ESP32");
    }
  }

  if (IsClientConnected)
  {
    if (client.available() > 0)
    {
      char thisChar = client.read();
      if (thisChar == '|')
      {
        clientData = "";
      }
      else if (thisChar == '%')
      {
        Serial.println(clientData);
        if (clientData.equals("OPENEN"))
        {
          Serial.println("Barrier is opening");
          digitalWrite(relayPin, LOW);
          delay(300);
          digitalWrite(relayPin, HIGH);
        }
      }
      else
      {
        clientData += thisChar;
      }
    }

   if(SerialPort.available())
    {
      data = SerialPort.readString();
      Serial.println(data);
      String result = remove_all_repeating_substrings(data);
      String entryData = extractData(result);

      if (isNumeric(entryData))
      {
        String sendTagData = "|ENTG-" + entryData + "%";
        client.println(sendTagData);
        Serial.println(sendTagData);
      }
      else
      {
        String sendEntryQRData = "|ENQR-" + entryData + "%";
        client.println(sendEntryQRData);
        Serial.println(sendEntryQRData);
      }
    }

    currentMillis = millis();
    if (currentMillis - previousMillis >= healthPacketInterval)
    {
      previousMillis = currentMillis;
      client.println("|HLT%");
    }

    if (!client.connected())
    {
      Serial.println("Client Disconnected");
      IsClientConnected = false;
      reconnectMillis = millis();
    }

    if ((!client.connected()) && (Ethernet.linkStatus() == LinkOFF))
    {
      Serial.println("Attempting to reconnect...");
      delay(100);
      esp_task_wdt_init(15, true);
      server.begin();
    }
  }
}
*/
