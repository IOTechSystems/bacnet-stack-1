#include "bacnet/config.h"
#include "bacnet/dcc.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacaddr.h"
#include "bacnet/npdu.h"
#include "bacnet/restart.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/services.h"
#include "bacnet/basic/tsm/tsm.h"
#include "bacnet/datalink/datalink.h"


#ifndef MAX_RESTART_RECIPIENTS
#define MAX_RESTART_RECIPIENTS 16
#endif
BACNET_RECIPIENT_LIST Restart_Notification_Recipients[MAX_RESTART_RECIPIENTS];

/** Handle a request to list all the restart notification recipients.
 *
 *  Invoked by a request to read the Device object's
 *  PROP_RESTART_NOTIFICATION_RECIPIENTS.
 *  Loops through the list of restart recipients, and, for each valid one,
 *  adds its data to the APDU.
 *
 *  @param apdu [out] Buffer in which the APDU contents are built.
 *  @param max_apdu [in] Max length of the APDU buffer.
 *
 *  @return How many bytes were encoded in the buffer, or
 *   BACNET_STATUS_ABORT if the response would not fit within the buffer.
 */
int handler_restart_encode_recipients(uint8_t *apdu, int max_apdu)
{
    return restart_encode_restart_recipients(
        apdu, max_apdu, &Restart_Notification_Recipients[0]);
}

bool handler_restart_recipient_write(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    bool status = false;

    /* fixme: handle writing of the recipient list */
    wp_data->error_class = ERROR_CLASS_PROPERTY;
    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;

    return status;
}

static void Send_Directed_UCOV_Notify(
    uint8_t *buffer, unsigned buffer_len, BACNET_ADDRESS *dest, BACNET_COV_DATA *cov_data)
{
    int pdu_len = 0;
    BACNET_ADDRESS dummy_dest;
    BACNET_NPDU_DATA npdu_data;

    pdu_len =
        ucov_notify_encode_pdu(buffer, buffer_len, &dummy_dest, &npdu_data, cov_data);
    datalink_send_pdu(dest, &npdu_data, &buffer[0], pdu_len);
}

static void Send_Restart_Remote(
    BACNET_ADDRESS *dest, BACNET_TIME tm, BACNET_RESTART_REASON reason)
{
    BACNET_COV_DATA cov_data;
    BACNET_PROPERTY_VALUE props[3] = { { 0 } };

    cov_data.subscriberProcessIdentifier = 0;
    cov_data.initiatingDeviceIdentifier = Device_Object_Instance_Number();
    cov_data.monitoredObjectIdentifier.type = OBJECT_DEVICE;
    cov_data.monitoredObjectIdentifier.instance = 0;
    cov_data.timeRemaining = 0;
    cov_data.listOfValues = props;

    props[0].propertyIdentifier = PROP_SYSTEM_STATUS;
    props[0].value.tag = BACNET_APPLICATION_TAG_ENUMERATED;
    props[0].value.type.Enumerated = Device_System_Status();
    props[0].next = &props[1];

    props[1].propertyIdentifier = PROP_TIME_OF_DEVICE_RESTART;
    props[1].value.tag = BACNET_APPLICATION_TAG_TIME;
    props[1].value.type.Time = tm;
    props[1].next = &props[2];

    props[2].propertyIdentifier = PROP_LAST_RESTART_REASON;
    props[2].value.tag = BACNET_APPLICATION_TAG_ENUMERATED;
    props[2].value.type.Enumerated = reason;

    Send_Directed_UCOV_Notify(&Handler_Transmit_Buffer[0],
        sizeof(Handler_Transmit_Buffer), dest, &cov_data);
}

void handler_restart_send(BACNET_TIME tm, BACNET_RESTART_REASON reason)
{
    unsigned index = 0;
    bool status = false;

    for (index = 0; index < MAX_RESTART_RECIPIENTS; index++) {
        if (Restart_Notification_Recipients[index].tag == 1) {
            if (status) {
                Send_Restart_Remote(&Restart_Notification_Recipients[index].type.address,
                    tm, reason);
            }
        }
    }
}

bool handler_restart_recipient_address_set(
    unsigned index, BACNET_ADDRESS *address)
{
    bool status = false;

    if (address && (index < MAX_RESTART_RECIPIENTS)) {
        Restart_Notification_Recipients[index].tag = 1;
        bacnet_address_copy(&Restart_Notification_Recipients[index].type.address, address);
        status = true;
    }

    return status;
}

void handler_restart_init(void)
{
    unsigned i = 0;

    /* connect linked list */
    for (; i < (MAX_RESTART_RECIPIENTS - 1); i++) {
        Restart_Notification_Recipients[i].next = &Restart_Notification_Recipients[i + 1];
        Restart_Notification_Recipients[i + 1].next = NULL;
    }
    for (i = 0; i < MAX_RESTART_RECIPIENTS; i++) {
        Restart_Notification_Recipients[i].tag = 0xFF;
    }
}
