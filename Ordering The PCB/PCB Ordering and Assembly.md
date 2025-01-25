# The PCB

## Ordering the PCB and other components

This details how to order the SolarBeam PCB from JLCPCB. The process is likely similar with other PCB manufacturers.

1. Visit https://cart.jlcpcb.com/quote to set up an order.
2. Log in at the top right of the page.
3. Download the zipped "SolarBeam_gerbs.zip" folder, and upload it using the big blue button.
4. Select the following options in the menu beneath this button:\
Base Material: FR-4\
Layers: 2\
Dimensions: (Automatically fills, DO NOT change this)\
PCB Qty: Any multiple of 5\
Product Type: Industrial/Consumer Electronics\
Different Design: 1\
Delivery Format: Single PCB\
PCB Thickness: 1.6\
PCB Color/Silkscreen: Your preference, the fastest delivery is green/white\
Surface Finish: HASL (with lead)\
Outer Copper Weight: 1 oz\
Via Covering: Untented\
Min Via Hole Size: 0.3mm\
Board Outline Tolerance: ±0.2mm (Regular)\
Confirm Production File: No (or your preference)\
Mark on PCB: Order Number (or your preference)\
Electrical Test: Flying Probe Fully Test\
Gold Fingers: No\
Castellated Holes: No\
Edge Plating: No\
\
5. Select the “PCB Assembly” option below this menu, and select the following options in the new menu:\
PCBA Type: Standard (economic may work but is untested)\
Assembly Side: Top\
PCBA Qty: (the same as PCB Qty above)\
Edge Rails: Added By JLCPCB\
Confirm Parts Placement: No\ 
Parts Selection: By Customer\
\
(Advanced Options)\
Photo Confirmation: No\ 
Conformal Coating: No\ 
Packaging: Antistatic Bubble Film\  
Solder Paste: Sn 96.5%\ 
Add paste: No\
Board Cleaning: No\ 
Bake Components: No\
Depanel Boards: No (It’s easy to do yourself, just takes a bit of force)\ 
Flying Probe Test: No\ 
Others: No\

6. Now, on the right hand side, double check the charge details and select the faster board printing that comes free with PCB assembly services, and choose a shipping method. You may have coupons available underneath this option as well. 
7. Click the blue “next” button, and under “Bill of Materials” at top of the page and upload brainBox_bom.csv and brainbox_cpl.csv found in the pcba folder where prompted. 
8. Process them and ensure that all the components are still in stock at JLCPCB. (If they are not, equivalent parts are available, and you will need to update the brainBox_bom.csv file. Simply search JLCPCB for the text in the “Footprint” column, and when you’ve found an exact copy, replace the LCSC code in the next column with a new one.)
9. Select “next” at the bottom right. This brings up a “component placements viewer” which may incorrectly show some components rotated 90 degrees. Simply ignore this and select “next” again. 
10. Ensure that your pricing is right, select a product description (I had chosen “electronic musical instruments”), and save the order to your cart. 
11. Double check everything and select the secure checkout button when you’re satisfied to continue your order. 
12. Check the "SolarBeam_bom-USER.csv" and order all of the components here. They will need to be soldered onto the board afterwards. 


## Assembly

From the JLCPCB settings the PCB will arrive attached to a larger panel which can be removed with some force or cutting. Additionally, smaller circuit components have been soldered on, but the listed components will need to be manually soldered on as marked by the designators on the PCB. Make sure to solder in a well-ventilated area and to not use too much solder to avoid contacts touching and causing shorts.

1. J1 (DC Barrel Jack)
2. J2 (2 Pin Jumper)
3. Use spares for mounting the Teensy to the PCB.
4. J3 (2x9 Pin Jumper)
5. You can forego ordering parts for the 2 Pin Jumper and cut off 2 pins for J2 but might be tricky with needing to cut one segment.
6. C1 (33+ uF Electrolytic Capacitor)
7. Ensure polarity on capacitor is correct to the PCB as indicated. Look for the indicators on the capacitor (negative markings, shorter length wire) to match with the PCB before soldering.
8. D2 (RGB LED)
9. This is a 4-pin component with the layout having the first pin be short followed by the longest second pin then the next two in decending length to correspond to the RGB values. The marking on the PCB will orient the component with the first pin marked in a square.
10. U2 (Teensy 4.0)
11. Comes without header pins, so use spare pin jumpers to align and mount the component to
the PCB.
12. RV1-8
13. Does not come with knobs, but standard knobs 6mm in diameter will fit.
14. J4-11
15. The listed component will fit the cutouts for the PCB, but other jacks might as well.

When soldering is complete, to ensure the PCB is assembled correctly, plug in the Teensy to a computer and supply power to the board and check to see if the LED indicator light is turned on.
The Teensy should also be identified in the system's USB ports

