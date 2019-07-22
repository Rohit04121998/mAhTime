## Which battery and charger?

*Tuscan Gold* (model: TG-003) NiCd, NiMH battery charger to charge the *GP Battery* (17AAAH, 1.2V, C = 170mAh, NiMH,*1/3rd AAA*).

![1/3rd AAA battery](https://github.com/sudhi345/Measurement-Of-Capacity-Of-A-Battery/blob/master/Charger_hacked/battery_GP.jpg)

## Why the charger needs modifications?

Rated current capacity of the charger for the AAA size batteries (2S) is 100mA which is NOT the recommended amount of current for the 
mentioned tiny batteries. For the NiMH battery it is recommemnded to charge at 0.1C or below (usually 0.05C) but very low current through 
the batteries will result in a long charge duration.  Another problem with these tiny batteries is that they can not be installed in the given 
slot as that is for full sized AAA batteries.

## What are the modifications?

1. The over current problem was solved by adding a 147ohm resistor is series with the charger terminals and achieved a current flow of around 
12mA (~0.07C). Thses batteries (NiMH/NiCd) don't have floating terminal potential like Li-Ion batteries. So the change in voltage due to the 
added resistor across the terminals doesn't matter actually.
2. A pair of Molex connector was added to the charger with the wires connecting the terminals (One connector for 2S configuration. Resistor and
connections are invisible).

![Hacked charger](https://github.com/sudhi345/Measurement-Of-Capacity-Of-A-Battery/blob/master/Charger_hacked/charger_tuscan.jpg)


