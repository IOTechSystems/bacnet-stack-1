# Whats Changed in v1.0.2

### Related to Device Service

- Change `BIG_OBJECTS` definition to `SEVER_OBJECTS`
    - This definition has now increased in scope to over almost all basic bacnet object files
    - Any of these files are now not built for both Azure Sphere and standard builds of the library
    - These files will still be included in a build of the simulator as that build process uses the Makefile

### Related to Simulator

- Updated names of Lua present value setting functions  eg. setAnalogInput() -> setAnalogInputPresentValue()
- Updated some initial values of the simulators object properties