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

#include <stdio.h>
#include <string.h>

#include <LongBow/runtime.h>

#include <parc/security/parc_SigningAlgorithm.h>

static struct {
    PARCSigningAlgorithm alg;
    char *name;
} _signingAlgorithm_ToString[] = {
    { PARCSigningAlgortihm_NULL, "PARCSigningAlgortihm_NULL" },
    { PARCSigningAlgorithm_RSA,  "PARCSigningAlgorithm_RSA"  },
    { PARCSigningAlgorithm_DSA,  "PARCSigningAlgorithm_DSA"  },
    { PARCSigningAlgorithm_HMAC, "PARCSigningAlgorithm_HMAC" },
    { 0,                         NULL                        }
};

const char *
parcSigningAlgorithm_ToString(PARCSigningAlgorithm alg)
{
    for (int i = 0; _signingAlgorithm_ToString[i].name != NULL; i++) {
        if (_signingAlgorithm_ToString[i].alg == alg) {
            return _signingAlgorithm_ToString[i].name;
        }
    }
    return NULL;
}

PARCSigningAlgorithm
parcSigningAlgorithm_FromString(const char *name)
{
    for (int i = 0; _signingAlgorithm_ToString[i].name != NULL; i++) {
        if (strcmp(_signingAlgorithm_ToString[i].name, name) == 0) {
            return _signingAlgorithm_ToString[i].alg;
        }
    }
    return PARCSigningAlgorithm_UNKNOWN;
}

PARCSigningAlgorithm
parcSigningAlgorithm_GetSigningAlgorithm(PARCCryptoSuite suite)
{
    switch (suite) {
        case PARCCryptoSuite_DSA_SHA256:
            return PARCSigningAlgorithm_DSA;

        case PARCCryptoSuite_RSA_SHA256:      // fallthrough
        case PARCCryptoSuite_RSA_SHA512:
            return PARCSigningAlgorithm_RSA;

        case PARCCryptoSuite_HMAC_SHA256:     // fallthrough
        case PARCCryptoSuite_HMAC_SHA512:
            return PARCSigningAlgorithm_HMAC;

        case PARCCryptoSuite_NULL_CRC32C:
            return PARCSigningAlgortihm_NULL;

        default:
            trapIllegalValue(suit, "Unknown crypto suite: %d", suite);
    }
}
