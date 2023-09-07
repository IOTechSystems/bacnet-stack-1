#include <stdint.h>
#include "bacnet/bacenum.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacapp.h"
#include "bacnet/restart.h"

/** @file restart.c  Encode/Decode restart notification recipient list  APDUs  */

/** Handle a request to encode the list of restart notification recipients.
 *
 *  Invoked by a request to read the Device object's
 *  PROP_RESTART_NOTIFICATION_RECIPIENTS.
 *  Loops through the list of restart notification recipients, and, for each one,
 *  adds its data to the APDU.
 *
 *   BACnetRecipient ::= CHOICE {
 *       device [0] BACnetObjectIdentifier,
 *       address [1] BACnetAddress
 *   }
 *
 *   BACnetAddress ::= SEQUENCE {
 *       network-number Unsigned16, -- A value of 0 indicates the local network
 *       mac-address OCTET STRING -- A string of length 0 indicates a broadcast
 *   }
 *
 *  @param apdu [out] Buffer in which the APDU contents are built.
 *  @param max_apdu [in] Max length of the APDU buffer.
 *  @param recipient [in] BACNET_RECIPIENT_LIST type linked list of recipients.
 *
 *  @return How many bytes were encoded in the buffer, or
 *   BACNET_STATUS_ABORT if the response would not fit within the buffer.
 */
int restart_encode_restart_recipients(
    uint8_t *apdu, unsigned max_apdu, BACNET_RECIPIENT_LIST *recipient)
{
    int len = 0;
    int apdu_len = 0;
    BACNET_OCTET_STRING octet_string;
    BACNET_RECIPIENT_LIST *pRecipient;

    if ((!apdu) || (max_apdu < 1) || (!recipient)) {
        return (0);
    }

    pRecipient = recipient;
    while (pRecipient != NULL) {
        if (pRecipient->tag == 0) {
            if (max_apdu >= (1 + 4)) {
                /* CHOICE - device [0] BACnetObjectIdentifier */
                len = encode_context_object_id(&apdu[apdu_len], 0,
                    pRecipient->type.device.type,
                    pRecipient->type.device.instance);
                apdu_len += len;
            } else {
                return BACNET_STATUS_ABORT;
            }
        } else if (pRecipient->tag == 1) {
            if (pRecipient->type.address.net) {
                len = (int)(1 + 3 + 2 + pRecipient->type.address.len + 1);
            } else {
                len = (int)(1 + 3 + 2 + pRecipient->type.address.mac_len + 1);
            }
            if (max_apdu >= (unsigned)len) {
                /* CHOICE - address [1] BACnetAddress - opening */
                len = encode_opening_tag(&apdu[apdu_len], 1);
                apdu_len += len;
                /* network-number Unsigned16, */
                /* -- A value of 0 indicates the local network */
                len = encode_application_unsigned(
                    &apdu[apdu_len], pRecipient->type.address.net);
                apdu_len += len;
                /* mac-address OCTET STRING */
                /* -- A string of length 0 indicates a broadcast */
                if (pRecipient->type.address.net == BACNET_BROADCAST_NETWORK) {
                    octetstring_init(&octet_string, NULL, 0);
                } else if (pRecipient->type.address.net) {
                    octetstring_init(&octet_string,
                        &pRecipient->type.address.adr[0],
                        pRecipient->type.address.len);
                } else {
                    octetstring_init(&octet_string,
                        &pRecipient->type.address.mac[0],
                        pRecipient->type.address.mac_len);
                }
                len = encode_application_octet_string(
                    &apdu[apdu_len], &octet_string);
                apdu_len += len;
                /* CHOICE - address [1] BACnetAddress - closing */
                len = encode_closing_tag(&apdu[apdu_len], 1);
                apdu_len += len;
            } else {
                /* not a valid tag - don't encode this one */
            }
        }
        pRecipient = pRecipient->next;
    }

    return apdu_len;
}

