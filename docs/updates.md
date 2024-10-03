# Updates

## 2024-09-25

First attempt at using some fairly simple code to sniff the traffic on the LIN bus for the inductive phone charger did not really go well. May have been as simple as a timing problem, may have been that the code I found wasn't built to handle a real LIN bus and was intended for a bus of your own devices. I have ordered an inexpensive USB logic analyzer and am hoping looking at the raw signals will help me understand how to either modify the code I am using or write a whole new library if necessary.

I have also ordered a TJA1020 transceiver module. I am hoping not to need it for just sniffing the traffic but it might make development easier and will basically be required if we ever want to send data back to the controller and pretend to be the real trailer module.

You may have noticed up there that I mentioned the inductive phone charger LIN and not the trailer LIN. The connector for the trailer LIN, [X041](./X041.md), is pretty firmly trapped in the bumper in a way that prevents any access until you take the bumper off. I will reposition it when I install my hitch since I'll be in there anyway and I'm likely going to need to connect to it often during testing. Until then, for the purposes of testing whether or not I can even read data from one of the LIN buses, the bus for the inductive phone charger is much easier to access.

For reference, it's on PIN 6 of connector X930F in my model 3. That connector is the one in the bottom rear of the center console, the same one that OBD adapters are usually made to connect to. Pin 7 is conveniently a ground pin. You can disconnect that plug, stick two male jumper wires in, and have access to the LIN bus while sitting comfortably in the back seat of the car. If/when I also need 12V it can also be pulled off that connector, for example pin 15 (big one in the bottom left) looks to be the 12V for the USB ports.

I will likely continue to use that LIN bus for testing until I can reliably read data from the bus, then move to the trailer bus.

## 2024-09-26

While I wait for the USB logic analyzer to arrive I realized that I could probably use the ESP32 itself as a logic analyzer. Using [this project](https://github.com/EUA/ESP32_LogicAnalyzer) with a tweak to the `platformio.ini` file to use `espressif32@4.1` to fix the compatibility with `esp32doit-devkit-v1`, I was able to succesfully read the data from the inductive charger LIN bus, though with the sensors disconnected so there was no response data (ideally I will be able to intercept it while active). That was enough to confirm some details of that LIN bus that will hopefully translate over. I'll update the [LIN bus doc](./LIN-Decoding.md) with some of that info. Additionally the sigrok/PulseView file itself is in the phase0 docs folder, [here](/src/phase0/data/IC_LIN_Trace).

Assuming the setup is the same for the trailer LIN, we at least know the protocol version (2), baud rate (19200), and to expect the amount of data per response to change between 2, 4, and 8 bytes. I think that might be why the code I was using earlier failed since I believe it was assuming the response was always 8 bytes. If I can change that to pick the right number of response bytes, maybe it'll work.

## 2024-09-29

Installed my hitch yesterday and freed the connector for the trailer wiring so that I can easily connect/disconnect from it. Ideally anyone following this will not need to move it and can leave it in the relative safety of the upper part of the bumper on the support beam, but I need to be able to freely and temporarily connect things to it.

With that available I was able to get readings from the LIN bus with the turn signals, brakes, and lights on. Now I just have to analyze the different frames and see what's different in each scenario.

## 2024-10-02

For Phase 1, the prototype controller, I am swapping to a Pi Pico W from the ESP32. I want to experiment with different hardware a bit and there are aspects of the Pico and its production that I prefer. Sadly the nice OTA library I was using doesn't play as nice, though might be able to come back without the async functionality. Also realized the other day that the MOSFETs that I got are better for low-side switching and we need high-side switching, so I got some others I need to do testing with. Annoyingly it looks like I need to use both types to be able to switch it. I have some experience with building circuits and PCBs and such, but not quite the level that would make this project easy for me.