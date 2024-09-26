# TrailerController

An open-source trailer controller emulating a Tesla trailer control module.

The goal of this project is to openly document how to create a controller that plugs into the gray 3-pin connector included on Tesla vehicles, to be able to control a 4-pin US trailer connector without needing to interface with any of the vehicle's other wiring.

The vehicle used for testing is a US-spec, Fremont factory, 2022 Tesla Model 3 LR AWD built November 2021, which puts it as SOP5. The vehicle is a HW3, MCU2 (Intel Atom), with a standard 12V lead-acid battery and ultrasonic sensors. Vehicles built starting around December 2021 shipped with a higher voltage lithium ion low-voltage battery (around 15V) and MCU3 (AMD Ryzen) with no ultrasonic sensors, and while this project should hopefully work for them the higher voltage may require some component changes.

At the time of writing, this project is intended to be passive, only reading information from the vehicle computers without attempting to send any commands to the vehicle. This is safer from both a security perspective and from the perspective of potentially damaging the vehicle electronics.

## Motivation

Currently it is difficult or impossible to find documentation of how to interface with the Tesla trailer wiring to get a standard 4-pin connector. In Europe, the "gray connector" (as it's often referred to online, see [teslamotorsclub forum](https://teslamotorsclub.com/tmc/threads/model-3-tow-hitch-installation-ecohitch-and-near-factory-wiring.323758/)) connects to the Trailer Control Unit which interprets the signals from the vehicle's computer and adapts to the standard [European 13 pin connector](https://en.wikipedia.org/wiki/ISO_standards_for_trailer_connectors#13-pin_trailer_connector_(ISO_11446)). In the US the controller is missing but the wiring up to it is present (as of 2019), terminating in the gray connector with id `X041` in the Tesla service manual.

I have seen reports that you can use the OEM trailer control module for a Tesla Model Y with the Model 3, to get the US 7-pin connector, part number `1764896-98-D`. Used on ebay they look to run about $300 USD but provide an OEM route for this functionality. This alone does not enable the trailer mode in the software, but there are ways to do that with the Tesla Toolbox software.

There is an existing third-party module available online which goes by the part number `TM3-TDW06-US` which does precisely what this project aims to do. I cannot speak to the quality of the product, but it retails for about $400 USD and as far as I can tell the hardware and software are not open-source.

Instead of reusing this conveniently placed connector, most people opt for using "dumb" wiring harnesses that monitor the wires for the tail lights and turn signals and run a wire to a 12v source (sometimes all the way up to the battery in the front of the car) for power. Some of these wiring harnesses require electrically connecting to the wires by clamping onto and piercing the insulation of the wires, such as the "passive" wiring harness provided with the Stealth Hitch. Others monitor the wires using inductive sensors that wrap around the wire but do not pierce the insulation, but still require finding appropriate wires to intercept.

I have seen reports of people doing exactly what this project aims to do, ie listen to the communication from the vehicle over that connector and actuate the appropriate lights on the trailer, but have yet to see anyone document the process in a way that can be used by the community. Even if the wiring is relatively simple, decoding the communication is an annoying process to repeat for everyone who wants to create their own controller. I hope this project will eliminate the amount of rework required and provide a suitable starting point for anyone looking to create their own controller.

## General Plan

I expect there to be at least 3 phases of this project.

### Phase 0 - Decoding and Documenting

Documenting information about the connector on the vehicle and developing a controller that can listen to communications from the vehicle, so that we can also decode them and establish which messages indicate that the turn signals, brake lights, and headlights/taillights are active.

### Phase 1 - Functional Prototype

Taking the info from Phase 0 and building the first prototype trailer controller that will properly actuate the trailer lights via a 4-pin connector. This might be as basic as a bread-board or perf-board and not water-resistant, just a proof of concept of everything working together. This is sufficient for the purposes of this project, in that it provides all of the information required to build a custom trailer controller including functional software and hardware examples. It does not, however, get me personally to a state where I can use a trailer with my car for very long or in inclement weather.

### Phase 2 - Permanent Controller

Enhancing the prototype into something that can be installed permanently in the vehicle is a bit of a stretch goal for the project, but something that I want to personally achieve for my vehicle. This may involve custom PCBs and enclosures and additions to the hardware that are not required for a minimally viable product, eg temperature sensors and detecting if a trailer is connected. At this point I may also investigate sending data back to the vehicle to let it know if a trailer is connected, so that trailer mode could be engaged if enabled in the software (the controller cannot enable tow mode itself, the main software must be updated through Toolbox to allow it). While all the hardware and software produced at this stage will remain open-source, I may opt to sell completed or kit versions of the final result if there is interest.

## Docs

Check the docs folder for the most up-to-date information about the connector, communication, and the latest project status. These documents will be living documents during the research and development of this project.
