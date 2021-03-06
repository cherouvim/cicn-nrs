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
 */
#include <config.h>

#include <LongBow/runtime.h>

#include <ccnx/common/ccnx_Interest.h>

#include <ccnx/common/internal/ccnx_InterestDefault.h>
#include <ccnx/common/ccnx_InterestPayloadId.h>
#include <ccnx/common/ccnx_WireFormatMessage.h>

#include <parc/algol/parc_Memory.h>

#include <parc/algol/parc_DisplayIndented.h>

#include <stdio.h>

//by wschoi

//#define LOG_CHECK
static const CCNxInterestInterface *_defaultImplementation = &CCNxInterestFacadeV1_Implementation;

CCNxInterest *
ccnxInterest_Create(const CCNxName *name,
                    uint32_t lifetime,
                    const PARCBuffer *keyId,
                    const PARCBuffer *contentObjectHash)
{
    return ccnxInterest_CreateWithImpl(_defaultImplementation,
                                       name,
                                       lifetime,
                                       keyId,
                                       contentObjectHash,
                                       CCNxInterestDefault_HopLimit);
}

CCNxInterest *
ccnxInterest_CreateWithImpl(const CCNxInterestInterface *impl,
                            const CCNxName *name,
                            const uint32_t interestLifetime,
                            const PARCBuffer *keyId,
                            const PARCBuffer *contentObjectHash,
                            const uint32_t hopLimit)
{
    CCNxInterest *result = NULL;

    if (impl->create != NULL) {
        result = impl->create(name, interestLifetime, keyId, contentObjectHash, hopLimit);


        // And set the dictionary's interface pointer to the one we just used to create this.
        ccnxTlvDictionary_SetMessageInterface(result, impl);
    } else {
        trapNotImplemented("Interest implementations must implement create()");
    }
    return result;
}


CCNxInterest *
ccnxInterest_CreateSimple(const CCNxName *name)
{

    return ccnxInterest_Create(name,
                               CCNxInterestDefault_LifetimeMilliseconds,
                               NULL,
                               NULL);
}

void
ccnxInterest_AssertValid(const CCNxInterest *interest)
{
    assertNotNull(interest, "Must be a non-null pointer to a CCNxInterest.");

    // Check for required fields in the underlying dictionary. Case 1036
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
    assertNotNull(impl, "Interest must have an valid implementation pointer.");

    if (impl->assertValid != NULL) {
        impl->assertValid(interest);
    }
}

CCNxInterest *
ccnxInterest_Acquire(const CCNxInterest *instance)
{
    return ccnxTlvDictionary_Acquire(instance);
}

void
ccnxInterest_Release(CCNxInterest **instanceP)
{
    ccnxTlvDictionary_Release(instanceP);
}

bool
ccnxInterest_Equals(const CCNxInterest *a, const CCNxInterest *b)
{
    if (a == b) {
        return true;
    }
    if (a == NULL || b == NULL) {
        return false;
    }

    CCNxInterestInterface *implA = ccnxInterestInterface_GetInterface(a);
    CCNxInterestInterface *implB = ccnxInterestInterface_GetInterface(b);

    if (implA != implB) {
        return false;
    }

    CCNxName *nameA = implA->getName(a);
    CCNxName *nameB = implB->getName(b);

    PARCBuffer *keyA = implA->getKeyIdRestriction(a);
    PARCBuffer *keyB = implB->getKeyIdRestriction(b);

    uint64_t lifetimeA = implA->getLifetime(a);
    uint64_t lifetimeB = implB->getLifetime(b);

    if (ccnxName_Equals(nameA, nameB)) {
        if (parcBuffer_Equals(keyA, keyB)) {
            // Must compare the excludes.
            if (lifetimeA == lifetimeB) {
                return true;
            }
        }
    }

    return false;
}

