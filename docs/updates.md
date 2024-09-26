# Updates

## 2024-09-25

First attempt at using some fairly simple code to sniff the traffic on the LIN bus for the inductive phone charger did not really go well. May have been as simple as a timing problem, may have been that the code I found wasn't built to handle a real LIN bus and was intended for a bus of your own devices. I have ordered an inexpensive USB logic analyzer and am hoping looking at the raw signals will help me understand how to either modify the code I am using or write a whole new library if necessary.

I have also ordered a TJA1020 transceiver module. I am hoping not to need it for just sniffing the traffic but it might make development easier and will basically be required if we ever want to send data back to the controller and pretend to be the real trailer module.

You may have noticed up there that I mentioned the inductive phone charger LIN and not the trailer LIN. The connector for the trailer LIN, [X041](./X041.md), is pretty firmly trapped in the bumper in a way that prevents any access until you take the bumper off. I will reposition it when I install my hitch since I'll be in there anyway and I'm likely going to need to connect to it often during testing. Until then, for the purposes of testing whether or not I can even read data from one of the LIN buses, the bus for the inductive phone charger is much easier to access.

For reference, it's on PIN 6 of connector X930F in my model 3. That connector is the one in the bottom rear of the center console, the same one that OBD adapters are usually made to connect to. Pin 7 is conveniently a ground pin. You can disconnect that plug, stick two male jumper wires in, and have access to the LIN bus while sitting comfortably in the back seat of the car. If/when I also need 12V it can also be pulled off that connector, for example pin 15 (big one in the bottom left) looks to be the 12V for the USB ports.

I will likely continue to use that LIN bus for testing until I can reliably read data from the bus, then move to the trailer bus.