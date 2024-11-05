# Prototype 1

This is the result of Phase 1, a fully functional but fairly basic initial prototype. This can be used as-is, though if you're going to make it I recommend some tweaks that you can look into implementing yourself.

## Final Result First

Here's the final result first, if you want to read more about the PCB and the components you can keep scrolling down.

![Prototype Picture](/docs/images/prototype/prototype1.jpg)
![Prototype Picture](/docs/images/prototype/prototype2.jpg)
![Prototype Picture](/docs/images/prototype/prototype3.jpg)
![Prototype Picture](/docs/images/prototype/prototype4.jpg)
![Prototype Picture](/docs/images/prototype/prototype5.jpg)

## PCB Design

The PCB was designed with LibrePCB, which is free and cross-platform. The files for it can be found in the pcb/phase1 folder and LibrePCB will interface directly with PCBWay (who I used) and other fabs to get boards uploaded and manufactured without having to do any exporting of files or much in the way of configuration. I did opt to put `WayWayWay` at the bottom of the board to force the PCBWay serial into that location, make sure to check that option if you use PCBWay or remove that text if you don't care.

I am not very experienced with PCB design so there are definitely improvements that can be made. I wanted to provide what I created and used as a stepping stone for anyone else who might want to do something similar. If you are more experienced or think you can make something better, please do and share it!

![Prototype Board](/docs/images/prototype/prototype_board.png)
![Prototype Schematics](/docs/images/prototype/prototype_schematics.png)

## Enclosure

The enclosure was designed in OnShape. I tried to learn FreeCAD and it was too frustrating for me coming from Fusion 360, but I wanted something that worked on Linux. With OnShape being browser based it worked fine in Ubuntu or on my Windows desktop. With the public link you should be able to copy it to your own account to modify, or export it in whatever format you want. Again, I'm experienced enough to make something **functional** but not necessarily something **good**. I am not sure how this will hold up over time, or how water resistant it is. I printed it with a 0.6mm nozzle at 0.4mm layer height, but it was designed to also support 0.4mm nozzles for the walls. There's a rough model of the PCB with the terminal blocks, Pico, and MOSFETs included to help with positioning.

https://cad.onshape.com/documents/38c75cc318c0ba98099f4117/w/3ef348a47c70d4d10d24442a/e/2bfd670c77f872a4c310e10b?renderMode=0&uiState=672972c0c370e522f0c73bde

## Bill of Materials

If you want to build your own copy of one of these boards, here are the materials I used.

### Main PCB

| Name                                     | Quantity | Link                                                                                                              | Notes                                                                                                                                                                                                                                                                                                                          |
| ---------------------------------------- | -------- | ----------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Pi Pico W                                | 1        | [Raspberry Pi Official Site](https://www.raspberrypi.com/products/raspberry-pi-pico/?variant=raspberry-pi-pico-w) | You can use the non-wifi Pico if you want more security, but lose out on easy updates and control.                                                                                                                                                                                                                             |
| 1x20 female pin headers                  | 2        | generic                                                                                                           | For the Pi Pico. You could optionally solder it directly. I had a 1x40 already that I cut in half.                                                                                                                                                                                                                             |
| IRLB8721 N-Channel MOSFET TO-220 package | 3        | https://www.amazon.com/dp/B07LG5BCDY                                                                              | These are overkill for this application, you really just need something that operates at 3V3 gate voltage. They will not carry any real current.                                                                                                                                                                               |
| IRF4905 P-Channel MOSFET TO-220 package  | 3        | https://www.amazon.com/dp/B0CBKH44K5                                                                              | There are probably better choices but I had bought the N-Channels first before I realized you can't high-side switch with them and these are what I could get quickly.                                                                                                                                                         |
| 3V3 Regulator                            | 1        | https://www.amazon.com/dp/B0B779CGJQ                                                                              | You could use a linear regulator instead for simplicity but that adds more heat. These have crappy spacing, 0.3625"x0.6". I used single male pin headers to affix them to the main board. If you plan to plug in USB to the pico while it's onboard, I recommend adding a diode because this doesn't like being run backwards. |
| 47kΩ resistor                            | 1        | generic                                                                                                           | Only non-10kΩ resistor, used to drop the LIN Bus voltage to something the Pico can work with. This is R1 on the board.                                                                                                                                                                                                         |
| 10kΩ resistor                            | 7        | generic                                                                                                           | Used in the voltage divider for the LIN Bus and as pull-down/pull-up resistors on the MOSFETs.                                                                                                                                                                                                                                 |
| 4x5mm/0.2" terminal blocks               | 2        | generic, maybe https://www.amazon.com/dp/B0D1GMMTZ5                                                               | I already owned ones that were 2x so I used two next to each other. You can use a 3x on the car side, the fourth hole doesn't do anything but provide the option of using a 4x or two 2x terminals.                                                                                                                            |

### Enclosure/Other

| Name                                                      | Quantity | Link                                                | Notes                                                                                                                                                                 |
| --------------------------------------------------------- | -------- | --------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| M3 Heat-Set Inserts                                       | 10       | https://www.amazon.com/dp/B0CS6VZYL8                | 4 for the PCB, 6 for the lid                                                                                                                                          |
| M3 bolts                                                  | 10       | generic                                             | Mine were 6mm so that's what I designed around, but they are WAY too long. Get shorter ones, like 3 or 4mm. I like hex caps personally.                               |
| Trailer Wire Harness                                      | 1        | https://www.amazon.com/dp/B0CM3Q5XL8                | The hole in the side is designed to fit the shielding of this pretty tightly. Others may work just as well.                                                           |
| Trailer Wire Tester                                       | 1        | https://www.amazon.com/dp/B07RT9CN3W                | Purely optional, nice for bench testing or if you don't have a trailer to test with.                                                                                  |
| Sumitomo 6188-0129 connector (with wire seals and blades) | 1        | https://www.corsa-technic.com/item.php?item_id=1846 | See [X041.md](/docs/X041.md) for more details.                                                                                                                        |
| 12 gauge wire                                             | 3        | generic                                             | I had 12 gauge on-hand in red/black/green so that's what I used and what the holes are sized for.                                                                     |
| Wire ferrules                                             | 3 or 7   | https://www.amazon.com/gp/product/B07WRQN45C        | These are generally recommended when you're using terminal blocks and stranded wire, but not strictly required.                                                       |
| 10-amp In-line Automotive Fuse                            | 1        | https://www.amazon.com/gp/product/B01E5MM63C        | I've had this set for a while. A fuse is not strictly required but highly recommended. Might be tricky to replace if you tuck the TCU on top of the aluminum support. |