/** Handle a request to decode a list of restart notification recipients.
 *
 *  Invoked by a request to write the Device object's
 *  PROP_RESTART_NOTIFICATION_RECIPIENTS.
 *  Loops through the list of timesync recipients, and, for each one,
 *  adds its data from the APDU.
 *
 *   BACnetRecipient ::= CHOICE {
 *       device [0] BACnetObjectIdentifier,
 *       address [1] BACnetAddress
 *   }
 *
 *   BACnetAddress ::= SEQUENCE {
 *       network-number Unsigned16, -- A value of 0 indicates the local network
 *       mac-address OCTET STRING -- A string of length 0 indicates a broadcast
 *   }
 *
 *  @param apdu [in] Buffer in which the APDU contents are read
 *  @param max_apdu [in] length of the APDU buffer.
 *  @param recipient [out] BACNET_RECIPIENT_LIST type linked list of recipients.
 *
 *  @return How many bytes were decoded from the buffer, or
 *   BACNET_STATUS_ABORT if there was a problem decoding the buffer
 */
int restart_decode_restart_recipients(
    uint8_t *apdu, unsigned max_apdu, BACNET_RECIPIENT_LIST *recipient)
{
    int len = 0;
    int apdu_len = 0;
    int tag_len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;
    BACNET_UNSIGNED_INTEGER unsigned_value = 0;
    BACNET_OCTET_STRING octet_string;
    BACNET_RECIPIENT_LIST *pRecipient;

    if ((!apdu) || (max_apdu < 1) || (!recipient)) {
        return BACNET_STATUS_ABORT;
    }

    pRecipient = recipient;
    while (pRecipient != NULL) {
        /* device [0] BACnetObjectIdentifier */
        if (decode_is_context_tag(&apdu[apdu_len], 0)) {
            pRecipient->tag = 0;
            if ((unsigned)(apdu_len + 4) > max_apdu) {
                return BACNET_STATUS_ABORT;
            }
            len = decode_context_object_id(&apdu[apdu_len], 0,
                &pRecipient->type.device.type,
                &pRecipient->type.device.instance);
            if (len < 0) {
                return BACNET_STATUS_ABORT;
            }
            apdu_len += len;
        } else if (decode_is_context_tag(&apdu[apdu_len], 1)) {
            apdu_len += 1;
            pRecipient->tag = 1;
            /* network-number Unsigned16 */
            tag_len = decode_tag_number_and_value(
                &apdu[apdu_len], &tag_number, &len_value_type);
            apdu_len += tag_len;
            if ((unsigned)apdu_len > max_apdu) {
                return BACNET_STATUS_ABORT;
            }
            if (tag_number != BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                return BACNET_STATUS_ABORT;
            }
            len = decode_unsigned(
                &apdu[apdu_len], len_value_type, &unsigned_value);
            pRecipient->type.address.net = (uint16_t)unsigned_value;
            apdu_len += len;
            if ((unsigned)apdu_len > max_apdu) {
                return BACNET_STATUS_ABORT;
            }
            /* mac-address OCTET STRING */
            tag_len = decode_tag_number_and_value(
                &apdu[apdu_len], &tag_number, &len_value_type);
            apdu_len += tag_len;
            if (tag_number != BACNET_APPLICATION_TAG_OCTET_STRING) {
                return BACNET_STATUS_ABORT;
            }
            len = bacnet_octet_string_decode(&apdu[apdu_len],
                max_apdu - apdu_len, len_value_type, &octet_string);
            if (len < 0) {
                return BACNET_STATUS_ERROR;
            }
            apdu_len += len;
            if (octetstring_length(&octet_string) == 0) {
                /* -- A string of length 0 indicates a broadcast */
            } else if (pRecipient->type.address.net) {
                pRecipient->type.address.len =
                    octetstring_copy_value(&pRecipient->type.address.adr[0],
                        sizeof(pRecipient->type.address.adr), &octet_string);
            } else {
                pRecipient->type.address.mac_len =
                    octetstring_copy_value(&pRecipient->type.address.mac[0],
                        sizeof(pRecipient->type.address.mac), &octet_string);
            }
        } else {
            return BACNET_STATUS_ABORT;
        }
        pRecipient = pRecipient->next;
    }

    return apdu_len;
}
