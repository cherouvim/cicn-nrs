/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdio.h>
#include <config.h>
#include <string.h>
#include <LongBow/runtime.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

#include <ccnx/forwarder/metis/tlv/metis_TlvSkeleton.h>

#include <ccnx/forwarder/metis/tlv/metis_TlvSchemaV0.h>
#include <ccnx/forwarder/metis/tlv/metis_TlvSchemaV1.h>

//by wschoi
//#define LOG_CHECK

#define INDEX_NAME 0
#define INDEX_KEYID 1
#define INDEX_OBJHASH 2
#define INDEX_HOPLIMIT 3
#define INDEX_INTLIFETIME 4
#define INDEX_CACHETIME 5
#define INDEX_EXPIRYTIME 6
#define INDEX_CPI 7
#define INDEX_FRAGMENTPAYLOAD 8
#define INDEX_CERTIFICATE 9
#define INDEX_PUBKEY 10
#define INDEX_PATHLABEL 11

//by wschoi
#define INDEX_PAYLOADTYPE 12
#define INDEX_GETNAME 13
#define INDEX_PAYLOADGETNAME 14
#define INDEX_KEYNAME 15
#define INDEX_REGISTRATION 16
#define INDEX_REGISTRATION_ADD 17
#define INDEX_REGISTRATION_DEL 18
#define INDEX_REGISTRATION_DEREG 19
#define INDEX_REGNAME 20
#define INDEX_REG_ADD_NAME 21
#define INDEX_REG_DEL_NAME 22
#define INDEX_REG_DEREG_NAME 23

/**
 * The non-opaque representation of the MetisTlvSkeleton.
 *
 * IMPORTANT: if you change this structure, you must make sure the corresponding
 * opaque structure in metis_TlvSkeleton.h has at least that much memory in it.
 */
typedef struct internal_skeleton {
    const struct metis_tlv_ops *tlvOps;
    uint8_t *packet;
    MetisLogger *logger;

    MetisTlvExtent array[MetisTlvSkeleton_ArrayLength];
} _InternalSkeleton;

static void
_assertInvariants(const _InternalSkeleton *skeleton)
{
    assertNotNull(skeleton->tlvOps, "Invalid skeleton, does not have a schema ops");
    assertNotNull(skeleton->packet, "Invalid skeleton, does not have a packet buffer");
}

/**
 * Initialize the skeleton memory
 *
 * Clears all the extents to {0, 0} and sets the tlvOps and packet members for further parsing.
 *
 * @param [in] skeleton The skeleton to initialize
 * @param [in] tlvOps The parser operations to use
 * @param [in] packet the packet buffer (points to byte "0" of the fixed header)
 *
 * Example:
 * @code
 * {
 *    MetisTlvSkeleton skeleton;
 *    _initialize(&skeleton, &MetisTlvSchemaV0_Ops, packet);
 * }
 * @endcode
 */
static void
_initialize(_InternalSkeleton *skeleton, const struct metis_tlv_ops *tlvOps, uint8_t *packet, MetisLogger *logger)
{
    memset(skeleton, 0, sizeof(MetisTlvSkeleton));
    skeleton->packet = packet;
    skeleton->tlvOps = tlvOps;
    skeleton->logger = logger;
    _assertInvariants(skeleton);
}


bool
metisTlvSkeleton_Parse(MetisTlvSkeleton *opaque, uint8_t *packet, MetisLogger *logger)
{
    // do not assert invariants here.  Parse will setup the invariants.
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    uint8_t version = packet[0];

    switch (version) {
        case 0:
            _initialize(skeleton, &MetisTlvSchemaV0_Ops, packet, logger);
            return MetisTlvSchemaV0_Ops.parse(opaque);

        case 1:
            _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
            return MetisTlvSchemaV1_Ops.parse(opaque);

        default:
            if (metisLogger_IsLoggable(logger, MetisLoggerFacility_Message, PARCLogLevel_Warning)) {
                metisLogger_Log(logger, MetisLoggerFacility_Message, PARCLogLevel_Warning, __func__,
                                "Parsing unknown packet version %u", version);
            }
            break;
    }
    return false;
}

// ==========================================================
// Setters

	void
