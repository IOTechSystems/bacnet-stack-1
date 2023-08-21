/**************************************************************************
 *
 * Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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

/* Binary Output Objects - customize for your use */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacenum.h"
#include "bacnet/bacapp.h"
#include "bacnet/config.h" /* the custom stuff */
#include "bacnet/rp.h"
#include "bacnet/wp.h"
#include "bacnet/basic/object/bo.h"
#include "bacnet/basic/services.h"

/* When all the priorities are level null, the present value returns */
/* the Relinquish Default value */
#define RELINQUISH_DEFAULT BINARY_INACTIVE
/* Here is our Priority Array.*/
static BINARY_OUTPUT_DESCR *BO_Descr = NULL;
static size_t BO_Descr_Size = 0;
static pthread_mutex_t BO_Descr_Mutex = PTHREAD_MUTEX_INITIALIZER;


/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Binary_Output_Properties_Required[] = { PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME, PROP_OBJECT_TYPE, PROP_PRESENT_VALUE, PROP_STATUS_FLAGS,
    PROP_EVENT_STATE, PROP_OUT_OF_SERVICE, PROP_POLARITY, PROP_PRIORITY_ARRAY,
    PROP_RELINQUISH_DEFAULT, -1 };

static const int Binary_Output_Properties_Optional[] = { PROP_DESCRIPTION,
    PROP_ACTIVE_TEXT, PROP_INACTIVE_TEXT, -1 };

static const int Binary_Output_Properties_Proprietary[] = { -1 };

void Binary_Output_Property_Lists(
    const int **pRequired, const int **pOptional, const int **pProprietary)
{
    if (pRequired) {
        *pRequired = Binary_Output_Properties_Required;
    }
    if (pOptional) {
        *pOptional = Binary_Output_Properties_Optional;
    }
    if (pProprietary) {
        *pProprietary = Binary_Output_Properties_Proprietary;
    }

    return;
}

void Binary_Output_Set_Properties(
    uint32_t object_instance, 
    const char *object_name, 
    BACNET_BINARY_PV value,
    bool out_of_service   
)
{
    unsigned int index = Binary_Output_Instance_To_Index(object_instance);
    if (index >= BO_Descr_Size)
    {
        return;
    }   

    Binary_Output_Name_Set(object_instance, object_name);
    Binary_Output_Present_Value_Set(object_instance, value,1);
    
    pthread_mutex_lock(&BO_Descr_Mutex);
    BO_Descr[index].Out_Of_Service = out_of_service;
    pthread_mutex_unlock(&BO_Descr_Mutex);
}


void Binary_Output_Add(size_t count)
{
    size_t prev_size = BO_Descr_Size;
    size_t new_size = BO_Descr_Size + count;
   
    pthread_mutex_lock(&BO_Descr_Mutex);
    BINARY_OUTPUT_DESCR *tmp = realloc(BO_Descr, sizeof(*BO_Descr) * new_size);
    if (NULL == tmp) //unsuccessful resize
    {
        pthread_mutex_unlock(&BO_Descr_Mutex);
        return;
    }
    BO_Descr_Size = new_size;
    BO_Descr = tmp;
    pthread_mutex_unlock(&BO_Descr_Mutex);

    //initialize object properties
    char name_buffer[64];
    for(size_t i = prev_size; i < new_size; i++ )
    {
        pthread_mutex_lock(&BO_Descr_Mutex);
        BO_Descr[i].Name = NULL;
        pthread_mutex_unlock(&BO_Descr_Mutex);

        snprintf(name_buffer, 64, "binary_output_%zu", i);
        Binary_Output_Set_Properties(
            i,
            name_buffer,
            BINARY_ACTIVE,
            false    
        );
    }
}


void Binary_Output_Free(void)
{
    if (NULL == BO_Descr) return;    

    pthread_mutex_lock(&BO_Descr_Mutex);

    for(unsigned int i=0; i < BO_Descr_Size; i++)
    {
        free(BO_Descr[i].Name);
    }

    free(BO_Descr);
    BO_Descr = NULL;
    BO_Descr_Size = 0;
   
    pthread_mutex_unlock(&BO_Descr_Mutex);
}

