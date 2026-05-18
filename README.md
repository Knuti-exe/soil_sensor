# Soil Moisture Sensor
This project utilizes an ESP32-C3 powered by a battery. The main purpose of the device is to:
- Measure soil moisture.
- Monitor battery voltage.
- Broadcast data via BLE (Bluetooth Low Energy).

The device acts as a BLE beacon, broadcasting data to a gateway. The gateway then forwards this information to a central system using other protocols, such as MQTT over Wi-Fi (see a sample implementation in [Window Roller](https://github.com/Knuti-exe/esp-idf_window-roller)).

Why BLE instead of Wi-Fi?
Using BLE is more efficient than sending data directly via Wi-Fi/MQTT because:
- No DHCP overhead: Obtaining an IP address via DHCP is time-consuming (though it can be optimized).
- Lower power consumption: The Wi-Fi radio is significantly more power-hungry than BLE.
- Connectionless overhead: Standard MQTT requires a stable TCP connection and an acknowledgment from the Broker, which keeps the radio active for longer.

With BLE, the device wakes up, measures data, and broadcasts it in under 1 second.

Hardware Components
- ESP32-C3 SuperMini
- Li-ion Battery (~700mAh in my case)
- TP4056 charging module
- Capacitive Soil Moisture Sensor v2.0
- 2 x 100kΩ Resistors (Voltage divider)

> [!WARNING]
> I am using a simple voltage divider to scale the battery voltage down, preventing the ESP32 ADC from being exposed to voltages above its limit (4.2V from a fully charged battery).

> [!NOTE]
> While a small capacitor on the ADC pin is recommended for noise reduction, I have implemented multisampling in the code to ensure measurement stability and accuracy.

Roadmap / TODO
- [x] Initial Readme
- [x] BLE Broadcasting
- [x] ADC Implementation (Moisture & Battery)
- [x] Deep Sleep optimization
- [x] Battery status monitoring
- [x] Support for multiple moisture sensors
- [x] Bug fixes and stability improvements
- [ ] Code refactoring (optional)