metisTlvSkeleton_SetPayload(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{

	_InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
	skeleton->array[INDEX_PAYLOADTYPE].offset = offset;
	skeleton->array[INDEX_PAYLOADTYPE].length = length;

	if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
		metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
				"Set Payload extent {%u, %u}", offset, length);
	}
}

//wschoi
	void
metisTlvSkeleton_SetKeyname(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
	//by wschoi
#ifdef LOG_CHECK
	printf("metisTlvSkeleton_SetKeyname()\n\n");
#endif
	_InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
	skeleton->array[INDEX_KEYNAME].offset = offset;
	skeleton->array[INDEX_KEYNAME].length = length;

	if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
		metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
				"Set Getname extent {%u, %u}", offset, length);
	}
}





 void
metisTlvSkeleton_SetGetname(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
//by wschoi
#ifdef LOG_CHECK
	printf("metisTlvSkeleton_SetGetname()\n\n");
#endif
        _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
        skeleton->array[INDEX_GETNAME].offset = offset;
        skeleton->array[INDEX_GETNAME].length = length;

        if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
                metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                                "Set Getname extent {%u, %u}", offset, length);
        }
}

	void
metisTlvSkeleton_SetPayloadGetname(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
	//by wschoi
#ifdef LOG_CHECK
	printf("metisTlvSkeleton_metisTlvSkeleton_SetPayloadGetname()\n\n");
#endif
	_InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
	skeleton->array[INDEX_PAYLOADGETNAME].offset = offset;
	skeleton->array[INDEX_PAYLOADGETNAME].length = length;

	if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
		metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
				"Set Getname extent {%u, %u}", offset, length);
	}
}


//by wschoi

//reg
	void
metisTlvSkeleton_SetRegistration(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
	_InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
	skeleton->array[INDEX_REGISTRATION].offset = offset;
	skeleton->array[INDEX_REGISTRATION].length = length;
#ifdef LOG_CHECK
	printf("metisTlvSkeleton_SetRegistration(), check T_REG type, offset=%d, v_length=%d\n", offset, length);
#endif


	if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
		metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
				"Set REGISTRATION extent {%u, %u}", offset, length);
	}
}


//add
        void
metisTlvSkeleton_SetRegistration_add(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
        _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
        skeleton->array[INDEX_REGISTRATION_ADD].offset = offset;
        skeleton->array[INDEX_REGISTRATION_ADD].length = length;
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_SetRegistration_add(), check T_ADD type, offset=%d, v_length=%d\n", offset, length);
#endif


        if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
                metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                                "Set REGISTRATION_add extent {%u, %u}", offset, length);
        }
}


//del
        void
metisTlvSkeleton_SetRegistration_del(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
        _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
        skeleton->array[INDEX_REGISTRATION_DEL].offset = offset;
        skeleton->array[INDEX_REGISTRATION_DEL].length = length;
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_SetRegistration_del(), check T_DEL type, offset=%d, v_length=%d\n", offset, length);
#endif


        if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
                metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                                "Set REGISTRATION_del extent {%u, %u}", offset, length);
        }
}

//dereg
        void
metisTlvSkeleton_SetRegistration_dereg(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
        _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
        skeleton->array[INDEX_REGISTRATION_DEREG].offset = offset;
        skeleton->array[INDEX_REGISTRATION_DEREG].length = length;
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_SetRegistration_dereg(), check T_DEREG type, offset=%d, v_length=%d\n", offset, length);
#endif


        if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
                metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                                "Set REGISTRATION_dereg extent {%u, %u}", offset, length);
        }
}












MetisTlvExtent
metisTlvSkeleton_GetPayloadName(const MetisTlvSkeleton *skeleton)
{
	//by wschoi
#ifdef LOG_CHECK
	printf("metisTlvSkeleton_GetPayloadName()\n\n");
#endif
    return skeleton->array[INDEX_PAYLOADTYPE];
}


//by wschoi
MetisTlvExtent
metisTlvSkeleton_GetKeyname(const MetisTlvSkeleton *skeleton)
{
#ifdef LOG_CHECK
	printf("metisTlvSkeleton_GetKeyname()\n\n");
#endif
	return skeleton->array[INDEX_KEYNAME];
}




//by wschoi
MetisTlvExtent
metisTlvSkeleton_GetGetname(const MetisTlvSkeleton *skeleton)
{
#ifdef LOG_CHECK
	printf("metisTlvSkeleton_GetGetname()\n\n");
#endif
    return skeleton->array[INDEX_GETNAME];
}

