# Whats Changed in v1.0.2

### Related to Device Service

- Change `BIG_OBJECTS` definition to `BACNET_SIMULATOR`
    - This definition has now increased in scope to over almost all basic bacnet object files
    - Any of these files are now not built for both Azure Sphere and standard builds of the library
    - These files will still be included in a build of the simulator as that build process uses the Makefile
- Disable BACnet routing as is currently not used in the device service
    - Because of this, we can now set our device ID as something other than 0
- Updated Device Service object name

### Related to Simulator

- Updated names of Lua present value setting functions eg. setAnalogInput() -> setAnalogInputPresentValue()
- Updated some initial values of the simulators object properties
- Updated identifying values of device object
- Updated handling logic of script and populate arguments
- Updated default behaviour when no arguments provided where device instance is 1234 and it populates the sim with 1 of each object
- Updated how the priority arrays are initialised for Binary Analog Output, Binary Output and Binary Value
- Removed any rules on setting certain priority levels
- Updated device object name for simulator