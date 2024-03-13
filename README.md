# WIFI-USB

This is a project designed for the <a href="https://www.lilygo.cc/products/t-dongle-s3?variant=42455191519413">LilyGo T-Dongle-S3</a> to provide a wifi accessible usb drive to remotely transfer files to 3d printers, lasers, cnc machines that have no network capabilities for control.<br>
the dongle will act like a normal usb drive in the target machine but it will allow you to manage the files on it via a web interface over wifi.
 


you must use the <b>LilyGo T-Dongle-S3</b> with this project.<br>

<a href="https://www.lilygo.cc/products/t-dongle-s3?variant=42455191486645">LilyGo T-Dongle-S3 With LCD</a> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <a href="https://www.lilygo.cc/products/t-dongle-s3?variant=42455191519413">LilyGo T-Dongle-S3 Without LCD</a><br><br>
<img src=https://github.com/stooged/WIFI-USB/blob/main/images/dongle0.jpg> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <img src=https://github.com/stooged/WIFI-USB/blob/main/images/dongle.jpg><br>
<img src=https://github.com/stooged/WIFI-USB/blob/main/images/dongle1.jpg><br>

<br>



you will need to add a sd card to the dongle, the sd card slot is in the bottom of the usb port as pictured below, you will need to remove the dummy card and insert a fat32 formatted sd card.<br>

<img src=https://github.com/stooged/WIFI-USB/blob/main/images/dongle2.jpg><br>
<br>


## Libraries

the project is built using <b><a href=https://github.com/me-no-dev/ESPAsyncWebServer>ESPAsyncWebServer</a> and <a href=https://github.com/me-no-dev/AsyncTCP>AsyncTCP</a></b> so you need to add these libraries to arduino.

<a href=https://github.com/me-no-dev/ESPAsyncWebServer>ESPAsyncWebServer</a><br>
<a href=https://github.com/me-no-dev/AsyncTCP>AsyncTCP</a><br>

<br>

if you wish to use the dongle with the lcd screen to display wifi info you will need to add the <a href=https://github.com/stooged/TFT_eSPI>TFT_eSPI</a> library from my github.<br><br>
<a href=https://github.com/stooged/TFT_eSPI>TFT_eSPI</a><br>
<br>

## Uploading the firmware to the dongle

installation is simple you just use the arduino ide to flash the sketch/firmware to the dongle.<br>

use the following board settings<br>

<img src=https://github.com/stooged/WIFI-USB/blob/main/images/board.jpg>