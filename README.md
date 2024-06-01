in this i have used esp32 with enc28j60 module. connection is something like this 

1) EthernetENC  -       ESP32
VCC       -       smps 5v
GND       -       smps gnd
cs        -       D5
si        -       D23
sck       -       D18
s0        -       D19

2) SCANNER    -    ESP32
vcc        -   smps 5V
gnd        -   smps gnd
Tx         -   RX0
RX         -   TX0

3) ESP32 ke miniusb port ko esp32 ko connect krenge. aur usb side wale wires ko cut krke smps se connect krenge esp32 ko power dene ke liye
