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

/**
 * @brief A structure of functions representing an Interest implementation.
 *
 * The underlying implementation should support the CCNxInterest API.
 *
 */

#ifndef CCNx_Common_ccnx_internal_InterestInterface_h
#define CCNx_Common_ccnx_internal_InterestInterface_h

#include <ccnx/common/internal/ccnx_TlvDictionary.h>

#include <ccnx/common/internal/ccnx_InterestPayloadIdMethod.h>
#include <ccnx/common/ccnx_InterestPayloadId.h>
//by wschoi
#include <ccnx/common/ccnx_PayloadType.h>

typedef struct ccnx_interest_interface {
    /** A human-readable label for this implementation */
    char                 *description;

    /** @see ccnxInterest_Create */
    CCNxTlvDictionary *(*create)(const CCNxName * name,                  // required
                                 const uint32_t lifetimeMilliseconds,    // may use DefaultLimetimeMilliseconds
                                 const PARCBuffer * keyId,               // may be NULL
                                 const PARCBuffer * contentObjectHash,   // may be NULL
                                 const uint32_t hopLimit);

    /** @see ccnxInterest_CreateSimple */
    CCNxTlvDictionary *(*createSimple)(const CCNxName * name);

    /** @see ccnxInterest_GetName */
    CCNxName          *(*getName)(const CCNxTlvDictionary * dict);
    
	//by wschoi
	CCNxName          *(*getKeyName)(const CCNxTlvDictionary * dict);

    /** @see ccnxInterest_SetLifetime */
    bool (*setLifetime)(CCNxTlvDictionary *dict, uint32_t lifetime);

    /** @see ccnxInterest_GetLifetime */
    uint32_t (*getLifetime)(const CCNxTlvDictionary *dict);

    /** @see ccnxInterest_SetHopLimit */
    bool (*setHopLimit)(CCNxTlvDictionary *dict, uint32_t hopLimit);

    /** @see ccnxInterest_GetHopLimit */
    uint32_t (*getHopLimit)(const CCNxTlvDictionary *dict);

    /** @see ccnxInterest_SetKeyIdRestriction */
    bool (*setKeyIdRestriction)(CCNxTlvDictionary *dict, const PARCBuffer *keyId);

    /** @see ccnxInterest_GetKeyIdRestriction */
    PARCBuffer        *(*getKeyIdRestriction)(const CCNxTlvDictionary * dict);

    /** @see ccnxInterest_SetPayload */
    bool (*setPayload)(CCNxTlvDictionary *dict, const PARCBuffer *payload);
//by wschoi
	bool (*setPayload_lookup)(CCNxTlvDictionary *dict, const PARCBuffer *payload);


	//by wschoi
	//REGISTRATION
#if 0
		//refresh
	bool (*setPayload_refresh_key)(CCNxTlvDictionary *dict, const PARCBuffer *payload);
	bool (*setPayload_refresh_value)(CCNxTlvDictionary *dict, const PARCBuffer *payload);

#endif
		//dereg
	bool (*setPayload_dereg_key)(CCNxTlvDictionary *dict, const PARCBuffer *payload);
	bool (*setPayload_dereg_value)(CCNxTlvDictionary *dict, const PARCBuffer *payload);

		//del
	bool (*setPayload_del_key)(CCNxTlvDictionary *dict, const PARCBuffer *payload);
	bool (*setPayload_del_value)(CCNxTlvDictionary *dict, const PARCBuffer *payload);

		//add
	bool (*setPayload_add_key)(CCNxTlvDictionary *dict, const PARCBuffer *payload);
	bool (*setPayload_add_value)(CCNxTlvDictionary *dict, const PARCBuffer *payload);

		//registration
	bool (*setPayload_reg_key)(CCNxTlvDictionary *dict, const PARCBuffer *payload);
	bool (*setPayload_reg_value)(CCNxTlvDictionary *dict, const PARCBuffer *payload);


    /** @see ccnxInterest_SetPayloadAndId */
    bool (*setPayloadAndId)(CCNxTlvDictionary *dict, const PARCBuffer *payload);

    /** @see ccnxInterest_SetPayloadWithId */
    bool (*setPayloadWithId)(CCNxTlvDictionary *dict, const PARCBuffer *payload, const CCNxInterestPayloadId *id);

	//by wschoi
    bool (*setPayloadWithId_lookup)(CCNxTlvDictionary *dict, const PARCBuffer *payload, const CCNxInterestPayloadId *id);

	//by wschoi
    /** @see ccnxInterest_GetPayload */
    PARCBuffer        *(*getPayload)(const CCNxTlvDictionary * dict);

	//LOOKUP TYPE
    PARCBuffer        *(*getPayload_lookup)(const CCNxTlvDictionary * dict);
    PARCBuffer        *(*getPayload_KeyName)(const CCNxTlvDictionary * dict);

//by wschoi
	//REGISTRATION TYPE
	//registration
    PARCBuffer        *(*getPayload_reg_key)(const CCNxTlvDictionary * dict);
    PARCBuffer        *(*getPayload_reg_value)(const CCNxTlvDictionary * dict);

	//add
    PARCBuffer        *(*getPayload_add_key)(const CCNxTlvDictionary * dict);
    PARCBuffer        *(*getPayload_add_value)(const CCNxTlvDictionary * dict);

	//del
    PARCBuffer        *(*getPayload_del_key)(const CCNxTlvDictionary * dict);
    PARCBuffer        *(*getPayload_del_value)(const CCNxTlvDictionary * dict);
	
	//dereg
    PARCBuffer        *(*getPayload_dereg_key)(const CCNxTlvDictionary * dict);
    PARCBuffer        *(*getPayload_dereg_value)(const CCNxTlvDictionary * dict);

#if 0
	//refresh
    PARCBuffer        *(*getPayload_refresh_key)(const CCNxTlvDictionary * dict);
    PARCBuffer        *(*getPayload_refresh_value)(const CCNxTlvDictionary * dict);
#endif

    /** @see ccnxInterest_SetContentObjectHashRestriction */
    bool (*setContentObjectHashRestriction)(CCNxTlvDictionary *dict, const PARCBuffer *contentObjectHash);

    /** @see ccnxInterest_GetContentObjectHashRestriction */
    PARCBuffer        *(*getContentObjectHashRestriction)(const CCNxTlvDictionary * dict);

    /** @see ccnxInterest_Equals */
    bool (*equals)(const CCNxTlvDictionary *objectA, const CCNxTlvDictionary *objectB);
    /** @see ccnxInterest_AssertValid */
    void (*assertValid)(const CCNxTlvDictionary *dict);

    /** @see ccnxInterest_ToString */
    char              *(*toString)(const CCNxTlvDictionary * dict);
    /** @see ccnxInterest_Display */
    void (*display)(const CCNxTlvDictionary *interestDictionary, size_t indentation);
} CCNxInterestInterface;


