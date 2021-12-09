/**************************************************************************
 *
 * Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *********************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "bacnet/config.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/apdu.h"
#include "bacnet/dcc.h"
#include "bacnet/iam.h"
#include "bacnet/npdu.h"
#include "bacnet/getevent.h"
#include "bacnet/version.h"
#include "bacnet/basic/services.h"
#include "bacnet/datalink/dlenv.h"
#include "bacnet/basic/sys/filename.h"
#include "bacnet/basic/tsm/tsm.h"
#include "bacnet/basic/tsm/tsm.h"
#include "bacnet/datalink/datalink.h"
#include "bacnet/basic/binding/address.h"

#include "bacnet/basic/service/h_cov.h"
#include "bacnet/bacenum.h"

/* include the device object */
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/object/lc.h"
#include "bacnet/basic/object/trendlog.h"

#if defined(INTRINSIC_REPORTING)
#include "bacnet/basic/object/nc.h"
#endif /* defined(INTRINSIC_REPORTING) */
#if defined(BACFILE)
#include "bacnet/basic/object/bacfile.h"
#endif /* defined(BACFILE) */
#if defined(BAC_UCI)
#include "bacnet/basic/ucix/ucix.h"
#endif /* defined(BAC_UCI) */

/**---------SIMULATED Device scripting ----------**/

#include "bacnet/basic/object/ai.h"
#include "bacnet/basic/object/ao.h"
#include "bacnet/basic/object/av.h"
#include "bacnet/basic/object/bi.h"
#include "bacnet/basic/object/bo.h"
#include "bacnet/basic/object/bv.h"
#include "bacnet/basic/object/iv.h"
#include "bacnet/basic/object/piv.h"
#include "bacnet/basic/object/acc.h"
#include "bacnet/basic/object/bacfile.h"
#include "bacnet/basic/object/channel.h"
#include "bacnet/basic/object/command.h"
#include "bacnet/basic/object/csv.h"
#include "bacnet/basic/object/lc.h"
#include "bacnet/basic/object/lo.h"
#include "bacnet/basic/object/lsp.h"
#include "bacnet/basic/object/ms-input.h"
#include "bacnet/basic/object/mso.h"
#include "bacnet/basic/object/msv.h"
#include "bacnet/basic/object/nc.h"
#include "bacnet/basic/object/netport.h"
#include "bacnet/basic/object/osv.h"
#include "bacnet/basic/object/piv.h"
#include "bacnet/basic/object/schedule.h"
#include "bacnet/basic/object/trendlog.h"

#include "lua5.3/lua.h"
#include "lua5.3/lauxlib.h"
#include "lua5.3/lualib.h"

#include <pthread.h>

static void cleanup(void);

static bool running = true;

static lua_State *lua_update_state = NULL; //lua state for the update loop
static lua_State *lua_thread_state = NULL; //lua state for the thread

static pthread_t script_runner_pthread;
static bool script_running = false;

//ANALOG INPUT
static int set_analog_input_present_value (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  float value = lua_tonumber(L, 2);
  Analog_Input_Present_Value_Set(object_instance, value);
  return 0;
}

static int set_analog_input_name (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  char *name = (char *) lua_tostring(L,2);
  Analog_Input_Name_Set (object_instance, name);
  return 0;
}

static int create_analog_inputs(lua_State *L)
{
  size_t count = lua_tonumber(L, 1);
  Analog_Input_Add(count);
  return 0;
}

static int get_analog_input_present_value(lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  float value = Analog_Input_Present_Value(object_instance);
  lua_pushnumber(L, value);
  return 1;
}

//ANALOG OUTPUT
static int set_analog_output_present_value (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  float value = lua_tonumber(L, 2);
  uint8_t priority = lua_tonumber(L,3);
  Analog_Output_Present_Value_Set(object_instance, value, priority);
  return 0;
}

static int set_analog_output_name (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  char *name = (char *) lua_tostring(L,2);
  Analog_Output_Name_Set (object_instance, name);
  return 0;
}

static int create_analog_outputs(lua_State *L)
{
  size_t count = lua_tonumber(L, 1);
  Analog_Output_Add(count);
  return 0;
}

static int get_analog_output_present_value(lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  float value = Analog_Output_Present_Value(object_instance);
  lua_pushnumber(L, value);
  return 1;
}

//ANALOG VALUE
static int set_analog_value_present_value (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  float value = lua_tonumber(L, 2);
  unsigned int priority = lua_tonumber(L,3);
  Analog_Value_Present_Value_Set(object_instance, value, priority);
  return 0;
}

