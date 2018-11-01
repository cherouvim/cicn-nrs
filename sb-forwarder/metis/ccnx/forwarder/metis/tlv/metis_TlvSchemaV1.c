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


#include <config.h>

#include <LongBow/runtime.h>

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include <ccnx/forwarder/metis/tlv/metis_Tlv.h>
#include <ccnx/forwarder/metis/tlv/metis_TlvExtent.h>

#include <parc/algol/parc_Memory.h>
#include <parc/security/parc_CryptoHasher.h>

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




typedef struct __attribute__ ((__packed__)) metis_tlv_fixed_header {
    uint8_t version;
    uint8_t packetType;
    uint16_t packetLength;
    uint8_t interestHopLimit;
    uint8_t returnCode;
    uint8_t flags;
    uint8_t headerLength;
} _MetisTlvFixedHeaderV1;

#define METIS_PACKET_TYPE_INTEREST 0
#define METIS_PACKET_TYPE_CONTENT  1
#define METIS_PACKET_TYPE_INTERESTRETURN 2
#define METIS_PACKET_TYPE_HOPFRAG 4
#define METIS_PACKET_TYPE_CONTROL  0xA4

// -----------------------------
// in host byte order

#define T_NAME     0x0000

// perhop headers
#define T_INTLIFE    0x0001
#define T_CACHETIME  0x0002
#define T_PATHLABEL  0x0003
#define T_FLOW       0x0005

// Top-level TLVs
#define T_INTEREST   0x0001
#define T_OBJECT     0x0002
#define T_VALALG     0x0003
#define T_VALPAYLOAD 0x0004
#define T_HOPFRAG_PAYLOAD  0x0005
#define T_MANIFEST   0x0006

// inside interest
#define T_KEYIDRES    0x0002
#define T_OBJHASHRES  0x0003

//by wschoi, inside interest

#define T_GETNAME 0x0111
#define T_PAYLOAD_GETNAME 0x0112
#define T_KEYNAME 0x0113

#define T_REG 0x0114
#define T_REG_ACK 0x0116

#define T_ADD 0x0117
#define T_ADD_ACK 0x0119

#define T_DEL 0x0120
#define T_DEL_ACK 0x0122

#define T_DEREG 0x0123
#define T_DEREG_ACK 0x0125


// inside a content object
#define T_EXPIRYTIME  0x0006

// ValidationAlg

// these are the algorithms we need a KEYID for
#define T_RSA_SHA256     0x0006
#define T_EC_SECP_256K1  0x0007
#define T_EC_SECP_384R1  0x0008

#define T_KEYID          0x0009
#define T_PUBLICKEY      0x000B
#define T_CERT           0x000C

// inside a CPI
#define T_CPI            0xBEEF


// -----------------------------
// Internal API

/**
 * Parse the per-hop headers.
 *
 * Will return the absolute offset of the next byte to parse (i.e. 'endHeaders')
 *
 * @param [in] packet The packet buffer
 * @param [in] offset The first byte to begin parsing at
 * @param [in] endMessage The ceiling of bytes to parse
 * @param [in] skeleton The structure to fill in
 */
static void
_parsePerHopV1(const uint8_t *packet, size_t offset, size_t endHeaders, MetisTlvSkeleton *skeleton)
{
    const size_t tl_length = sizeof(MetisTlvType);

    // we only parse to the end of the per-hop headers or until we've found
    // the 2 headers we want (hoplimit, fragmentation header)
    while (offset + sizeof(MetisTlvType) < endHeaders) {
        const MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
        const uint16_t type = htons(tlv->type);
        const uint16_t v_length = htons(tlv->length);

        // move past the TL header
        offset += tl_length;

        size_t endSubSection = offset + v_length;
        if (endSubSection <= endHeaders) {
            switch (type) {
                case T_INTLIFE:
                    metisTlvSkeleton_SetInterestLifetime(skeleton, offset, v_length);
                    break;

                // should verify that we dont have both INTFRAG and OBJFRAG
                case T_CACHETIME:
                    metisTlvSkeleton_SetCacheTimeHeader(skeleton, offset, v_length);
                    break;

                case T_PATHLABEL:
                    metisTlvSkeleton_SetPathLabel(skeleton, offset, v_length);
                    break;

                default:
                    break;
            }
        }

        offset = endSubSection;
    }
}

