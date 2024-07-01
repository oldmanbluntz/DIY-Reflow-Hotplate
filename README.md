# DIY-Reflow-Hotplate
ESP32 code for a DIY reflow hotplate out of a clothing iron

This is a remake of the code for this:

https://www.instructables.com/The-Ultimate-DIY-Hot-Plate-Powered-by-ESP32-and-IR/

I have also added an SSD1306 based OLED screen to show the temperature data.

The code uses the SPIFFS on the ESP32 to house the webpage files (in the data folder), and saves the curve.txt to the MicroSD card.

The curve generator script is exactly that. You can modify the temperatures and curve parameters manually.
The code reads a new line ever 250ms, so there are 240 datapoints per minute needed. 
The script currently takes 2 minutes (480 datapoints) to ramp up to 120 degrees C, then holds at 120C for 2 minutes (480 data points),
then takes another 2 minutes (480 data points) to ramp up to 200C, and holds at 200C for 5 minutes (600 datapoints).

So you just modify the amount of datapoints and the temperatures to get a different curve. It's rudimentary, but works.