static int set_analog_value_name (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  char *name = (char *) lua_tostring(L,2);
  Analog_Value_Name_Set (object_instance, name);
  return 0;
}

static int create_analog_values(lua_State *L)
{
  size_t count = lua_tonumber(L, 1);
  Analog_Value_Add(count);
  return 0;
}

static int get_analog_value_present_value(lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  float value = Analog_Value_Present_Value(object_instance);
  lua_pushnumber(L, value);
  return 1;
}

//BINARY INPUT

static int set_binary_input_present_value (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  uint8_t value = lua_tonumber(L, 2);
  Binary_Input_Present_Value_Set(object_instance, value);
  return 0;
}

static int set_binary_input_name (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  char *name = (char *) lua_tostring(L,2);
  Binary_Input_Name_Set (object_instance, name);
  return 0;
}

static int create_binary_inputs(lua_State *L)
{
  size_t count = lua_tonumber(L, 1);
  Binary_Input_Add(count);
  return 0;
}

static int get_binary_input_present_value(lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  BACNET_BINARY_PV value = Binary_Input_Present_Value(object_instance);
  lua_pushnumber(L, (uint8_t) value);
  return 1;
}

//BINARY OUTPUT
static int set_binary_output_present_value (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  uint8_t value = lua_tonumber(L, 2);
  uint8_t priority = lua_tonumber(L,3);
  Binary_Output_Present_Value_Set(object_instance, value, priority);
  return 0;
}

static int set_binary_output_name (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  char *name = (char *) lua_tostring(L,2);
  Binary_Output_Name_Set (object_instance, name);
  return 0;
}

static int create_binary_outputs(lua_State *L)
{
  size_t count = lua_tonumber(L, 1);
  Binary_Output_Add(count);
  return 0;
}

static int get_binary_output_present_value(lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  BACNET_BINARY_PV value = Binary_Output_Present_Value(object_instance);
  lua_pushnumber(L, (uint8_t) value);
  return 1;
}

//BINARY VALUE
static int set_binary_value_present_value (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  uint8_t value = lua_tonumber(L, 2);
  uint8_t priority = lua_tonumber(L,3);
  Binary_Value_Present_Value_Set(object_instance, value, priority);
  return 0;
}

static int set_binary_value_name (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  char *name = (char *) lua_tostring(L,2);
  Binary_Value_Name_Set (object_instance, name);
  return 0;
}

static int create_binary_values(lua_State *L)
{
  size_t count = lua_tonumber(L, 1);
  Binary_Value_Add(count);
  return 0;
}

static int get_binary_value_present_value(lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  BACNET_BINARY_PV value = Binary_Value_Present_Value(object_instance);
  lua_pushnumber(L, (uint8_t) value);
  return 1;
}

//INTEGER VALUE
static int set_integer_value_present_value (lua_State *L)
{
  uint32_t object_instance = lua_tonumber (L, 1);
  int32_t value = lua_tonumber (L, 2);
  uint8_t priority = lua_tonumber (L, 3);
  Integer_Value_Present_Value_Set (object_instance, value, priority);
  return 0;
}

static int set_integer_value_name (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  char *name = (char *) lua_tostring(L,2);
  Integer_Value_Name_Set (object_instance, name);
  return 0;
}

static int create_integer_values (lua_State *L)
{
  size_t count = lua_tonumber(L, 1);
  Integer_Value_Add(count);
  return 0;
}

static int get_integer_value_present_value(lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  int value = Integer_Value_Present_Value(object_instance);
  lua_pushnumber(L, value);
  return 1;
}

//POSITIVE INTEGER VALUE
static int set_positive_integer_value_present_value (lua_State *L)
{
  uint32_t object_instance = lua_tonumber (L, 1);
  uint32_t value = lua_tonumber (L, 2);
  uint8_t priority = lua_tonumber (L, 3);
  PositiveInteger_Value_Present_Value_Set (object_instance, value, priority);
  return 0;
}

static int set_positive_integer_value_name (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  char *name = (char *) lua_tostring(L,2);
  PositiveInteger_Value_Name_Set (object_instance, name);
  return 0;
}

static int create_positive_integer_values (lua_State *L)
{
  size_t count = lua_tonumber(L, 1);
  PositiveInteger_Value_Add(count);
  return 0;
}

static int get_positive_integer_value_present_value(lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  unsigned int value = PositiveInteger_Value_Present_Value(object_instance);
  lua_pushnumber(L, value);
  return 1;
}

