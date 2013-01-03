Main display
============
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
 * **Lambda trim:** Fuel trim that is being applied as a result of feedback from the exhaust lambda sensors. There are two types of trim applied by the ECU, referred to as "short term" and "long term".