//by wschoi
	MetisTlvExtent
metisTlvSkeleton_GetPayloadGetname(const MetisTlvSkeleton *skeleton)
{
#ifdef LOG_CHECK
	printf("metisTlvSkeleton_GetPayloadGetname()\n\n");
#endif
	return skeleton->array[INDEX_PAYLOADGETNAME];
}


//by wschoi

//reg
        void
metisTlvSkeleton_SetRegname(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
        //by wschoi
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_SetGetname(), offset=%d, length=%d\n", offset, length);

#endif
        _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
        skeleton->array[INDEX_REGNAME].offset = offset;
        skeleton->array[INDEX_REGNAME].length = length;

        if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
                metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                                "Set GetReg_name extent {%u, %u}", offset, length);
        }
}


//add
        void
metisTlvSkeleton_SetReg_Add_name(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
        //by wschoi
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_SetReg_Add_name(), offset=%d, length=%d\n", offset, length);

#endif
        _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
        skeleton->array[INDEX_REG_ADD_NAME].offset = offset;
        skeleton->array[INDEX_REG_ADD_NAME].length = length;

        if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
                metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                                "Set GetReg_Add_name extent {%u, %u}", offset, length);
        }
}


//del
        void
metisTlvSkeleton_SetReg_Del_name(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
        //by wschoi
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_SetReg_Del_name(), offset=%d, length=%d\n", offset, length);

#endif
        _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
        skeleton->array[INDEX_REG_DEL_NAME].offset = offset;
        skeleton->array[INDEX_REG_DEL_NAME].length = length;

        if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
                metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                                "Set GetReg_Del_name extent {%u, %u}", offset, length);
        }
}
//dereg
        void
metisTlvSkeleton_SetReg_Dereg_name(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
        //by wschoi
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_SetGet_Dereg_name(), offset=%d, length=%d\n", offset, length);

#endif
        _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
        skeleton->array[INDEX_REG_DEREG_NAME].offset = offset;
        skeleton->array[INDEX_REG_DEREG_NAME].length = length;

        if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
                metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                                "Set GetReg_Dereg_name extent {%u, %u}", offset, length);
        }
}


//by wschoi

//reg
        MetisTlvExtent
metisTlvSkeleton_GetRegname(const MetisTlvSkeleton *skeleton)
{
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_GetRegname(), skeleton->array[INDEX_REGNAME].offset=%d, skeleton->array[INDEX_REGNAME].length=%d\n", skeleton->array[INDEX_REGNAME].offset, skeleton->array[INDEX_REGNAME].length);

#endif
        return skeleton->array[INDEX_REGNAME];
}


//add
        MetisTlvExtent
metisTlvSkeleton_GetReg_Add_name(const MetisTlvSkeleton *skeleton)
{
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_GetReg_Add_name(), skeleton->array[INDEX_REG_ADD_NAME].offset=%d, skeleton->array[INDEX_REG_ADD_NAME].length=%d\n", skeleton->array[INDEX_REG_ADD_NAME].offset, skeleton->array[INDEX_REG_ADD_NAME].length);

#endif
        return skeleton->array[INDEX_REG_ADD_NAME];
}


//del
        MetisTlvExtent
metisTlvSkeleton_GetReg_Del_name(const MetisTlvSkeleton *skeleton)
{
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_GetReg_Del_name(), skeleton->array[INDEX_REG_DEL_NAME].offset=%d, skeleton->array[INDEX_REG_DEL_NAME].length=%d\n", skeleton->array[INDEX_REG_DEL_NAME].offset, skeleton->array[INDEX_REG_DEL_NAME].length);

#endif
        return skeleton->array[INDEX_REG_DEL_NAME];
}

//dereg
        MetisTlvExtent
metisTlvSkeleton_GetReg_Dereg_name(const MetisTlvSkeleton *skeleton)
{
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_GetReg_Dereg_name(), skeleton->array[INDEX_REG_DEREGNAME].offset=%d, skeleton->array[INDEX_REG_DEREG_NAME].length=%d\n", skeleton->array[INDEX_REG_DEREG_NAME].offset, skeleton->array[INDEX_REG_DEREG_NAME].length);

#endif
        return skeleton->array[INDEX_REG_DEREG_NAME];
}