//ACCUMULATOR 
static int set_accumulator_value_present_value (lua_State *L)
{
  uint32_t object_instance = lua_tonumber (L, 1);
  uint32_t value = lua_tonumber (L, 2);
  Accumulator_Present_Value_Set (object_instance, value);
  return 0;
}

static int set_accumulator_name (lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  char *name = (char *) lua_tostring(L,2);
  Accumulator_Name_Set (object_instance, name);
  return 0;
}

static int create_accumulators(lua_State *L)
{
  size_t count = lua_tonumber(L, 1);
  Accumulator_Add(count);
  return 0;
}

static int get_accumulator_present_value(lua_State *L)
{
  uint32_t object_instance = lua_tonumber(L, 1);
  unsigned int value = Accumulator_Present_Value(object_instance);
  lua_pushnumber(L, value);
  return 1;
}

static int is_server_running(lua_State *L)
{
  lua_pushboolean(L, running);
  return 1;
}

static void setup_lua_callbacks(lua_State *L)
{
  static const struct luaL_Reg callbacks [] = {
      {"setAnalogInputPresentValue", set_analog_input_present_value},
      {"setAnalogOutputPresentValue", set_analog_output_present_value},
      {"setAnalogValuePresentValue", set_analog_value_present_value},
      {"setBinaryInputPresentValue", set_binary_input_present_value},
      {"setBinaryOutputPresentValue", set_binary_output_present_value},
      {"setBinaryValuePresentValue", set_binary_value_present_value},
      {"setIntegerValuePresentValue", set_integer_value_present_value},
      {"setPositiveIntegerValuePresentValue", set_positive_integer_value_present_value},
      {"setAccumulatorPresentValue", set_accumulator_value_present_value},

      {"setAnalogInputName", set_analog_input_name},
      {"setAnalogOutputName", set_analog_output_name},
      {"setAnalogValueName", set_analog_value_name},
      {"setBinaryInputName", set_binary_input_name},
      {"setBinaryOutputName", set_binary_output_name},
      {"setBinaryValueName", set_binary_value_name},
      {"setIntegerValueName", set_integer_value_name},
      {"setPositiveIntegerValueName", set_positive_integer_value_name},
      {"setAccumulatorName", set_accumulator_name},

      {"createAnalogInputs", create_analog_inputs},
      {"createAnalogOutputs", create_analog_outputs},
      {"createAnalogValues", create_analog_values},
      {"createBinaryInputs", create_binary_inputs},
      {"createBinaryOutputs", create_binary_outputs},
      {"createBinaryValues", create_binary_values},
      {"createIntegerValues", create_integer_values},
      {"createPositiveIntegerValues", create_positive_integer_values},
      {"createAccumulators", create_accumulators},

      {"getAnalogInputPresentValue", get_analog_input_present_value},
      {"getAnalogOutputPresentValue", get_analog_output_present_value},
      {"getAnalogValuePresentValue", get_analog_value_present_value},
      {"getBinaryInputPresentValue", get_binary_input_present_value},
      {"getBinaryOutputPresentValue", get_binary_output_present_value},
      {"getBinaryValuePresentValue", get_binary_value_present_value},
      {"getIntegerValuePresentValue", get_integer_value_present_value},
      {"getPositiveIntegerValuePresentValue", get_positive_integer_value_present_value},
      {"getAccumulatorPresentValue", get_accumulator_present_value},

      {NULL, NULL} //required
  };

  luaL_newlib(L, callbacks);
  lua_setglobal(L, "bacnet");

  //register a function so that the script can check if the server is running
  lua_register(L, "isBacnetRunning", is_server_running);
} 

static void lua_fail (lua_State *L)
{
  printf ("LUA ERROR: %s\n", lua_tostring(L, -1));  
}

static bool lua_call_function(lua_State *L, const char* function_name)
{
  lua_getglobal(L, function_name);
  if (lua_pcall (L, 0, 0, 0))
  {
    printf ("No '%s' function found.\n", function_name);
    lua_fail(L);
    return false;
  }     
  return true;        
}

static bool lua_init_state(lua_State **L, const char* file_path)
{
  *L = luaL_newstate();
  luaL_openlibs (*L);
  
  setup_lua_callbacks (*L);

  if (luaL_loadfile (*L, file_path) || lua_pcall (*L, 0, 0, 0)) 
  {
    lua_fail(*L);
    return false;
  }
  return true;
}

