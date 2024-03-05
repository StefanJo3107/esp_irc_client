# IRC bot client

IRC bot client for esp family of microcontrollers written in esp-idf

## Building and flashing

In order to build a project, esp-idf must be present on the system. For more info on how to set this up visit: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/
<br>
Once you have idf installed, you can run following commands:
- ```idf.py menuconfig``` - project configuration
- ```idf.py build``` - building the project
- ```idf.py flash monitor``` - flashing the code to the esp and starting serial monitor
 
## Project configuration

When you run ```idf.py menuconfig``` you'll be present with the ncurses menu. In the IRC Configuration menu option, you are able to configure IPV4 address and port, IRC server name, IRC username and IRC channel name.
