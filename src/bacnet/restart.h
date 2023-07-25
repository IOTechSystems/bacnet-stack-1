#ifndef RESTART_H
#define RESTART_H

#include <stdint.h>
#include "bacnet/bacnet_stack_exports.h"
#include "bacnet/recipients.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    BACNET_STACK_EXPORT
    int restart_encode_restart_recipients(
        uint8_t * apdu,
        unsigned max_apdu,
        BACNET_RECIPIENT_LIST * recipient);
    BACNET_STACK_EXPORT
    int restart_decode_restart_recipients(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_RECIPIENT_LIST * recipient);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