//cleanup and exit
static void simulated_cleanup(void) 
{
  if (script_running)
  {
    pthread_join (script_runner_pthread, NULL);   
    script_running = false;
  }

  if (NULL != lua_update_state)
  {
    lua_close (lua_update_state);
  }
}

//calls update function in lua script
static void simulated_update(void)
{
  if(!lua_call_function (lua_update_state, "Update"))
  {
    cleanup();
    exit(0);
  }
}

static void *lua_script_runner (void * ptr)
{
  lua_call_function (lua_thread_state, "Run");
  lua_close (lua_thread_state);
  return NULL;
}

static void init_update(const char* file_path)
{
  if (!lua_init_state (&lua_update_state, file_path))
  {
    lua_update_state = NULL;
    cleanup();
    exit(0);
  }
}

static void init_thread_runner(const char* file_path)
{
  if (!lua_init_state (&lua_thread_state, file_path))
  {
    lua_thread_state = NULL;
    cleanup();
    exit(0);
  }

  script_running = true;
  pthread_create (&script_runner_pthread, NULL, lua_script_runner, NULL);
}

static void simulated_init (const char * file_path)
{
  printf("Loading lua script...\n");
  fflush(stdout);

  init_thread_runner (file_path); 
  init_update (file_path);

  printf("Loaded lua script sucessfully.\n");
  fflush(stdout);
}

static void populate_sim (long instances)
{
  printf("Populating server with %ld of each object...\n", instances);

  Accumulator_Add (instances); 
  Analog_Input_Add (instances);
  Analog_Output_Add (instances);                   
  Analog_Value_Add (instances);
  bacfile_add (instances); 
  Binary_Input_Add (instances);
  Binary_Output_Add (instances);
  Binary_Value_Add (instances);
  Channel_Add (instances);
  Command_Add (instances);
  CharacterString_Value_Add (instances);
  Integer_Value_Add (instances);
  Load_Control_Add (instances);
  Lighting_Output_Add (instances);
  Life_Safety_Point_Add (instances);
  Multistate_Input_Add (instances);
  Multistate_Output_Add (instances); 
  Multistate_Value_Add (instances); 
#if defined(INTRINSIC_REPORTING)
  Notification_Class_Add (instances);
#endif
  Network_Port_Add (instances);
  OctetString_Value_Add (instances);
  PositiveInteger_Value_Add (instances);
  Schedule_Add (instances);
  Trend_Log_Add (instances);

  printf("Done \n");

}

/** @file server/main.c  Example server application using the BACnet Stack. */

/* (Doxygen note: The next two lines pull all the following Javadoc
 *  into the ServerDemo module.) */
/** @addtogroup ServerDemo */
/*@{*/

/* current version of the BACnet stack */
static const char *BACnet_Version = BACNET_VERSION_TEXT;

/** Buffer used for receiving */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };

/** Initialize the handlers we will utilize.
 * @see Device_Init, apdu_set_unconfirmed_handler, apdu_set_confirmed_handler
 */
static void Init_Service_Handlers(void)
{
    Device_Init(NULL);
    /* we need to handle who-is to support dynamic device binding */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS, handler_who_has);

    apdu_set_confirmed_handler( SERVICE_CONFIRMED_SUBSCRIBE_COV, handler_cov_subscribe);

#if 0
	/* 	BACnet Testing Observed Incident oi00107
		Server only devices should not indicate that they EXECUTE I-Am
		Revealed by BACnet Test Client v1.8.16 ( www.bac-test.com/bacnet-test-client-download )
			BITS: BIT00040
		Any discussions can be directed to edward@bac-test.com
		Please feel free to remove this comment when my changes accepted after suitable time for
		review by all interested parties. Say 6 months -> September 2016 */
	/* In this demo, we are the server only ( BACnet "B" device ) so we do not indicate
	   that we can execute the I-Am message */
    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
