# LIN Decoding

The communications from the vehicle to the trailer control unit are performed over the thinner middle wire in the connector (see [X041.md](./X041.md)) which communicate via a [Local Interconnect Network](https://en.wikipedia.org/wiki/Local_Interconnect_Network), ie LIN. Notably, LIN is not CAN (Controller Area Network), but instead is a cheaper and simpler serial communication protocol that is bidirectional via a single communication wire.

During Phase 0 of the project this page will (hopefully) document the communication happening via the LIN wire and identify the message(s) that provide the information we need to run the trailer lights.

## Target information

This is the information that we definitely will need to read from the LIN to activate the correct lights.

- Brake light status
- Left/Right turn signal status
    - Will the messages indicate the current state of the light within the cycle (on/off) or instead just that left/right is requested?
    - Are hazard lights communicated differently?
- Taillight/Headlight status
    - Specifically the taillights are what is required, but that may only be communicated by if the headlights are active
    - If headlights only, regular vs brights may be communicated differently?

## Hardware Info

To connect to the vehicle and read the data I am planning on using an ESP32 with one of the serial RX pins connected to the LIN wire. Since the LIN uses 12V and the ESP32 is 3V3 and we only need to read data, a simple voltage divider to cut the voltage to about 1/5 of normal should suffice to prevent the ESP32 from being overvolted. I plan on powering the ESP32 directly from the 12V wire via the VIN pin and use the onboard regulator, but in the future a separate (probably switching) regulator to drop to 3V3 that can definitely handle up to about 20V should be used instead. 

I am opting for an ESP32 because WiFi capability means I can update the firmware and get data without having to run a wire to the controller from my computer (meaning I can hopefully sit in the drivers seat and read data as I activate things) and because there are [existing LIN libraries](https://github.com/iDoka/awesome-linbus) for the ESP32. I may instead swap to a Raspberry Pi Pico W in the future, or to a custom RP2040/RP2350 board. Using a standard and relatively inexpensive development board means that everything is easily replicated by anyone else wanting to create a custom controller using this information.

The source code provided in this project is set up to use PlatformIO via Visual Studio Code, which should make it relatively simple to update and flash on your own hardware.

## LIN Information

To be determined...