//by wschoi



//registration
//reg
MetisTlvExtent
metisTlvSkeleton_GetRegistration(const MetisTlvSkeleton *skeleton)
{
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_GetRegistration(), skeleton->array[INDEX_REGISTRATION].offset %d, .length=%d\n", skeleton->array[INDEX_REGISTRATION].offset, skeleton->array[INDEX_REGISTRATION].length);
#endif
        return skeleton->array[INDEX_REGISTRATION];

}

//add
MetisTlvExtent
metisTlvSkeleton_GetRegistration_add(const MetisTlvSkeleton *skeleton)
{
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_GetRegistration_add(), skeleton->array[INDEX_REGISTRATION_ADD].offset %d, .length=%d\n", skeleton->array[INDEX_REGISTRATION_ADD].offset, skeleton->array[INDEX_REGISTRATION_ADD].length);
#endif
        return skeleton->array[INDEX_REGISTRATION_ADD];

}



//del
MetisTlvExtent
metisTlvSkeleton_GetRegistration_del(const MetisTlvSkeleton *skeleton)
{
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_GetRegistration_del(), skeleton->array[INDEX_REGISTRATION_DEL].offset %d, .length=%d\n", skeleton->array[INDEX_REGISTRATION_DEL].offset, skeleton->array[INDEX_REGISTRATION_DEL].length);
#endif
        return skeleton->array[INDEX_REGISTRATION_DEL];

}

//dereg
MetisTlvExtent
metisTlvSkeleton_GetRegistration_dereg(const MetisTlvSkeleton *skeleton)
{
#ifdef LOG_CHECK
        printf("metisTlvSkeleton_GetRegistration_dereg(), skeleton->array[INDEX_REGISTRATION_DEREG].offset %d, .length=%d\n", skeleton->array[INDEX_REGISTRATION_DEREG].offset, skeleton->array[INDEX_REGISTRATION_DEREG].length);
#endif
        return skeleton->array[INDEX_REGISTRATION_DEREG];

}




	void
metisTlvSkeleton_SetName(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    skeleton->array[INDEX_NAME].offset = offset;
    skeleton->array[INDEX_NAME].length = length;

    if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
        metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                        "Set name extent {%u, %u}", offset, length);
    }
}

void
metisTlvSkeleton_SetKeyId(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    skeleton->array[INDEX_KEYID].offset = offset;
    skeleton->array[INDEX_KEYID].length = length;

    if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
        metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                        "Set keyid extent {%u, %u}", offset, length);
    }
}

void
metisTlvSkeleton_SetCertificate(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    skeleton->array[INDEX_CERTIFICATE].offset = offset;
    skeleton->array[INDEX_CERTIFICATE].length = length;

    if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
        metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                        "Set certificate extent {%u, %u}", offset, length);
    }
}

void
metisTlvSkeleton_SetPublicKey(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    skeleton->array[INDEX_PUBKEY].offset = offset;
    skeleton->array[INDEX_PUBKEY].length = length;

    if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
        metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                        "Set public key extent {%u, %u}", offset, length);
    }
}

void
metisTlvSkeleton_SetObjectHash(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    skeleton->array[INDEX_OBJHASH].offset = offset;
    skeleton->array[INDEX_OBJHASH].length = length;

    if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
        metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                        "Set objhash extent {%u, %u}", offset, length);
    }
}

void
metisTlvSkeleton_SetHopLimit(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    skeleton->array[INDEX_HOPLIMIT].offset = offset;
    skeleton->array[INDEX_HOPLIMIT].length = length;

    if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
        metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                        "Set hoplimit extent {%u, %u}", offset, length);
    }
}

void
metisTlvSkeleton_SetInterestLifetime(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    skeleton->array[INDEX_INTLIFETIME].offset = offset;
    skeleton->array[INDEX_INTLIFETIME].length = length;

    if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
        metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                        "Set int lifetime extent {%u, %u}", offset, length);
    }
}

void
metisTlvSkeleton_SetPathLabel(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    skeleton->array[INDEX_PATHLABEL].offset = offset;
    skeleton->array[INDEX_PATHLABEL].length = length;

    if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
        metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                        "Set path label extent {%u, %u}", offset, length);
    }
}


