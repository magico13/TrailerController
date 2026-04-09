# Updates

## 2024-09-25

First attempt at using some fairly simple code to sniff the traffic on the LIN bus for the inductive phone charger did not really go well. May have been as simple as a timing problem, may have been that the code I found wasn't built to handle a real LIN bus and was intended for a bus of your own devices. I have ordered an inexpensive USB logic analyzer and am hoping looking at the raw signals will help me understand how to either modify the code I am using or write a whole new library if necessary.

I have also ordered a TJA1020 transceiver module. I am hoping not to need it for just sniffing the traffic but it might make development easier and will basically be required if we ever want to send data back to the controller and pretend to be the real trailer module.

You may have noticed up there that I mentioned the inductive phone charger LIN and not the trailer LIN. The connector for the trailer LIN, [X041](./X041.md), is pretty firmly trapped in the bumper in a way that prevents any access until you take the bumper off. I will reposition it when I install my hitch since I'll be in there anyway and I'm likely going to need to connect to it often during testing. Until then, for the purposes of testing whether or not I can even read data from one of the LIN buses, the bus for the inductive phone charger is much easier to access.

For reference, it's on PIN 6 of connector X930F in my model 3. That connector is the one in the bottom rear of the center console, the same one that OBD adapters are usually made to connect to. Pin 7 is conveniently a ground pin. You can disconnect that plug, stick two male jumper wires in, and have access to the LIN bus while sitting comfortably in the back seat of the car. If/when I also need 12V it can also be pulled off that connector, for example pin 15 (big one in the bottom left) looks to be the 12V for the USB ports.

I will likely continue to use that LIN bus for testing until I can reliably read data from the bus, then move to the trailer bus.

## 2024-09-26

