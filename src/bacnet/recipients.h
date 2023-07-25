#ifndef RECIPIENTS_H
#define RECIPIENTS_H

#include <stdint.h>
#include "bacnet/bacdef.h"

struct BACnet_Recipient_List;
typedef struct BACnet_Recipient_List {
    /*
       BACnetRecipient ::= CHOICE {
       device [0] BACnetObjectIdentifier,
       address [1] BACnetAddress
       }
     */
    uint8_t tag;
    union {
        BACNET_OBJECT_ID device;
        BACNET_ADDRESS address;
    } type;
    /* simple linked list */
    struct BACnet_Recipient_List *next;
} BACNET_RECIPIENT_LIST;

#endif