#endif

    /* set the handler for all the services we don't implement */
    /* It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler(handler_unrecognized_service);
    /* Set the handlers for any confirmed services that we support. */
    /* We must implement read property - it's required! */
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_READ_PROPERTY, handler_read_property);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_READ_PROP_MULTIPLE, handler_read_property_multiple);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_WRITE_PROPERTY, handler_write_property);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_WRITE_PROP_MULTIPLE, handler_write_property_multiple);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_READ_RANGE, handler_read_range);
#if defined(BACFILE)
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_ATOMIC_READ_FILE, handler_atomic_read_file);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_ATOMIC_WRITE_FILE, handler_atomic_write_file);
#endif
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_REINITIALIZE_DEVICE, handler_reinitialize_device);
    apdu_set_unconfirmed_handler(
        SERVICE_UNCONFIRMED_UTC_TIME_SYNCHRONIZATION, handler_timesync_utc);
    apdu_set_unconfirmed_handler(
        SERVICE_UNCONFIRMED_TIME_SYNCHRONIZATION, handler_timesync);

    apdu_set_unconfirmed_handler(
        SERVICE_UNCONFIRMED_COV_NOTIFICATION, handler_ucov_notification);
    /* handle communication so we can shutup when asked */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
        handler_device_communication_control);
    /* handle the data coming back from private requests */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_PRIVATE_TRANSFER,
        handler_unconfirmed_private_transfer);
#if defined(INTRINSIC_REPORTING)
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM, handler_alarm_ack);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_GET_EVENT_INFORMATION, handler_get_event_information);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_GET_ALARM_SUMMARY, handler_get_alarm_summary);
#endif /* defined(INTRINSIC_REPORTING) */
#if defined(BACNET_TIME_MASTER)
    handler_timesync_init();
#endif
}

static void print_usage(const char *filename)
{
    printf("Usage: %s [--populate instances] [--script script_path] [--instance instance_number] [--name device-name]\n", filename);
    printf("       [--version][--help]\n");
}

static void print_help(const char *filename)
{
    printf("Simulate a BACnet server device\n"
           "--script:\n"
           "Path to device simulation Lua script\n"
           "--instance:\n"
           "BACnet Device Object Instance number that you are\n"
           "trying simulate.\n"
           "--name:\n"
           "The Device object-name is the text name for the device.\n"
           "--populate:\n"
           "Populates the simulator with n instances of each object type\n"
           "\nExample:\n");
    printf("To simulate Device 123, use the following command:\n"
           "%s --instance 123\n",
        filename);
    printf("To simulate Device 123 named Fred, use following command:\n"
           "%s --instance 123 --name Fred\n", 
        filename);
    printf("To simulate Device 123 named Fred using the script example.lua , use following command:\n"
           "%s --script example.lua --instance 123 --name Fred\n",
        filename);
}

static void sigint(int a)
{
  running = false;
}

static void cleanup(void)
{
    datalink_cleanup();
    simulated_cleanup();
    Device_Cleanup();
}

/** Main function of server demo.
 *
 * @see Device_Set_Object_Instance_Number, dlenv_init, Send_I_Am,
 *      datalink_receive, npdu_handler,
 *      dcc_timer_seconds, datalink_maintenance_timer,
 *      Load_Control_State_Machine_Handler, handler_cov_task,
 *      tsm_timer_milliseconds
 *
 * @param argc [in] Arg count.
 * @param argv [in] Takes one argument: the Device Instance #.
 * @return 0 on success.
 */
