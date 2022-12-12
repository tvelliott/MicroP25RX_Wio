# MicroP25RX_Wio
<img src="https://github.com/tvelliott/MicroP25RX_Wio/blob/main/images/monitor.png">
<img src="https://github.com/tvelliott/MicroP25RX_Wio/blob/main/images/tglog.png">
Firmware source for the Wio-terminal portion of the MicroP25RX product


Build Instructions:

Install Arduino IDE For Your Platform
https://www.arduino.cc/en/software/  (DO NOT use versions newer than 1.8.xxx. They don't work as of 12-12-22)

1) Open The Arduino IDE, click on File > Preferences, and copy below url to Additional Boards Manager URLs:
https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
2) Click on Tools > Board > Board Manager and Search Wio Terminal in the Boards Manager.
3) Select your board and port. You'll need to select the entry in the Tools > Board menu that corresponds to your Arduino. Selecting the Wio Terminal.
4) Load the MicroP25RX_Wio file.  File->Open
5) Upload the program. Click the Upload button in the environment. Wait a few seconds and if the upload is successful, the message "Done uploading." will appear in the status bar.


Resources For The Wio Terminal
https://wiki.seeedstudio.com/Wio-Terminal-Getting-Started/
