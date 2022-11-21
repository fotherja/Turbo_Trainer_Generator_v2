# Turbo_Trainer_Generator_v2
Spinner NXT stationary bike drives a 149kv BLDC outrunner via a flat belt. Regen is done by the VESC 

I bought a second hand NXT Spin bike. They are well made with a heavy flywheel and an efficient single speed chain drive
I bought a flat belt and connected this such that the flywheel spins a 149kv BLDC Outrunner.
The 3-phase outrunner is connected to the VESC6 BLDC control board
The VESC communicates via UART with an arduino pro-mini
The Pro-mini reads the battery voltage and current from the VESC to measure power (into the battery)
The Pro-mini runs a PI controller which adjusts the requested braking torque such that a selected mechanical power is exerted on the crankshaft
I took lots of measurments to arrive at calibrated crankshaft powers of 50, 100, 150, 200, 250, 300 Watts
The regenerated power is fed into a 24V 20AH LiFePO4 battery (about 480Wh)
When the battery is reaching full capacity I have a grid tie inverter which discharged the battery into the electrical grid 
This is an incredibly efficient setup - I'd challenge anyone to have a more efficient way of turning human power into electrical power!
