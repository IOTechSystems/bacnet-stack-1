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
- Updated how the priority arrays are initialised for Analog Output, Binary Output and Binary Value
- Removed any rules on setting certain priority levels
- Updated device object name for simulator

# Whats Changed in v1.0.3

- Fixed infinite loop of RPM decode

# Whats Changed in v1.0.4

- Added new abilities for simulator: registerRecipient(), generateEvent(), createNotificationClasses()
- Added new decode and encode abilities for new event types:
  - EVENT_COMMAND_FAILURE
  - EVENT_ACCESS_EVENT
  - EVENT_DOUBLE_OUT_OF_RANGE
  - EVENT_SIGNED_OUT_OF_RANGE
  - EVENT_UNSIGNED_OUT_OF_RANGE
  - EVENT_CHANGE_OF_CHARACTERSTRING
  - EVENT_CHANGE_OF_TIMER

# Whats Changed in v1.0.5

- Added ability to change the Vendor Name

# Whats Changed in v1.0.6

- Added fix for uninitialised cov increment value for simulator

# Whats Changed in v1.0.7

- Updated present value property of Positive Integer Value to be uint64 instead of uint32

# Whats Changed in v1.0.8

- Support added for RISC-V Yocto Linux

# Whats Changed in v1.0.9

- Added new direct addressing functions for writing properties and registering COVs

# Whats Changed in v1.1.0

- Changed from global TSM invoke ID cache to a per device cache

# Whats Changed in v1.1.1

- Fixed BBMD registration failure check

# What's Changed in v1.1.2

- Added PROP_SCALE support

# What's Changed in v1.2.0

- Reduced number of outstanding transactions per device (lower memory footprint)

# What's Changed in v1.2.1

- Added SubscribeCOVProperty support (client-side)

# What's Changed in v1.2.2

- Fixed array decoding in COV notifications