void Binary_Output_Objects_Init(void)
{
    unsigned i, j;
    static bool initialized = false;

    if (!initialized) {
        initialized = true;

        /* initialize all the analog output priority arrays to NULL */
        pthread_mutex_lock(&BO_Descr_Mutex);
        for (i = 0; i < BO_Descr_Size; i++) {
            for (j = 0; j < BACNET_MAX_PRIORITY; j++)
            {    
                BO_Descr[i].Level[j] = BINARY_NULL;
            }
            BO_Descr[i].Name = NULL;
        }
        pthread_mutex_unlock(&BO_Descr_Mutex);
    }
}

void Binary_Output_Init(void)
{

}

void Binary_Output_Cleanup(void)
{
    Binary_Output_Free();
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Binary_Output_Valid_Instance(uint32_t object_instance)
{
    if (object_instance < BO_Descr_Size) {
        return true;
    }

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Binary_Output_Count(void)
{
    return BO_Descr_Size;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Binary_Output_Index_To_Instance(unsigned index)
{
    return index;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Binary_Output_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = BO_Descr_Size;

    if (object_instance < BO_Descr_Size) {
        index = object_instance;
    }

    return index;
}

BACNET_BINARY_PV Binary_Output_Present_Value(uint32_t object_instance)
{
    BACNET_BINARY_PV value = RELINQUISH_DEFAULT;
    unsigned index = 0;
    unsigned i = 0;

    index = Binary_Output_Instance_To_Index(object_instance);
    if (index < BO_Descr_Size) {

        pthread_mutex_lock(&BO_Descr_Mutex);
        for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
            if (BO_Descr[index].Level[i] != BINARY_NULL) {
                value = BO_Descr[index].Level[i];
                break;
            }
        }
        pthread_mutex_unlock(&BO_Descr_Mutex);
    }

    return value;
}

bool Binary_Output_Present_Value_Set(
        uint32_t object_instance,
        BACNET_BINARY_PV binary_value,
        unsigned priority)
{
    unsigned index = 0;
    bool status = false;

    index = Binary_Output_Instance_To_Index(object_instance);
    if (index < BO_Descr_Size) {
        if (priority && (priority <= BACNET_MAX_PRIORITY) &&
            (priority != 6 /* reserved */)) {
            pthread_mutex_lock(&BO_Descr_Mutex);
            BO_Descr[index].Level[priority - 1] = binary_value;
            pthread_mutex_unlock(&BO_Descr_Mutex);
            status = true;
        }
    }
    return status;
}

bool Binary_Output_Out_Of_Service(uint32_t object_instance)
{
    bool value = false;
    unsigned index = 0;

    index = Binary_Output_Instance_To_Index(object_instance);
    if (index < BO_Descr_Size) {
        pthread_mutex_lock(&BO_Descr_Mutex);
        value = BO_Descr[index].Out_Of_Service;
        pthread_mutex_unlock(&BO_Descr_Mutex);
    }

    return value;
}

/* note: the object name must be unique within this device */
bool Binary_Output_Object_Name(
    uint32_t object_instance, BACNET_CHARACTER_STRING *object_name)
{
    static char text_string[32] = ""; /* okay for single thread */
    unsigned int index;
    bool status = false;

    index = Binary_Output_Instance_To_Index(object_instance);
    if (index >= BO_Descr_Size) {
        return status;
    }

    pthread_mutex_lock(&BO_Descr_Mutex);
    if (NULL != BO_Descr[index].Name)
    {
        snprintf(text_string, 32, "%s", BO_Descr[index].Name);   
    }
    else
    {
        sprintf(text_string, "BINARY OUTPUT %lu", (unsigned long)index);
    }
    pthread_mutex_unlock(&BO_Descr_Mutex);

    status = characterstring_init_ansi(object_name, text_string);

    return status;
}


bool Binary_Output_Name_Set(uint32_t object_instance, const char *new_name)
{
    if (NULL == BO_Descr) return false;

    unsigned int index;
    index = Binary_Output_Instance_To_Index(object_instance);
    if (index >= BO_Descr_Size)
    {
        return false;
    }

    pthread_mutex_lock(&BO_Descr_Mutex);
    free(BO_Descr[index].Name);
    BO_Descr[index].Name = calloc(strlen(new_name) + 1, sizeof(char));
    if (NULL != BO_Descr[index].Name)
    {
        strcpy(BO_Descr[index].Name, new_name);
    }
    pthread_mutex_unlock(&BO_Descr_Mutex);

    return true;
}

/* return apdu len, or BACNET_STATUS_ERROR on error */
int Binary_Output_Read_Property(BACNET_READ_PROPERTY_DATA *rpdata)
{
    int len = 0;
    int apdu_len = 0; /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    BACNET_BINARY_PV present_value = BINARY_INACTIVE;
    BACNET_POLARITY polarity = POLARITY_NORMAL;
    unsigned object_index = 0;
    unsigned i = 0;
    bool state = false;
    uint8_t *apdu = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(
                &apdu[0], OBJECT_BINARY_OUTPUT, rpdata->object_instance);
            break;
            /* note: Name and Description don't have to be the same.
               You could make Description writable and different */
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Binary_Output_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_BINARY_OUTPUT);
            break;
        case PROP_PRESENT_VALUE:
            present_value =
                Binary_Output_Present_Value(rpdata->object_instance);
            apdu_len = encode_application_enumerated(&apdu[0], present_value);
            break;
        case PROP_STATUS_FLAGS:
            /* note: see the details in the standard on how to use these */
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, false);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_EVENT_STATE:
            /* note: see the details in the standard on how to use this */
            apdu_len =
                encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
            break;
        case PROP_OUT_OF_SERVICE:
            object_index =
                Binary_Output_Instance_To_Index(rpdata->object_instance);
            pthread_mutex_lock(&BO_Descr_Mutex);
            state = BO_Descr[object_index].Out_Of_Service;
            pthread_mutex_unlock(&BO_Descr_Mutex);

            apdu_len = encode_application_boolean(&apdu[0], state);
            break;
        case PROP_POLARITY:
            apdu_len = encode_application_enumerated(&apdu[0], polarity);
            break;
        case PROP_PRIORITY_ARRAY:
            /* Array element zero is the number of elements in the array */
            if (rpdata->array_index == 0) {
                apdu_len =
                    encode_application_unsigned(&apdu[0], BACNET_MAX_PRIORITY);
                /* if no index was specified, then try to encode the entire list
                 */
                /* into one packet. */
            } else if (rpdata->array_index == BACNET_ARRAY_ALL) {
                object_index =
                    Binary_Output_Instance_To_Index(rpdata->object_instance);
                pthread_mutex_lock(&BO_Descr_Mutex);
                for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
                    /* FIXME: check if we have room before adding it to APDU */
                    if (BO_Descr[object_index].Level[i] == BINARY_NULL) {
                        len = encode_application_null(&apdu[apdu_len]);
                    } else {
                        present_value = BO_Descr[object_index].Level[i];
                        len = encode_application_enumerated(
                            &apdu[apdu_len], present_value);
                    }
                    /* add it if we have room */
                    if ((apdu_len + len) < MAX_APDU) {
                        apdu_len += len;
                    } else {
                        rpdata->error_code =
                            ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                        apdu_len = BACNET_STATUS_ABORT;
                        break;
                    }
                }
                pthread_mutex_unlock(&BO_Descr_Mutex);
            } else {
                object_index =
                    Binary_Output_Instance_To_Index(rpdata->object_instance);
                if (rpdata->array_index <= BACNET_MAX_PRIORITY) {
                    pthread_mutex_lock(&BO_Descr_Mutex);
                    if (BO_Descr[object_index].Level[rpdata->array_index -
                            1] == BINARY_NULL) {
                        apdu_len = encode_application_null(&apdu[apdu_len]);
                    } else {
                        present_value =
                            BO_Descr[object_index].Level[rpdata->array_index - 1];
                        apdu_len = encode_application_enumerated(
                            &apdu[apdu_len], present_value);
                    }
                    pthread_mutex_unlock(&BO_Descr_Mutex);
                } else {
                    rpdata->error_class = ERROR_CLASS_PROPERTY;
                    rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                    apdu_len = BACNET_STATUS_ERROR;
                }
            }
            break;
        case PROP_RELINQUISH_DEFAULT:
            present_value = RELINQUISH_DEFAULT;
            apdu_len = encode_application_enumerated(&apdu[0], present_value);
            break;
        case PROP_ACTIVE_TEXT:
            characterstring_init_ansi(&char_string, "on");
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_INACTIVE_TEXT:
            characterstring_init_ansi(&char_string, "off");
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_PRIORITY_ARRAY) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/* returns true if successful */
bool Binary_Output_Write_Property(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    bool status = false; /* return value */
    unsigned int object_index = 0;
    unsigned int priority = 0;
    BACNET_BINARY_PV level = BINARY_NULL;
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;

    /* decode the some of the request */
    len = bacapp_decode_application_data(
        wp_data->application_data, wp_data->application_data_len, &value);
    /* FIXME: len < application_data_len: more data? */
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    if ((wp_data->object_property != PROP_PRIORITY_ARRAY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                priority = wp_data->priority;
                /* Command priority 6 is reserved for use by Minimum On/Off
                   algorithm and may not be used for other purposes in any
                   object. */
                if (priority && (priority <= BACNET_MAX_PRIORITY) &&
                    (priority != 6 /* reserved */) &&
                    (value.type.Enumerated <= MAX_BINARY_PV)) {
                    level = (BACNET_BINARY_PV)value.type.Enumerated;
                    object_index = Binary_Output_Instance_To_Index(
                        wp_data->object_instance);
                    priority--;
                    pthread_mutex_lock(&BO_Descr_Mutex);
                    BO_Descr[object_index].Level[priority] = level;
                    pthread_mutex_unlock(&BO_Descr_Mutex);
                    /* Note: you could set the physical output here if we
                       are the highest priority.
                       However, if Out of Service is TRUE, then don't set the
                       physical output.  This comment may apply to the
                       main loop (i.e. check out of service before changing
                       output) */
                    status = true;
                } else if (priority == 6) {
                    /* Command priority 6 is reserved for use by Minimum On/Off
                       algorithm and may not be used for other purposes in any
                       object. */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                status = WPValidateArgType(&value, BACNET_APPLICATION_TAG_NULL,
                    &wp_data->error_class, &wp_data->error_code);
                if (status) {
                    level = BINARY_NULL;
                    object_index = Binary_Output_Instance_To_Index(
                        wp_data->object_instance);
                    priority = wp_data->priority;
                    if (priority && (priority <= BACNET_MAX_PRIORITY)) {
                        priority--;
                        pthread_mutex_lock(&BO_Descr_Mutex);
                        BO_Descr[object_index].Level[priority] = level;
                        pthread_mutex_unlock(&BO_Descr_Mutex);
                        /* Note: you could set the physical output here to the
                           next highest priority, or to the relinquish default
                           if no priorities are set. However, if Out of Service
                           is TRUE, then don't set the physical output.  This
                           comment may apply to the
                           main loop (i.e. check out of service before changing
                           output) */
                    } else {
                        status = false;
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    }
                }
            }
            break;
        case PROP_OUT_OF_SERVICE:
            status = WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                object_index =
                    Binary_Output_Instance_To_Index(wp_data->object_instance);
                pthread_mutex_lock(&BO_Descr_Mutex);
                BO_Descr[object_index].Out_Of_Service = value.type.Boolean;
                pthread_mutex_unlock(&BO_Descr_Mutex);
            }
            break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_STATUS_FLAGS:
        case PROP_RELIABILITY:
        case PROP_EVENT_STATE:
        case PROP_POLARITY:
        case PROP_PRIORITY_ARRAY:
        case PROP_RELINQUISH_DEFAULT:
        case PROP_ACTIVE_TEXT:
        case PROP_INACTIVE_TEXT:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }

    return status;
}

#ifdef BAC_TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

bool WPValidateArgType(BACNET_APPLICATION_DATA_VALUE *pValue,
    uint8_t ucExpectedTag,
    BACNET_ERROR_CLASS *pErrorClass,
    BACNET_ERROR_CODE *pErrorCode)
{
    pValue = pValue;
    ucExpectedTag = ucExpectedTag;
    pErrorClass = pErrorClass;
    pErrorCode = pErrorCode;

    return false;
}

void testBinaryOutput(Test *pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint16_t decoded_type = 0;
    uint32_t decoded_instance = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Binary_Output_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_BINARY_OUTPUT;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Binary_Output_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);

    return;
}

#ifdef TEST_BINARY_OUTPUT
int main(void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Binary Output", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBinaryOutput);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void)ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_BINARY_INPUT */
#endif /* BAC_TEST */
