#include <assert.h>
#include "bacdcode.h"
#include "npdu.h"
#include "device.h"
#include "datalink.h"
#include "timestamp.h"
#include "bacdevobjpropref.h"

int bacapp_encode_context_device_obj_property_ref(
    uint8_t * apdu,
	uint8_t tag_number,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE * value)
{
	int len;
	int apdu_len = 0;

	len = encode_opening_tag(&apdu[apdu_len], tag_number);
	apdu_len += len;
	
	len = bacapp_encode_device_obj_property_ref(&apdu[apdu_len], value);
	apdu_len += len;

	len = encode_closing_tag(&apdu[apdu_len], tag_number);
	apdu_len += len;

	return apdu_len;
}

int bacapp_encode_device_obj_property_ref(
    uint8_t * apdu,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE * value)
{
	int len;
	int apdu_len = 0;

	len = encode_context_object_id(&apdu[apdu_len], 0, 
				value->objectIdentifier.type,
				value->objectIdentifier.instance);
	apdu_len += len;

	len = encode_context_enumerated(&apdu[apdu_len], 1, 
				value->propertyIdentifier);
	apdu_len += len;

	if ( value->arrayIndex > 0 )
	{
		len = encode_context_unsigned(&apdu[apdu_len], 2, 
					value->arrayIndex);
		apdu_len += len;
	}
	len = encode_context_object_id(&apdu[apdu_len], 3, 
				value->deviceIndentifier.type,
				value->deviceIndentifier.instance);
	apdu_len += len;

	return apdu_len;
}

int bacapp_decode_device_obj_property_ref(
    uint8_t * apdu,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE * value)
{
	int len;
	int apdu_len = 0;

	if ( -1 == ( len = decode_context_object_id(&apdu[apdu_len], 0, 
							&value->objectIdentifier.type,
							&value->objectIdentifier.instance)) )
	{
		return -1;
	}
	apdu_len += len;

	if ( -1 == ( len = decode_context_enumerated(&apdu[apdu_len], 1, 
							&value->propertyIdentifier)))
	{
		return -1;
	}
	apdu_len += len;

	if ( decode_is_context_tag(&apdu[apdu_len], 2 ))
	{
		if ( -1 == ( len = decode_context_unsigned(&apdu[apdu_len], 2, 
								&value->arrayIndex)))
		{
			return -1;
		}
		apdu_len += len;
	}
	else
	{
		value->arrayIndex = 0;
	}
		
	if ( decode_is_context_tag(&apdu[apdu_len], 3 ))
	{
		if ( -1 == ( len = decode_context_object_id(&apdu[apdu_len], 3, 
							&value->deviceIndentifier.type,
							&value->deviceIndentifier.instance)) )
		{
			return -1;
		}
		apdu_len += len;
	}
	else
	{
		value->deviceIndentifier.instance = 0;
		value->deviceIndentifier.type = 0;
	}

	return apdu_len;
}

int bacapp_decode_context_device_obj_property_ref(
    uint8_t * apdu,
	uint8_t tag_number,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE * value)
{
	int len = 0;
	int section_length;

	if (decode_is_opening_tag_number(&apdu[len], tag_number)) {
		len++;
		section_length = bacapp_decode_device_obj_property_ref(
								&apdu[len], value);

		if ( section_length == -1 )
		{
			len = -1;
		}
		else
		{
			len += section_length;
			if (decode_is_closing_tag_number(&apdu[len], tag_number)) {
				len++;
			}
			else
			{
				len = -1;
			}
		}
	}
	else
	{
		len = -1;
	}
	return len;
}

#ifdef TEST

void testDevIdPropRef(
    Test * pTest)
{
	BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE	inData;
	BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE	outData;
	uint8_t buffer[MAX_APDU];
	int inLen;
	int outLen;


	inData.objectIdentifier.instance = 0x1234;
	inData.objectIdentifier.type     = 15;

	inData.propertyIdentifier        = 25;

	inData.arrayIndex                = 0x5678;

	inData.deviceIndentifier.instance = 0x4343;
	inData.deviceIndentifier.type     = 28;

	inLen = bacapp_encode_device_obj_property_ref(buffer, &inData);
	outLen = bacapp_decode_device_obj_property_ref(buffer, &outData);

	ct_test(pTest, 	outLen == inLen);

	ct_test(pTest, 	inData.objectIdentifier.instance == outData.objectIdentifier.instance);
	ct_test(pTest, 	inData.objectIdentifier.type == outData.objectIdentifier.type);

	ct_test(pTest, 	inData.propertyIdentifier == outData.propertyIdentifier);

	ct_test(pTest, 	inData.arrayIndex == outData.arrayIndex);

	ct_test(pTest, 	inData.deviceIndentifier.instance == outData.deviceIndentifier.instance);
	ct_test(pTest, 	inData.deviceIndentifier.type == outData.deviceIndentifier.type);
}
#ifdef TEST_DEV_ID_PROP_REF

int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Prop Ref", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testDevIdPropRef);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}

#endif // TEST_DEV_ID_PROP_REF
#endif // TEST