While I wait for the USB logic analyzer to arrive I realized that I could probably use the ESP32 itself as a logic analyzer. Using [this project](https://github.com/EUA/ESP32_LogicAnalyzer) with a tweak to the `platformio.ini` file to use `espressif32@4.1` to fix the compatibility with `esp32doit-devkit-v1`, I was able to successfully read the data from the inductive charger LIN bus, though with the sensors disconnected so there was no response data (ideally I will be able to intercept it while active). That was enough to confirm some details of that LIN bus that will hopefully translate over. I'll update the [LIN bus doc](./LIN-Decoding.md) with some of that info. Additionally the sigrok/PulseView file itself is in the phase0 docs folder, [here](/src/phase0/data/IC_LIN_Trace).

Assuming the setup is the same for the trailer LIN, we at least know the protocol version (2), baud rate (19200), and to expect the amount of data per response to change between 2, 4, and 8 bytes. I think that might be why the code I was using earlier failed since I believe it was assuming the response was always 8 bytes. If I can change that to pick the right number of response bytes, maybe it'll work.

## 2024-09-29

Installed my hitch yesterday and freed the connector for the trailer wiring so that I can easily connect/disconnect from it. Ideally anyone following this will not need to move it and can leave it in the relative safety of the upper part of the bumper on the support beam, but I need to be able to freely and temporarily connect things to it.

With that available I was able to get readings from the LIN bus with the turn signals, brakes, and lights on. Now I just have to analyze the different frames and see what's different in each scenario.

## 2024-10-02

For Phase 1, the prototype controller, I am swapping to a Pi Pico W from the ESP32. I want to experiment with different hardware a bit and there are aspects of the Pico and its production that I prefer. Sadly the nice OTA library I was using doesn't play as nice, though might be able to come back without the async functionality. Also realized the other day that the MOSFETs that I got are better for low-side switching and we need high-side switching, so I got some others I need to do testing with. Annoyingly it looks like I need to use both types to be able to switch it. I have some experience with building circuits and PCBs and such, but not quite the level that would make this project easy for me.

## 2024-10-06

Prototype PCB design is finished enough to send off to get the first samples. Fingers crossed I didn't mess anything up too badly, I'll know in about a week if the estimates are to be trusted. With only 1 oz copper the lighting traces might only be good for about 3 amps, I'll have to test one of the boards at high current and see how it behaves. Ideally for a final product I'd double the copper but that increased the cost significantly, not worth it for a prototype where I know I'm only going to drive LEDs and I don't know if the board is even gonna work. Here's hoping I'll have some assembly and testing news in my next update.

## 2024-11-04

Prototype boards arrived and I've built one up. Works more or less perfectly! I did tweak the silk screen a bit since then to hopefully make the labels for the terminals more clear when the terminal blocks are attached, right now they are mostly covered. Designed and printed a pretty basic enclosure but it's both too big and too small. The horizontal space inside is a little tight, you may want to make it a bit wider to give more room for the wires coming into the terminal blocks. But it's also pretty tall, which could be improved if you bent the MOSFETs instead of leaving them straight like I did. You could go even thinner if you directly mounted the Pico. As it stands I've got it tucked into the space between the back of the bumper and the battery, my cable from the car is pulled all the way down so I can easily plug it in or take it out. I do not know if the case as designed will fit inside the bumper on top of the aluminum support where the connector from the car is normally located. I'll post pics and some more info in [prototype.md](/docs/prototype.md)

## 2026-04-05

Between the last update and now I had kept the prototype in my car for several months, over the winter even, and had used it to tow a Home Depot 5'x8' trailer once without issue. I took it out to see how it was doing, especially because I hadn't sealed the holes in the enclosure and where it sat on the underbody tray would often end up with puddles of water. Several of the exterior screws I used had rusted but the PCB itself looked fine, however once screw got stuck to the brass insert and I ended up ripping it out of the enclosure so I needed to print a new one. I figured since I needed to print a new enclosure anyway, it'd be a good time to redesign it to be smaller in the hope of fitting it higher up in the bumper and away from standing water. I'll put a few pictures of the new design up.

I've periodically been making small updates to the firmware. I'm trying to rewrite the LIN decoder because although it works perfectly fine for the short message used for the lights control, it struggles with longer messages and keeping up with the full unfiltered message list. I'd like to add a built-in data logger for monitoring the LIN bus in case anything changes or I want to try to monitor any of the other messages on the bus, although this particular one should only be the trailer hitch ECU and the wireless charger ECU.

Also since then, the Highland Model 3 in the US has gotten an official tow hitch, and the Highland appears to use the exact same wiring setup, meaning the ECU for that model should work just fine in earlier model years. You can find that ECU on the Tesla parts catalog under `1112581-00-A` and per their website it's only $60, though on eBay it's over $100. I have ordered one to try it out, if that works without issue then it would definitely be the recommended way to handle the trailer lights even with an aftermarket hitch. If you go this way, make sure to also order `1446560-00-A` which is the wiring from the ECU to the 4-pin connector, which is an additional $30.

And finally, I will provide some documentation around the LIN bus that you can access in the center console. I noticed I hadn't documented that thoroughly, including writing down the connector number (it's `X930`) anywhere for looking up the pinout, which caused me a lot of wasted time trying to find that again when I wanted to set up a more permanent test setup in my car with the same connector as what's in the bumper area.

## 2026-04-08

Good news! The parts listed above from Tesla seem to work perfectly! Even without the towing software installed (which is technically already installed but not enabled, you supposedly can enable it with Toolbox but that's like $75 minimum just to rent the software for a day), when plugging in a trailer (or just a tester in this case) the car will display a yellow trailer icon on the screen. It will also show red if one of the lights doesn't appear to be working, though doesn't seem to tell you which of them is faulty. Based on the lights on my tester, the ECU just sends a pulse every second to see if the lights are working and a trailer is attached. I am not sure if that only happens in park or even in drive and I haven't had a chance to sniff the LIN bus to see what data the ECU is sending back to the main controller.
