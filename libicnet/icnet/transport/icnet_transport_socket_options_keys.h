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

#ifndef ICNET_TRANSPORT_OPTIONS_KEYS_H_
#define ICNET_TRANSPORT_OPTIONS_KEYS_H_

namespace icnet {

namespace transport {

typedef enum {
  RAAQM = 0, VEGAS = 1
} TransportProtocolAlgorithms;

typedef enum {
  INPUT_BUFFER_SIZE = 101,
  OUTPUT_BUFFER_SIZE = 102,
  NAME_PREFIX = 103,
  NAME_SUFFIX = 104,
  MAX_INTEREST_RETX = 105,
  DATA_PACKET_SIZE = 106,
  INTEREST_LIFETIME = 107,
  CONTENT_OBJECT_EXPIRY_TIME = 108,
  KEY_LOCATOR = 110,
  SIGNATURE_TYPE = 111,
  MIN_WINDOW_SIZE = 112,
  MAX_WINDOW_SIZE = 113,
  CURRENT_WINDOW_SIZE = 114,
  ASYNC_MODE = 115,
  MAKE_MANIFEST = 116,
  PORTAL = 117,
  RUNNING = 118,
} GeneralTransportOptions;

typedef enum {
  SAMPLE_NUMBER = 201,
  GAMMA_VALUE = 202,
  BETA_VALUE = 203,
  DROP_FACTOR = 204,
  MINIMUM_DROP_PROBABILITY = 205,
  PATH_ID = 206,
  RTT_STATS = 207,
} RaaqmTransportOptions;

typedef enum {
  RATE_ESTIMATION_ALPHA = 301,
  RATE_ESTIMATION_OBSERVER = 302,
  RATE_ESTIMATION_BATCH_PARAMETER = 303,
  RATE_ESTIMATION_CHOICE = 304,
} RateEstimationOptions;

typedef enum {
  INTEREST_OUTPUT = 401,
  INTEREST_RETRANSMISSION = 402,
  INTEREST_EXPIRED = 403,
  INTEREST_SATISFIED = 404,
  CONTENT_OBJECT_INPUT = 411,
  MANIFEST_INPUT = 412,
  CONTENT_OBJECT_TO_VERIFY = 413,
  CONTENT_RETRIEVED = 414,
} ConsumerCallbacksOptions;

typedef enum {
  INTEREST_INPUT = 501,
  INTEREST_DROP = 502,
  INTEREST_PASS = 503,
  CACHE_HIT = 506,
  CACHE_MISS = 508,
  NEW_CONTENT_OBJECT = 509,
  CONTENT_OBJECT_SIGN = 513,
  CONTENT_OBJECT_READY = 510,
  CONTENT_OBJECT_OUTPUT = 511,
} ProducerCallbacksOptions;

typedef enum {
  VIRTUAL_DOWNLOAD = 601, USE_CFG_FILE = 603,
} OtherOptions;

typedef enum {
  SHA_256 = 701, RSA_256 = 702,
} SignatureType;

} // end namespace transport

} // end namespace icnet

#endif // ICNET_TRANSPORT_OPTIONS_KEYS_H_