int main(int argc, char *argv[])
{
    
    BACNET_ADDRESS src = { 0 }; /* address where message came from */
    uint16_t pdu_len = 0;
    unsigned timeout = 1; /* milliseconds */
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    uint32_t elapsed_seconds = 0;
    uint32_t elapsed_milliseconds = 0;
    uint32_t address_binding_tmr = 0;
#if defined(INTRINSIC_REPORTING)
    uint32_t recipient_scan_tmr = 0;
#endif
#if defined(BACNET_TIME_MASTER)
    BACNET_DATE_TIME bdatetime;
#endif
#if defined(BAC_UCI)
    int uciId = 0;
    struct uci_context *ctx;
#endif
    int argi = 0;
    const char *filename = NULL;
    const char *devicename = NULL;
    const char *scriptpath = NULL;
    long instance_num = 0;
    bool instance_set = false;
    bool using_script = false;
    long populate_instances = 0;

    filename = filename_remove_path(argv[0]);
    for (argi = 1; argi < argc; argi++) {
        if (strcmp(argv[argi], "--help") == 0) {
            print_usage(filename);
            print_help(filename);
            return 0;
        }
        if (strcmp(argv[argi], "--version") == 0) {
            printf("%s %s\n", filename, BACNET_VERSION_TEXT);
            printf("Copyright (C) 2014 by Steve Karg and others.\n"
                   "This is free software; see the source for copying "
                   "conditions.\n"
                   "There is NO warranty; not even for MERCHANTABILITY or\n"
                   "FITNESS FOR A PARTICULAR PURPOSE.\n");
            return 0;
        }

        if (strcmp(argv[argi], "--script") == 0) {
          if (argi == argc - 1)
          {
            print_usage (filename);
            return 0;
          }
          argi++;
          scriptpath = argv[argi];
          using_script = true;
        }

        if (strcmp(argv[argi], "--instance") == 0) {
          if (argi == argc - 1)
          {
            print_usage (filename);
            return 0;
          }
          argi++;
          instance_num = strtol(argv[argi], NULL, 0);
          instance_set = true;
        }

        if (strcmp(argv[argi], "--name") == 0) {
          if (argi == argc - 1)
          {
            print_usage (filename);
            return 0;
          }
          argi++;
          devicename = argv[argi]; 
        }

        if (strcmp(argv[argi], "--populate") == 0) {
          if (argi == argc - 1)
          {
            print_usage (filename);
            return 0;
          }
          argi++;
          populate_instances = strtol(argv[argi], NULL, 0);
        }

    }

#if defined(BAC_UCI)
    ctx = ucix_init("bacnet_dev");
    if (!ctx)
        fprintf(stderr, "Failed to load config file bacnet_dev\n");
    uciId = ucix_get_option_int(ctx, "bacnet_dev", "0", "Id", 0);
    printf("ID: %i", uciId);
    if (uciId != 0) {
        Device_Set_Object_Instance_Number(uciId);
    } else {
#endif /* defined(BAC_UCI) */
        /* allow the device ID to be set */

#if defined(BAC_UCI)
    }
    ucix_cleanup(ctx);
#endif /* defined(BAC_UCI) */
    
    if (instance_set)
    {
      Device_Set_Object_Instance_Number(instance_num);
    }

    printf("IOTech BACnet Simulator\n"
           "BACnet Stack Version %s\n"
           "BACnet Device ID: %u\n"
           "Max APDU: %d\n",
        BACnet_Version, Device_Object_Instance_Number(), MAX_APDU);
    fflush(stdout);
    /* load any static address bindings to show up
       in our device bindings list */
    address_init();
    Init_Service_Handlers();

    if (devicename) {
        Device_Object_Name_ANSI_Init(devicename);
    }

    dlenv_init();
    atexit(datalink_cleanup);

    /* configure the timeout values */
    last_seconds = time(NULL);
    /* broadcast an I-Am on startup */
    Send_I_Am(&Handler_Transmit_Buffer[0]);
  
    if (scriptpath != NULL)
    {
      simulated_init(scriptpath);
    }

    if (populate_instances > 0)
    {
      populate_sim (populate_instances);
    }

    signal(SIGINT, sigint);

    /* loop forever */
    while (running) {    
        /* input */
        current_seconds = time(NULL);

        /* returns 0 bytes on timeout */
        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);

        /* process */
        if (pdu_len) {
            npdu_handler(&src, &Rx_Buf[0], pdu_len);
        }
        /* at least one second has passed */
        elapsed_seconds = (uint32_t)(current_seconds - last_seconds);
        if (elapsed_seconds) {
            last_seconds = current_seconds;
            dcc_timer_seconds(elapsed_seconds);
            datalink_maintenance_timer(elapsed_seconds);
            dlenv_maintenance_timer(elapsed_seconds);
            Load_Control_State_Machine_Handler();
            elapsed_milliseconds = elapsed_seconds * 1000;
            handler_cov_timer_seconds(elapsed_seconds);
            tsm_timer_milliseconds(elapsed_milliseconds);
            trend_log_timer(elapsed_seconds);
#if defined(INTRINSIC_REPORTING)
            Device_local_reporting();
#endif
#if defined(BACNET_TIME_MASTER)
            Device_getCurrentDateTime(&bdatetime);
            handler_timesync_task(&bdatetime);
#endif
        }

        while (!handler_cov_fsm());

        /* scan cache address */
        address_binding_tmr += elapsed_seconds;
        if (address_binding_tmr >= 60) {
            address_cache_timer(address_binding_tmr);
            address_binding_tmr = 0;
        }
#if defined(INTRINSIC_REPORTING)
        /* try to find addresses of recipients */
        recipient_scan_tmr += elapsed_seconds;
        if (recipient_scan_tmr >= NC_RESCAN_RECIPIENTS_SECS) {
            Notification_Class_find_recipient();
            recipient_scan_tmr = 0;
        }
#endif
        /* output */
        if (using_script)
        {
          simulated_update();
        }
        
    }

    cleanup();
    return 0;
}

/* @} */

/* End group ServerDemo */