/**
 * The SchemaV1 Interest implementaton
 */
extern CCNxInterestInterface CCNxInterestFacadeV1_Implementation;

/**
 * Given a CCNxTlvDictionary representing a CCNxInterest, return the address of the CCNxInterestInterface
 * instance that should be used to access the Interest. This will also update the CCNxTlvDictionary's interface
 * pointer for future references.
 *
 * @param interestDictionary - a {@link CCNxTlvDictionary} representing a CCNxInterest.
 * @return the address of the `CCNxContentObjectInterface` instance that should be used to access the CCNxInterest.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/boose/roo/pie");
 *
 *     CCNxInterest *interestV1 =
 *          ccnxInterest_CreateWithImpl(&CCNxInterestFacadeV1_Implementation,
 *                                      name,
 *                                      CCNxInterestDefault_LifetimeMilliseconds,
 *                                      NULL,
 *                                      NULL,
 *                                      CCNxInterestDefault_HopLimit);
 *
 *     assertTrue(ccnxInterestInterface_GetInterface(interestV1) == &CCNxInterestFacadeV1_Implementation,
 *                "Expected V1 Implementation");
 *
 *     ccnxName_Release(&name);
 *     ccnxInterest_Release(&interestV1);
 * } * @endcode
 */
CCNxInterestInterface *ccnxInterestInterface_GetInterface(const CCNxTlvDictionary *interestDictionary);
#endif