void
metisTlvSkeleton_SetCacheTimeHeader(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    skeleton->array[INDEX_CACHETIME].offset = offset;
    skeleton->array[INDEX_CACHETIME].length = length;

    if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
        metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                        "Set cachetime extent {%u, %u}", offset, length);
    }
}

void
metisTlvSkeleton_SetExpiryTime(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    skeleton->array[INDEX_EXPIRYTIME].offset = offset;
    skeleton->array[INDEX_EXPIRYTIME].length = length;

    if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
        metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                        "Set expirytime extent {%u, %u}", offset, length);
    }
}

void
metisTlvSkeleton_SetCPI(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    skeleton->array[INDEX_CPI].offset = offset;
    skeleton->array[INDEX_CPI].length = length;

    if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
        metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                        "Set cpi extent {%u, %u}", offset, length);
    }
}

bool
metisTlvSkeleton_UpdateHopLimit(MetisTlvSkeleton *opaque, uint8_t hoplimit)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    bool updated = false;
    if (!metisTlvExtent_Equals(&skeleton->array[INDEX_HOPLIMIT], &metisTlvExtent_NotFound)) {
        if (skeleton->array[INDEX_HOPLIMIT].length == 1) {
            updated = true;
            uint8_t *value = skeleton->packet + skeleton->array[INDEX_HOPLIMIT].offset;
            *value = hoplimit;

            if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
                metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                                "set hoplimit %u", hoplimit);
            }
        }
    }
    return updated;
}

bool
metisTlvSkeleton_UpdatePathLabel(MetisTlvSkeleton *opaque, uint8_t outFace)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    bool updated = false;
    if (!metisTlvExtent_Equals(&skeleton->array[INDEX_PATHLABEL], &metisTlvExtent_NotFound)) {
        if (skeleton->array[INDEX_PATHLABEL].length == 1) {
            updated = true;
            uint8_t *value = skeleton->packet + skeleton->array[INDEX_PATHLABEL].offset;
            uint8_t oldPathLabel = (uint8_t) (*value);
            uint8_t tmp = (oldPathLabel << 1) | (oldPathLabel >> 7);
            uint8_t newPathLabel = (tmp ^ outFace);
            *value = newPathLabel;
            if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
                metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                                "set pathlabel %u", newPathLabel);
            }
        }
    }
    return updated;
}

bool
metisTlvSkeleton_ResetPathLabel(MetisTlvSkeleton *opaque)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    bool updated = false;
    if (!metisTlvExtent_Equals(&skeleton->array[INDEX_PATHLABEL], &metisTlvExtent_NotFound)) {
        if (skeleton->array[INDEX_PATHLABEL].length == 1) {
            updated = true;
            uint8_t *value = skeleton->packet + skeleton->array[INDEX_PATHLABEL].offset;
            *value = 0;
            if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
                metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                                "reset pathlabel 0");
            }
        }
    }
    return updated;
}

void
metisTlvSkeleton_SetFragmentPayload(MetisTlvSkeleton *opaque, size_t offset, size_t length)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    skeleton->array[INDEX_FRAGMENTPAYLOAD].offset = offset;
    skeleton->array[INDEX_FRAGMENTPAYLOAD].length = length;

    if (metisLogger_IsLoggable(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug)) {
        metisLogger_Log(skeleton->logger, MetisLoggerFacility_Message, PARCLogLevel_Debug, __func__,
                        "Set fragment payload extent {%u, %u}", offset, length);
    }
}



// ==========================================================

MetisTlvExtent
metisTlvSkeleton_GetName(const MetisTlvSkeleton *skeleton)
{
    return skeleton->array[INDEX_NAME];
}

MetisTlvExtent
metisTlvSkeleton_GetKeyId(const MetisTlvSkeleton *skeleton)
{
    return skeleton->array[INDEX_KEYID];
}

MetisTlvExtent
metisTlvSkeleton_GetCertificate(const MetisTlvSkeleton *skeleton)
{
    return skeleton->array[INDEX_CERTIFICATE];
}

MetisTlvExtent
metisTlvSkeleton_GetPublicKey(const MetisTlvSkeleton *skeleton)
{
    return skeleton->array[INDEX_PUBKEY];
}

MetisTlvExtent
metisTlvSkeleton_GetObjectHash(const MetisTlvSkeleton *skeleton)
{
    return skeleton->array[INDEX_OBJHASH];
}

