#ifndef HANDLER_RESTART_H
#define HANDLER_RESTART_H

#include <stdbool.h>
#include <stdint.h>
#include "bacnet/bacnet_stack_exports.h"
#include "bacnet/bacdef.h"
#include "bacnet/wp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    BACNET_STACK_EXPORT
    int handler_restart_encode_recipients(
        uint8_t * apdu,
        int max_apdu);
    BACNET_STACK_EXPORT
    bool handler_restart_recipient_write(
        BACNET_WRITE_PROPERTY_DATA * wp_data);
    BACNET_STACK_EXPORT
    void handler_restart_send(
        BACNET_RESTART_REASON reason);
    BACNET_STACK_EXPORT
    bool handler_restart_recipient_address_set(
        unsigned index,
        BACNET_ADDRESS * address);
    BACNET_STACK_EXPORT
    void handler_restart_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
