# DIY-Reflow-Hotplate
ESP32 code for a DIY reflow hotplate out of a clothing iron

This is a remake of the code for this:

https://www.instructables.com/The-Ultimate-DIY-Hot-Plate-Powered-by-ESP32-and-IR/

I have also added an SSD1306 based OLED screen to show the temperature data.

The code uses the SPIFFS on the ESP32 to house the webpage, and saves the curve.txt to the MicroSD card.
