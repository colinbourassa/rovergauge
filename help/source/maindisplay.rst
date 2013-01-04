Main display
============
 * **MIL:** Lit red when the malfunction indicator lamp (MIL) is being lit by the ECU. This indicates that at least fault code is set.
 * **Communications:** The green lamp is lit when the software is successfully reading data from the ECU, and the red lamp is lit when there was a problem communicating with the ECU (caused by a bad connection, disconnected cable, misconfigured USB adapter, etc.) Both lamps will be off if there has not been any attempt to read from the ECU.
 * **Engine temperature:** Displays the temperature read by the engine/coolant temperature sensor. The green and red colored areas (representing "nominal" and "warning" temperature levels) are approximate.
 * **Road speed:** Displays road speed. Note that some vehicles (including TVR) use a different road speed transducer setup, and the measurement on those vehicles may not be updated above a certain speed.
 * **RPM (tachometer):** Displays engine speed in revolutions per minute. The redline is an adjustable cosmetic feature in this software, and does not represent any electronic limit in the ECU.
 * **Fuel temperature:** Displays the fuel temperature, which is also sometimes considered as the under-bonnet temperature.
 * **MAF reading:** Displays the reading from the mass airflow sensor as percentage of its maximum range. This display has two modes:
 
   - *Linear*: Shows the reading as a percentage of the maximum airflow. It will vary linearly as the sensor measures more mass.
   - *Direct*: Shows the reading as a percentage of the maximum sensor voltage (5VDC). It will vary logarithmically as the sensor measures more mass.

 * **Throttle position:** Displays the throttle position as a percentage of wide-open throttle (WOT). This display has two modes:

   - *Absolute*: Calculates the percentage by dividing the raw reading by the maximum theoretical reading. Even when the throttle is closed and resting on its stop, the "Absolute" reading will still show some positive value.
   - *Corrected*: Adjusts the minimum end of the range so that a closed throttle shows as 0%.

 * **Idle bypass position:** Displays the position of the idle air control stepper motor. 100% represents fully-open (greatest airflow) and 0% fully closed (restricted airflow).
 * **Target idle RPM:** The engine idle speed that the ECU would attempt to achieve under the current conditions.
 * **Selected gear:** The state of the neutral switch (for automatic gearboxes), or an indicator that a manual gearbox is fitted.
 * **Main voltage:** The voltage at the main relay.
 * **Lambda trim:** Fuel trim that is being applied as a result of feedback from the exhaust lambda sensors. There are two types of trim applied by the ECU, referred to as "short term" and "long term". The full effect of these fuel trim values is still under investigation.

   - *Short term*: These will remain at their initialized values during warm-up, and then typically change rapidly while the engine is running.
   - *Long term*:

 * **Current fuel map:** ID of the currently-selected fuel map, from 1 to 5. For most North American specification (NAS) Land Rovers, this will always be Map 5. In other markets, the map may be selected with the use of a "tune resistor" in the 14CUX wiring harness. A special sixth map, Map 0, is used for fail-safe/limp-home conditions.
 * **Adjustment factor:** This is a value associated with the current fuel map, and it seems to be used to adjust the fueling values for the engine displacement. It will change significantly between fuel maps intended for different engine displacements.
 * **Fuel map:** Displays the currently-selected fuel map as a 16x8 matrix. Each cell is colored as a quick visual indicator of the fueling values; higher color saturation corresponds to more fuel. Columns in the matrix correspond to different engine speeds, and rows correspond to different engine loads. The cell with the value closest to the fueling value currently in use will be highlighted by the software.
 * **Fuel pump relay:** The green lamp will be lit when the ECU is attempting to run the fuel pump.
