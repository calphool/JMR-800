# JMR-800

![Picture of PG-800](PG-800.png)
![PCB View of JMR-800](3d_image.png)

**JMR-800** is a modern, open-source clone of the Roland PG-800 hardware programmer, built around the Teensy 4.1 microcontroller. It provides real-time 
parameter control for vintage Roland synthesizers such as the JX-8P, MKS-70, and JX-10 using the original PG-800 communication protocol.

The JMR-800 replicates the original PG-800's functionality and physical interface, enabling direct, tactile editing of synthesizer parameters through a 
hands-on control surface.

## Features

- Compatible with Roland synths that support the PG-800 interface
- Built on Teensy 4.1 with fast, low-latency signal handling
- Accurate emulation of the PG-800 protocol (Clock, Ready, and Data lines)
- Designed for easy assembly and expansion
- Open hardware and firmware

## Status

This project is currently in active development. Basic communication and parameter changes are being validated. Contributions, forks, and collaboration are 
welcome.

April 10, 2025

- Redesigned PCB to get rid of the 74HC165 input bit shifter and replaced it with another analog mux/demux.  The total number of parameters that can be 
tweaked with the JX8P with the Vecoven PWM enhancement is 52.  So, I need to support at least 52 pots, and I can only support 48 with three muxes.  I mean
there are enough ADCs on the Teensy 4.1, that I probably could hack it and get those last 4 with separate ADC pins on the Teensy, but that's a pretty 
ineligant design, so I'll just add another mux/demux, get rid of the bit shifter, and limit my use of push buttons to just a few.  I technically only 
need one or two (the PG-800 only has 2), but I'll probably come up with a few odds and ends to implement (like maybe a way to load a whole sound from 
the JMR-800 by tweaking all of the parameters based on loading something from an SD card or something), so having a few more won't hurt.

- Decided to lay out the whole interface with my 3D printer as a template first.  This will allow me to cut cleaner holes into the aluminum case I think.
I'm using an aluminum case from Jameco that looks pretty retro / period correct.


April 14, 2025

Worked on the aluminum case a bit today.  Ran out of potentiometers, so ordered some more.

![Picture of controller in process](in_process_pic_1.png)


April 17, 2025

Worked on the case some more.  Completed the potentiometer installation, got some knobs and installed them, though I don't like the fact that I didn't get 
an even number of red and blue, so I need to get another kit of them.  I also don't like the white ones, I think I'll remove those and replace with black.  
Finally, I couldn't find black 3mm screws, so I had to use stainless steel.  I'm going to change that, it ruins the asthetic.

Once I get all that settled, it's back to the software and firmware.  I redesigned the PCB about a week ago, but I think I'm going to need to do a slight 
additional redesign.  The four push buttons have two LEDs on them, a red one and a green one.  I really don't know exactly what I'm going to use them 
for yet, but I'm going to need an output shift register to manage all 8 of them (as well as current limiting resistors for each one, which would probably 
be best on the board rather than dangling and inline with the wires.

The software already works in a prototype way (two potentiometers that I tested with), so hopefully there won't be any surprises when I get all of them 
running.  I also added an additional 3.3V regulator that's optional on the new design, since I'm not altogether sure that the tiny little regulator on the 
Teensy can handle all the load I'm throwing at it.

![Picture of controller in process - top view](in_process_pic_2.jpg)


April 29, 2025

![completed hardware](in_process_pic_3.jpg)

Got the new PCB and the rest of the hardware put together.  I wrote some temporary code that implements a "TEST MODE" that shows the condition of all 
the knobs.

May 3, 2025

I've updated the code quite a bit.  It now has 3 modes.  Run mode, Test mode, and Config mode.  I've been working on the Config mode.  You can change 
into these different modes by pressing Button 1 and Button 2 simultaneously.  When you configure a knob, you can change its name, its command byte, 
and select which type of knob it is.  I've got the command byte selection finished, and I'm working on the name update code now.  Once I get all the 
config stuff finished (perhaps this evening), I'll begin working on the Run mode.  This stuff was prototyped already with the earlier version of the 
board, so I'm pretty sure the code for interacting with the synth will work.

I may eventually implement some kind of "preset mode" that allows you to scroll through a given configuration of knobs, select it, and it would play 
all 56 knobs into the synthesizer.  This may be a submenu of the Run mode.  One step at a time of course...

May 5, 2025

***It's alive***!  I got the first fully functional code put together and just did about 15 minutes of testing.  It definitely works.  Though there's 
plenty to tweak.  Right now it's ignoring the parameter type and just sending values from 0 - 127, which does work, but it causes the screen to flicker 
on the synth for the parameters that only have a few set values.  I'm also wondering about how to manage noise from the potentiometers.  Although I 
used a smoothing algorithm, which does make them a lot less noisy, you still get odd little blips from time to time, and since they take whatever 
value the knob is at at the time, if you've just changed patches it can make a dramatic change to the sound.  I wonder if I should implement a 
kind of "lock / unlock" with one of the switches.  I could use green for "I'm unlocked and sending data to the synth" vs red for "I'm currently 
locked and not sending anything to the synth."

Lots and lots of small tweaks to work on now, but at least it's working.  The PWM mod that I installed from https://www.vecoven.com/superjx/styled/pwm.html
is pretty cool, especially with knobs that tweak its behavior in real time.

The other tweaks that the vecoven upgrade made to my firmware on my synth still takes some getting used to.  I'm going to have to read through the 
manuals for the vecoven upgrade really carefully, because it's pretty easy to get lost in the menus.  Maybe I could implement a "notepad" in the 
OLED screen of the JMR800 so it can remind me of stuff for the Vecoven upgrade.

One thing that I noticed that I'm going to have to deal with is the fact that not all the pots return the full range from 0 - 127 when sent through the 
ADC.  They came in a big bag from Aliexpress, so they're probably pretty low quality.  I'm going to need to implement some way to measure their min and 
max values, probably in the config mode, and then scale the pot value for each pot in software.


May 11, 2025

Finished all the screen stuff for Run mode, which is a pretty big milestone.  Also added an SVG file that represents the console layout. Working on an 
overlay using a Glowforge and acrylic.

## License

This project is released under the MIT License.

