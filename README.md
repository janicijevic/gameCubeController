# Arduino GameCube Controller Interface

This project was made to connect a GameCube controller to a computer using an Arduino board. In order to preserve the controller I soldered some wires to a board on the GameCube to access the pins of the controller. These wires were connected to the Arduino which impersonates the console and communicates with the controller. The Arduino then passes the information to the computer through the serial port where a python script parses the data and emulates a controller using vJoy.  




# Getting started

* Install [vjoy](http://vjoystick.sourceforge.net/site/index.php/download-a-install/download)  
* Install python
* Install the python modules from requirements.txt using `pip install -r requirements.txt`
* Upload code to Arduino then run the python script with the Arduino connected (change "COM3" to which ever port the Arduino is connected to)