CCNxName *
ccnxInterest_GetName(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
	
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    CCNxName *result = NULL;

    if (impl->getName != NULL) {
        result = impl->getName(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetName");
    }
    return result;
}

//by wschoi

CCNxName *
ccnxInterest_GetKeyName(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
	
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    CCNxName *result = NULL;

    if (impl->getName != NULL) {
        result = impl->getKeyName(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetKeyName");
    }
    return result;
}


bool
ccnxInterest_SetContentObjectHashRestriction(CCNxInterest *interest, const PARCBuffer *contentObjectHash)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    bool result = false;

    if (impl->setContentObjectHashRestriction != NULL) {
        result = impl->setContentObjectHashRestriction(interest, contentObjectHash);
    } else {
        trapNotImplemented("ccnxInterest_SetContentObjectHashRestriction");
    }
    return result;
}

PARCBuffer *
ccnxInterest_GetContentObjectHashRestriction(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    PARCBuffer *result = NULL;

    if (impl->getContentObjectHashRestriction != NULL) {
        result = impl->getContentObjectHashRestriction(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetContentObjectHash");
    }
    return result;
}

bool
ccnxInterest_SetKeyIdRestriction(CCNxInterest *interest, const PARCBuffer *keyId)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    bool result = false;

    if (impl->setKeyIdRestriction != NULL) {
        result = impl->setKeyIdRestriction(interest, keyId);
    } else {
        trapNotImplemented("ccnxInterest_SetKeyIdRestriction");
    }
    return result;
}

PARCBuffer *
ccnxInterest_GetKeyIdRestriction(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    PARCBuffer *result = NULL;

    if (impl->getKeyIdRestriction != NULL) {
        result = impl->getKeyIdRestriction(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetKeyIdRestriction");
    }
    return result;
}

bool
ccnxInterest_SetLifetime(CCNxInterest *interest, uint32_t lifetime)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    bool result = false;

    if (impl->setLifetime != NULL) {
        result = impl->setLifetime(interest, lifetime);
    } else {
        trapNotImplemented("ccnxInterest_SetLifetime");
    }
    return result;
}

uint32_t
ccnxInterest_GetLifetime(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    uint32_t result = 0;

    if (impl->getLifetime != NULL) {
        result = impl->getLifetime(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetLifetime");
    }


    return result;
}
//by wschoi

#if 0
	bool
ccnxInterest_SetPayload_lookup(CCNxInterest *interest, const PARCBuffer *payload)
{
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
	bool result = false;
	if (impl->setPayload != NULL) {
		result = impl->setPayload_lookup(interest, payload);
		printf("##########ccnxInterest_SetPayload_lookup\n\n");
	}else {
		trapNotImplemented("ccnxInterest_SetPayload");
	}
	return result;
}
#endif



//by wschoi
//refresh
#if 0
bool
ccnxInterest_SetPayload_refresh_key(CCNxInterest *interest, const PARCBuffer *payload)
{
printf("ccnxInterest_SetPayload_refresh_key() start\n");
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
	bool result = false;
	if (impl->setPayload_refresh_key != NULL) {
		result = impl->setPayload_refresh_key(interest, payload);
	}else {
		trapNotImplemented("ccnxInterest_SetPayload_refresh_key");
	}

printf("ccnxInterest_SetPayload_refresh_key() end\n");
	return result;
}

bool
ccnxInterest_SetPayload_refresh_value(CCNxInterest *interest, const PARCBuffer *payload)
{
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
	bool result = false;
	if (impl->setPayload_refresh_value != NULL) {
		result = impl->setPayload_refresh_value(interest, payload);
	}else {
		trapNotImplemented("cccnxInterest_SetPayload_refresh_value");
	}
	return result;
}


#endif



//by wschoi
//dereg
#if 1
bool
ccnxInterest_SetPayload_dereg_key(CCNxInterest *interest, const PARCBuffer *payload)
{
#ifdef LOG_CHECK
	printf("ccnxInterest_SetPayload_dereg_key() start\n");
#endif
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
	bool result = false;
	if (impl->setPayload_dereg_key != NULL) {
		result = impl->setPayload_dereg_key(interest, payload);
	}else {
		trapNotImplemented("ccnxInterest_SetPayload_dereg_key");
	}
#ifdef LOG_CHECK
	printf("ccnxInterest_SetPayload_dereg_key() end\n");
#endif
	return result;
}

bool
ccnxInterest_SetPayload_dereg_value(CCNxInterest *interest, const PARCBuffer *payload)
{
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
	bool result = false;
	if (impl->setPayload_dereg_value != NULL) {
		result = impl->setPayload_dereg_value(interest, payload);
	}else {
		trapNotImplemented("cccnxInterest_SetPayload_dereg_value");
	}
	return result;
}


#endif




//by wschoi
//del
#if 1
bool
ccnxInterest_SetPayload_del_key(CCNxInterest *interest, const PARCBuffer *payload)
{
#ifdef LOG_CHECK
	printf("ccnxInterest_SetPayload_del_key() start\n");
#endif
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
	bool result = false;
	if (impl->setPayload_del_key != NULL) {
		result = impl->setPayload_del_key(interest, payload);
	}else {
		trapNotImplemented("ccnxInterest_SetPayload_del_key");
	}
#ifdef LOG_CHECK
	printf("ccnxInterest_SetPayload_del_key() end\n");
#endif
	return result;
}

bool
ccnxInterest_SetPayload_del_value(CCNxInterest *interest, const PARCBuffer *payload)
{
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
	bool result = false;
	if (impl->setPayload_del_value != NULL) {
		result = impl->setPayload_del_value(interest, payload);
	}else {
		trapNotImplemented("cccnxInterest_SetPayload_del_value");
	}
	return result;
}


#endif



//by wschoi
//add
#if 1
bool
ccnxInterest_SetPayload_add_key(CCNxInterest *interest, const PARCBuffer *payload)
{
#ifdef LOG_CHECK
	printf("ccnxInterest_SetPayload_add_key() start\n");
#endif
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
	bool result = false;
	if (impl->setPayload_add_key != NULL) {
		result = impl->setPayload_add_key(interest, payload);
	}else {
		trapNotImplemented("ccnxInterest_SetPayload_add_key");
	}
#ifdef LOG_CHECK
	printf("ccnxInterest_SetPayload_add_key() end\n");
#endif
	return result;
}

bool
ccnxInterest_SetPayload_add_value(CCNxInterest *interest, const PARCBuffer *payload)
{
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
	bool result = false;
	if (impl->setPayload_add_value != NULL) {
		result = impl->setPayload_add_value(interest, payload);
	}else {
		trapNotImplemented("cccnxInterest_SetPayload_add_value");
	}
	return result;
}

#endif



//by wschoi
//registration
#if 1
bool
ccnxInterest_SetPayload_reg_key(CCNxInterest *interest, const PARCBuffer *payload)
{
#ifdef LOG_CHECK
	printf("ccnxInterest_SetPayload_reg_key() start\n");
#endif
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
	bool result = false;
	if (impl->setPayload_reg_key != NULL) {
		result = impl->setPayload_reg_key(interest, payload);
	}else {
		trapNotImplemented("ccnxInterest_SetPayload_reg_key");
	}
#ifdef LOG_CHECK
	printf("ccnxInterest_SetPayload_reg_key() end\n");
#endif
	return result;
}

bool
ccnxInterest_SetPayload_reg_value(CCNxInterest *interest, const PARCBuffer *payload)
{
#ifdef LOG_CHECK
	printf("ccnxInterest_SetPayload_reg_value() start\n");
#endif
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
	bool result = false;
	if (impl->setPayload_reg_value != NULL) {
		result = impl->setPayload_reg_value(interest, payload);
	}else {
		trapNotImplemented("cccnxInterest_SetPayload_reg_value");
	}
printf("ccnxInterest_SetPayload_reg_value() end\n");
	return result;
}


#endif

//by wschoi
#if 1
bool
ccnxInterest_SetPayload_lookup(CCNxInterest *interest, const PARCBuffer *payload)
{
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
	bool result = false;
	if (impl->setPayload_lookup != NULL) {
		result = impl->setPayload_lookup(interest, payload);
	}else {
		trapNotImplemented("ccnxInterest_SetPayload_lookup");
	}
	return result;
}

#endif

bool
ccnxInterest_SetPayload(CCNxInterest *interest, const PARCBuffer *payload)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    bool result = false;

    if (impl->setPayload != NULL) {
        result = impl->setPayload(interest, payload);
    } else {
        trapNotImplemented("ccnxInterest_SetPayload");
    }
    return result;
}

bool
ccnxInterest_SetPayloadAndId(CCNxInterest *interest, const PARCBuffer *payload)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    bool result = false;

    if (impl->setPayloadAndId != NULL) {
        result = impl->setPayloadAndId(interest, payload);
    } else {
        trapNotImplemented("ccnxInterest_SetPayloadAndId");
    }
    return result;
}