static void
_parseSignatureParameters(const uint8_t *packet, size_t offset, size_t endSection, struct tlv_skeleton *skeleton)
{
    // Scan the section for KeyId, and optional Certificate or PublicKey.
    while (offset + sizeof(MetisTlvType) < endSection) {
        const MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
        const uint16_t type = htons(tlv->type);
        const uint16_t v_length = htons(tlv->length);

        // move past the TL header
        offset += sizeof(MetisTlvType);

        size_t endSubSection = offset + v_length;
        if (endSubSection <= endSection) {
            switch (type) {
                case T_KEYID:
                    metisTlvSkeleton_SetKeyId(skeleton, offset, v_length);
                    break;

                case T_CERT:
                    metisTlvSkeleton_SetCertificate(skeleton, offset, v_length);
                    break;

                case T_PUBLICKEY:
                    metisTlvSkeleton_SetPublicKey(skeleton, offset, v_length);

                default:
                    break;
            }
        }
        offset += v_length;
    }
}

static void
_parseValidationType(const uint8_t *packet, size_t offset, size_t endSection, struct tlv_skeleton *skeleton)
{
    if (offset + sizeof(MetisTlvType) < endSection) {
        const MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
        const uint16_t type = htons(tlv->type);
        const uint16_t v_length = htons(tlv->length);

        // move past the TL header
        offset += sizeof(MetisTlvType);

        size_t endSubSection = offset + v_length;
        if (endSubSection <= endSection) {
            switch (type) {
                // These are the Validation Algorithms that have a usable KeyId
                case T_EC_SECP_256K1: // fallthrough
                case T_EC_SECP_384R1: // fallthrough
                case T_RSA_SHA256:
                    _parseSignatureParameters(packet, offset, endSubSection, skeleton);
                    return;

                default:
                    break;
            }
        }
    }
}

static size_t
_parseValidationAlg(const uint8_t *packet, size_t offset, size_t endMessage, struct tlv_skeleton *skeleton)
{
    size_t endSection = endMessage;

    if (offset + sizeof(MetisTlvType) < endMessage) {
        const MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
        const uint16_t type = htons(tlv->type);
        const uint16_t v_length = htons(tlv->length);

        offset += sizeof(MetisTlvType);
        endSection = offset + v_length;

        // make sure we don't have container overrun
        if (endSection <= endMessage && type == T_VALALG) {
            _parseValidationType(packet, offset, endSection, skeleton);
        }
    }

    return endSection;
}