MetisTlvExtent
metisTlvSkeleton_GetHopLimit(const MetisTlvSkeleton *skeleton)
{
    return skeleton->array[INDEX_HOPLIMIT];
}

MetisTlvExtent
metisTlvSkeleton_GetInterestLifetime(const MetisTlvSkeleton *skeleton)
{
    return skeleton->array[INDEX_INTLIFETIME];
}

MetisTlvExtent
metisTlvSkeleton_GetPathLabel(const MetisTlvSkeleton *skeleton)
{
    return skeleton->array[INDEX_PATHLABEL];
}

MetisTlvExtent
metisTlvSkeleton_GetCacheTimeHeader(const MetisTlvSkeleton *skeleton)
{
    return skeleton->array[INDEX_CACHETIME];
}

MetisTlvExtent
metisTlvSkeleton_GetExpiryTime(const MetisTlvSkeleton *skeleton)
{
    return skeleton->array[INDEX_EXPIRYTIME];
}

MetisTlvExtent
metisTlvSkeleton_GetCPI(const MetisTlvSkeleton *skeleton)
{
    return skeleton->array[INDEX_CPI];
}

MetisTlvExtent
metisTlvSkeleton_GetFragmentPayload(const MetisTlvSkeleton *skeleton)
{
    return skeleton->array[INDEX_FRAGMENTPAYLOAD];
}


const uint8_t *
metisTlvSkeleton_GetPacket(const MetisTlvSkeleton *opaque)
{
    const _InternalSkeleton *skeleton = (const _InternalSkeleton *) opaque;
    _assertInvariants(skeleton);
    return skeleton->packet;
}

PARCCryptoHash *
metisTlvSkeleton_ComputeContentObjectHash(const MetisTlvSkeleton *opaque)
{
    const _InternalSkeleton *skeleton = (const _InternalSkeleton *) opaque;
    _assertInvariants(skeleton);
    return skeleton->tlvOps->computeContentObjectHash(skeleton->packet);
}

size_t
metisTlvSkeleton_TotalPacketLength(const MetisTlvSkeleton *opaque)
{
    const _InternalSkeleton *skeleton = (const _InternalSkeleton *) opaque;
    _assertInvariants(skeleton);
    return skeleton->tlvOps->totalPacketLength(skeleton->packet);
}

bool
metisTlvSkeleton_IsPacketTypeInterest(const MetisTlvSkeleton *opaque)
{
    const _InternalSkeleton *skeleton = (const _InternalSkeleton *) opaque;
    _assertInvariants(skeleton);
    return skeleton->tlvOps->isPacketTypeInterest(skeleton->packet);
}

bool
metisTlvSkeleton_IsPacketTypeContentObject(const MetisTlvSkeleton *opaque)
{
    const _InternalSkeleton *skeleton = (const _InternalSkeleton *) opaque;
    _assertInvariants(skeleton);
    return skeleton->tlvOps->isPacketTypeContentObject(skeleton->packet);
}

bool
metisTlvSkeleton_IsPacketTypeControl(const MetisTlvSkeleton *opaque)
{
    const _InternalSkeleton *skeleton = (const _InternalSkeleton *) opaque;
    _assertInvariants(skeleton);
    return skeleton->tlvOps->isPacketTypeControl(skeleton->packet);
}

bool
metisTlvSkeleton_IsPacketTypeInterestReturn(const MetisTlvSkeleton *opaque)
{
    const _InternalSkeleton *skeleton = (const _InternalSkeleton *) opaque;
    _assertInvariants(skeleton);
    return skeleton->tlvOps->isPacketTypeInterestReturn(skeleton->packet);
}

bool
metisTlvSkeleton_IsPacketTypeHopByHopFragment(const MetisTlvSkeleton *opaque)
{
    const _InternalSkeleton *skeleton = (const _InternalSkeleton *) opaque;
    _assertInvariants(skeleton);
    return skeleton->tlvOps->isPacketTypeHopByHopFragment(skeleton->packet);
}

MetisLogger *
metisTlvSkeleton_GetLogger(const MetisTlvSkeleton *opaque)
{
    const _InternalSkeleton *skeleton = (const _InternalSkeleton *) opaque;
    _assertInvariants(skeleton);
    return skeleton->logger;
}