bool
ccnxInterest_SetPayloadWithId(CCNxInterest *interest, const PARCBuffer *payload, const CCNxInterestPayloadId *payloadId)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    bool result = false;

    if (impl->setPayloadWithId != NULL) {
        result = impl->setPayloadWithId(interest, payload, payloadId);
    } else {
        trapNotImplemented("ccnxInterest_SetPayloadWithId");
    }
    return result;
}
//by wschoi

	bool
ccnxInterest_SetPayloadWithId_lookup(CCNxInterest *interest, const PARCBuffer *payload, const CCNxInterestPayloadId *payloadId)
{
#ifdef LOG_CHECK
	printf("##########ccnxInterest_SetPayloadWithId_lookup\n\n");
#endif
	ccnxInterest_OptionalAssertValid(interest);
	CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

	bool result = false;

	if (impl->setPayloadWithId != NULL) {
		result = impl->setPayloadWithId_lookup(interest, payload, payloadId);
#ifdef LOG_CHECK
		printf("##########ccnxInterest_SetPayloadWithId_lookup\n\n");
#endif
	} else {
		trapNotImplemented("ccnxInterest_SetPayloadWithId");
	}
	return result;
}




//by wschoi
//LOOKUP TYPE
PARCBuffer *
ccnxInterest_GetPayload_lookup(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    PARCBuffer *result = NULL;

    if (impl->getPayload != NULL) {
        result = impl->getPayload_lookup(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload_lookup");
    }
    return result;
}

PARCBuffer *
ccnxInterest_GetPayload_KeyName(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);


    PARCBuffer *result = NULL;

    if (impl->getPayload != NULL) {
        result = impl->getPayload_KeyName(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload_KeyName");
    }
    return result;
}

//REGISTRATION TYPE
//registration
PARCBuffer *
ccnxInterest_GetPayload_reg_key(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);


    PARCBuffer *result = NULL;

    if (impl->getPayload_reg_key != NULL) {
        result = impl->getPayload_reg_key(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload_reg_key");
    }
    return result;
}

PARCBuffer *
ccnxInterest_GetPayload_reg_value(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);


    PARCBuffer *result = NULL;

    if (impl->getPayload_reg_value != NULL) {
        result = impl->getPayload_reg_value(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload_reg_value");
    }
    return result;
}



//add
PARCBuffer *
ccnxInterest_GetPayload_add_key(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);


    PARCBuffer *result = NULL;

    if (impl->getPayload_add_key != NULL) {
        result = impl->getPayload_add_key(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload_add_key");
    }
    return result;
}

PARCBuffer *
ccnxInterest_GetPayload_add_value(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);


    PARCBuffer *result = NULL;

    if (impl->getPayload_add_value != NULL) {
        result = impl->getPayload_add_value(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload_add_key");
    }
    return result;
}


//delete
PARCBuffer *
ccnxInterest_GetPayload_del_key(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);


    PARCBuffer *result = NULL;

    if (impl->getPayload_del_key != NULL) {
        result = impl->getPayload_del_key(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload_del_key");
    }
    return result;
}

PARCBuffer *
ccnxInterest_GetPayload_del_value(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);


    PARCBuffer *result = NULL;

    if (impl->getPayload_del_value != NULL) {
        result = impl->getPayload_del_value(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload_del_value");
    }
    return result;
}


//dereg
PARCBuffer *
ccnxInterest_GetPayload_dereg_key(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);


    PARCBuffer *result = NULL;

    if (impl->getPayload_dereg_key != NULL) {
        result = impl->getPayload_dereg_key(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload_dereg_key");
    }
    return result;
}

PARCBuffer *
ccnxInterest_GetPayload_dereg_value(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);


    PARCBuffer *result = NULL;

    if (impl->getPayload_dereg_value != NULL) {
        result = impl->getPayload_dereg_value(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload_dereg_value");
    }
    return result;
}


//refresh
#if 0
PARCBuffer *
ccnxInterest_GetPayload_refresh_key(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);


    PARCBuffer *result = NULL;

    if (impl->getPayload_refresh_key != NULL) {
        result = impl->getPayload_refresh_key(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload_refresh_key");
    }
    return result;
}

PARCBuffer *
ccnxInterest_GetPayload_refresh_value(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);


    PARCBuffer *result = NULL;

    if (impl->getPayload_refresh_value != NULL) {
        result = impl->getPayload_refresh_value(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload_refresh_value");
    }
    return result;
}
#endif



PARCBuffer *
ccnxInterest_GetPayload(const CCNxInterest *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    PARCBuffer *result = NULL;

    if (impl->getPayload != NULL) {
        result = impl->getPayload(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetPayload");
    }
    return result;
}

bool
ccnxInterest_SetHopLimit(CCNxTlvDictionary *interest, uint32_t hopLimit)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    bool result = false;

    if (impl->setHopLimit != NULL) {
        result = impl->setHopLimit(interest, hopLimit);
    } else {
        trapNotImplemented("ccnxInterest_GetSetHopLimit");
    }
    // Make sure any attached wire format buffers are in sync with the dictionary
    ccnxWireFormatMessage_SetHopLimit(interest, hopLimit);
    return result;
}

uint32_t
ccnxInterest_GetHopLimit(const CCNxTlvDictionary *interest)
{
    ccnxInterest_OptionalAssertValid(interest);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    uint32_t result = 0;

    if (impl->getHopLimit != NULL) {
        result = impl->getHopLimit(interest);
    } else {
        trapNotImplemented("ccnxInterest_GetHopLimit");
    }
    return result;
}

void
ccnxInterest_Display(const CCNxInterest *interest, int indentation)
{
    ccnxInterest_OptionalAssertValid(interest);

    parcDisplayIndented_PrintLine(indentation, "CCNxInterest@%p {\n", interest);
    ccnxName_Display(ccnxInterest_GetName(interest), indentation + 1);

    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    if (impl->display != NULL) {
        impl->display(interest, 1);
    }

    parcDisplayIndented_PrintLine(indentation, "}\n");
}

char *
ccnxInterest_ToString(const CCNxInterest *interest)
{
    char *result = NULL;

    ccnxInterest_OptionalAssertValid(interest);

    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);

    if (impl->toString != NULL) {
        result = impl->toString(interest);
    } else {
        char *name = ccnxName_ToString(ccnxInterest_GetName(interest));
        uint32_t lifetime = ccnxInterest_GetLifetime(interest);

        char *string;
        int failure = asprintf(&string, "CCNxInterest{.name=\"%s\" .lifetime=%dms}", name, lifetime);
        assertTrue(failure > 0, "Error from asprintf");

        parcMemory_Deallocate((void **) &name);

        result = parcMemory_StringDuplicate(string, strlen(string));

        free(string);
    }

    return result;
}