/**
 * Parse the "value" of a T_OBJECT
 *
 * 'offset' should point to the first byte of the "value" of the T_OBJECT container
 *
 * @param [in] packet The packet buffer
 * @param [in] offset The first byte to begin parsing at
 * @param [in] endSection The ceiling of bytes to parse
 * @param [in] skeleton The structure to fill in
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
	static void
_parseObjectV1(const uint8_t *packet, size_t offset, size_t endSection, MetisTlvSkeleton *skeleton)
{
	int foundCount = 0;
	int foundCountLimit=4;

	// parse to the end or until we find the two things we need (name, keyid)
	while (offset < endSection && foundCount < foundCountLimit) {
		const MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
		const uint16_t type = htons(tlv->type);
		const uint16_t v_length = htons(tlv->length);


		// move past the TL header
		offset += sizeof(MetisTlvType);
		size_t endSubSection = offset + v_length;

		//by wschoi
#ifdef LOG_CHECK
		printf("_parseObjectV1, offset %d, endMessage=endSection %d, v_length %d, endSubSection%d\n\n", offset, endSection, v_length, endSubSection);
printf("type is : %x, in _parseObjectV1\n", type);
#endif
		if (endSubSection <= endSection) {
			switch (type) {
				case T_NAME:
//					if(endSubSection<endSection)
					if(endSubSection<endSection || foundCount==0)
					{
						metisTlvSkeleton_SetName(skeleton, offset, v_length);
					}
					else
					{
#ifdef LOG_CHECK
						printf("before metisTlvSkeleton_SetPayloadGetname, _parseObjectV1, offset %d, endMessage=endSection %d, v_length %d, endSubSection%d\n\n", offset, endSection, v_length, endSubSection);
#endif
						metisTlvSkeleton_SetPayloadGetname(skeleton, offset, v_length);
//						metisTlvSkeleton_SetKeyname(skeleton, offset, v_length);
					}

					MetisTlvExtent extent_name = metisTlvSkeleton_GetName(&skeleton);
#ifdef LOG_CHECK
					printf("T_NAME metisTlvSkeleton_GetName extent_name.offset = %d, extent_name.length = %d \n\n", extent_name.offset, extent_name.length);
#endif
					foundCount++;
					break;

				case T_EXPIRYTIME:
					metisTlvSkeleton_SetExpiryTime(skeleton, offset, v_length);
					foundCount++;
					break;

					//by wschoi
				case T_PAYLOAD_GETNAME:
					metisTlvSkeleton_SetPayload(skeleton, offset, v_length);
					MetisTlvExtent extent = metisTlvSkeleton_GetPayloadName(&skeleton);
#ifdef LOG_CHECK
					printf("T_PAYLOAD_GETNAME metisTlvSkeleton_GetPayloadName extent.offset= %d, extent.length=%d \n\n", extent.offset, extent.length);
#endif
					endSubSection=offset;
					foundCount++;
					break;


				default:
					break;
			}
		}

		offset = endSubSection;
	}
}

#if 0
	static void
_parseObjectV1(const uint8_t *packet, size_t offset, size_t endSection, MetisTlvSkeleton *skeleton)
{
    int foundCount = 0;

    // parse to the end or until we find the two things we need (name, keyid)
    while (offset < endSection && foundCount < 2) {
        const MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
        const uint16_t type = htons(tlv->type);
        const uint16_t v_length = htons(tlv->length);

        // move past the TL header
        offset += sizeof(MetisTlvType);
        size_t endSubSection = offset + v_length;
        if (endSubSection <= endSection) {
            switch (type) {
                case T_NAME:
                    metisTlvSkeleton_SetName(skeleton, offset, v_length);
                    foundCount++;
                    break;

                case T_EXPIRYTIME:
                    metisTlvSkeleton_SetExpiryTime(skeleton, offset, v_length);
                    foundCount++;
                    break;

                default:
                    break;
            }
        }

        offset = endSubSection;
    }
}
#endif

/**
 * Parse the "value" of a T_INTEREST
 *
 * 'offset' should point to the first byte of the "value" of the T_INTEREST container
 *
 * @param [in] packet The packet buffer
 * @param [in] offset The first byte to begin parsing at
 * @param [in] endSection The ceiling of bytes to parse
 * @param [in] skeleton The structure to fill in
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_parseInterestV1(const uint8_t *packet, size_t offset, size_t endSection, struct tlv_skeleton *skeleton)
{
    int foundCount = 0;
//by wschoi
int foundCountLimit=4;
//int foundCountLimit=2;


    // parse to the end or until we find all 3 things (name, keyid, objecthash)
    while (offset < endSection && foundCount < foundCountLimit) {
        const MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
        const uint16_t type = htons(tlv->type);
        const uint16_t v_length = htons(tlv->length);


        // skip past the TLV header
        offset += sizeof(MetisTlvType);
        size_t endSubSection = offset + v_length;

		//by wschoi
#ifdef LOG_CHECK
		printf("_parseInterestV1, offset %d, endMessage=endSection %d, v_length %d, endSubSection%d, type 0x%x\n\n", offset, endSection, v_length, endSubSection, type);
#endif


        if (endSubSection <= endSection) {
            switch (type) {
		    case T_NAME:
#if 0
			    if(endSubSection<endSection || foundCount==0)
			    {
				    metisTlvSkeleton_SetName(skeleton, offset, v_length);
			    }
			    else
			    {

				    metisTlvSkeleton_SetGetname(skeleton, offset, v_length);
					metisTlvSkeleton_SetKeyname(skeleton, offset, v_length);
			    }

			    MetisTlvExtent extent_name = metisTlvSkeleton_GetName(&skeleton);
			    printf("T_NAME metisTlvSkeleton_GetName extent_name.offset = %d, extent_name.length = %d \n\n", extent_name.offset, extent_name.length);
			    foundCount++;
			    break;
#endif


   if(skeleton->array[INDEX_REGISTRATION].length>0)
                                        {

#ifdef LOG_CHECK
                                                printf("_parse_InterestV1(), skeleton->array[INDEX_REGISTRATION].length>0\n");
#endif

                                                metisTlvSkeleton_SetRegname(skeleton, offset, v_length);
                                        }
                                        else if(skeleton->array[INDEX_REGISTRATION_ADD].length>0)
                                        {

#ifdef LOG_CHECK
                                                printf("_parse_InterestV1(), skeleton->array[INDEX_REGISTRATION_ADD].length>0\n");
#endif

                                                metisTlvSkeleton_SetReg_Add_name(skeleton, offset, v_length);



					}
                                        else if(skeleton->array[INDEX_REGISTRATION_DEL].length>0)
                                        {

#ifdef LOG_CHECK
                                                printf("_parse_InterestV1(), skeleton->array[INDEX_REGISTRATION_DEL].length>0\n");
#endif

                                                metisTlvSkeleton_SetReg_Del_name(skeleton, offset, v_length);
                                        }
                                        else if(skeleton->array[INDEX_REGISTRATION_DEREG].length>0)
                                        {

#ifdef LOG_CHECK
                                                printf("_parse_InterestV1(), skeleton->array[INDEX_REGISTRATION_DEREG].length>0\n");
#endif

						metisTlvSkeleton_SetReg_Dereg_name(skeleton, offset, v_length);
                                        }

                                        else if(skeleton->array[INDEX_PAYLOADTYPE].length>0)
                                        {
#ifdef LOG_CHECK
                                                printf("into metisTlvSkeleton_SetKeyname()\n");
#endif

						metisTlvSkeleton_SetGetname(skeleton, offset, v_length);
                                                metisTlvSkeleton_SetKeyname(skeleton, offset, v_length);
                                        }
                                        else
                                        {
                                                metisTlvSkeleton_SetName(skeleton, offset, v_length);
                                        }

					foundCount++;
					break;


		    case T_KEYIDRES:
			    metisTlvSkeleton_SetKeyId(skeleton, offset, v_length);
			    foundCount++;
			    break;

		    case T_OBJHASHRES:
			    metisTlvSkeleton_SetObjectHash(skeleton, offset, v_length);
			    foundCount++;
			    break;
//by wschoi
		    case T_REG:
#ifdef LOG_CHECK
			    printf("_parseInterestV1(), check T_REG type, offset=%d, v_length=%d\n", offset, v_length);
#endif
			    metisTlvSkeleton_SetRegistration(skeleton, offset, v_length);
			    endSubSection=offset;
			    foundCount++;
			    break;

//by wschoi
		    case T_ADD:
#ifdef LOG_CHECK
			    printf("_parseInterestV1(), check T_ADD type, offset=%d, v_length=%d\n", offset, v_length);
#endif
			    metisTlvSkeleton_SetRegistration_add(skeleton, offset, v_length);
			    endSubSection=offset;
			    foundCount++;
			    break;

//by wschoi
		    case T_DEL:
#ifdef LOG_CHECK
			    printf("_parseInterestV1(), check T_DEL type, offset=%d, v_length=%d\n", offset, v_length);
#endif
			    metisTlvSkeleton_SetRegistration_del(skeleton, offset, v_length);
			    endSubSection=offset;
			    foundCount++;
			    break;

//by wschoi
		    case T_DEREG:
#ifdef LOG_CHECK
			    printf("_parseInterestV1(), check T_DEREG type, offset=%d, v_length=%d\n", offset, v_length);
#endif
			    metisTlvSkeleton_SetRegistration_dereg(skeleton, offset, v_length);
			    endSubSection=offset;
			    foundCount++;
			    break;

			    //by wschoi
		    case T_GETNAME:
			    metisTlvSkeleton_SetPayload(skeleton, offset, v_length);
			    MetisTlvExtent extent = metisTlvSkeleton_GetPayloadName(&skeleton);
#ifdef LOG_CHECK
			    printf("T_GETNAME metisTlvSkeleton_GetPayloadName extent.offset= %d, extent.length=%d \n\n", extent.offset, extent.length);
#endif

			    endSubSection=offset;
			    foundCount++;
			    break;

			    //by wschoi
		    case T_KEYNAME:
			    metisTlvSkeleton_SetPayload(skeleton, offset, v_length);
			    MetisTlvExtent extent_key = metisTlvSkeleton_GetPayloadName(&skeleton);
#ifdef LOG_CHECK
			    printf("T_KEYNAME metisTlvSkeleton_GetPayloadName extent_key.offset= %d, extent_key.length=%d \n\n", extent_key.offset, extent_key.length);
#endif

			    endSubSection=offset;
			    foundCount++;
			    break;


                default:
                    break;
            }
        }

        offset = endSubSection;
    }
}

/**
 * Parses the message body
 *
 * 'offset' should point to the first byte of the T_INTEREST, T_CONTENTOBJECT, etc.
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return number The absolute byte offset of the next location to parse
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static size_t
_parseMessage(const uint8_t *packet, size_t offset, size_t endMessage, struct tlv_skeleton *skeleton)
{
    size_t endSection = endMessage;

    if (offset + sizeof(MetisTlvType) < endMessage) {
        const MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
        const uint16_t type = htons(tlv->type);
        const uint16_t v_length = htons(tlv->length);

        offset += sizeof(MetisTlvType);
        size_t endSubSection = offset + v_length;

        // make sure we don't have container overrun
        if (endSubSection <= endMessage) {
            switch (type) {
                case T_INTEREST:
                    _parseInterestV1(packet, offset, endSubSection, skeleton);
                    break;

                case T_MANIFEST:
                case T_OBJECT:
                    _parseObjectV1(packet, offset, endSubSection, skeleton);
                    break;

                case T_CPI:
                    // There is nothing nested here, its just the value
                    metisTlvSkeleton_SetCPI(skeleton, offset, v_length);
                    break;

                case T_HOPFRAG_PAYLOAD:
                    // There is nothing nested here, its just the value
                    metisTlvSkeleton_SetFragmentPayload(skeleton, offset, v_length);
                    break;

                default:
                    break;
            }

            endSection = endSubSection;
        }
    }
    return endSection;
}

static PARCCryptoHash *
_computeHash(const uint8_t *packet, size_t offset, size_t endMessage)
{
    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBytes(hasher, packet + offset, endMessage - offset);
    PARCCryptoHash *hash = parcCryptoHasher_Finalize(hasher);
    parcCryptoHasher_Release(&hasher);
    return hash;
}

// ==================
// TlvOps functions

static PARCBuffer *
_encodeControlPlaneInformation(const CCNxControl *cpiControlMessage)
{
    PARCJSON *json = ccnxControl_GetJson(cpiControlMessage);
    char *str = parcJSON_ToCompactString(json);

    // include +1 because we need the NULL byte
    size_t len = strlen(str) + 1;

    size_t packetLength = sizeof(_MetisTlvFixedHeaderV1) + sizeof(MetisTlvType) + len;
    PARCBuffer *packet = parcBuffer_Allocate(packetLength);

    _MetisTlvFixedHeaderV1 hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.version = 1;
    hdr.packetType = METIS_PACKET_TYPE_CONTROL;
    hdr.packetLength = htons(packetLength);
    hdr.headerLength = 8;

    parcBuffer_PutArray(packet, sizeof(hdr), (uint8_t *) &hdr);

    MetisTlvType tlv = { .type = htons(T_CPI), .length = htons(len) };
    parcBuffer_PutArray(packet, sizeof(tlv), (uint8_t *) &tlv);

    parcBuffer_PutArray(packet, len, (uint8_t *) str);

    parcMemory_Deallocate((void **) &str);
    return parcBuffer_Flip(packet);
}


static bool
_isPacketTypeInterest(const uint8_t *packet)
{
    _MetisTlvFixedHeaderV1 *hdr = (_MetisTlvFixedHeaderV1 *) packet;
    return (hdr->packetType == METIS_PACKET_TYPE_INTEREST);
}

static bool
_isPacketTypeInterestReturn(const uint8_t *packet)
{
    _MetisTlvFixedHeaderV1 *hdr = (_MetisTlvFixedHeaderV1 *) packet;
    return (hdr->packetType == METIS_PACKET_TYPE_INTERESTRETURN);
}

static bool
_isPacketTypeContentObject(const uint8_t *packet)
{
    _MetisTlvFixedHeaderV1 *hdr = (_MetisTlvFixedHeaderV1 *) packet;
    return (hdr->packetType == METIS_PACKET_TYPE_CONTENT);
}

static bool
_isPacketTypeControl(const uint8_t *packet)
{
    _MetisTlvFixedHeaderV1 *hdr = (_MetisTlvFixedHeaderV1 *) packet;
    return (hdr->packetType == METIS_PACKET_TYPE_CONTROL);
}

static bool
_isPacketTypeHopByHopFragment(const uint8_t *packet)
{
    _MetisTlvFixedHeaderV1 *hdr = (_MetisTlvFixedHeaderV1 *) packet;
    return (hdr->packetType == METIS_PACKET_TYPE_HOPFRAG);
}

static size_t
_fixedHeaderLength(const uint8_t *packet)
{
    return sizeof(_MetisTlvFixedHeaderV1);
}

static size_t
_totalHeaderLength(const uint8_t *packet)
{
    _MetisTlvFixedHeaderV1 *hdr = (_MetisTlvFixedHeaderV1 *) packet;
    return hdr->headerLength;
}

static size_t
_totalPacketLength(const uint8_t *packet)
{
    _MetisTlvFixedHeaderV1 *hdr = (_MetisTlvFixedHeaderV1 *) packet;
    return htons(hdr->packetLength);
}

static PARCCryptoHash *
_computeContentObjectHash(const uint8_t *packet)
{
    assertNotNull(packet, "Parameter packet must be non-null");
    _MetisTlvFixedHeaderV1 *hdr = (_MetisTlvFixedHeaderV1 *) packet;
    if (hdr->packetType == METIS_PACKET_TYPE_CONTENT) {
        const size_t endHeaders = _totalHeaderLength(packet);
        const size_t endPacket = _totalPacketLength(packet);
        return _computeHash(packet, endHeaders, endPacket);
    }

    return NULL;
}


static bool
_goodPacketType(uint8_t packetType)
{
    bool goodType = false;
    if (packetType == METIS_PACKET_TYPE_INTEREST || packetType == METIS_PACKET_TYPE_CONTENT ||
        packetType == METIS_PACKET_TYPE_CONTROL || packetType == METIS_PACKET_TYPE_INTERESTRETURN ||
        packetType == METIS_PACKET_TYPE_HOPFRAG) {
        goodType = true;
    }
    return goodType;
}

static bool
_parse(MetisTlvSkeleton *skeleton)
{
    bool success = false;

    _MetisTlvFixedHeaderV1 *hdr = (_MetisTlvFixedHeaderV1 *) metisTlvSkeleton_GetPacket(skeleton);

    // this function should only be called for version 1 packets
    trapUnexpectedStateIf(hdr->version != 1, "Version not 1");

    if (_goodPacketType(hdr->packetType) && hdr->headerLength >= sizeof(_MetisTlvFixedHeaderV1)) {
        size_t endHeaders = hdr->headerLength;
        size_t endPacket = htons(hdr->packetLength);

        if (endPacket >= endHeaders) {
            if (_isPacketTypeInterest((uint8_t *) hdr)) {
                metisTlvSkeleton_SetHopLimit(skeleton, 4, 1);
            }

            _parsePerHopV1(metisTlvSkeleton_GetPacket(skeleton), sizeof(_MetisTlvFixedHeaderV1), endHeaders, skeleton);
            size_t offset = _parseMessage(metisTlvSkeleton_GetPacket(skeleton), endHeaders, endPacket, skeleton);
            _parseValidationAlg(metisTlvSkeleton_GetPacket(skeleton), offset, endPacket, skeleton);
            success = true;
        }
    }

    return success;
}

const MetisTlvOps MetisTlvSchemaV1_Ops = {
    .parse                         = _parse,
    .computeContentObjectHash      = _computeContentObjectHash,
    .encodeControlPlaneInformation = _encodeControlPlaneInformation,
    .fixedHeaderLength             = _fixedHeaderLength,
    .totalHeaderLength             = _totalHeaderLength,
    .totalPacketLength             = _totalPacketLength,
    .isPacketTypeInterest          = _isPacketTypeInterest,
    .isPacketTypeContentObject     = _isPacketTypeContentObject,
    .isPacketTypeInterestReturn    = _isPacketTypeInterestReturn,
    .isPacketTypeHopByHopFragment  = _isPacketTypeHopByHopFragment,
    .isPacketTypeControl           = _isPacketTypeControl,
};
