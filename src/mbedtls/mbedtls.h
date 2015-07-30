/*
 * MbedTLS Source Code Library Header
 */

#include "me.h"
#include "osdep.h"


#if ME_COM_MBEDTLS

#define EMBEDTHIS 1





/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/config.h ************/


/**
 * \file config.h
 *
 * \brief Configuration options (set of defines)
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * This set of compile-time options may be used to enable
 * or disable features selectively, and reduce the global
 * memory footprint.
 */
#ifndef POLARSSL_CONFIG_H
#define POLARSSL_CONFIG_H

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

/**
 * \name SECTION: System support
 *
 * This section sets system specific settings.
 * \{
 */

/**
 * \def POLARSSL_HAVE_INT8
 *
 * The system uses 8-bit wide native integers.
 *
 * \deprecated The compiler should be able to generate code for 32-bit
 * arithmetic (required by C89). This code is likely to be at least as
 * efficient as ours.
 *
 * Uncomment if native integers are 8-bit wide.
 */
//#define POLARSSL_HAVE_INT8

/**
 * \def POLARSSL_HAVE_INT16
 *
 * The system uses 16-bit wide native integers.
 *
 * \deprecated The compiler should be able to generate code for 32-bit
 * arithmetic (required by C89). This code is likely to be at least as
 * efficient as ours.
 *
 * Uncomment if native integers are 16-bit wide.
 */
//#define POLARSSL_HAVE_INT16

/**
 * \def POLARSSL_HAVE_LONGLONG
 *
 * The compiler supports the 'long long' type.
 * (Only used on 32-bit platforms)
 */
#define POLARSSL_HAVE_LONGLONG

/**
 * \def POLARSSL_HAVE_ASM
 *
 * The compiler has support for asm().
 *
 * Requires support for asm() in compiler.
 *
 * Used in:
 *      library/timing.c
 *      library/padlock.c
 *      include/polarssl/bn_mul.h
 *
 * Comment to disable the use of assembly code.
 */
#define POLARSSL_HAVE_ASM

/**
 * \def POLARSSL_HAVE_SSE2
 *
 * CPU supports SSE2 instruction set.
 *
 * Uncomment if the CPU supports SSE2 (IA-32 specific).
 */
//#define POLARSSL_HAVE_SSE2

/**
 * \def POLARSSL_HAVE_TIME
 *
 * System has time.h and time() / localtime()  / gettimeofday().
 *
 * Comment if your system does not support time functions
 */
#define POLARSSL_HAVE_TIME

/**
 * \def POLARSSL_HAVE_IPV6
 *
 * System supports the basic socket interface for IPv6 (RFC 3493),
 * specifically getaddrinfo(), freeaddrinfo() and struct sockaddr_storage.
 *
 * Note: on Windows/MingW, XP or higher is required.
 *
 * \warning As of 1.3.11, *not* using this flag when POLARSSL_NET_C is
 * defined, is deprecated. The alternative legacy code will be removed in 2.0.
 *
 * Comment if your system does not support the IPv6 socket interface
 */
#define POLARSSL_HAVE_IPV6

/**
 * \def POLARSSL_PLATFORM_MEMORY
 *
 * Enable the memory allocation layer.
 *
 * By default mbed TLS uses the system-provided malloc() and free().
 * This allows different allocators (self-implemented or provided) to be
 * provided to the platform abstraction layer.
 *
 * Enabling POLARSSL_PLATFORM_MEMORY without the
 * POLARSSL_PLATFORM_{FREE,MALLOC}_MACROs will provide
 * "platform_set_malloc_free()" allowing you to set an alternative malloc() and
 * free() function pointer at runtime.
 *
 * Enabling POLARSSL_PLATFORM_MEMORY and specifying
 * POLARSSL_PLATFORM_{MALLOC,FREE}_MACROs will allow you to specify the
 * alternate function at compile time.
 *
 * Requires: POLARSSL_PLATFORM_C
 *
 * Enable this layer to allow use of alternative memory allocators.
 */
//#define POLARSSL_PLATFORM_MEMORY

/**
 * \def POLARSSL_PLATFORM_NO_STD_FUNCTIONS
 *
 * Do not assign standard functions in the platform layer (e.g. malloc() to
 * POLARSSL_PLATFORM_STD_MALLOC and printf() to POLARSSL_PLATFORM_STD_PRINTF)
 *
 * This makes sure there are no linking errors on platforms that do not support
 * these functions. You will HAVE to provide alternatives, either at runtime
 * via the platform_set_xxx() functions or at compile time by setting
 * the POLARSSL_PLATFORM_STD_XXX defines, or enabling a
 * POLARSSL_PLATFORM_XXX_MACRO.
 *
 * Requires: POLARSSL_PLATFORM_C
 *
 * Uncomment to prevent default assignment of standard functions in the
 * platform layer.
 */
//#define POLARSSL_PLATFORM_NO_STD_FUNCTIONS

/**
 * \def POLARSSL_PLATFORM_XXX_ALT
 *
 * Uncomment a macro to let mbed TLS support the function in the platform
 * abstraction layer.
 *
 * Example: In case you uncomment POLARSSL_PLATFORM_PRINTF_ALT, mbed TLS will
 * provide a function "platform_set_printf()" that allows you to set an
 * alternative printf function pointer.
 *
 * All these define require POLARSSL_PLATFORM_C to be defined!
 *
 * WARNING: POLARSSL_PLATFORM_SNPRINTF_ALT is not available on Windows
 * for compatibility reasons.
 *
 * WARNING: POLARSSL_PLATFORM_XXX_ALT cannot be defined at the same time as
 * POLARSSL_PLATFORM_XXX_MACRO!
 *
 * Uncomment a macro to enable alternate implementation of specific base
 * platform function
 */
//#define POLARSSL_PLATFORM_EXIT_ALT
//#define POLARSSL_PLATFORM_FPRINTF_ALT
//#define POLARSSL_PLATFORM_PRINTF_ALT
//#define POLARSSL_PLATFORM_SNPRINTF_ALT

/**
 * \def POLARSSL_DEPRECATED_WARNING
 *
 * Mark deprecated functions so that they generate a warning if used.
 * Functions deprecated in one version will usually be removed in the next
 * version. You can enable this to help you prepare the transition to a new
 * major version by making sure your code is not using these functions.
 *
 * This only works with GCC and Clang. With other compilers, you may want to
 * use POLARSSL_DEPRECATED_REMOVED
 *
 * Uncomment to get warnings on using deprecated functions.
 */
//#define POLARSSL_DEPRECATED_WARNING

/**
 * \def POLARSSL_DEPRECATED_REMOVED
 *
 * Remove deprecated functions so that they generate an error if used.
 * Functions deprecated in one version will usually be removed in the next
 * version. You can enable this to help you prepare the transition to a new
 * major version by making sure your code is not using these functions.
 *
 * Uncomment to get errors on using deprecated functions.
 */
//#define POLARSSL_DEPRECATED_REMOVED

/* \} name SECTION: System support */

/**
 * \name SECTION: mbed TLS feature support
 *
 * This section sets support for features that are or are not needed
 * within the modules that are enabled.
 * \{
 */

/**
 * \def POLARSSL_TIMING_ALT
 *
 * Uncomment to provide your own alternate implementation for hardclock(),
 * get_timer(), set_alarm() and m_sleep().
 *
 * Only works if you have POLARSSL_TIMING_C enabled.
 *
 * You will need to provide a header "timing_alt.h" and an implementation at
 * compile time.
 */
//#define POLARSSL_TIMING_ALT

/**
 * \def POLARSSL_XXX_ALT
 *
 * Uncomment a macro to let mbed TLS use your alternate core implementation of
 * a symmetric or hash algorithm (e.g. platform specific assembly optimized
 * implementations). Keep in mind that the function prototypes should remain
 * the same.
 *
 * Example: In case you uncomment POLARSSL_AES_ALT, mbed TLS will no longer
 * provide the "struct aes_context" definition and omit the base function
 * declarations and implementations. "aes_alt.h" will be included from
 * "aes.h" to include the new function definitions.
 *
 * Uncomment a macro to enable alternate implementation for core algorithm
 * functions
 */
//#define POLARSSL_AES_ALT
//#define POLARSSL_ARC4_ALT
//#define POLARSSL_BLOWFISH_ALT
//#define POLARSSL_CAMELLIA_ALT
//#define POLARSSL_DES_ALT
//#define POLARSSL_XTEA_ALT
//#define POLARSSL_MD2_ALT
//#define POLARSSL_MD4_ALT
//#define POLARSSL_MD5_ALT
//#define POLARSSL_RIPEMD160_ALT
//#define POLARSSL_SHA1_ALT
//#define POLARSSL_SHA256_ALT
//#define POLARSSL_SHA512_ALT

/**
 * \def POLARSSL_AES_ROM_TABLES
 *
 * Store the AES tables in ROM.
 *
 * Uncomment this macro to store the AES tables in ROM.
 */
//#define POLARSSL_AES_ROM_TABLES

/**
 * \def POLARSSL_CAMELLIA_SMALL_MEMORY
 *
 * Use less ROM for the Camellia implementation (saves about 768 bytes).
 *
 * Uncomment this macro to use less memory for Camellia.
 */
//#define POLARSSL_CAMELLIA_SMALL_MEMORY

/**
 * \def POLARSSL_CIPHER_MODE_CBC
 *
 * Enable Cipher Block Chaining mode (CBC) for symmetric ciphers.
 */
#define POLARSSL_CIPHER_MODE_CBC

/**
 * \def POLARSSL_CIPHER_MODE_CFB
 *
 * Enable Cipher Feedback mode (CFB) for symmetric ciphers.
 */
#define POLARSSL_CIPHER_MODE_CFB

/**
 * \def POLARSSL_CIPHER_MODE_CTR
 *
 * Enable Counter Block Cipher mode (CTR) for symmetric ciphers.
 */
#define POLARSSL_CIPHER_MODE_CTR

/**
 * \def POLARSSL_CIPHER_NULL_CIPHER
 *
 * Enable NULL cipher.
 * Warning: Only do so when you know what you are doing. This allows for
 * encryption or channels without any security!
 *
 * Requires POLARSSL_ENABLE_WEAK_CIPHERSUITES as well to enable
 * the following ciphersuites:
 *      TLS_ECDH_ECDSA_WITH_NULL_SHA
 *      TLS_ECDH_RSA_WITH_NULL_SHA
 *      TLS_ECDHE_ECDSA_WITH_NULL_SHA
 *      TLS_ECDHE_RSA_WITH_NULL_SHA
 *      TLS_ECDHE_PSK_WITH_NULL_SHA384
 *      TLS_ECDHE_PSK_WITH_NULL_SHA256
 *      TLS_ECDHE_PSK_WITH_NULL_SHA
 *      TLS_DHE_PSK_WITH_NULL_SHA384
 *      TLS_DHE_PSK_WITH_NULL_SHA256
 *      TLS_DHE_PSK_WITH_NULL_SHA
 *      TLS_RSA_WITH_NULL_SHA256
 *      TLS_RSA_WITH_NULL_SHA
 *      TLS_RSA_WITH_NULL_MD5
 *      TLS_RSA_PSK_WITH_NULL_SHA384
 *      TLS_RSA_PSK_WITH_NULL_SHA256
 *      TLS_RSA_PSK_WITH_NULL_SHA
 *      TLS_PSK_WITH_NULL_SHA384
 *      TLS_PSK_WITH_NULL_SHA256
 *      TLS_PSK_WITH_NULL_SHA
 *
 * Uncomment this macro to enable the NULL cipher and ciphersuites
 */
//#define POLARSSL_CIPHER_NULL_CIPHER

/**
 * \def POLARSSL_CIPHER_PADDING_XXX
 *
 * Uncomment or comment macros to add support for specific padding modes
 * in the cipher layer with cipher modes that support padding (e.g. CBC)
 *
 * If you disable all padding modes, only full blocks can be used with CBC.
 *
 * Enable padding modes in the cipher layer.
 */
#define POLARSSL_CIPHER_PADDING_PKCS7
#define POLARSSL_CIPHER_PADDING_ONE_AND_ZEROS
#define POLARSSL_CIPHER_PADDING_ZEROS_AND_LEN
#define POLARSSL_CIPHER_PADDING_ZEROS

/**
 * \def POLARSSL_ENABLE_WEAK_CIPHERSUITES
 *
 * Enable weak ciphersuites in SSL / TLS.
 * Warning: Only do so when you know what you are doing. This allows for
 * channels with virtually no security at all!
 *
 * This enables the following ciphersuites:
 *      TLS_RSA_WITH_DES_CBC_SHA
 *      TLS_DHE_RSA_WITH_DES_CBC_SHA
 *
 * Uncomment this macro to enable weak ciphersuites
 */
//#define POLARSSL_ENABLE_WEAK_CIPHERSUITES

/**
 * \def POLARSSL_REMOVE_ARC4_CIPHERSUITES
 *
 * Remove RC4 ciphersuites by default in SSL / TLS.
 * This flag removes the ciphersuites based on RC4 from the default list as
 * returned by ssl_list_ciphersuites(). However, it is still possible to
 * enable (some of) them with ssl_set_ciphersuites() by including them
 * explicitly.
 *
 * Uncomment this macro to remove RC4 ciphersuites by default.
 */
//#define POLARSSL_REMOVE_ARC4_CIPHERSUITES

/**
 * \def POLARSSL_ECP_XXXX_ENABLED
 *
 * Enables specific curves within the Elliptic Curve module.
 * By default all supported curves are enabled.
 *
 * Comment macros to disable the curve and functions for it
 */
#define POLARSSL_ECP_DP_SECP192R1_ENABLED
#define POLARSSL_ECP_DP_SECP224R1_ENABLED
#define POLARSSL_ECP_DP_SECP256R1_ENABLED
#define POLARSSL_ECP_DP_SECP384R1_ENABLED
#define POLARSSL_ECP_DP_SECP521R1_ENABLED
#define POLARSSL_ECP_DP_SECP192K1_ENABLED
#define POLARSSL_ECP_DP_SECP224K1_ENABLED
#define POLARSSL_ECP_DP_SECP256K1_ENABLED
#define POLARSSL_ECP_DP_BP256R1_ENABLED
#define POLARSSL_ECP_DP_BP384R1_ENABLED
#define POLARSSL_ECP_DP_BP512R1_ENABLED
//#define POLARSSL_ECP_DP_M221_ENABLED  // Not implemented yet!
#define POLARSSL_ECP_DP_M255_ENABLED
//#define POLARSSL_ECP_DP_M383_ENABLED  // Not implemented yet!
//#define POLARSSL_ECP_DP_M511_ENABLED  // Not implemented yet!

/**
 * \def POLARSSL_ECP_NIST_OPTIM
 *
 * Enable specific 'modulo p' routines for each NIST prime.
 * Depending on the prime and architecture, makes operations 4 to 8 times
 * faster on the corresponding curve.
 *
 * Comment this macro to disable NIST curves optimisation.
 */
#define POLARSSL_ECP_NIST_OPTIM

/**
 * \def POLARSSL_ECDSA_DETERMINISTIC
 *
 * Enable deterministic ECDSA (RFC 6979).
 * Standard ECDSA is "fragile" in the sense that lack of entropy when signing
 * may result in a compromise of the long-term signing key. This is avoided by
 * the deterministic variant.
 *
 * Requires: POLARSSL_HMAC_DRBG_C
 *
 * Comment this macro to disable deterministic ECDSA.
 */
#define POLARSSL_ECDSA_DETERMINISTIC

/**
 * \def POLARSSL_KEY_EXCHANGE_PSK_ENABLED
 *
 * Enable the PSK based ciphersuite modes in SSL / TLS.
 *
 * This enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_PSK_WITH_AES_256_GCM_SHA384
 *      TLS_PSK_WITH_AES_256_CBC_SHA384
 *      TLS_PSK_WITH_AES_256_CBC_SHA
 *      TLS_PSK_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_PSK_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_PSK_WITH_AES_128_GCM_SHA256
 *      TLS_PSK_WITH_AES_128_CBC_SHA256
 *      TLS_PSK_WITH_AES_128_CBC_SHA
 *      TLS_PSK_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_PSK_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_PSK_WITH_3DES_EDE_CBC_SHA
 *      TLS_PSK_WITH_RC4_128_SHA
 */
#define POLARSSL_KEY_EXCHANGE_PSK_ENABLED

/**
 * \def POLARSSL_KEY_EXCHANGE_DHE_PSK_ENABLED
 *
 * Enable the DHE-PSK based ciphersuite modes in SSL / TLS.
 *
 * Requires: POLARSSL_DHM_C
 *
 * This enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_DHE_PSK_WITH_AES_256_GCM_SHA384
 *      TLS_DHE_PSK_WITH_AES_256_CBC_SHA384
 *      TLS_DHE_PSK_WITH_AES_256_CBC_SHA
 *      TLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_DHE_PSK_WITH_AES_128_GCM_SHA256
 *      TLS_DHE_PSK_WITH_AES_128_CBC_SHA256
 *      TLS_DHE_PSK_WITH_AES_128_CBC_SHA
 *      TLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA
 *      TLS_DHE_PSK_WITH_RC4_128_SHA
 */
#define POLARSSL_KEY_EXCHANGE_DHE_PSK_ENABLED

/**
 * \def POLARSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED
 *
 * Enable the ECDHE-PSK based ciphersuite modes in SSL / TLS.
 *
 * Requires: POLARSSL_ECDH_C
 *
 * This enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384
 *      TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA
 *      TLS_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256
 *      TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA
 *      TLS_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA
 *      TLS_ECDHE_PSK_WITH_RC4_128_SHA
 */
#define POLARSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED

/**
 * \def POLARSSL_KEY_EXCHANGE_RSA_PSK_ENABLED
 *
 * Enable the RSA-PSK based ciphersuite modes in SSL / TLS.
 *
 * Requires: POLARSSL_RSA_C, POLARSSL_PKCS1_V15,
 *           POLARSSL_X509_CRT_PARSE_C
 *
 * This enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_RSA_PSK_WITH_AES_256_GCM_SHA384
 *      TLS_RSA_PSK_WITH_AES_256_CBC_SHA384
 *      TLS_RSA_PSK_WITH_AES_256_CBC_SHA
 *      TLS_RSA_PSK_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_RSA_PSK_WITH_AES_128_GCM_SHA256
 *      TLS_RSA_PSK_WITH_AES_128_CBC_SHA256
 *      TLS_RSA_PSK_WITH_AES_128_CBC_SHA
 *      TLS_RSA_PSK_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA
 *      TLS_RSA_PSK_WITH_RC4_128_SHA
 */
#define POLARSSL_KEY_EXCHANGE_RSA_PSK_ENABLED

/**
 * \def POLARSSL_KEY_EXCHANGE_RSA_ENABLED
 *
 * Enable the RSA-only based ciphersuite modes in SSL / TLS.
 *
 * Requires: POLARSSL_RSA_C, POLARSSL_PKCS1_V15,
 *           POLARSSL_X509_CRT_PARSE_C
 *
 * This enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_RSA_WITH_AES_256_GCM_SHA384
 *      TLS_RSA_WITH_AES_256_CBC_SHA256
 *      TLS_RSA_WITH_AES_256_CBC_SHA
 *      TLS_RSA_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256
 *      TLS_RSA_WITH_CAMELLIA_256_CBC_SHA
 *      TLS_RSA_WITH_AES_128_GCM_SHA256
 *      TLS_RSA_WITH_AES_128_CBC_SHA256
 *      TLS_RSA_WITH_AES_128_CBC_SHA
 *      TLS_RSA_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_RSA_WITH_CAMELLIA_128_CBC_SHA
 *      TLS_RSA_WITH_3DES_EDE_CBC_SHA
 *      TLS_RSA_WITH_RC4_128_SHA
 *      TLS_RSA_WITH_RC4_128_MD5
 */
#define POLARSSL_KEY_EXCHANGE_RSA_ENABLED

/**
 * \def POLARSSL_KEY_EXCHANGE_DHE_RSA_ENABLED
 *
 * Enable the DHE-RSA based ciphersuite modes in SSL / TLS.
 *
 * Requires: POLARSSL_DHM_C, POLARSSL_RSA_C, POLARSSL_PKCS1_V15,
 *           POLARSSL_X509_CRT_PARSE_C
 *
 * This enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_DHE_RSA_WITH_AES_256_GCM_SHA384
 *      TLS_DHE_RSA_WITH_AES_256_CBC_SHA256
 *      TLS_DHE_RSA_WITH_AES_256_CBC_SHA
 *      TLS_DHE_RSA_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256
 *      TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA
 *      TLS_DHE_RSA_WITH_AES_128_GCM_SHA256
 *      TLS_DHE_RSA_WITH_AES_128_CBC_SHA256
 *      TLS_DHE_RSA_WITH_AES_128_CBC_SHA
 *      TLS_DHE_RSA_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA
 *      TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA
 */
#define POLARSSL_KEY_EXCHANGE_DHE_RSA_ENABLED

/**
 * \def POLARSSL_KEY_EXCHANGE_ECDHE_RSA_ENABLED
 *
 * Enable the ECDHE-RSA based ciphersuite modes in SSL / TLS.
 *
 * Requires: POLARSSL_ECDH_C, POLARSSL_RSA_C, POLARSSL_PKCS1_V15,
 *           POLARSSL_X509_CRT_PARSE_C
 *
 * This enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384
 *      TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384
 *      TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
 *      TLS_ECDHE_RSA_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
 *      TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256
 *      TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA
 *      TLS_ECDHE_RSA_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA
 *      TLS_ECDHE_RSA_WITH_RC4_128_SHA
 */
#define POLARSSL_KEY_EXCHANGE_ECDHE_RSA_ENABLED

/**
 * \def POLARSSL_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
 *
 * Enable the ECDHE-ECDSA based ciphersuite modes in SSL / TLS.
 *
 * Requires: POLARSSL_ECDH_C, POLARSSL_ECDSA_C, POLARSSL_X509_CRT_PARSE_C,
 *
 * This enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384
 *      TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384
 *      TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA
 *      TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256
 *      TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256
 *      TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA
 *      TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA
 *      TLS_ECDHE_ECDSA_WITH_RC4_128_SHA
 */
#define POLARSSL_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED

/**
 * \def POLARSSL_KEY_EXCHANGE_ECDH_ECDSA_ENABLED
 *
 * Enable the ECDH-ECDSA based ciphersuite modes in SSL / TLS.
 *
 * Requires: POLARSSL_ECDH_C, POLARSSL_X509_CRT_PARSE_C
 *
 * This enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_ECDH_ECDSA_WITH_RC4_128_SHA
 *      TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA
 *      TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA
 *      TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA
 *      TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256
 *      TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384
 *      TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256
 *      TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384
 *      TLS_ECDH_ECDSA_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_ECDH_ECDSA_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_ECDH_ECDSA_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_ECDH_ECDSA_WITH_CAMELLIA_256_GCM_SHA384
 */
#define POLARSSL_KEY_EXCHANGE_ECDH_ECDSA_ENABLED

/**
 * \def POLARSSL_KEY_EXCHANGE_ECDH_RSA_ENABLED
 *
 * Enable the ECDH-RSA based ciphersuite modes in SSL / TLS.
 *
 * Requires: POLARSSL_ECDH_C, POLARSSL_X509_CRT_PARSE_C
 *
 * This enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_ECDH_RSA_WITH_RC4_128_SHA
 *      TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA
 *      TLS_ECDH_RSA_WITH_AES_128_CBC_SHA
 *      TLS_ECDH_RSA_WITH_AES_256_CBC_SHA
 *      TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256
 *      TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384
 *      TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256
 *      TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384
 *      TLS_ECDH_RSA_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_ECDH_RSA_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_ECDH_RSA_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_ECDH_RSA_WITH_CAMELLIA_256_GCM_SHA384
 */
#define POLARSSL_KEY_EXCHANGE_ECDH_RSA_ENABLED

/**
 * \def POLARSSL_PK_PARSE_EC_EXTENDED
 *
 * Enhance support for reading EC keys using variants of SEC1 not allowed by
 * RFC 5915 and RFC 5480.
 *
 * Currently this means parsing the SpecifiedECDomain choice of EC
 * parameters (only known groups are supported, not arbitrary domains, to
 * avoid validation issues).
 *
 * Disable if you only need to support RFC 5915 + 5480 key formats.
 */
#define POLARSSL_PK_PARSE_EC_EXTENDED

/**
 * \def POLARSSL_ERROR_STRERROR_BC
 *
 * Make available the backward compatible error_strerror() next to the
 * current polarssl_strerror().
 *
 * \deprecated Do not define this and use polarssl_strerror() instead
 *
 * Disable if you want to really remove the error_strerror() name
 */
#define POLARSSL_ERROR_STRERROR_BC

/**
 * \def POLARSSL_ERROR_STRERROR_DUMMY
 *
 * Enable a dummy error function to make use of polarssl_strerror() in
 * third party libraries easier when POLARSSL_ERROR_C is disabled
 * (no effect when POLARSSL_ERROR_C is enabled).
 *
 * You can safely disable this if POLARSSL_ERROR_C is enabled, or if you're
 * not using polarssl_strerror() or error_strerror() in your application.
 *
 * Disable if you run into name conflicts and want to really remove the
 * polarssl_strerror()
 */
#define POLARSSL_ERROR_STRERROR_DUMMY

/**
 * \def POLARSSL_GENPRIME
 *
 * Enable the prime-number generation code.
 *
 * Requires: POLARSSL_BIGNUM_C
 */
#define POLARSSL_GENPRIME

/**
 * \def POLARSSL_FS_IO
 *
 * Enable functions that use the filesystem.
 */
#define POLARSSL_FS_IO

/**
 * \def POLARSSL_NO_DEFAULT_ENTROPY_SOURCES
 *
 * Do not add default entropy sources. These are the platform specific,
 * hardclock and HAVEGE based poll functions.
 *
 * This is useful to have more control over the added entropy sources in an
 * application.
 *
 * Uncomment this macro to prevent loading of default entropy functions.
 */
//#define POLARSSL_NO_DEFAULT_ENTROPY_SOURCES

/**
 * \def POLARSSL_NO_PLATFORM_ENTROPY
 *
 * Do not use built-in platform entropy functions.
 * This is useful if your platform does not support
 * standards like the /dev/urandom or Windows CryptoAPI.
 *
 * Uncomment this macro to disable the built-in platform entropy functions.
 */
//#define POLARSSL_NO_PLATFORM_ENTROPY

/**
 * \def POLARSSL_ENTROPY_FORCE_SHA256
 *
 * Force the entropy accumulator to use a SHA-256 accumulator instead of the
 * default SHA-512 based one (if both are available).
 *
 * Requires: POLARSSL_SHA256_C
 *
 * On 32-bit systems SHA-256 can be much faster than SHA-512. Use this option
 * if you have performance concerns.
 *
 * This option is only useful if both POLARSSL_SHA256_C and
 * POLARSSL_SHA512_C are defined. Otherwise the available hash module is used.
 */
//#define POLARSSL_ENTROPY_FORCE_SHA256

/**
 * \def POLARSSL_MEMORY_DEBUG
 *
 * Enable debugging of buffer allocator memory issues. Automatically prints
 * (to stderr) all (fatal) messages on memory allocation issues. Enables
 * function for 'debug output' of allocated memory.
 *
 * Requires: POLARSSL_MEMORY_BUFFER_ALLOC_C
 *
 * Uncomment this macro to let the buffer allocator print out error messages.
 */
//#define POLARSSL_MEMORY_DEBUG

/**
 * \def POLARSSL_MEMORY_BACKTRACE
 *
 * Include backtrace information with each allocated block.
 *
 * Requires: POLARSSL_MEMORY_BUFFER_ALLOC_C
 *           GLIBC-compatible backtrace() an backtrace_symbols() support
 *
 * Uncomment this macro to include backtrace information
 */
//#define POLARSSL_MEMORY_BACKTRACE

/**
 * \def POLARSSL_PKCS1_V15
 *
 * Enable support for PKCS#1 v1.5 encoding.
 *
 * Requires: POLARSSL_RSA_C
 *
 * This enables support for PKCS#1 v1.5 operations.
 */
#define POLARSSL_PKCS1_V15

/**
 * \def POLARSSL_PKCS1_V21
 *
 * Enable support for PKCS#1 v2.1 encoding.
 *
 * Requires: POLARSSL_MD_C, POLARSSL_RSA_C
 *
 * This enables support for RSAES-OAEP and RSASSA-PSS operations.
 */
#define POLARSSL_PKCS1_V21

/**
 * \def POLARSSL_RSA_NO_CRT
 *
 * Do not use the Chinese Remainder Theorem for the RSA private operation.
 *
 * Uncomment this macro to disable the use of CRT in RSA.
 *
 */
//#define POLARSSL_RSA_NO_CRT

/**
 * \def POLARSSL_SELF_TEST
 *
 * Enable the checkup functions (*_self_test).
 */
#define POLARSSL_SELF_TEST

/**
 * \def POLARSSL_SSL_AEAD_RANDOM_IV
 *
 * Generate a random IV rather than using the record sequence number as a
 * nonce for ciphersuites using and AEAD algorithm (GCM or CCM).
 *
 * Using the sequence number is generally recommended.
 *
 * Uncomment this macro to always use random IVs with AEAD ciphersuites.
 */
//#define POLARSSL_SSL_AEAD_RANDOM_IV

/**
 * \def POLARSSL_SSL_ALL_ALERT_MESSAGES
 *
 * Enable sending of alert messages in case of encountered errors as per RFC.
 * If you choose not to send the alert messages, mbed TLS can still communicate
 * with other servers, only debugging of failures is harder.
 *
 * The advantage of not sending alert messages, is that no information is given
 * about reasons for failures thus preventing adversaries of gaining intel.
 *
 * Enable sending of all alert messages
 */
#define POLARSSL_SSL_ALERT_MESSAGES

/**
 * \def POLARSSL_SSL_DEBUG_ALL
 *
 * Enable the debug messages in SSL module for all issues.
 * Debug messages have been disabled in some places to prevent timing
 * attacks due to (unbalanced) debugging function calls.
 *
 * If you need all error reporting you should enable this during debugging,
 * but remove this for production servers that should log as well.
 *
 * Uncomment this macro to report all debug messages on errors introducing
 * a timing side-channel.
 *
 */
//#define POLARSSL_SSL_DEBUG_ALL

/** \def POLARSSL_SSL_ENCRYPT_THEN_MAC
 *
 * Enable support for Encrypt-then-MAC, RFC 7366.
 *
 * This allows peers that both support it to use a more robust protection for
 * ciphersuites using CBC, providing deep resistance against timing attacks
 * on the padding or underlying cipher.
 *
 * This only affects CBC ciphersuites, and is useless if none is defined.
 *
 * Requires: POLARSSL_SSL_PROTO_TLS1    or
 *           POLARSSL_SSL_PROTO_TLS1_1  or
 *           POLARSSL_SSL_PROTO_TLS1_2
 *
 * Comment this macro to disable support for Encrypt-then-MAC
 */
#define POLARSSL_SSL_ENCRYPT_THEN_MAC

/** \def POLARSSL_SSL_EXTENDED_MASTER_SECRET
 *
 * Enable support for Extended Master Secret, aka Session Hash
 * (draft-ietf-tls-session-hash-02).
 *
 * This was introduced as "the proper fix" to the Triple Handshake familiy of
 * attacks, but it is recommended to always use it (even if you disable
 * renegotiation), since it actually fixes a more fundamental issue in the
 * original SSL/TLS design, and has implications beyond Triple Handshake.
 *
 * Requires: POLARSSL_SSL_PROTO_TLS1    or
 *           POLARSSL_SSL_PROTO_TLS1_1  or
 *           POLARSSL_SSL_PROTO_TLS1_2
 *
 * Comment this macro to disable support for Extended Master Secret.
 */
#define POLARSSL_SSL_EXTENDED_MASTER_SECRET

/**
 * \def POLARSSL_SSL_FALLBACK_SCSV
 *
 * Enable support for FALLBACK_SCSV (draft-ietf-tls-downgrade-scsv-00).
 *
 * For servers, it is recommended to always enable this, unless you support
 * only one version of TLS, or know for sure that none of your clients
 * implements a fallback strategy.
 *
 * For clients, you only need this if you're using a fallback strategy, which
 * is not recommended in the first place, unless you absolutely need it to
 * interoperate with buggy (version-intolerant) servers.
 *
 * Comment this macro to disable support for FALLBACK_SCSV
 */
#define POLARSSL_SSL_FALLBACK_SCSV

/**
 * \def POLARSSL_SSL_HW_RECORD_ACCEL
 *
 * Enable hooking functions in SSL module for hardware acceleration of
 * individual records.
 *
 * Uncomment this macro to enable hooking functions.
 */
//#define POLARSSL_SSL_HW_RECORD_ACCEL

/**
 * \def POLARSSL_SSL_CBC_RECORD_SPLITTING
 *
 * Enable 1/n-1 record splitting for CBC mode in SSLv3 and TLS 1.0.
 *
 * This is a countermeasure to the BEAST attack, which also minimizes the risk
 * of interoperability issues compared to sending 0-length records.
 *
 * Comment this macro to disable 1/n-1 record splitting.
 */
#define POLARSSL_SSL_CBC_RECORD_SPLITTING

/**
 * \def POLARSSL_SSL_DISABLE_RENEGOTIATION
 *
 * Disable support for TLS renegotiation.
 *
 * The two main uses of renegotiation are (1) refresh keys on long-lived
 * connections and (2) client authentication after the initial handshake.
 * If you don't need renegotiation, it's probably better to disable it, since
 * it has been associated with security issues in the past and is easy to
 * misuse/misunderstand.
 *
 * Warning: in the next stable branch, this switch will be replaced by
 * POLARSSL_SSL_RENEGOTIATION to enable support for renegotiation.
 *
 * Uncomment this to disable support for renegotiation.
 */
//#define POLARSSL_SSL_DISABLE_RENEGOTIATION

/**
 * \def POLARSSL_SSL_SRV_SUPPORT_SSLV2_CLIENT_HELLO
 *
 * Enable support for receiving and parsing SSLv2 Client Hello messages for the
 * SSL Server module (POLARSSL_SSL_SRV_C).
 *
 * Comment this macro to disable support for SSLv2 Client Hello messages.
 */
#define POLARSSL_SSL_SRV_SUPPORT_SSLV2_CLIENT_HELLO

/**
 * \def POLARSSL_SSL_SRV_RESPECT_CLIENT_PREFERENCE
 *
 * Pick the ciphersuite according to the client's preferences rather than ours
 * in the SSL Server module (POLARSSL_SSL_SRV_C).
 *
 * Uncomment this macro to respect client's ciphersuite order
 */
//#define POLARSSL_SSL_SRV_RESPECT_CLIENT_PREFERENCE

/**
 * \def POLARSSL_SSL_MAX_FRAGMENT_LENGTH
 *
 * Enable support for RFC 6066 max_fragment_length extension in SSL.
 *
 * Comment this macro to disable support for the max_fragment_length extension
 */
#define POLARSSL_SSL_MAX_FRAGMENT_LENGTH

/**
 * \def POLARSSL_SSL_PROTO_SSL3
 *
 * Enable support for SSL 3.0.
 *
 * Requires: POLARSSL_MD5_C
 *           POLARSSL_SHA1_C
 *
 * Comment this macro to disable support for SSL 3.0
 */
#define POLARSSL_SSL_PROTO_SSL3

/**
 * \def POLARSSL_SSL_PROTO_TLS1
 *
 * Enable support for TLS 1.0.
 *
 * Requires: POLARSSL_MD5_C
 *           POLARSSL_SHA1_C
 *
 * Comment this macro to disable support for TLS 1.0
 */
#define POLARSSL_SSL_PROTO_TLS1

/**
 * \def POLARSSL_SSL_PROTO_TLS1_1
 *
 * Enable support for TLS 1.1.
 *
 * Requires: POLARSSL_MD5_C
 *           POLARSSL_SHA1_C
 *
 * Comment this macro to disable support for TLS 1.1
 */
#define POLARSSL_SSL_PROTO_TLS1_1

/**
 * \def POLARSSL_SSL_PROTO_TLS1_2
 *
 * Enable support for TLS 1.2.
 *
 * Requires: POLARSSL_SHA1_C or POLARSSL_SHA256_C or POLARSSL_SHA512_C
 *           (Depends on ciphersuites)
 *
 * Comment this macro to disable support for TLS 1.2
 */
#define POLARSSL_SSL_PROTO_TLS1_2

/**
 * \def POLARSSL_SSL_ALPN
 *
 * Enable support for RFC 7301 Application Layer Protocol Negotiation.
 *
 * Comment this macro to disable support for ALPN.
 */
#define POLARSSL_SSL_ALPN

/**
 * \def POLARSSL_SSL_SESSION_TICKETS
 *
 * Enable support for RFC 5077 session tickets in SSL.
 *
 * Requires: POLARSSL_AES_C
 *           POLARSSL_SHA256_C
 *           POLARSSL_CIPHER_MODE_CBC
 *
 * Comment this macro to disable support for SSL session tickets
 */
#define POLARSSL_SSL_SESSION_TICKETS

/**
 * \def POLARSSL_SSL_SERVER_NAME_INDICATION
 *
 * Enable support for RFC 6066 server name indication (SNI) in SSL.
 *
 * Requires: POLARSSL_X509_CRT_PARSE_C
 *
 * Comment this macro to disable support for server name indication in SSL
 */
#define POLARSSL_SSL_SERVER_NAME_INDICATION

/**
 * \def POLARSSL_SSL_TRUNCATED_HMAC
 *
 * Enable support for RFC 6066 truncated HMAC in SSL.
 *
 * Comment this macro to disable support for truncated HMAC in SSL
 */
#define POLARSSL_SSL_TRUNCATED_HMAC

/**
 * \def POLARSSL_SSL_SET_CURVES
 *
 * Enable ssl_set_curves().
 *
 * This is disabled by default since it breaks binary compatibility with the
 * 1.3.x line. If you choose to enable it, you will need to rebuild your
 * application against the new header files, relinking will not be enough.
 * It will be enabled by default, or no longer an option, in the 1.4 branch.
 *
 * Uncomment to make ssl_set_curves() available.
 */
//#define POLARSSL_SSL_SET_CURVES

/**
 * \def POLARSSL_THREADING_ALT
 *
 * Provide your own alternate threading implementation.
 *
 * Requires: POLARSSL_THREADING_C
 *
 * Uncomment this to allow your own alternate threading implementation.
 */
//#define POLARSSL_THREADING_ALT

/**
 * \def POLARSSL_THREADING_PTHREAD
 *
 * Enable the pthread wrapper layer for the threading layer.
 *
 * Requires: POLARSSL_THREADING_C
 *
 * Uncomment this to enable pthread mutexes.
 */
//#define POLARSSL_THREADING_PTHREAD

/**
 * \def POLARSSL_VERSION_FEATURES
 *
 * Allow run-time checking of compile-time enabled features. Thus allowing users
 * to check at run-time if the library is for instance compiled with threading
 * support via version_check_feature().
 *
 * Requires: POLARSSL_VERSION_C
 *
 * Comment this to disable run-time checking and save ROM space
 */
#define POLARSSL_VERSION_FEATURES

/**
 * \def POLARSSL_X509_ALLOW_EXTENSIONS_NON_V3
 *
 * If set, the X509 parser will not break-off when parsing an X509 certificate
 * and encountering an extension in a v1 or v2 certificate.
 *
 * Uncomment to prevent an error.
 */
//#define POLARSSL_X509_ALLOW_EXTENSIONS_NON_V3

/**
 * \def POLARSSL_X509_ALLOW_UNSUPPORTED_CRITICAL_EXTENSION
 *
 * If set, the X509 parser will not break-off when parsing an X509 certificate
 * and encountering an unknown critical extension.
 *
 * Uncomment to prevent an error.
 */
//#define POLARSSL_X509_ALLOW_UNSUPPORTED_CRITICAL_EXTENSION

/**
 * \def POLARSSL_X509_CHECK_KEY_USAGE
 *
 * Enable verification of the keyUsage extension (CA and leaf certificates).
 *
 * Disabling this avoids problems with mis-issued and/or misused
 * (intermediate) CA and leaf certificates.
 *
 * \warning Depending on your PKI use, disabling this can be a security risk!
 *
 * Comment to skip keyUsage checking for both CA and leaf certificates.
 */
#define POLARSSL_X509_CHECK_KEY_USAGE

/**
 * \def POLARSSL_X509_CHECK_EXTENDED_KEY_USAGE
 *
 * Enable verification of the extendedKeyUsage extension (leaf certificates).
 *
 * Disabling this avoids problems with mis-issued and/or misused certificates.
 *
 * \warning Depending on your PKI use, disabling this can be a security risk!
 *
 * Comment to skip extendedKeyUsage checking for certificates.
 */
#define POLARSSL_X509_CHECK_EXTENDED_KEY_USAGE

/**
 * \def POLARSSL_X509_RSASSA_PSS_SUPPORT
 *
 * Enable parsing and verification of X.509 certificates, CRLs and CSRS
 * signed with RSASSA-PSS (aka PKCS#1 v2.1).
 *
 * Comment this macro to disallow using RSASSA-PSS in certificates.
 */
#define POLARSSL_X509_RSASSA_PSS_SUPPORT

/**
 * \def POLARSSL_ZLIB_SUPPORT
 *
 * If set, the SSL/TLS module uses ZLIB to support compression and
 * decompression of packet data.
 *
 * \warning TLS-level compression MAY REDUCE SECURITY! See for example the
 * CRIME attack. Before enabling this option, you should examine with care if
 * CRIME or similar exploits may be a applicable to your use case.
 *
 * Used in: library/ssl_tls.c
 *          library/ssl_cli.c
 *          library/ssl_srv.c
 *
 * This feature requires zlib library and headers to be present.
 *
 * Uncomment to enable use of ZLIB
 */
//#define POLARSSL_ZLIB_SUPPORT
/* \} name SECTION: mbed TLS feature support */

/**
 * \name SECTION: mbed TLS modules
 *
 * This section enables or disables entire modules in mbed TLS
 * \{
 */

/**
 * \def POLARSSL_AESNI_C
 *
 * Enable AES-NI support on x86-64.
 *
 * Module:  library/aesni.c
 * Caller:  library/aes.c
 *
 * Requires: POLARSSL_HAVE_ASM
 *
 * This modules adds support for the AES-NI instructions on x86-64
 */
#define POLARSSL_AESNI_C

/**
 * \def POLARSSL_AES_C
 *
 * Enable the AES block cipher.
 *
 * Module:  library/aes.c
 * Caller:  library/ssl_tls.c
 *          library/pem.c
 *          library/ctr_drbg.c
 *
 * This module enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA
 *      TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA
 *      TLS_ECDH_RSA_WITH_AES_128_CBC_SHA
 *      TLS_ECDH_RSA_WITH_AES_256_CBC_SHA
 *      TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256
 *      TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384
 *      TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256
 *      TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384
 *      TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256
 *      TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384
 *      TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256
 *      TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384
 *      TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384
 *      TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384
 *      TLS_DHE_RSA_WITH_AES_256_GCM_SHA384
 *      TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384
 *      TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384
 *      TLS_DHE_RSA_WITH_AES_256_CBC_SHA256
 *      TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA
 *      TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
 *      TLS_DHE_RSA_WITH_AES_256_CBC_SHA
 *      TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256
 *      TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
 *      TLS_DHE_RSA_WITH_AES_128_GCM_SHA256
 *      TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256
 *      TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256
 *      TLS_DHE_RSA_WITH_AES_128_CBC_SHA256
 *      TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA
 *      TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA
 *      TLS_DHE_RSA_WITH_AES_128_CBC_SHA
 *      TLS_DHE_PSK_WITH_AES_256_GCM_SHA384
 *      TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384
 *      TLS_DHE_PSK_WITH_AES_256_CBC_SHA384
 *      TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA
 *      TLS_DHE_PSK_WITH_AES_256_CBC_SHA
 *      TLS_DHE_PSK_WITH_AES_128_GCM_SHA256
 *      TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256
 *      TLS_DHE_PSK_WITH_AES_128_CBC_SHA256
 *      TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA
 *      TLS_DHE_PSK_WITH_AES_128_CBC_SHA
 *      TLS_RSA_WITH_AES_256_GCM_SHA384
 *      TLS_RSA_WITH_AES_256_CBC_SHA256
 *      TLS_RSA_WITH_AES_256_CBC_SHA
 *      TLS_RSA_WITH_AES_128_GCM_SHA256
 *      TLS_RSA_WITH_AES_128_CBC_SHA256
 *      TLS_RSA_WITH_AES_128_CBC_SHA
 *      TLS_RSA_PSK_WITH_AES_256_GCM_SHA384
 *      TLS_RSA_PSK_WITH_AES_256_CBC_SHA384
 *      TLS_RSA_PSK_WITH_AES_256_CBC_SHA
 *      TLS_RSA_PSK_WITH_AES_128_GCM_SHA256
 *      TLS_RSA_PSK_WITH_AES_128_CBC_SHA256
 *      TLS_RSA_PSK_WITH_AES_128_CBC_SHA
 *      TLS_PSK_WITH_AES_256_GCM_SHA384
 *      TLS_PSK_WITH_AES_256_CBC_SHA384
 *      TLS_PSK_WITH_AES_256_CBC_SHA
 *      TLS_PSK_WITH_AES_128_GCM_SHA256
 *      TLS_PSK_WITH_AES_128_CBC_SHA256
 *      TLS_PSK_WITH_AES_128_CBC_SHA
 *
 * PEM_PARSE uses AES for decrypting encrypted keys.
 */
#define POLARSSL_AES_C

/**
 * \def POLARSSL_ARC4_C
 *
 * Enable the ARCFOUR stream cipher.
 *
 * Module:  library/arc4.c
 * Caller:  library/ssl_tls.c
 *
 * This module enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_ECDH_ECDSA_WITH_RC4_128_SHA
 *      TLS_ECDH_RSA_WITH_RC4_128_SHA
 *      TLS_ECDHE_ECDSA_WITH_RC4_128_SHA
 *      TLS_ECDHE_RSA_WITH_RC4_128_SHA
 *      TLS_ECDHE_PSK_WITH_RC4_128_SHA
 *      TLS_DHE_PSK_WITH_RC4_128_SHA
 *      TLS_RSA_WITH_RC4_128_SHA
 *      TLS_RSA_WITH_RC4_128_MD5
 *      TLS_RSA_PSK_WITH_RC4_128_SHA
 *      TLS_PSK_WITH_RC4_128_SHA
 */
#define POLARSSL_ARC4_C

/**
 * \def POLARSSL_ASN1_PARSE_C
 *
 * Enable the generic ASN1 parser.
 *
 * Module:  library/asn1.c
 * Caller:  library/x509.c
 *          library/dhm.c
 *          library/pkcs12.c
 *          library/pkcs5.c
 *          library/pkparse.c
 */
#define POLARSSL_ASN1_PARSE_C

/**
 * \def POLARSSL_ASN1_WRITE_C
 *
 * Enable the generic ASN1 writer.
 *
 * Module:  library/asn1write.c
 * Caller:  library/ecdsa.c
 *          library/pkwrite.c
 *          library/x509_create.c
 *          library/x509write_crt.c
 *          library/x509write_csr.c
 */
#define POLARSSL_ASN1_WRITE_C

/**
 * \def POLARSSL_BASE64_C
 *
 * Enable the Base64 module.
 *
 * Module:  library/base64.c
 * Caller:  library/pem.c
 *
 * This module is required for PEM support (required by X.509).
 */
#define POLARSSL_BASE64_C

/**
 * \def POLARSSL_BIGNUM_C
 *
 * Enable the multi-precision integer library.
 *
 * Module:  library/bignum.c
 * Caller:  library/dhm.c
 *          library/ecp.c
 *          library/ecdsa.c
 *          library/rsa.c
 *          library/ssl_tls.c
 *
 * This module is required for RSA, DHM and ECC (ECDH, ECDSA) support.
 */
#define POLARSSL_BIGNUM_C

/**
 * \def POLARSSL_BLOWFISH_C
 *
 * Enable the Blowfish block cipher.
 *
 * Module:  library/blowfish.c
 */
#define POLARSSL_BLOWFISH_C

/**
 * \def POLARSSL_CAMELLIA_C
 *
 * Enable the Camellia block cipher.
 *
 * Module:  library/camellia.c
 * Caller:  library/ssl_tls.c
 *
 * This module enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_ECDH_ECDSA_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_ECDH_ECDSA_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_ECDH_RSA_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_ECDH_RSA_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_ECDH_ECDSA_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_ECDH_ECDSA_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_ECDH_RSA_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_ECDH_RSA_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_ECDHE_RSA_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_DHE_RSA_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256
 *      TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA
 *      TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_ECDHE_RSA_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_DHE_RSA_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA
 *      TLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_RSA_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256
 *      TLS_RSA_WITH_CAMELLIA_256_CBC_SHA
 *      TLS_RSA_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_RSA_WITH_CAMELLIA_128_CBC_SHA
 *      TLS_RSA_PSK_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_RSA_PSK_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256
 *      TLS_PSK_WITH_CAMELLIA_256_GCM_SHA384
 *      TLS_PSK_WITH_CAMELLIA_256_CBC_SHA384
 *      TLS_PSK_WITH_CAMELLIA_128_GCM_SHA256
 *      TLS_PSK_WITH_CAMELLIA_128_CBC_SHA256
 */
#define POLARSSL_CAMELLIA_C

/**
 * \def POLARSSL_CCM_C
 *
 * Enable the Counter with CBC-MAC (CCM) mode for 128-bit block cipher.
 *
 * Module:  library/ccm.c
 *
 * Requires: POLARSSL_AES_C or POLARSSL_CAMELLIA_C
 *
 * This module enables the AES-CCM ciphersuites, if other requisites are
 * enabled as well.
 */
#define POLARSSL_CCM_C

/**
 * \def POLARSSL_CERTS_C
 *
 * Enable the test certificates.
 *
 * Module:  library/certs.c
 * Caller:
 *
 * Requires: POLARSSL_PEM_PARSE_C
 *
 * This module is used for testing (ssl_client/server).
 */
#define POLARSSL_CERTS_C

/**
 * \def POLARSSL_CIPHER_C
 *
 * Enable the generic cipher layer.
 *
 * Module:  library/cipher.c
 * Caller:  library/ssl_tls.c
 *
 * Uncomment to enable generic cipher wrappers.
 */
#define POLARSSL_CIPHER_C

/**
 * \def POLARSSL_CTR_DRBG_C
 *
 * Enable the CTR_DRBG AES-256-based random generator.
 *
 * Module:  library/ctr_drbg.c
 * Caller:
 *
 * Requires: POLARSSL_AES_C
 *
 * This module provides the CTR_DRBG AES-256 random number generator.
 */
#define POLARSSL_CTR_DRBG_C

/**
 * \def POLARSSL_DEBUG_C
 *
 * Enable the debug functions.
 *
 * Module:  library/debug.c
 * Caller:  library/ssl_cli.c
 *          library/ssl_srv.c
 *          library/ssl_tls.c
 *
 * This module provides debugging functions.
 */
#define POLARSSL_DEBUG_C

/**
 * \def POLARSSL_DES_C
 *
 * Enable the DES block cipher.
 *
 * Module:  library/des.c
 * Caller:  library/pem.c
 *          library/ssl_tls.c
 *
 * This module enables the following ciphersuites (if other requisites are
 * enabled as well):
 *      TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA
 *      TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA
 *      TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA
 *      TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA
 *      TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA
 *      TLS_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA
 *      TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA
 *      TLS_RSA_WITH_3DES_EDE_CBC_SHA
 *      TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA
 *      TLS_PSK_WITH_3DES_EDE_CBC_SHA
 *
 * PEM_PARSE uses DES/3DES for decrypting encrypted keys.
 */
#define POLARSSL_DES_C

/**
 * \def POLARSSL_DHM_C
 *
 * Enable the Diffie-Hellman-Merkle module.
 *
 * Module:  library/dhm.c
 * Caller:  library/ssl_cli.c
 *          library/ssl_srv.c
 *
 * This module is used by the following key exchanges:
 *      DHE-RSA, DHE-PSK
 */
#define POLARSSL_DHM_C

/**
 * \def POLARSSL_ECDH_C
 *
 * Enable the elliptic curve Diffie-Hellman library.
 *
 * Module:  library/ecdh.c
 * Caller:  library/ssl_cli.c
 *          library/ssl_srv.c
 *
 * This module is used by the following key exchanges:
 *      ECDHE-ECDSA, ECDHE-RSA, DHE-PSK
 *
 * Requires: POLARSSL_ECP_C
 */
#define POLARSSL_ECDH_C

/**
 * \def POLARSSL_ECDSA_C
 *
 * Enable the elliptic curve DSA library.
 *
 * Module:  library/ecdsa.c
 * Caller:
 *
 * This module is used by the following key exchanges:
 *      ECDHE-ECDSA
 *
 * Requires: POLARSSL_ECP_C, POLARSSL_ASN1_WRITE_C, POLARSSL_ASN1_PARSE_C
 */
#define POLARSSL_ECDSA_C

/**
 * \def POLARSSL_ECP_C
 *
 * Enable the elliptic curve over GF(p) library.
 *
 * Module:  library/ecp.c
 * Caller:  library/ecdh.c
 *          library/ecdsa.c
 *
 * Requires: POLARSSL_BIGNUM_C and at least one POLARSSL_ECP_DP_XXX_ENABLED
 */
#define POLARSSL_ECP_C

/**
 * \def POLARSSL_ENTROPY_C
 *
 * Enable the platform-specific entropy code.
 *
 * Module:  library/entropy.c
 * Caller:
 *
 * Requires: POLARSSL_SHA512_C or POLARSSL_SHA256_C
 *
 * This module provides a generic entropy pool
 */
#define POLARSSL_ENTROPY_C

/**
 * \def POLARSSL_ERROR_C
 *
 * Enable error code to error string conversion.
 *
 * Module:  library/error.c
 * Caller:
 *
 * This module enables polarssl_strerror().
 */
#define POLARSSL_ERROR_C

/**
 * \def POLARSSL_GCM_C
 *
 * Enable the Galois/Counter Mode (GCM) for AES.
 *
 * Module:  library/gcm.c
 *
 * Requires: POLARSSL_AES_C or POLARSSL_CAMELLIA_C
 *
 * This module enables the AES-GCM and CAMELLIA-GCM ciphersuites, if other
 * requisites are enabled as well.
 */
#define POLARSSL_GCM_C

/**
 * \def POLARSSL_HAVEGE_C
 *
 * Enable the HAVEGE random generator.
 *
 * Warning: the HAVEGE random generator is not suitable for virtualized
 *          environments
 *
 * Warning: the HAVEGE random generator is dependent on timing and specific
 *          processor traits. It is therefore not advised to use HAVEGE as
 *          your applications primary random generator or primary entropy pool
 *          input. As a secondary input to your entropy pool, it IS able add
 *          the (limited) extra entropy it provides.
 *
 * Module:  library/havege.c
 * Caller:
 *
 * Requires: POLARSSL_TIMING_C
 *
 * Uncomment to enable the HAVEGE random generator.
 */
//#define POLARSSL_HAVEGE_C

/**
 * \def POLARSSL_HMAC_DRBG_C
 *
 * Enable the HMAC_DRBG random generator.
 *
 * Module:  library/hmac_drbg.c
 * Caller:
 *
 * Requires: POLARSSL_MD_C
 *
 * Uncomment to enable the HMAC_DRBG random number geerator.
 */
#define POLARSSL_HMAC_DRBG_C

/**
 * \def POLARSSL_MD_C
 *
 * Enable the generic message digest layer.
 *
 * Module:  library/md.c
 * Caller:
 *
 * Uncomment to enable generic message digest wrappers.
 */
#define POLARSSL_MD_C

/**
 * \def POLARSSL_MD2_C
 *
 * Enable the MD2 hash algorithm.
 *
 * Module:  library/md2.c
 * Caller:
 *
 * Uncomment to enable support for (rare) MD2-signed X.509 certs.
 */
//#define POLARSSL_MD2_C

/**
 * \def POLARSSL_MD4_C
 *
 * Enable the MD4 hash algorithm.
 *
 * Module:  library/md4.c
 * Caller:
 *
 * Uncomment to enable support for (rare) MD4-signed X.509 certs.
 */
//#define POLARSSL_MD4_C

/**
 * \def POLARSSL_MD5_C
 *
 * Enable the MD5 hash algorithm.
 *
 * Module:  library/md5.c
 * Caller:  library/md.c
 *          library/pem.c
 *          library/ssl_tls.c
 *
 * This module is required for SSL/TLS and X.509.
 * PEM_PARSE uses MD5 for decrypting encrypted keys.
 */
#define POLARSSL_MD5_C

/**
 * \def POLARSSL_MEMORY_C
 *
 * \deprecated Use POLARSSL_PLATFORM_MEMORY instead.
 *
 * Depends on: POLARSSL_PLATFORM_C
 */
//#define POLARSSL_MEMORY_C

/**
 * \def POLARSSL_MEMORY_BUFFER_ALLOC_C
 *
 * Enable the buffer allocator implementation that makes use of a (stack)
 * based buffer to 'allocate' dynamic memory. (replaces malloc() and free()
 * calls)
 *
 * Module:  library/memory_buffer_alloc.c
 *
 * Requires: POLARSSL_PLATFORM_C
 *           POLARSSL_PLATFORM_MEMORY (to use it within mbed TLS)
 *
 * Enable this module to enable the buffer memory allocator.
 */
//#define POLARSSL_MEMORY_BUFFER_ALLOC_C

/**
 * \def POLARSSL_NET_C
 *
 * Enable the TCP/IP networking routines.
 *
 * \warning As of 1.3.11, it is deprecated to enable this module without
 * POLARSSL_HAVE_IPV6. The alternative legacy code will be removed in 2.0.
 *
 * Module:  library/net.c
 *
 * This module provides TCP/IP networking routines.
 */
#define POLARSSL_NET_C

/**
 * \def POLARSSL_OID_C
 *
 * Enable the OID database.
 *
 * Module:  library/oid.c
 * Caller:  library/asn1write.c
 *          library/pkcs5.c
 *          library/pkparse.c
 *          library/pkwrite.c
 *          library/rsa.c
 *          library/x509.c
 *          library/x509_create.c
 *          library/x509_crl.c
 *          library/x509_crt.c
 *          library/x509_csr.c
 *          library/x509write_crt.c
 *          library/x509write_csr.c
 *
 * This modules translates between OIDs and internal values.
 */
#define POLARSSL_OID_C

/**
 * \def POLARSSL_PADLOCK_C
 *
 * Enable VIA Padlock support on x86.
 *
 * Module:  library/padlock.c
 * Caller:  library/aes.c
 *
 * Requires: POLARSSL_HAVE_ASM
 *
 * This modules adds support for the VIA PadLock on x86.
 */
#define POLARSSL_PADLOCK_C

/**
 * \def POLARSSL_PBKDF2_C
 *
 * Enable PKCS#5 PBKDF2 key derivation function.
 *
 * \deprecated Use POLARSSL_PKCS5_C instead
 *
 * Module:  library/pbkdf2.c
 *
 * Requires: POLARSSL_PKCS5_C
 *
 * This module adds support for the PKCS#5 PBKDF2 key derivation function.
 */
#define POLARSSL_PBKDF2_C

/**
 * \def POLARSSL_PEM_PARSE_C
 *
 * Enable PEM decoding / parsing.
 *
 * Module:  library/pem.c
 * Caller:  library/dhm.c
 *          library/pkparse.c
 *          library/x509_crl.c
 *          library/x509_crt.c
 *          library/x509_csr.c
 *
 * Requires: POLARSSL_BASE64_C
 *
 * This modules adds support for decoding / parsing PEM files.
 */
#define POLARSSL_PEM_PARSE_C

/**
 * \def POLARSSL_PEM_WRITE_C
 *
 * Enable PEM encoding / writing.
 *
 * Module:  library/pem.c
 * Caller:  library/pkwrite.c
 *          library/x509write_crt.c
 *          library/x509write_csr.c
 *
 * Requires: POLARSSL_BASE64_C
 *
 * This modules adds support for encoding / writing PEM files.
 */
#define POLARSSL_PEM_WRITE_C

/**
 * \def POLARSSL_PK_C
 *
 * Enable the generic public (asymetric) key layer.
 *
 * Module:  library/pk.c
 * Caller:  library/ssl_tls.c
 *          library/ssl_cli.c
 *          library/ssl_srv.c
 *
 * Requires: POLARSSL_RSA_C or POLARSSL_ECP_C
 *
 * Uncomment to enable generic public key wrappers.
 */
#define POLARSSL_PK_C

/**
 * \def POLARSSL_PK_PARSE_C
 *
 * Enable the generic public (asymetric) key parser.
 *
 * Module:  library/pkparse.c
 * Caller:  library/x509_crt.c
 *          library/x509_csr.c
 *
 * Requires: POLARSSL_PK_C
 *
 * Uncomment to enable generic public key parse functions.
 */
#define POLARSSL_PK_PARSE_C

/**
 * \def POLARSSL_PK_WRITE_C
 *
 * Enable the generic public (asymetric) key writer.
 *
 * Module:  library/pkwrite.c
 * Caller:  library/x509write.c
 *
 * Requires: POLARSSL_PK_C
 *
 * Uncomment to enable generic public key write functions.
 */
#define POLARSSL_PK_WRITE_C

/**
 * \def POLARSSL_PKCS5_C
 *
 * Enable PKCS#5 functions.
 *
 * Module:  library/pkcs5.c
 *
 * Requires: POLARSSL_MD_C
 *
 * This module adds support for the PKCS#5 functions.
 */
#define POLARSSL_PKCS5_C

/**
 * \def POLARSSL_PKCS11_C
 *
 * Enable wrapper for PKCS#11 smartcard support.
 *
 * Module:  library/pkcs11.c
 * Caller:  library/pk.c
 *
 * Requires: POLARSSL_PK_C
 *
 * This module enables SSL/TLS PKCS #11 smartcard support.
 * Requires the presence of the PKCS#11 helper library (libpkcs11-helper)
 */
//#define POLARSSL_PKCS11_C

/**
 * \def POLARSSL_PKCS12_C
 *
 * Enable PKCS#12 PBE functions.
 * Adds algorithms for parsing PKCS#8 encrypted private keys
 *
 * Module:  library/pkcs12.c
 * Caller:  library/pkparse.c
 *
 * Requires: POLARSSL_ASN1_PARSE_C, POLARSSL_CIPHER_C, POLARSSL_MD_C
 * Can use:  POLARSSL_ARC4_C
 *
 * This module enables PKCS#12 functions.
 */
#define POLARSSL_PKCS12_C

/**
 * \def POLARSSL_PLATFORM_C
 *
 * Enable the platform abstraction layer that allows you to re-assign
 * functions like malloc(), free(), snprintf(), printf(), fprintf(), exit()
 *
 * Enabling POLARSSL_PLATFORM_C enables to use of POLARSSL_PLATFORM_XXX_ALT
 * or POLARSSL_PLATFORM_XXX_MACRO directives, allowing the functions mentioned
 * above to be specified at runtime or compile time respectively.
 *
 * Module:  library/platform.c
 * Caller:  Most other .c files
 *
 * This module enables abstraction of common (libc) functions.
 */
#define POLARSSL_PLATFORM_C

/**
 * \def POLARSSL_RIPEMD160_C
 *
 * Enable the RIPEMD-160 hash algorithm.
 *
 * Module:  library/ripemd160.c
 * Caller:  library/md.c
 *
 */
#define POLARSSL_RIPEMD160_C

/**
 * \def POLARSSL_RSA_C
 *
 * Enable the RSA public-key cryptosystem.
 *
 * Module:  library/rsa.c
 * Caller:  library/ssl_cli.c
 *          library/ssl_srv.c
 *          library/ssl_tls.c
 *          library/x509.c
 *
 * This module is used by the following key exchanges:
 *      RSA, DHE-RSA, ECDHE-RSA, RSA-PSK
 *
 * Requires: POLARSSL_BIGNUM_C, POLARSSL_OID_C
 */
#define POLARSSL_RSA_C

/**
 * \def POLARSSL_SHA1_C
 *
 * Enable the SHA1 cryptographic hash algorithm.
 *
 * Module:  library/sha1.c
 * Caller:  library/md.c
 *          library/ssl_cli.c
 *          library/ssl_srv.c
 *          library/ssl_tls.c
 *          library/x509write_crt.c
 *
 * This module is required for SSL/TLS and SHA1-signed certificates.
 */
#define POLARSSL_SHA1_C

/**
 * \def POLARSSL_SHA256_C
 *
 * Enable the SHA-224 and SHA-256 cryptographic hash algorithms.
 * (Used to be POLARSSL_SHA2_C)
 *
 * Module:  library/sha256.c
 * Caller:  library/entropy.c
 *          library/md.c
 *          library/ssl_cli.c
 *          library/ssl_srv.c
 *          library/ssl_tls.c
 *
 * This module adds support for SHA-224 and SHA-256.
 * This module is required for the SSL/TLS 1.2 PRF function.
 */
#define POLARSSL_SHA256_C

/**
 * \def POLARSSL_SHA512_C
 *
 * Enable the SHA-384 and SHA-512 cryptographic hash algorithms.
 * (Used to be POLARSSL_SHA4_C)
 *
 * Module:  library/sha512.c
 * Caller:  library/entropy.c
 *          library/md.c
 *          library/ssl_cli.c
 *          library/ssl_srv.c
 *
 * This module adds support for SHA-384 and SHA-512.
 */
#define POLARSSL_SHA512_C

/**
 * \def POLARSSL_SSL_CACHE_C
 *
 * Enable simple SSL cache implementation.
 *
 * Module:  library/ssl_cache.c
 * Caller:
 *
 * Requires: POLARSSL_SSL_CACHE_C
 */
#define POLARSSL_SSL_CACHE_C

/**
 * \def POLARSSL_SSL_CLI_C
 *
 * Enable the SSL/TLS client code.
 *
 * Module:  library/ssl_cli.c
 * Caller:
 *
 * Requires: POLARSSL_SSL_TLS_C
 *
 * This module is required for SSL/TLS client support.
 */
#define POLARSSL_SSL_CLI_C

/**
 * \def POLARSSL_SSL_SRV_C
 *
 * Enable the SSL/TLS server code.
 *
 * Module:  library/ssl_srv.c
 * Caller:
 *
 * Requires: POLARSSL_SSL_TLS_C
 *
 * This module is required for SSL/TLS server support.
 */
#define POLARSSL_SSL_SRV_C

/**
 * \def POLARSSL_SSL_TLS_C
 *
 * Enable the generic SSL/TLS code.
 *
 * Module:  library/ssl_tls.c
 * Caller:  library/ssl_cli.c
 *          library/ssl_srv.c
 *
 * Requires: POLARSSL_CIPHER_C, POLARSSL_MD_C
 *           and at least one of the POLARSSL_SSL_PROTO_* defines
 *
 * This module is required for SSL/TLS.
 */
#define POLARSSL_SSL_TLS_C

/**
 * \def POLARSSL_THREADING_C
 *
 * Enable the threading abstraction layer.
 * By default mbed TLS assumes it is used in a non-threaded environment or that
 * contexts are not shared between threads. If you do intend to use contexts
 * between threads, you will need to enable this layer to prevent race
 * conditions.
 *
 * Module:  library/threading.c
 *
 * This allows different threading implementations (self-implemented or
 * provided).
 *
 * You will have to enable either POLARSSL_THREADING_ALT or
 * POLARSSL_THREADING_PTHREAD.
 *
 * Enable this layer to allow use of mutexes within mbed TLS
 */
//#define POLARSSL_THREADING_C

/**
 * \def POLARSSL_TIMING_C
 *
 * Enable the portable timing interface.
 *
 * Module:  library/timing.c
 * Caller:  library/havege.c
 *
 * This module is used by the HAVEGE random number generator.
 */
#define POLARSSL_TIMING_C

/**
 * \def POLARSSL_VERSION_C
 *
 * Enable run-time version information.
 *
 * Module:  library/version.c
 *
 * This module provides run-time version information.
 */
#define POLARSSL_VERSION_C

/**
 * \def POLARSSL_X509_USE_C
 *
 * Enable X.509 core for using certificates.
 *
 * Module:  library/x509.c
 * Caller:  library/x509_crl.c
 *          library/x509_crt.c
 *          library/x509_csr.c
 *
 * Requires: POLARSSL_ASN1_PARSE_C, POLARSSL_BIGNUM_C, POLARSSL_OID_C,
 *           POLARSSL_PK_PARSE_C
 *
 * This module is required for the X.509 parsing modules.
 */
#define POLARSSL_X509_USE_C

/**
 * \def POLARSSL_X509_CRT_PARSE_C
 *
 * Enable X.509 certificate parsing.
 *
 * Module:  library/x509_crt.c
 * Caller:  library/ssl_cli.c
 *          library/ssl_srv.c
 *          library/ssl_tls.c
 *
 * Requires: POLARSSL_X509_USE_C
 *
 * This module is required for X.509 certificate parsing.
 */
#define POLARSSL_X509_CRT_PARSE_C

/**
 * \def POLARSSL_X509_CRL_PARSE_C
 *
 * Enable X.509 CRL parsing.
 *
 * Module:  library/x509_crl.c
 * Caller:  library/x509_crt.c
 *
 * Requires: POLARSSL_X509_USE_C
 *
 * This module is required for X.509 CRL parsing.
 */
#define POLARSSL_X509_CRL_PARSE_C

/**
 * \def POLARSSL_X509_CSR_PARSE_C
 *
 * Enable X.509 Certificate Signing Request (CSR) parsing.
 *
 * Module:  library/x509_csr.c
 * Caller:  library/x509_crt_write.c
 *
 * Requires: POLARSSL_X509_USE_C
 *
 * This module is used for reading X.509 certificate request.
 */
#define POLARSSL_X509_CSR_PARSE_C

/**
 * \def POLARSSL_X509_CREATE_C
 *
 * Enable X.509 core for creating certificates.
 *
 * Module:  library/x509_create.c
 *
 * Requires: POLARSSL_BIGNUM_C, POLARSSL_OID_C, POLARSSL_PK_WRITE_C
 *
 * This module is the basis for creating X.509 certificates and CSRs.
 */
#define POLARSSL_X509_CREATE_C

/**
 * \def POLARSSL_X509_CRT_WRITE_C
 *
 * Enable creating X.509 certificates.
 *
 * Module:  library/x509_crt_write.c
 *
 * Requires: POLARSSL_CREATE_C
 *
 * This module is required for X.509 certificate creation.
 */
#define POLARSSL_X509_CRT_WRITE_C

/**
 * \def POLARSSL_X509_CSR_WRITE_C
 *
 * Enable creating X.509 Certificate Signing Requests (CSR).
 *
 * Module:  library/x509_csr_write.c
 *
 * Requires: POLARSSL_CREATE_C
 *
 * This module is required for X.509 certificate request writing.
 */
#define POLARSSL_X509_CSR_WRITE_C

/**
 * \def POLARSSL_XTEA_C
 *
 * Enable the XTEA block cipher.
 *
 * Module:  library/xtea.c
 * Caller:
 */
#define POLARSSL_XTEA_C

/* \} name SECTION: mbed TLS modules */

/**
 * \name SECTION: Module configuration options
 *
 * This section allows for the setting of module specific sizes and
 * configuration options. The default values are already present in the
 * relevant header files and should suffice for the regular use cases.
 *
 * Our advice is to enable options and change their values here
 * only if you have a good reason and know the consequences.
 *
 * Please check the respective header file for documentation on these
 * parameters (to prevent duplicate documentation).
 * \{
 */

/* MPI / BIGNUM options */
//#define POLARSSL_MPI_WINDOW_SIZE            6 /**< Maximum windows size used. */
//#define POLARSSL_MPI_MAX_SIZE            1024 /**< Maximum number of bytes for usable MPIs. */

/* CTR_DRBG options */
//#define CTR_DRBG_ENTROPY_LEN               48 /**< Amount of entropy used per seed by default (48 with SHA-512, 32 with SHA-256) */
//#define CTR_DRBG_RESEED_INTERVAL        10000 /**< Interval before reseed is performed by default */
//#define CTR_DRBG_MAX_INPUT                256 /**< Maximum number of additional input bytes */
//#define CTR_DRBG_MAX_REQUEST             1024 /**< Maximum number of requested bytes per call */
//#define CTR_DRBG_MAX_SEED_INPUT           384 /**< Maximum size of (re)seed buffer */

/* HMAC_DRBG options */
//#define POLARSSL_HMAC_DRBG_RESEED_INTERVAL   10000 /**< Interval before reseed is performed by default */
//#define POLARSSL_HMAC_DRBG_MAX_INPUT           256 /**< Maximum number of additional input bytes */
//#define POLARSSL_HMAC_DRBG_MAX_REQUEST        1024 /**< Maximum number of requested bytes per call */
//#define POLARSSL_HMAC_DRBG_MAX_SEED_INPUT      384 /**< Maximum size of (re)seed buffer */

/* ECP options */
//#define POLARSSL_ECP_MAX_BITS             521 /**< Maximum bit size of groups */
//#define POLARSSL_ECP_WINDOW_SIZE            6 /**< Maximum window size used */
//#define POLARSSL_ECP_FIXED_POINT_OPTIM      1 /**< Enable fixed-point speed-up */

/* Entropy options */
//#define ENTROPY_MAX_SOURCES                20 /**< Maximum number of sources supported */
//#define ENTROPY_MAX_GATHER                128 /**< Maximum amount requested from entropy sources */

/* Memory buffer allocator options */
//#define POLARSSL_MEMORY_ALIGN_MULTIPLE      4 /**< Align on multiples of this value */

/* Platform options */
//#define POLARSSL_PLATFORM_STD_MEM_HDR   <stdlib.h> /**< Header to include if POLARSSL_PLATFORM_NO_STD_FUNCTIONS is defined. Don't define if no header is needed. */
//#define POLARSSL_PLATFORM_STD_MALLOC        malloc /**< Default allocator to use, can be undefined */
//#define POLARSSL_PLATFORM_STD_FREE            free /**< Default free to use, can be undefined */
//#define POLARSSL_PLATFORM_STD_EXIT            exit /**< Default exit to use, can be undefined */
//#define POLARSSL_PLATFORM_STD_FPRINTF      fprintf /**< Default fprintf to use, can be undefined */
//#define POLARSSL_PLATFORM_STD_PRINTF        printf /**< Default printf to use, can be undefined */
//#define POLARSSL_PLATFORM_STD_SNPRINTF    snprintf /**< Default snprintf to use, can be undefined */

/* To Use Function Macros POLARSSL_PLATFORM_C must be enabled */
/* POLARSSL_PLATFORM_XXX_MACRO and POLARSSL_PLATFORM_XXX_ALT cannot both be defined */
//#define POLARSSL_PLATFORM_MALLOC_MACRO        malloc /**< Default allocator macro to use, can be undefined */
//#define POLARSSL_PLATFORM_FREE_MACRO            free /**< Default free macro to use, can be undefined */
//#define POLARSSL_PLATFORM_EXIT_MACRO            exit /**< Default exit macro to use, can be undefined */
//#define POLARSSL_PLATFORM_FPRINTF_MACRO      fprintf /**< Default fprintf macro to use, can be undefined */
//#define POLARSSL_PLATFORM_PRINTF_MACRO        printf /**< Default printf macro to use, can be undefined */
//#define POLARSSL_PLATFORM_SNPRINTF_MACRO    snprintf /**< Default snprintf macro to use, can be undefined */

/* SSL Cache options */
//#define SSL_CACHE_DEFAULT_TIMEOUT       86400 /**< 1 day  */
//#define SSL_CACHE_DEFAULT_MAX_ENTRIES      50 /**< Maximum entries in cache */

/* SSL options */
//#define SSL_MAX_CONTENT_LEN             16384 /**< Size of the input / output buffer */
//#define SSL_DEFAULT_TICKET_LIFETIME     86400 /**< Lifetime of session tickets (if enabled) */
//#define POLARSSL_PSK_MAX_LEN               32 /**< Max size of TLS pre-shared keys, in bytes (default 256 bits) */

/**
 * Complete list of ciphersuites to use, in order of preference.
 *
 * \warning No dependency checking is done on that field! This option can only
 * be used to restrict the set of available ciphersuites. It is your
 * responsibility to make sure the needed modules are active.
 *
 * Use this to save a few hundred bytes of ROM (default ordering of all
 * available ciphersuites) and a few to a few hundred bytes of RAM.
 *
 * The value below is only an example, not the default.
 */
//#define SSL_CIPHERSUITES TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256

/* Debug options */
//#define POLARSSL_DEBUG_DFL_MODE POLARSSL_DEBUG_LOG_FULL /**< Default log: Full or Raw */

/* X509 options */
//#define POLARSSL_X509_MAX_INTERMEDIATE_CA   8   /**< Maximum number of intermediate CAs in a verification chain. */

/* \} name SECTION: Module configuration options */



#endif /* POLARSSL_CONFIG_H */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/embedthis.h ************/


/*
    embedthis.h - MbedTLS customization Header

    Pak will blend this header with others to create a single master header "mbedtls.h".

    The order of headers is:

        me.h            Generated by MakeMe
        config.h        Standard configuration for MbedTLS
        embedthis.h     This file
        check_config.h  MbedTLS config validation
        ***             Rest of MbedTLS headers
 */

#undef POLARSSL_SELF_TEST
#define POLARSSL_REMOVE_ARC4_CIPHERSUITES

#if ME_COM_MPR || ME_MPR_PRODUCT || ME_MULTITHREAD
    #define POLARSSL_THREADING_C
    #define POLARSSL_THREADING_ALT
    typedef struct MprMutex* threading_mutex_t;
#endif

#if ME_DEBUG
    #define POLARSSL_SSL_DEBUG_ALL
    #define POLARSSL_DEBUG_C
#endif
#if ME_CPU_ARCH == ME_CPU_X86 || ME_CPU_ARCH == ME_CPU_X64
    #define POLARSSL_HAVE_SSE2
#endif
#if VXWORKS
    #undef POLARSSL_HAVE_TIME
#endif

/*
    Map MakeMe configuration into MbedTLS defines
 */
#if ME_MBEDTLS_COMPACT
    #undef POLARSSL_BLOWFISH_C
    #undef POLARSSL_CAMELLIA_C
    #undef POLARSSL_DES_C
    #undef POLARSSL_PADLOCK_C
    #undef POLARSSL_KEY_EXCHANGE_PSK_ENABLED
    #undef POLARSSL_KEY_EXCHANGE_RSA_PSK_ENABLED
    #undef POLARSSL_KEY_EXCHANGE_DHE_PSK_ENABLED
    #undef POLARSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED
    #undef POLARSSL_ARC4_C
    #undef POLARSSL_AES_ROM_TABLES
    #undef POLARSSL_XTEA_C
#endif

/*
    If mbedtls.NAME is defined, then override the MbedTLS definition from config.h
 */
#if ME_MBEDTLS_BLOWFISH
    #define POLARSSL_BLOWFISH_C
#elif ME_MBEDTLS_BLOWFISH == 0
    #undef POLARSSL_BLOWFISH_C
#endif

#if ME_MBEDTLS_CAMELLIA
    #define POLARSSL_CAMELLIA_C
#elif ME_MBEDTLS_CAMELLIA == 0
    #undef POLARSSL_CAMELLIA_C
#endif

#if ME_MBEDTLS_DES
    #define POLARSSL_DES_C
#elif ME_MBEDTLS_DES == 0
    #undef POLARSSL_DES_C
#endif

#if ME_MBEDTLS_PADLOCK
    #define POLARSSL_PADLOCK_C
#elif ME_MBEDTLS_PADLOCK == 0
    #undef POLARSSL_PADLOCK_C
#endif

#if ME_MBEDTLS_PSK
    #define POLARSSL_KEY_EXCHANGE_PSK_ENABLED
    #define POLARSSL_KEY_EXCHANGE_RSA_PSK_ENABLED
    #define POLARSSL_KEY_EXCHANGE_DHE_PSK_ENABLED
    #define POLARSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED
#elif ME_MBEDTLS_PSK == 0
    #undef POLARSSL_KEY_EXCHANGE_PSK_ENABLED
    #undef POLARSSL_KEY_EXCHANGE_RSA_PSK_ENABLED
    #undef POLARSSL_KEY_EXCHANGE_DHE_PSK_ENABLED
    #undef POLARSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED
#endif

#if ME_MBEDTLS_ARC4
    #define POLARSSL_ARC4_C
#elif ME_MBEDTLS_ARC4 == 0
    #undef POLARSSL_ARC4_C
#endif

#if ME_MBEDTLS_AES_ROM_TABLES
    #define POLARSSL_AES_ROM_TABLES
#elif ME_MBEDTLS_AES_ROM_TABLES == 0
    #undef POLARSSL_AES_ROM_TABLES
#endif

#if ME_MBEDTLS_XTEA
    #define POLARSSL_XTEA_C
#elif ME_MBEDTLS_XTEA == 0
    #undef POLARSSL_XTEA_C
#endif



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/check_config.h ************/


/**
 * \file check_config.h
 *
 * \brief Consistency checks for configuration options
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * It is recommended to include this file from your config.h
 * in order to catch dependency issues early.
 */

#ifndef POLARSSL_CHECK_CONFIG_H
#define POLARSSL_CHECK_CONFIG_H

#if defined(POLARSSL_DEPRECATED_WARNING) && \
    !defined(__GNUC__) && !defined(__clang__)
#error "POLARSSL_DEPRECATED_WARNING only works with GCC and Clang"
#endif

#if defined(POLARSSL_NET_C) && !defined(POLARSSL_HAVE_IPV6)
#if defined(POLARSSL_DEPRECATED_WARNING)
#warning "Using POLARSSL_NET_C without POLARSSL_HAVE_IPV6 is deprecated"
#endif
#if defined(POLARSSL_DEPRECATED_REMOVED)
#define POLARSSL_HAVE_IPV6
#endif
#endif /* POLARSSL_NET_C && !POLARSSL_HAVE_IPV6 */

#if defined(POLARSSL_AESNI_C) && !defined(POLARSSL_HAVE_ASM)
#error "POLARSSL_AESNI_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_CERTS_C) && !defined(POLARSSL_PEM_PARSE_C)
#error "POLARSSL_CERTS_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_CTR_DRBG_C) && !defined(POLARSSL_AES_C)
#error "POLARSSL_CTR_DRBG_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_DHM_C) && !defined(POLARSSL_BIGNUM_C)
#error "POLARSSL_DHM_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_ECDH_C) && !defined(POLARSSL_ECP_C)
#error "POLARSSL_ECDH_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_ECDSA_C) &&            \
    ( !defined(POLARSSL_ECP_C) ||           \
      !defined(POLARSSL_ASN1_PARSE_C) ||    \
      !defined(POLARSSL_ASN1_WRITE_C) )
#error "POLARSSL_ECDSA_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_ECDSA_DETERMINISTIC) && !defined(POLARSSL_HMAC_DRBG_C)
#error "POLARSSL_ECDSA_DETERMINISTIC defined, but not all prerequisites"
#endif

#if defined(POLARSSL_ECP_C) && ( !defined(POLARSSL_BIGNUM_C) || (   \
    !defined(POLARSSL_ECP_DP_SECP192R1_ENABLED) &&                  \
    !defined(POLARSSL_ECP_DP_SECP224R1_ENABLED) &&                  \
    !defined(POLARSSL_ECP_DP_SECP256R1_ENABLED) &&                  \
    !defined(POLARSSL_ECP_DP_SECP384R1_ENABLED) &&                  \
    !defined(POLARSSL_ECP_DP_SECP521R1_ENABLED) &&                  \
    !defined(POLARSSL_ECP_DP_BP256R1_ENABLED)   &&                  \
    !defined(POLARSSL_ECP_DP_BP384R1_ENABLED)   &&                  \
    !defined(POLARSSL_ECP_DP_BP512R1_ENABLED)   &&                  \
    !defined(POLARSSL_ECP_DP_SECP192K1_ENABLED) &&                  \
    !defined(POLARSSL_ECP_DP_SECP224K1_ENABLED) &&                  \
    !defined(POLARSSL_ECP_DP_SECP256K1_ENABLED) ) )
#error "POLARSSL_ECP_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_ENTROPY_C) && (!defined(POLARSSL_SHA512_C) &&      \
                                    !defined(POLARSSL_SHA256_C))
#error "POLARSSL_ENTROPY_C defined, but not all prerequisites"
#endif
#if defined(POLARSSL_ENTROPY_C) && defined(POLARSSL_SHA512_C) &&         \
    defined(CTR_DRBG_ENTROPY_LEN) && (CTR_DRBG_ENTROPY_LEN > 64)
#error "CTR_DRBG_ENTROPY_LEN value too high"
#endif
#if defined(POLARSSL_ENTROPY_C) &&                                            \
    ( !defined(POLARSSL_SHA512_C) || defined(POLARSSL_ENTROPY_FORCE_SHA256) ) \
    && defined(CTR_DRBG_ENTROPY_LEN) && (CTR_DRBG_ENTROPY_LEN > 32)
#error "CTR_DRBG_ENTROPY_LEN value too high"
#endif
#if defined(POLARSSL_ENTROPY_C) && \
    defined(POLARSSL_ENTROPY_FORCE_SHA256) && !defined(POLARSSL_SHA256_C)
#error "POLARSSL_ENTROPY_FORCE_SHA256 defined, but not all prerequisites"
#endif

#if defined(POLARSSL_GCM_C) && (                                        \
        !defined(POLARSSL_AES_C) && !defined(POLARSSL_CAMELLIA_C) )
#error "POLARSSL_GCM_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_HAVEGE_C) && !defined(POLARSSL_TIMING_C)
#error "POLARSSL_HAVEGE_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_HMAC_DRBG) && !defined(POLARSSL_MD_C)
#error "POLARSSL_HMAC_DRBG_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_KEY_EXCHANGE_ECDH_ECDSA_ENABLED) &&                 \
    ( !defined(POLARSSL_ECDH_C) || !defined(POLARSSL_X509_CRT_PARSE_C) )
#error "POLARSSL_KEY_EXCHANGE_ECDH_ECDSA_ENABLED defined, but not all prerequisites"
#endif

#if defined(POLARSSL_KEY_EXCHANGE_ECDH_RSA_ENABLED) &&                 \
    ( !defined(POLARSSL_ECDH_C) || !defined(POLARSSL_X509_CRT_PARSE_C) )
#error "POLARSSL_KEY_EXCHANGE_ECDH_RSA_ENABLED defined, but not all prerequisites"
#endif

#if defined(POLARSSL_KEY_EXCHANGE_DHE_PSK_ENABLED) && !defined(POLARSSL_DHM_C)
#error "POLARSSL_KEY_EXCHANGE_DHE_PSK_ENABLED defined, but not all prerequisites"
#endif

#if defined(POLARSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED) &&                     \
    !defined(POLARSSL_ECDH_C)
#error "POLARSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED defined, but not all prerequisites"
#endif

#if defined(POLARSSL_KEY_EXCHANGE_DHE_RSA_ENABLED) &&                   \
    ( !defined(POLARSSL_DHM_C) || !defined(POLARSSL_RSA_C) ||           \
      !defined(POLARSSL_X509_CRT_PARSE_C) || !defined(POLARSSL_PKCS1_V15) )
#error "POLARSSL_KEY_EXCHANGE_DHE_RSA_ENABLED defined, but not all prerequisites"
#endif

#if defined(POLARSSL_KEY_EXCHANGE_ECDHE_RSA_ENABLED) &&                 \
    ( !defined(POLARSSL_ECDH_C) || !defined(POLARSSL_RSA_C) ||          \
      !defined(POLARSSL_X509_CRT_PARSE_C) || !defined(POLARSSL_PKCS1_V15) )
#error "POLARSSL_KEY_EXCHANGE_ECDHE_RSA_ENABLED defined, but not all prerequisites"
#endif

#if defined(POLARSSL_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED) &&                 \
    ( !defined(POLARSSL_ECDH_C) || !defined(POLARSSL_ECDSA_C) ||          \
      !defined(POLARSSL_X509_CRT_PARSE_C) )
#error "POLARSSL_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED defined, but not all prerequisites"
#endif

#if defined(POLARSSL_KEY_EXCHANGE_RSA_PSK_ENABLED) &&                   \
    ( !defined(POLARSSL_RSA_C) || !defined(POLARSSL_X509_CRT_PARSE_C) || \
      !defined(POLARSSL_PKCS1_V15) )
#error "POLARSSL_KEY_EXCHANGE_RSA_PSK_ENABLED defined, but not all prerequisites"
#endif

#if defined(POLARSSL_KEY_EXCHANGE_RSA_ENABLED) &&                       \
    ( !defined(POLARSSL_RSA_C) || !defined(POLARSSL_X509_CRT_PARSE_C) || \
      !defined(POLARSSL_PKCS1_V15) )
#error "POLARSSL_KEY_EXCHANGE_RSA_ENABLED defined, but not all prerequisites"
#endif

#if defined(POLARSSL_MEMORY_C) && !defined(POLARSSL_PLATFORM_C)
#error "POLARSSL_MEMORY_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_MEMORY_BUFFER_ALLOC_C) &&                          \
    ( !defined(POLARSSL_PLATFORM_C) || !defined(POLARSSL_PLATFORM_MEMORY) )
#error "POLARSSL_MEMORY_BUFFER_ALLOC_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PADLOCK_C) && !defined(POLARSSL_HAVE_ASM)
#error "POLARSSL_PADLOCK_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PBKDF2_C) && !defined(POLARSSL_MD_C)
#error "POLARSSL_PBKDF2_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PEM_PARSE_C) && !defined(POLARSSL_BASE64_C)
#error "POLARSSL_PEM_PARSE_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PEM_WRITE_C) && !defined(POLARSSL_BASE64_C)
#error "POLARSSL_PEM_WRITE_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PK_C) && \
    ( !defined(POLARSSL_RSA_C) && !defined(POLARSSL_ECP_C) )
#error "POLARSSL_PK_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PK_PARSE_C) && !defined(POLARSSL_PK_C)
#error "POLARSSL_PK_PARSE_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PK_WRITE_C) && !defined(POLARSSL_PK_C)
#error "POLARSSL_PK_WRITE_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PKCS11_C) && !defined(POLARSSL_PK_C)
#error "POLARSSL_PKCS11_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_EXIT_ALT) && !defined(POLARSSL_PLATFORM_C)
#error "POLARSSL_PLATFORM_EXIT_ALT defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_EXIT_MACRO) && !defined(POLARSSL_PLATFORM_C)
#error "POLARSSL_PLATFORM_EXIT_MACRO defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_EXIT_MACRO) &&\
    ( defined(POLARSSL_PLATFORM_STD_EXIT) ||\
        defined(POLARSSL_PLATFORM_EXIT_ALT) )
#error "POLARSSL_PLATFORM_EXIT_MACRO and POLARSSL_PLATFORM_STD_EXIT/POLARSSL_PLATFORM_EXIT_ALT cannot be defined simultaneously"
#endif

#if defined(POLARSSL_PLATFORM_FPRINTF_ALT) && !defined(POLARSSL_PLATFORM_C)
#error "POLARSSL_PLATFORM_FPRINTF_ALT defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_FPRINTF_MACRO) && !defined(POLARSSL_PLATFORM_C)
#error "POLARSSL_PLATFORM_FPRINTF_MACRO defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_FPRINTF_MACRO) &&\
    ( defined(POLARSSL_PLATFORM_STD_FPRINTF) ||\
        defined(POLARSSL_PLATFORM_FPRINTF_ALT) )
#error "POLARSSL_PLATFORM_FPRINTF_MACRO and POLARSSL_PLATFORM_STD_FPRINTF/POLARSSL_PLATFORM_FPRINTF_ALT cannot be defined simultaneously"
#endif

#if defined(POLARSSL_PLATFORM_FREE_MACRO) &&\
    ( !defined(POLARSSL_PLATFORM_C) || !defined(POLARSSL_PLATFORM_MEMORY) )
#error "POLARSSL_PLATFORM_FREE_MACRO defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_FREE_MACRO) &&\
    defined(POLARSSL_PLATFORM_STD_FREE)
#error "POLARSSL_PLATFORM_FREE_MACRO and POLARSSL_PLATFORM_STD_FREE cannot be defined simultaneously"
#endif

#if defined(POLARSSL_PLATFORM_FREE_MACRO) && !defined(POLARSSL_PLATFORM_MALLOC_MACRO)
#error "POLARSSL_PLATFORM_MALLOC_MACRO must be defined if POLARSSL_PLATFORM_FREE_MACRO is"
#endif

#if defined(POLARSSL_PLATFORM_MALLOC_MACRO) &&\
    ( !defined(POLARSSL_PLATFORM_C) || !defined(POLARSSL_PLATFORM_MEMORY) )
#error "POLARSSL_PLATFORM_MALLOC_MACRO defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_MALLOC_MACRO) &&\
    defined(POLARSSL_PLATFORM_STD_MALLOC)
#error "POLARSSL_PLATFORM_MALLOC_MACRO and POLARSSL_PLATFORM_STD_MALLOC cannot be defined simultaneously"
#endif

#if defined(POLARSSL_PLATFORM_MALLOC_MACRO) && !defined(POLARSSL_PLATFORM_FREE_MACRO)
#error "POLARSSL_PLATFORM_FREE_MACRO must be defined if POLARSSL_PLATFORM_MALLOC_MACRO is"
#endif

#if defined(POLARSSL_PLATFORM_MEMORY) && !defined(POLARSSL_PLATFORM_C)
#error "POLARSSL_PLATFORM_MEMORY defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_PRINTF_ALT) && !defined(POLARSSL_PLATFORM_C)
#error "POLARSSL_PLATFORM_PRINTF_ALT defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_PRINTF_MACRO) && !defined(POLARSSL_PLATFORM_C)
#error "POLARSSL_PLATFORM_PRINTF_MACRO defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_PRINTF_MACRO) &&\
    ( defined(POLARSSL_PLATFORM_STD_PRINTF) ||\
        defined(POLARSSL_PLATFORM_PRINTF_ALT) )
#error "POLARSSL_PLATFORM_PRINTF_MACRO and POLARSSL_PLATFORM_STD_PRINTF/POLARSSL_PLATFORM_PRINTF_ALT cannot be defined simultaneously"
#endif

#if defined(POLARSSL_PLATFORM_SNPRINTF_ALT) && !defined(POLARSSL_PLATFORM_C)
#error "POLARSSL_PLATFORM_SNPRINTF_ALT defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_SNPRINTF_ALT) && ( defined(_WIN32)\
    && !defined(EFIX64) && !defined(EFI32) )
#error "POLARSSL_PLATFORM_SNPRINTF_ALT defined but not available on Windows"
#endif

#if defined(POLARSSL_PLATFORM_SNPRINTF_MACRO) && !defined(POLARSSL_PLATFORM_C)
#error "POLARSSL_PLATFORM_SNPRINTF_MACRO defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_SNPRINTF_MACRO) &&\
    ( defined(POLARSSL_PLATFORM_STD_SNPRINTF) ||\
        defined(POLARSSL_PLATFORM_SNPRINTF_ALT) )
#error "POLARSSL_PLATFORM_SNPRINTF_MACRO and POLARSSL_PLATFORM_STD_SNPRINTF/POLARSSL_PLATFORM_SNPRINTF_ALT cannot be defined simultaneously"
#endif

#if defined(POLARSSL_PLATFORM_STD_MEM_HDR) &&\
    !defined(POLARSSL_PLATFORM_NO_STD_FUNCTIONS)
#error "POLARSSL_PLATFORM_STD_MEM_HDR defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_STD_MALLOC) && !defined(POLARSSL_PLATFORM_MEMORY)
#error "POLARSSL_PLATFORM_STD_MALLOC defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_STD_MALLOC) && !defined(POLARSSL_PLATFORM_MEMORY)
#error "POLARSSL_PLATFORM_STD_MALLOC defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_STD_FREE) && !defined(POLARSSL_PLATFORM_MEMORY)
#error "POLARSSL_PLATFORM_STD_FREE defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_STD_EXIT) &&\
    !defined(POLARSSL_PLATFORM_EXIT_ALT)
#error "POLARSSL_PLATFORM_STD_EXIT defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_STD_FPRINTF) &&\
    !defined(POLARSSL_PLATFORM_FPRINTF_ALT)
#error "POLARSSL_PLATFORM_STD_FPRINTF defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_STD_PRINTF) &&\
    !defined(POLARSSL_PLATFORM_PRINTF_ALT)
#error "POLARSSL_PLATFORM_STD_PRINTF defined, but not all prerequisites"
#endif

#if defined(POLARSSL_PLATFORM_STD_SNPRINTF) &&\
    !defined(POLARSSL_PLATFORM_SNPRINTF_ALT)
#error "POLARSSL_PLATFORM_STD_SNPRINTF defined, but not all prerequisites"
#endif

#if defined(POLARSSL_RSA_C) && ( !defined(POLARSSL_BIGNUM_C) ||         \
    !defined(POLARSSL_OID_C) )
#error "POLARSSL_RSA_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_X509_RSASSA_PSS_SUPPORT) &&                        \
    ( !defined(POLARSSL_RSA_C) || !defined(POLARSSL_PKCS1_V21) )
#error "POLARSSL_X509_RSASSA_PSS_SUPPORT defined, but not all prerequisites"
#endif

#if defined(POLARSSL_SSL_PROTO_SSL3) && ( !defined(POLARSSL_MD5_C) ||     \
    !defined(POLARSSL_SHA1_C) )
#error "POLARSSL_SSL_PROTO_SSL3 defined, but not all prerequisites"
#endif

#if defined(POLARSSL_SSL_PROTO_TLS1) && ( !defined(POLARSSL_MD5_C) ||     \
    !defined(POLARSSL_SHA1_C) )
#error "POLARSSL_SSL_PROTO_TLS1 defined, but not all prerequisites"
#endif

#if defined(POLARSSL_SSL_PROTO_TLS1_1) && ( !defined(POLARSSL_MD5_C) ||     \
    !defined(POLARSSL_SHA1_C) )
#error "POLARSSL_SSL_PROTO_TLS1_1 defined, but not all prerequisites"
#endif

#if defined(POLARSSL_SSL_PROTO_TLS1_2) && ( !defined(POLARSSL_SHA1_C) &&     \
    !defined(POLARSSL_SHA256_C) && !defined(POLARSSL_SHA512_C) )
#error "POLARSSL_SSL_PROTO_TLS1_2 defined, but not all prerequisites"
#endif

#if defined(POLARSSL_SSL_CLI_C) && !defined(POLARSSL_SSL_TLS_C)
#error "POLARSSL_SSL_CLI_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_SSL_TLS_C) && ( !defined(POLARSSL_CIPHER_C) ||     \
    !defined(POLARSSL_MD_C) )
#error "POLARSSL_SSL_TLS_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_SSL_SRV_C) && !defined(POLARSSL_SSL_TLS_C)
#error "POLARSSL_SSL_SRV_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_SSL_TLS_C) && (!defined(POLARSSL_SSL_PROTO_SSL3) && \
    !defined(POLARSSL_SSL_PROTO_TLS1) && !defined(POLARSSL_SSL_PROTO_TLS1_1) && \
    !defined(POLARSSL_SSL_PROTO_TLS1_2))
#error "POLARSSL_SSL_TLS_C defined, but no protocols are active"
#endif

#if defined(POLARSSL_SSL_TLS_C) && (defined(POLARSSL_SSL_PROTO_SSL3) && \
    defined(POLARSSL_SSL_PROTO_TLS1_1) && !defined(POLARSSL_SSL_PROTO_TLS1))
#error "Illegal protocol selection"
#endif

#if defined(POLARSSL_SSL_TLS_C) && (defined(POLARSSL_SSL_PROTO_TLS1) && \
    defined(POLARSSL_SSL_PROTO_TLS1_2) && !defined(POLARSSL_SSL_PROTO_TLS1_1))
#error "Illegal protocol selection"
#endif

#if defined(POLARSSL_SSL_TLS_C) && (defined(POLARSSL_SSL_PROTO_SSL3) && \
    defined(POLARSSL_SSL_PROTO_TLS1_2) && (!defined(POLARSSL_SSL_PROTO_TLS1) || \
    !defined(POLARSSL_SSL_PROTO_TLS1_1)))
#error "Illegal protocol selection"
#endif

#if defined(POLARSSL_SSL_ENCRYPT_THEN_MAC) &&   \
    !defined(POLARSSL_SSL_PROTO_TLS1)   &&      \
    !defined(POLARSSL_SSL_PROTO_TLS1_1) &&      \
    !defined(POLARSSL_SSL_PROTO_TLS1_2)
#error "POLARSSL_SSL_ENCRYPT_THEN_MAC defined, but not all prerequsites"
#endif

#if defined(POLARSSL_SSL_EXTENDED_MASTER_SECRET) && \
    !defined(POLARSSL_SSL_PROTO_TLS1)   &&          \
    !defined(POLARSSL_SSL_PROTO_TLS1_1) &&          \
    !defined(POLARSSL_SSL_PROTO_TLS1_2)
#error "POLARSSL_SSL_EXTENDED_MASTER_SECRET defined, but not all prerequsites"
#endif

#if defined(POLARSSL_SSL_SESSION_TICKETS) && defined(POLARSSL_SSL_TLS_C) && \
    ( !defined(POLARSSL_AES_C) || !defined(POLARSSL_SHA256_C) ||            \
      !defined(POLARSSL_CIPHER_MODE_CBC) )
#error "POLARSSL_SSL_SESSION_TICKETS_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_SSL_CBC_RECORD_SPLITTING) && \
    !defined(POLARSSL_SSL_PROTO_SSL3) && !defined(POLARSSL_SSL_PROTO_TLS1)
#error "POLARSSL_SSL_CBC_RECORD_SPLITTING defined, but not all prerequisites"
#endif

#if defined(POLARSSL_SSL_SERVER_NAME_INDICATION) && \
        !defined(POLARSSL_X509_CRT_PARSE_C)
#error "POLARSSL_SSL_SERVER_NAME_INDICATION defined, but not all prerequisites"
#endif

#if defined(POLARSSL_THREADING_PTHREAD)
#if !defined(POLARSSL_THREADING_C) || defined(POLARSSL_THREADING_IMPL)
#error "POLARSSL_THREADING_PTHREAD defined, but not all prerequisites"
#endif
#define POLARSSL_THREADING_IMPL
#endif

#if defined(POLARSSL_THREADING_ALT)
#if !defined(POLARSSL_THREADING_C) || defined(POLARSSL_THREADING_IMPL)
#error "POLARSSL_THREADING_ALT defined, but not all prerequisites"
#endif
#define POLARSSL_THREADING_IMPL
#endif

#if defined(POLARSSL_THREADING_C) && !defined(POLARSSL_THREADING_IMPL)
#error "POLARSSL_THREADING_C defined, single threading implementation required"
#endif
#undef POLARSSL_THREADING_IMPL

#if defined(POLARSSL_VERSION_FEATURES) && !defined(POLARSSL_VERSION_C)
#error "POLARSSL_VERSION_FEATURES defined, but not all prerequisites"
#endif

#if defined(POLARSSL_X509_USE_C) && ( !defined(POLARSSL_BIGNUM_C) ||  \
    !defined(POLARSSL_OID_C) || !defined(POLARSSL_ASN1_PARSE_C) ||      \
    !defined(POLARSSL_PK_PARSE_C) )
#error "POLARSSL_X509_USE_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_X509_CREATE_C) && ( !defined(POLARSSL_BIGNUM_C) ||  \
    !defined(POLARSSL_OID_C) || !defined(POLARSSL_ASN1_WRITE_C) ||       \
    !defined(POLARSSL_PK_WRITE_C) )
#error "POLARSSL_X509_CREATE_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_X509_CRT_PARSE_C) && ( !defined(POLARSSL_X509_USE_C) )
#error "POLARSSL_X509_CRT_PARSE_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_X509_CRL_PARSE_C) && ( !defined(POLARSSL_X509_USE_C) )
#error "POLARSSL_X509_CRL_PARSE_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_X509_CSR_PARSE_C) && ( !defined(POLARSSL_X509_USE_C) )
#error "POLARSSL_X509_CSR_PARSE_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_X509_CRT_WRITE_C) && ( !defined(POLARSSL_X509_CREATE_C) )
#error "POLARSSL_X509_CRT_WRITE_C defined, but not all prerequisites"
#endif

#if defined(POLARSSL_X509_CSR_WRITE_C) && ( !defined(POLARSSL_X509_CREATE_C) )
#error "POLARSSL_X509_CSR_WRITE_C defined, but not all prerequisites"
#endif

#endif /* POLARSSL_CHECK_CONFIG_H */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/platform.h ************/


/**
 * \file platform.h
 *
 * \brief mbed TLS Platform abstraction layer
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_PLATFORM_H
#define POLARSSL_PLATFORM_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

/* Temporary compatibility hack for to keep MEMORY_C working */
#if defined(POLARSSL_MEMORY_C) && !defined(POLARSSL_PLATFORM_MEMORY)
#define POLARSSL_PLATFORM_MEMORY
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \name SECTION: Module settings
 *
 * The configuration options you can set for this module are in this section.
 * Either change them in config.h or define them on the compiler command line.
 * \{
 */

#if !defined(POLARSSL_PLATFORM_NO_STD_FUNCTIONS)
#include <stdio.h>
#include <stdlib.h>
#if !defined(POLARSSL_PLATFORM_STD_SNPRINTF)
#define POLARSSL_PLATFORM_STD_SNPRINTF   snprintf /**< Default snprintf to use  */
#endif
#if !defined(POLARSSL_PLATFORM_STD_PRINTF)
#define POLARSSL_PLATFORM_STD_PRINTF   printf /**< Default printf to use  */
#endif
#if !defined(POLARSSL_PLATFORM_STD_FPRINTF)
#define POLARSSL_PLATFORM_STD_FPRINTF fprintf /**< Default fprintf to use */
#endif
#if !defined(POLARSSL_PLATFORM_STD_MALLOC)
#define POLARSSL_PLATFORM_STD_MALLOC   malloc /**< Default allocator to use */
#endif
#if !defined(POLARSSL_PLATFORM_STD_FREE)
#define POLARSSL_PLATFORM_STD_FREE       free /**< Default free to use */
#endif
#if !defined(POLARSSL_PLATFORM_STD_EXIT)
#define POLARSSL_PLATFORM_STD_EXIT      exit /**< Default free to use */
#endif
#else /* POLARSSL_PLATFORM_NO_STD_FUNCTIONS */
#if defined(POLARSSL_PLATFORM_STD_MEM_HDR)
#include POLARSSL_PLATFORM_STD_MEM_HDR
#endif
#endif /* POLARSSL_PLATFORM_NO_STD_FUNCTIONS */

/* \} name SECTION: Module settings */

/*
 * The function pointers for malloc and free
 */
#if defined(POLARSSL_PLATFORM_MEMORY)
#if defined(POLARSSL_PLATFORM_FREE_MACRO) && \
    defined(POLARSSL_PLATFORM_MALLOC_MACRO)
#define polarssl_free       POLARSSL_PLATFORM_FREE_MACRO
#define polarssl_malloc     POLARSSL_PLATFORM_MALLOC_MACRO
#else
/* For size_t */
#include <stddef.h>
extern void * (*polarssl_malloc)( size_t len );
extern void (*polarssl_free)( void *ptr );

/**
 * \brief   Set your own memory implementation function pointers
 *
 * \param malloc_func   the malloc function implementation
 * \param free_func     the free function implementation
 *
 * \return              0 if successful
 */
int platform_set_malloc_free( void * (*malloc_func)( size_t ),
                              void (*free_func)( void * ) );
#endif /* POLARSSL_PLATFORM_FREE_MACRO && POLARSSL_PLATFORM_MALLOC_MACRO */
#else /* !POLARSSL_PLATFORM_MEMORY */
#define polarssl_free       free
#define polarssl_malloc     malloc
#endif /* POLARSSL_PLATFORM_MEMORY && !POLARSSL_PLATFORM_{FREE,MALLOC}_MACRO */

/*
 * The function pointers for fprintf
 */
#if defined(POLARSSL_PLATFORM_FPRINTF_ALT)
/* We need FILE * */
#include <stdio.h>
extern int (*polarssl_fprintf)( FILE *stream, const char *format, ... );

/**
 * \brief   Set your own fprintf function pointer
 *
 * \param fprintf_func   the fprintf function implementation
 *
 * \return              0
 */
int platform_set_fprintf( int (*fprintf_func)( FILE *stream, const char *,
                                               ... ) );
#else
#if defined(POLARSSL_PLATFORM_FPRINTF_MACRO)
#define polarssl_fprintf    POLARSSL_PLATFORM_FPRINTF_MACRO
#else
#define polarssl_fprintf    fprintf
#endif /* POLARSSL_PLATFORM_FPRINTF_MACRO */
#endif /* POLARSSL_PLATFORM_FPRINTF_ALT */

/*
 * The function pointers for printf
 */
#if defined(POLARSSL_PLATFORM_PRINTF_ALT)
extern int (*polarssl_printf)( const char *format, ... );

/**
 * \brief   Set your own printf function pointer
 *
 * \param printf_func   the printf function implementation
 *
 * \return              0
 */
int platform_set_printf( int (*printf_func)( const char *, ... ) );
#else /* !POLARSSL_PLATFORM_PRINTF_ALT */
#if defined(POLARSSL_PLATFORM_PRINTF_MACRO)
#define polarssl_printf     POLARSSL_PLATFORM_PRINTF_MACRO
#else
#define polarssl_printf     printf
#endif /* POLARSSL_PLATFORM_PRINTF_MACRO */
#endif /* POLARSSL_PLATFORM_PRINTF_ALT */

/*
 * The function pointers for snprintf
 */
#if defined(POLARSSL_PLATFORM_SNPRINTF_ALT)
extern int (*polarssl_snprintf)( char * s, size_t n, const char * format, ... );

/**
 * \brief   Set your own snprintf function pointer
 *
 * \param snprintf_func   the snprintf function implementation
 *
 * \return              0
 */
int platform_set_snprintf( int (*snprintf_func)( char * s, size_t n,
                                                 const char * format, ... ) );
#else /* POLARSSL_PLATFORM_SNPRINTF_ALT */
#if defined(POLARSSL_PLATFORM_SNPRINTF_MACRO)
#define polarssl_snprintf   POLARSSL_PLATFORM_SNPRINTF_MACRO
#else
#define polarssl_snprintf   snprintf
#endif /* POLARSSL_PLATFORM_SNPRINTF_MACRO */
#endif /* POLARSSL_PLATFORM_SNPRINTF_ALT */

/*
 * The function pointers for exit
 */
#if defined(POLARSSL_PLATFORM_EXIT_ALT)
extern void (*polarssl_exit)( int status );

/**
 * \brief   Set your own exit function pointer
 *
 * \param exit_func   the exit function implementation
 *
 * \return              0
 */
int platform_set_exit( void (*exit_func)( int status ) );
#else
#if defined(POLARSSL_PLATFORM_EXIT_MACRO)
#define polarssl_exit   POLARSSL_PLATFORM_EXIT_MACRO
#else
#define polarssl_exit   exit
#endif /* POLARSSL_PLATFORM_EXIT_MACRO */
#endif /* POLARSSL_PLATFORM_EXIT_ALT */

#ifdef __cplusplus
}
#endif

#endif /* platform.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/threading.h ************/


/**
 * \file threading.h
 *
 * \brief Threading abstraction layer
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_THREADING_H
#define POLARSSL_THREADING_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POLARSSL_ERR_THREADING_FEATURE_UNAVAILABLE         -0x001A  /**< The selected feature is not available. */
#define POLARSSL_ERR_THREADING_BAD_INPUT_DATA              -0x001C  /**< Bad input parameters to function. */
#define POLARSSL_ERR_THREADING_MUTEX_ERROR                 -0x001E  /**< Locking / unlocking / free failed with error code. */

#if defined(POLARSSL_THREADING_PTHREAD)
#include <pthread.h>
typedef pthread_mutex_t threading_mutex_t;
#endif

#if defined(POLARSSL_THREADING_ALT)
/* You should define the threading_mutex_t type in your header */


/**
 * \brief           Set your alternate threading implementation function
 *                  pointers
 *
 * \param mutex_init    the init function implementation
 * \param mutex_free    the free function implementation
 * \param mutex_lock    the lock function implementation
 * \param mutex_unlock  the unlock function implementation
 *
 * \return              0 if successful
 */
int threading_set_alt( int (*mutex_init)( threading_mutex_t * ),
                       int (*mutex_free)( threading_mutex_t * ),
                       int (*mutex_lock)( threading_mutex_t * ),
                       int (*mutex_unlock)( threading_mutex_t * ) );
#endif /* POLARSSL_THREADING_ALT_C */

#if defined(POLARSSL_THREADING_C)
/*
 * The function pointers for mutex_init, mutex_free, mutex_ and mutex_unlock
 *
 * All these functions are expected to work or the result will be undefined.
 */
extern int (*polarssl_mutex_init)( threading_mutex_t *mutex );
extern int (*polarssl_mutex_free)( threading_mutex_t *mutex );
extern int (*polarssl_mutex_lock)( threading_mutex_t *mutex );
extern int (*polarssl_mutex_unlock)( threading_mutex_t *mutex );
#endif

#ifdef __cplusplus
}
#endif

#endif /* threading.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/bignum.h ************/


/**
 * \file bignum.h
 *
 * \brief  Multi-precision integer library
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_BIGNUM_H
#define POLARSSL_BIGNUM_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(POLARSSL_FS_IO)
#include <stdio.h>
#endif

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
#if (_MSC_VER <= 1200)
typedef   signed short  int16_t;
typedef unsigned short uint16_t;
#else
typedef  INT16  int16_t;
typedef UINT16 uint16_t;
#endif
typedef  INT32  int32_t;
typedef  INT64  int64_t;
typedef UINT32 uint32_t;
typedef UINT64 uint64_t;
#elif !VXWORKS
#include <inttypes.h>
#endif /* _MSC_VER && !EFIX64 && !EFI32 */

#define POLARSSL_ERR_MPI_FILE_IO_ERROR                     -0x0002  /**< An error occurred while reading from or writing to a file. */
#define POLARSSL_ERR_MPI_BAD_INPUT_DATA                    -0x0004  /**< Bad input parameters to function. */
#define POLARSSL_ERR_MPI_INVALID_CHARACTER                 -0x0006  /**< There is an invalid character in the digit string. */
#define POLARSSL_ERR_MPI_BUFFER_TOO_SMALL                  -0x0008  /**< The buffer is too small to write to. */
#define POLARSSL_ERR_MPI_NEGATIVE_VALUE                    -0x000A  /**< The input arguments are negative or result in illegal output. */
#define POLARSSL_ERR_MPI_DIVISION_BY_ZERO                  -0x000C  /**< The input argument for division is zero, which is not allowed. */
#define POLARSSL_ERR_MPI_NOT_ACCEPTABLE                    -0x000E  /**< The input arguments are not acceptable. */
#define POLARSSL_ERR_MPI_MALLOC_FAILED                     -0x0010  /**< Memory allocation failed. */

#define MPI_CHK(f) do { if( ( ret = f ) != 0 ) goto cleanup; } while( 0 )

/*
 * Maximum size MPIs are allowed to grow to in number of limbs.
 */
#define POLARSSL_MPI_MAX_LIMBS                             10000

#if !defined(POLARSSL_MPI_WINDOW_SIZE)
/*
 * Maximum window size used for modular exponentiation. Default: 6
 * Minimum value: 1. Maximum value: 6.
 *
 * Result is an array of ( 2 << POLARSSL_MPI_WINDOW_SIZE ) MPIs used
 * for the sliding window calculation. (So 64 by default)
 *
 * Reduction in size, reduces speed.
 */
#define POLARSSL_MPI_WINDOW_SIZE                           6        /**< Maximum windows size used. */
#endif /* !POLARSSL_MPI_WINDOW_SIZE */

#if !defined(POLARSSL_MPI_MAX_SIZE)
/*
 * Maximum size of MPIs allowed in bits and bytes for user-MPIs.
 * ( Default: 512 bytes => 4096 bits, Maximum tested: 2048 bytes => 16384 bits )
 *
 * Note: Calculations can results temporarily in larger MPIs. So the number
 * of limbs required (POLARSSL_MPI_MAX_LIMBS) is higher.
 */
#define POLARSSL_MPI_MAX_SIZE                              1024     /**< Maximum number of bytes for usable MPIs. */
#endif /* !POLARSSL_MPI_MAX_SIZE */

#define POLARSSL_MPI_MAX_BITS                              ( 8 * POLARSSL_MPI_MAX_SIZE )    /**< Maximum number of bits for usable MPIs. */

/*
 * When reading from files with mpi_read_file() and writing to files with
 * mpi_write_file() the buffer should have space
 * for a (short) label, the MPI (in the provided radix), the newline
 * characters and the '\0'.
 *
 * By default we assume at least a 10 char label, a minimum radix of 10
 * (decimal) and a maximum of 4096 bit numbers (1234 decimal chars).
 * Autosized at compile time for at least a 10 char label, a minimum radix
 * of 10 (decimal) for a number of POLARSSL_MPI_MAX_BITS size.
 *
 * This used to be statically sized to 1250 for a maximum of 4096 bit
 * numbers (1234 decimal chars).
 *
 * Calculate using the formula:
 *  POLARSSL_MPI_RW_BUFFER_SIZE = ceil(POLARSSL_MPI_MAX_BITS / ln(10) * ln(2)) +
 *                                LabelSize + 6
 */
#define POLARSSL_MPI_MAX_BITS_SCALE100          ( 100 * POLARSSL_MPI_MAX_BITS )
#define LN_2_DIV_LN_10_SCALE100                 332
#define POLARSSL_MPI_RW_BUFFER_SIZE             ( ((POLARSSL_MPI_MAX_BITS_SCALE100 + LN_2_DIV_LN_10_SCALE100 - 1) / LN_2_DIV_LN_10_SCALE100) + 10 + 6 )

/*
 * Define the base integer type, architecture-wise
 */
#if defined(POLARSSL_HAVE_INT8)
typedef   signed char  t_sint;
typedef unsigned char  t_uint;
typedef uint16_t       t_udbl;
#define POLARSSL_HAVE_UDBL
#else
#if defined(POLARSSL_HAVE_INT16)
typedef  int16_t t_sint;
typedef uint16_t t_uint;
typedef uint32_t t_udbl;
#define POLARSSL_HAVE_UDBL
#else
  /*
   * 32-bit integers can be forced on 64-bit arches (eg. for testing purposes)
   * by defining POLARSSL_HAVE_INT32 and undefining POLARSSL_HAVE_ASM
   */
  #if ( ! defined(POLARSSL_HAVE_INT32) && \
          defined(_MSC_VER) && defined(_M_AMD64) )
    #define POLARSSL_HAVE_INT64
    typedef  int64_t t_sint;
    typedef uint64_t t_uint;
  #else
    #if ( ! defined(POLARSSL_HAVE_INT32) &&               \
          defined(__GNUC__) && (                          \
          defined(__amd64__) || defined(__x86_64__)    || \
          defined(__ppc64__) || defined(__powerpc64__) || \
          defined(__ia64__)  || defined(__alpha__)     || \
          (defined(__sparc__) && defined(__arch64__))  || \
          defined(__s390x__) || defined(__mips64) ) )
       #define POLARSSL_HAVE_INT64
       typedef  int64_t t_sint;
       typedef uint64_t t_uint;
       typedef unsigned int t_udbl __attribute__((mode(TI)));
       #define POLARSSL_HAVE_UDBL
    #else
       #define POLARSSL_HAVE_INT32
       typedef  int32_t t_sint;
       typedef uint32_t t_uint;
       #if ( defined(_MSC_VER) && defined(_M_IX86) )
         typedef uint64_t t_udbl;
         #define POLARSSL_HAVE_UDBL
       #else
         #if defined( POLARSSL_HAVE_LONGLONG )
           typedef unsigned long long t_udbl;
           #define POLARSSL_HAVE_UDBL
         #endif
       #endif
    #endif /* !POLARSSL_HAVE_INT32 && __GNUC__ && 64-bit platform */
  #endif /* !POLARSSL_HAVE_INT32 && _MSC_VER && _M_AMD64 */
#endif /* POLARSSL_HAVE_INT16 */
#endif /* POLARSSL_HAVE_INT8  */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          MPI structure
 */
typedef struct
{
    int s;              /*!<  integer sign      */
    size_t n;           /*!<  total # of limbs  */
    t_uint *p;          /*!<  pointer to limbs  */
}
mpi;

/**
 * \brief           Initialize one MPI (make internal references valid)
 *                  This just makes it ready to be set or freed,
 *                  but does not define a value for the MPI.
 *
 * \param X         One MPI to initialize.
 */
void mpi_init( mpi *X );

/**
 * \brief          Unallocate one MPI
 *
 * \param X        One MPI to unallocate.
 */
void mpi_free( mpi *X );

/**
 * \brief          Enlarge to the specified number of limbs
 *
 * \param X        MPI to grow
 * \param nblimbs  The target number of limbs
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_grow( mpi *X, size_t nblimbs );

/**
 * \brief          Resize down, keeping at least the specified number of limbs
 *
 * \param X        MPI to shrink
 * \param nblimbs  The minimum number of limbs to keep
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_shrink( mpi *X, size_t nblimbs );

/**
 * \brief          Copy the contents of Y into X
 *
 * \param X        Destination MPI
 * \param Y        Source MPI
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_copy( mpi *X, const mpi *Y );

/**
 * \brief          Swap the contents of X and Y
 *
 * \param X        First MPI value
 * \param Y        Second MPI value
 */
void mpi_swap( mpi *X, mpi *Y );

/**
 * \brief          Safe conditional assignement X = Y if assign is 1
 *
 * \param X        MPI to conditionally assign to
 * \param Y        Value to be assigned
 * \param assign   1: perform the assignment, 0: keep X's original value
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed,
 *
 * \note           This function is equivalent to
 *                      if( assign ) mpi_copy( X, Y );
 *                 except that it avoids leaking any information about whether
 *                 the assignment was done or not (the above code may leak
 *                 information through branch prediction and/or memory access
 *                 patterns analysis).
 */
int mpi_safe_cond_assign( mpi *X, const mpi *Y, unsigned char assign );

/**
 * \brief          Safe conditional swap X <-> Y if swap is 1
 *
 * \param X        First mpi value
 * \param Y        Second mpi value
 * \param assign   1: perform the swap, 0: keep X and Y's original values
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed,
 *
 * \note           This function is equivalent to
 *                      if( assign ) mpi_swap( X, Y );
 *                 except that it avoids leaking any information about whether
 *                 the assignment was done or not (the above code may leak
 *                 information through branch prediction and/or memory access
 *                 patterns analysis).
 */
int mpi_safe_cond_swap( mpi *X, mpi *Y, unsigned char assign );

/**
 * \brief          Set value from integer
 *
 * \param X        MPI to set
 * \param z        Value to use
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_lset( mpi *X, t_sint z );

/**
 * \brief          Get a specific bit from X
 *
 * \param X        MPI to use
 * \param pos      Zero-based index of the bit in X
 *
 * \return         Either a 0 or a 1
 */
int mpi_get_bit( const mpi *X, size_t pos );

/**
 * \brief          Set a bit of X to a specific value of 0 or 1
 *
 * \note           Will grow X if necessary to set a bit to 1 in a not yet
 *                 existing limb. Will not grow if bit should be set to 0
 *
 * \param X        MPI to use
 * \param pos      Zero-based index of the bit in X
 * \param val      The value to set the bit to (0 or 1)
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed,
 *                 POLARSSL_ERR_MPI_BAD_INPUT_DATA if val is not 0 or 1
 */
int mpi_set_bit( mpi *X, size_t pos, unsigned char val );

/**
 * \brief          Return the number of zero-bits before the least significant
 *                 '1' bit
 *
 * Note: Thus also the zero-based index of the least significant '1' bit
 *
 * \param X        MPI to use
 */
size_t mpi_lsb( const mpi *X );

/**
 * \brief          Return the number of bits up to and including the most
 *                 significant '1' bit'
 *
 * Note: Thus also the one-based index of the most significant '1' bit
 *
 * \param X        MPI to use
 */
size_t mpi_msb( const mpi *X );

/**
 * \brief          Return the total size in bytes
 *
 * \param X        MPI to use
 */
size_t mpi_size( const mpi *X );

/**
 * \brief          Import from an ASCII string
 *
 * \param X        Destination MPI
 * \param radix    Input numeric base
 * \param s        Null-terminated string buffer
 *
 * \return         0 if successful, or a POLARSSL_ERR_MPI_XXX error code
 */
int mpi_read_string( mpi *X, int radix, const char *s );

/**
 * \brief          Export into an ASCII string
 *
 * \param X        Source MPI
 * \param radix    Output numeric base
 * \param s        String buffer
 * \param slen     String buffer size
 *
 * \return         0 if successful, or a POLARSSL_ERR_MPI_XXX error code.
 *                 *slen is always updated to reflect the amount
 *                 of data that has (or would have) been written.
 *
 * \note           Call this function with *slen = 0 to obtain the
 *                 minimum required buffer size in *slen.
 */
int mpi_write_string( const mpi *X, int radix, char *s, size_t *slen );

#if defined(POLARSSL_FS_IO)
/**
 * \brief          Read X from an opened file
 *
 * \param X        Destination MPI
 * \param radix    Input numeric base
 * \param fin      Input file handle
 *
 * \return         0 if successful, POLARSSL_ERR_MPI_BUFFER_TOO_SMALL if
 *                 the file read buffer is too small or a
 *                 POLARSSL_ERR_MPI_XXX error code
 */
int mpi_read_file( mpi *X, int radix, FILE *fin );

/**
 * \brief          Write X into an opened file, or stdout if fout is NULL
 *
 * \param p        Prefix, can be NULL
 * \param X        Source MPI
 * \param radix    Output numeric base
 * \param fout     Output file handle (can be NULL)
 *
 * \return         0 if successful, or a POLARSSL_ERR_MPI_XXX error code
 *
 * \note           Set fout == NULL to print X on the console.
 */
int mpi_write_file( const char *p, const mpi *X, int radix, FILE *fout );
#endif /* POLARSSL_FS_IO */

/**
 * \brief          Import X from unsigned binary data, big endian
 *
 * \param X        Destination MPI
 * \param buf      Input buffer
 * \param buflen   Input buffer size
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_read_binary( mpi *X, const unsigned char *buf, size_t buflen );

/**
 * \brief          Export X into unsigned binary data, big endian.
 *                 Always fills the whole buffer, which will start with zeros
 *                 if the number is smaller.
 *
 * \param X        Source MPI
 * \param buf      Output buffer
 * \param buflen   Output buffer size
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_BUFFER_TOO_SMALL if buf isn't large enough
 */
int mpi_write_binary( const mpi *X, unsigned char *buf, size_t buflen );

/**
 * \brief          Left-shift: X <<= count
 *
 * \param X        MPI to shift
 * \param count    Amount to shift
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_shift_l( mpi *X, size_t count );

/**
 * \brief          Right-shift: X >>= count
 *
 * \param X        MPI to shift
 * \param count    Amount to shift
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_shift_r( mpi *X, size_t count );

/**
 * \brief          Compare unsigned values
 *
 * \param X        Left-hand MPI
 * \param Y        Right-hand MPI
 *
 * \return         1 if |X| is greater than |Y|,
 *                -1 if |X| is lesser  than |Y| or
 *                 0 if |X| is equal to |Y|
 */
int mpi_cmp_abs( const mpi *X, const mpi *Y );

/**
 * \brief          Compare signed values
 *
 * \param X        Left-hand MPI
 * \param Y        Right-hand MPI
 *
 * \return         1 if X is greater than Y,
 *                -1 if X is lesser  than Y or
 *                 0 if X is equal to Y
 */
int mpi_cmp_mpi( const mpi *X, const mpi *Y );

/**
 * \brief          Compare signed values
 *
 * \param X        Left-hand MPI
 * \param z        The integer value to compare to
 *
 * \return         1 if X is greater than z,
 *                -1 if X is lesser  than z or
 *                 0 if X is equal to z
 */
int mpi_cmp_int( const mpi *X, t_sint z );

/**
 * \brief          Unsigned addition: X = |A| + |B|
 *
 * \param X        Destination MPI
 * \param A        Left-hand MPI
 * \param B        Right-hand MPI
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_add_abs( mpi *X, const mpi *A, const mpi *B );

/**
 * \brief          Unsigned subtraction: X = |A| - |B|
 *
 * \param X        Destination MPI
 * \param A        Left-hand MPI
 * \param B        Right-hand MPI
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_NEGATIVE_VALUE if B is greater than A
 */
int mpi_sub_abs( mpi *X, const mpi *A, const mpi *B );

/**
 * \brief          Signed addition: X = A + B
 *
 * \param X        Destination MPI
 * \param A        Left-hand MPI
 * \param B        Right-hand MPI
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_add_mpi( mpi *X, const mpi *A, const mpi *B );

/**
 * \brief          Signed subtraction: X = A - B
 *
 * \param X        Destination MPI
 * \param A        Left-hand MPI
 * \param B        Right-hand MPI
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_sub_mpi( mpi *X, const mpi *A, const mpi *B );

/**
 * \brief          Signed addition: X = A + b
 *
 * \param X        Destination MPI
 * \param A        Left-hand MPI
 * \param b        The integer value to add
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_add_int( mpi *X, const mpi *A, t_sint b );

/**
 * \brief          Signed subtraction: X = A - b
 *
 * \param X        Destination MPI
 * \param A        Left-hand MPI
 * \param b        The integer value to subtract
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_sub_int( mpi *X, const mpi *A, t_sint b );

/**
 * \brief          Baseline multiplication: X = A * B
 *
 * \param X        Destination MPI
 * \param A        Left-hand MPI
 * \param B        Right-hand MPI
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_mul_mpi( mpi *X, const mpi *A, const mpi *B );

/**
 * \brief          Baseline multiplication: X = A * b
 *                 Note: despite the functon signature, b is treated as a
 *                 t_uint.  Negative values of b are treated as large positive
 *                 values.
 *
 * \param X        Destination MPI
 * \param A        Left-hand MPI
 * \param b        The integer value to multiply with
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_mul_int( mpi *X, const mpi *A, t_sint b );

/**
 * \brief          Division by mpi: A = Q * B + R
 *
 * \param Q        Destination MPI for the quotient
 * \param R        Destination MPI for the rest value
 * \param A        Left-hand MPI
 * \param B        Right-hand MPI
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed,
 *                 POLARSSL_ERR_MPI_DIVISION_BY_ZERO if B == 0
 *
 * \note           Either Q or R can be NULL.
 */
int mpi_div_mpi( mpi *Q, mpi *R, const mpi *A, const mpi *B );

/**
 * \brief          Division by int: A = Q * b + R
 *
 * \param Q        Destination MPI for the quotient
 * \param R        Destination MPI for the rest value
 * \param A        Left-hand MPI
 * \param b        Integer to divide by
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed,
 *                 POLARSSL_ERR_MPI_DIVISION_BY_ZERO if b == 0
 *
 * \note           Either Q or R can be NULL.
 */
int mpi_div_int( mpi *Q, mpi *R, const mpi *A, t_sint b );

/**
 * \brief          Modulo: R = A mod B
 *
 * \param R        Destination MPI for the rest value
 * \param A        Left-hand MPI
 * \param B        Right-hand MPI
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed,
 *                 POLARSSL_ERR_MPI_DIVISION_BY_ZERO if B == 0,
 *                 POLARSSL_ERR_MPI_NEGATIVE_VALUE if B < 0
 */
int mpi_mod_mpi( mpi *R, const mpi *A, const mpi *B );

/**
 * \brief          Modulo: r = A mod b
 *
 * \param r        Destination t_uint
 * \param A        Left-hand MPI
 * \param b        Integer to divide by
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed,
 *                 POLARSSL_ERR_MPI_DIVISION_BY_ZERO if b == 0,
 *                 POLARSSL_ERR_MPI_NEGATIVE_VALUE if b < 0
 */
int mpi_mod_int( t_uint *r, const mpi *A, t_sint b );

/**
 * \brief          Sliding-window exponentiation: X = A^E mod N
 *
 * \param X        Destination MPI
 * \param A        Left-hand MPI
 * \param E        Exponent MPI
 * \param N        Modular MPI
 * \param _RR      Speed-up MPI used for recalculations
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed,
 *                 POLARSSL_ERR_MPI_BAD_INPUT_DATA if N is negative or even or
 *                 if E is negative
 *
 * \note           _RR is used to avoid re-computing R*R mod N across
 *                 multiple calls, which speeds up things a bit. It can
 *                 be set to NULL if the extra performance is unneeded.
 */
int mpi_exp_mod( mpi *X, const mpi *A, const mpi *E, const mpi *N, mpi *_RR );

/**
 * \brief          Fill an MPI X with size bytes of random
 *
 * \param X        Destination MPI
 * \param size     Size in bytes
 * \param f_rng    RNG function
 * \param p_rng    RNG parameter
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_fill_random( mpi *X, size_t size,
                     int (*f_rng)(void *, unsigned char *, size_t),
                     void *p_rng );

/**
 * \brief          Greatest common divisor: G = gcd(A, B)
 *
 * \param G        Destination MPI
 * \param A        Left-hand MPI
 * \param B        Right-hand MPI
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int mpi_gcd( mpi *G, const mpi *A, const mpi *B );

/**
 * \brief          Modular inverse: X = A^-1 mod N
 *
 * \param X        Destination MPI
 * \param A        Left-hand MPI
 * \param N        Right-hand MPI
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed,
 *                 POLARSSL_ERR_MPI_BAD_INPUT_DATA if N is negative or nil
                   POLARSSL_ERR_MPI_NOT_ACCEPTABLE if A has no inverse mod N
 */
int mpi_inv_mod( mpi *X, const mpi *A, const mpi *N );

/**
 * \brief          Miller-Rabin primality test
 *
 * \param X        MPI to check
 * \param f_rng    RNG function
 * \param p_rng    RNG parameter
 *
 * \return         0 if successful (probably prime),
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed,
 *                 POLARSSL_ERR_MPI_NOT_ACCEPTABLE if X is not prime
 */
int mpi_is_prime( mpi *X,
                  int (*f_rng)(void *, unsigned char *, size_t),
                  void *p_rng );

/**
 * \brief          Prime number generation
 *
 * \param X        Destination MPI
 * \param nbits    Required size of X in bits
 *                 ( 3 <= nbits <= POLARSSL_MPI_MAX_BITS )
 * \param dh_flag  If 1, then (X-1)/2 will be prime too
 * \param f_rng    RNG function
 * \param p_rng    RNG parameter
 *
 * \return         0 if successful (probably prime),
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed,
 *                 POLARSSL_ERR_MPI_BAD_INPUT_DATA if nbits is < 3
 */
int mpi_gen_prime( mpi *X, size_t nbits, int dh_flag,
                   int (*f_rng)(void *, unsigned char *, size_t),
                   void *p_rng );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int mpi_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* bignum.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/net.h ************/


/**
 * \file net.h
 *
 * \brief Network communication functions
 *
 *  Copyright (C) 2006-2011, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_NET_H
#define POLARSSL_NET_H

#include <stddef.h>

#define POLARSSL_ERR_NET_UNKNOWN_HOST                      -0x0056  /**< Failed to get an IP address for the given hostname. */
#define POLARSSL_ERR_NET_SOCKET_FAILED                     -0x0042  /**< Failed to open a socket. */
#define POLARSSL_ERR_NET_CONNECT_FAILED                    -0x0044  /**< The connection to the given server / port failed. */
#define POLARSSL_ERR_NET_BIND_FAILED                       -0x0046  /**< Binding of the socket failed. */
#define POLARSSL_ERR_NET_LISTEN_FAILED                     -0x0048  /**< Could not listen on the socket. */
#define POLARSSL_ERR_NET_ACCEPT_FAILED                     -0x004A  /**< Could not accept the incoming connection. */
#define POLARSSL_ERR_NET_RECV_FAILED                       -0x004C  /**< Reading information from the socket failed. */
#define POLARSSL_ERR_NET_SEND_FAILED                       -0x004E  /**< Sending information through the socket failed. */
#define POLARSSL_ERR_NET_CONN_RESET                        -0x0050  /**< Connection was reset by peer. */
#define POLARSSL_ERR_NET_WANT_READ                         -0x0052  /**< Connection requires a read call. */
#define POLARSSL_ERR_NET_WANT_WRITE                        -0x0054  /**< Connection requires a write call. */

#define POLARSSL_NET_LISTEN_BACKLOG         10 /**< The backlog that listen() should use. */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Initiate a TCP connection with host:port
 *
 * \param fd       Socket to use
 * \param host     Host to connect to
 * \param port     Port to connect to
 *
 * \return         0 if successful, or one of:
 *                      POLARSSL_ERR_NET_SOCKET_FAILED,
 *                      POLARSSL_ERR_NET_UNKNOWN_HOST,
 *                      POLARSSL_ERR_NET_CONNECT_FAILED
 */
int net_connect( int *fd, const char *host, int port );

/**
 * \brief          Create a listening socket on bind_ip:port.
 *                 If bind_ip == NULL, all interfaces are binded.
 *
 * \param fd       Socket to use
 * \param bind_ip  IP to bind to, can be NULL
 * \param port     Port number to use
 *
 * \return         0 if successful, or one of:
 *                      POLARSSL_ERR_NET_SOCKET_FAILED,
 *                      POLARSSL_ERR_NET_BIND_FAILED,
 *                      POLARSSL_ERR_NET_LISTEN_FAILED
 */
int net_bind( int *fd, const char *bind_ip, int port );

/**
 * \brief           Accept a connection from a remote client
 *
 * \param bind_fd   Relevant socket
 * \param client_fd Will contain the connected client socket
 * \param client_ip Will contain the client IP address
 *                  Must be at least 4 bytes, or 16 if IPv6 is supported
 *
 * \return          0 if successful, POLARSSL_ERR_NET_ACCEPT_FAILED, or
 *                  POLARSSL_ERR_NET_WANT_READ is bind_fd was set to
 *                  non-blocking and accept() is blocking.
 */
int net_accept( int bind_fd, int *client_fd, void *client_ip );

/**
 * \brief          Set the socket blocking
 *
 * \param fd       Socket to set
 *
 * \return         0 if successful, or a non-zero error code
 */
int net_set_block( int fd );

/**
 * \brief          Set the socket non-blocking
 *
 * \param fd       Socket to set
 *
 * \return         0 if successful, or a non-zero error code
 */
int net_set_nonblock( int fd );

/**
 * \brief          Portable usleep helper
 *
 * \param usec     Amount of microseconds to sleep
 *
 * \note           Real amount of time slept will not be less than
 *                 select()'s timeout granularity (typically, 10ms).
 */
void net_usleep( unsigned long usec );

/**
 * \brief          Read at most 'len' characters. If no error occurs,
 *                 the actual amount read is returned.
 *
 * \param ctx      Socket
 * \param buf      The buffer to write to
 * \param len      Maximum length of the buffer
 *
 * \return         This function returns the number of bytes received,
 *                 or a non-zero error code; POLARSSL_ERR_NET_WANT_READ
 *                 indicates read() is blocking.
 */
int net_recv( void *ctx, unsigned char *buf, size_t len );

/**
 * \brief          Write at most 'len' characters. If no error occurs,
 *                 the actual amount read is returned.
 *
 * \param ctx      Socket
 * \param buf      The buffer to read from
 * \param len      The length of the buffer
 *
 * \return         This function returns the number of bytes sent,
 *                 or a non-zero error code; POLARSSL_ERR_NET_WANT_WRITE
 *                 indicates write() is blocking.
 */
int net_send( void *ctx, const unsigned char *buf, size_t len );

/**
 * \brief          Gracefully shutdown the connection
 *
 * \param fd       The socket to close
 */
void net_close( int fd );

#ifdef __cplusplus
}
#endif

#endif /* net.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/dhm.h ************/


/**
 * \file dhm.h
 *
 * \brief Diffie-Hellman-Merkle key exchange
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_DHM_H
#define POLARSSL_DHM_H



/*
 * DHM Error codes
 */
#define POLARSSL_ERR_DHM_BAD_INPUT_DATA                    -0x3080  /**< Bad input parameters to function. */
#define POLARSSL_ERR_DHM_READ_PARAMS_FAILED                -0x3100  /**< Reading of the DHM parameters failed. */
#define POLARSSL_ERR_DHM_MAKE_PARAMS_FAILED                -0x3180  /**< Making of the DHM parameters failed. */
#define POLARSSL_ERR_DHM_READ_PUBLIC_FAILED                -0x3200  /**< Reading of the public values failed. */
#define POLARSSL_ERR_DHM_MAKE_PUBLIC_FAILED                -0x3280  /**< Making of the public value failed. */
#define POLARSSL_ERR_DHM_CALC_SECRET_FAILED                -0x3300  /**< Calculation of the DHM secret failed. */
#define POLARSSL_ERR_DHM_INVALID_FORMAT                    -0x3380  /**< The ASN.1 data is not formatted correctly. */
#define POLARSSL_ERR_DHM_MALLOC_FAILED                     -0x3400  /**< Allocation of memory failed. */
#define POLARSSL_ERR_DHM_FILE_IO_ERROR                     -0x3480  /**< Read/write of file failed. */

/**
 * RFC 2409 defines a number of standardized Diffie-Hellman groups
 * that can be used.
 * RFC 3526 defines a number of standardized Diffie-Hellman groups
 * for IKE.
 * RFC 5114 defines a number of standardized Diffie-Hellman groups
 * that can be used.
 *
 * Some are included here for convenience.
 *
 * Included are:
 *  RFC 2409 6.2.  1024-bit MODP Group (Second Oakley Group)
 *  RFC 3526 3.    2048-bit MODP Group
 *  RFC 3526 4.    3072-bit MODP Group
 *  RFC 5114 2.1.  1024-bit MODP Group with 160-bit Prime Order Subgroup
 *  RFC 5114 2.2.  2048-bit MODP Group with 224-bit Prime Order Subgroup
 */
#define POLARSSL_DHM_RFC2409_MODP_1024_P               \
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1" \
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD" \
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245" \
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED" \
    "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE65381" \
    "FFFFFFFFFFFFFFFF"

#define POLARSSL_DHM_RFC2409_MODP_1024_G          "02"

#define POLARSSL_DHM_RFC3526_MODP_2048_P               \
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1" \
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD" \
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245" \
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED" \
    "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D" \
    "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F" \
    "83655D23DCA3AD961C62F356208552BB9ED529077096966D" \
    "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B" \
    "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9" \
    "DE2BCBF6955817183995497CEA956AE515D2261898FA0510" \
    "15728E5A8AACAA68FFFFFFFFFFFFFFFF"

#define POLARSSL_DHM_RFC3526_MODP_2048_G          "02"

#define POLARSSL_DHM_RFC3526_MODP_3072_P               \
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1" \
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD" \
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245" \
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED" \
    "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D" \
    "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F" \
    "83655D23DCA3AD961C62F356208552BB9ED529077096966D" \
    "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B" \
    "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9" \
    "DE2BCBF6955817183995497CEA956AE515D2261898FA0510" \
    "15728E5A8AAAC42DAD33170D04507A33A85521ABDF1CBA64" \
    "ECFB850458DBEF0A8AEA71575D060C7DB3970F85A6E1E4C7" \
    "ABF5AE8CDB0933D71E8C94E04A25619DCEE3D2261AD2EE6B" \
    "F12FFA06D98A0864D87602733EC86A64521F2B18177B200C" \
    "BBE117577A615D6C770988C0BAD946E208E24FA074E5AB31" \
    "43DB5BFCE0FD108E4B82D120A93AD2CAFFFFFFFFFFFFFFFF"

#define POLARSSL_DHM_RFC3526_MODP_3072_G          "02"

#define POLARSSL_DHM_RFC5114_MODP_1024_P               \
    "B10B8F96A080E01DDE92DE5EAE5D54EC52C99FBCFB06A3C6" \
    "9A6A9DCA52D23B616073E28675A23D189838EF1E2EE652C0" \
    "13ECB4AEA906112324975C3CD49B83BFACCBDD7D90C4BD70" \
    "98488E9C219A73724EFFD6FAE5644738FAA31A4FF55BCCC0" \
    "A151AF5F0DC8B4BD45BF37DF365C1A65E68CFDA76D4DA708" \
    "DF1FB2BC2E4A4371"

#define POLARSSL_DHM_RFC5114_MODP_1024_G               \
    "A4D1CBD5C3FD34126765A442EFB99905F8104DD258AC507F" \
    "D6406CFF14266D31266FEA1E5C41564B777E690F5504F213" \
    "160217B4B01B886A5E91547F9E2749F4D7FBD7D3B9A92EE1" \
    "909D0D2263F80A76A6A24C087A091F531DBF0A0169B6A28A" \
    "D662A4D18E73AFA32D779D5918D08BC8858F4DCEF97C2A24" \
    "855E6EEB22B3B2E5"

#define POLARSSL_DHM_RFC5114_MODP_2048_P               \
    "AD107E1E9123A9D0D660FAA79559C51FA20D64E5683B9FD1" \
    "B54B1597B61D0A75E6FA141DF95A56DBAF9A3C407BA1DF15" \
    "EB3D688A309C180E1DE6B85A1274A0A66D3F8152AD6AC212" \
    "9037C9EDEFDA4DF8D91E8FEF55B7394B7AD5B7D0B6C12207" \
    "C9F98D11ED34DBF6C6BA0B2C8BBC27BE6A00E0A0B9C49708" \
    "B3BF8A317091883681286130BC8985DB1602E714415D9330" \
    "278273C7DE31EFDC7310F7121FD5A07415987D9ADC0A486D" \
    "CDF93ACC44328387315D75E198C641A480CD86A1B9E587E8" \
    "BE60E69CC928B2B9C52172E413042E9B23F10B0E16E79763" \
    "C9B53DCF4BA80A29E3FB73C16B8E75B97EF363E2FFA31F71" \
    "CF9DE5384E71B81C0AC4DFFE0C10E64F"

#define POLARSSL_DHM_RFC5114_MODP_2048_G              \
    "AC4032EF4F2D9AE39DF30B5C8FFDAC506CDEBE7B89998CAF"\
    "74866A08CFE4FFE3A6824A4E10B9A6F0DD921F01A70C4AFA"\
    "AB739D7700C29F52C57DB17C620A8652BE5E9001A8D66AD7"\
    "C17669101999024AF4D027275AC1348BB8A762D0521BC98A"\
    "E247150422EA1ED409939D54DA7460CDB5F6C6B250717CBE"\
    "F180EB34118E98D119529A45D6F834566E3025E316A330EF"\
    "BB77A86F0C1AB15B051AE3D428C8F8ACB70A8137150B8EEB"\
    "10E183EDD19963DDD9E263E4770589EF6AA21E7F5F2FF381"\
    "B539CCE3409D13CD566AFBB48D6C019181E1BCFE94B30269"\
    "EDFE72FE9B6AA4BD7B5A0F1C71CFFF4C19C418E1F6EC0179"\
    "81BC087F2A7065B384B890D3191F2BFA"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          DHM context structure
 */
typedef struct
{
    size_t len; /*!<  size(P) in chars  */
    mpi P;      /*!<  prime modulus     */
    mpi G;      /*!<  generator         */
    mpi X;      /*!<  secret value      */
    mpi GX;     /*!<  self = G^X mod P  */
    mpi GY;     /*!<  peer = G^Y mod P  */
    mpi K;      /*!<  key = GY^X mod P  */
    mpi RP;     /*!<  cached R^2 mod P  */
    mpi Vi;     /*!<  blinding value    */
    mpi Vf;     /*!<  un-blinding value */
    mpi pX;     /*!<  previous X        */
}
dhm_context;

/**
 * \brief          Initialize DHM context
 *
 * \param ctx      DHM context to be initialized
 */
void dhm_init( dhm_context *ctx );

/**
 * \brief          Parse the ServerKeyExchange parameters
 *
 * \param ctx      DHM context
 * \param p        &(start of input buffer)
 * \param end      end of buffer
 *
 * \return         0 if successful, or an POLARSSL_ERR_DHM_XXX error code
 */
int dhm_read_params( dhm_context *ctx,
                     unsigned char **p,
                     const unsigned char *end );

/**
 * \brief          Setup and write the ServerKeyExchange parameters
 *
 * \param ctx      DHM context
 * \param x_size   private value size in bytes
 * \param output   destination buffer
 * \param olen     number of chars written
 * \param f_rng    RNG function
 * \param p_rng    RNG parameter
 *
 * \note           This function assumes that ctx->P and ctx->G
 *                 have already been properly set (for example
 *                 using mpi_read_string or mpi_read_binary).
 *
 * \return         0 if successful, or an POLARSSL_ERR_DHM_XXX error code
 */
int dhm_make_params( dhm_context *ctx, int x_size,
                     unsigned char *output, size_t *olen,
                     int (*f_rng)(void *, unsigned char *, size_t),
                     void *p_rng );

/**
 * \brief          Import the peer's public value G^Y
 *
 * \param ctx      DHM context
 * \param input    input buffer
 * \param ilen     size of buffer
 *
 * \return         0 if successful, or an POLARSSL_ERR_DHM_XXX error code
 */
int dhm_read_public( dhm_context *ctx,
                     const unsigned char *input, size_t ilen );

/**
 * \brief          Create own private value X and export G^X
 *
 * \param ctx      DHM context
 * \param x_size   private value size in bytes
 * \param output   destination buffer
 * \param olen     must be equal to ctx->P.len
 * \param f_rng    RNG function
 * \param p_rng    RNG parameter
 *
 * \return         0 if successful, or an POLARSSL_ERR_DHM_XXX error code
 */
int dhm_make_public( dhm_context *ctx, int x_size,
                     unsigned char *output, size_t olen,
                     int (*f_rng)(void *, unsigned char *, size_t),
                     void *p_rng );

/**
 * \brief          Derive and export the shared secret (G^Y)^X mod P
 *
 * \param ctx      DHM context
 * \param output   destination buffer
 * \param olen     on entry, must hold the size of the destination buffer
 *                 on exit, holds the actual number of bytes written
 * \param f_rng    RNG function, for blinding purposes
 * \param p_rng    RNG parameter
 *
 * \return         0 if successful, or an POLARSSL_ERR_DHM_XXX error code
 *
 * \note           If non-NULL, f_rng is used to blind the input as
 *                 countermeasure against timing attacks. Blinding is
 *                 automatically used if and only if our secret value X is
 *                 re-used and costs nothing otherwise, so it is recommended
 *                 to always pass a non-NULL f_rng argument.
 */
int dhm_calc_secret( dhm_context *ctx,
                     unsigned char *output, size_t *olen,
                     int (*f_rng)(void *, unsigned char *, size_t),
                     void *p_rng );

/**
 * \brief          Free and clear the components of a DHM key
 *
 * \param ctx      DHM context to free and clear
 */
void dhm_free( dhm_context *ctx );

#if defined(POLARSSL_ASN1_PARSE_C)
/** \ingroup x509_module */
/**
 * \brief          Parse DHM parameters
 *
 * \param dhm      DHM context to be initialized
 * \param dhmin    input buffer
 * \param dhminlen size of the buffer
 *
 * \return         0 if successful, or a specific DHM or PEM error code
 */
int dhm_parse_dhm( dhm_context *dhm, const unsigned char *dhmin,
                   size_t dhminlen );

#if defined(POLARSSL_FS_IO)
/** \ingroup x509_module */
/**
 * \brief          Load and parse DHM parameters
 *
 * \param dhm      DHM context to be initialized
 * \param path     filename to read the DHM Parameters from
 *
 * \return         0 if successful, or a specific DHM or PEM error code
 */
int dhm_parse_dhmfile( dhm_context *dhm, const char *path );
#endif /* POLARSSL_FS_IO */
#endif /* POLARSSL_ASN1_PARSE_C */

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int dhm_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* dhm.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/md.h ************/


/**
 * \file md.h
 *
 * \brief Generic message digest wrapper
 *
 * \author Adriaan de Jong <dejong@fox-it.com>
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_MD_H
#define POLARSSL_MD_H

#include <stddef.h>

#if defined(_MSC_VER) && !defined(inline)
#define inline _inline
#else
#if defined(__ARMCC_VERSION) && !defined(inline)
#define inline __inline
#endif /* __ARMCC_VERSION */
#endif /*_MSC_VER */

#define POLARSSL_ERR_MD_FEATURE_UNAVAILABLE                -0x5080  /**< The selected feature is not available. */
#define POLARSSL_ERR_MD_BAD_INPUT_DATA                     -0x5100  /**< Bad input parameters to function. */
#define POLARSSL_ERR_MD_ALLOC_FAILED                       -0x5180  /**< Failed to allocate memory. */
#define POLARSSL_ERR_MD_FILE_IO_ERROR                      -0x5200  /**< Opening or reading of file failed. */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    POLARSSL_MD_NONE=0,
    POLARSSL_MD_MD2,
    POLARSSL_MD_MD4,
    POLARSSL_MD_MD5,
    POLARSSL_MD_SHA1,
    POLARSSL_MD_SHA224,
    POLARSSL_MD_SHA256,
    POLARSSL_MD_SHA384,
    POLARSSL_MD_SHA512,
    POLARSSL_MD_RIPEMD160,
} md_type_t;

#if defined(POLARSSL_SHA512_C)
#define POLARSSL_MD_MAX_SIZE         64  /* longest known is SHA512 */
#else
#define POLARSSL_MD_MAX_SIZE         32  /* longest known is SHA256 or less */
#endif

/**
 * Message digest information. Allows message digest functions to be called
 * in a generic way.
 */
typedef struct {
    /** Digest identifier */
    md_type_t type;

    /** Name of the message digest */
    const char * name;

    /** Output length of the digest function */
    int size;

    /** Digest initialisation function */
    void (*starts_func)( void *ctx );

    /** Digest update function */
    void (*update_func)( void *ctx, const unsigned char *input, size_t ilen );

    /** Digest finalisation function */
    void (*finish_func)( void *ctx, unsigned char *output );

    /** Generic digest function */
    void (*digest_func)( const unsigned char *input, size_t ilen,
                         unsigned char *output );

    /** Generic file digest function */
    int (*file_func)( const char *path, unsigned char *output );

    /** HMAC Initialisation function */
    void (*hmac_starts_func)( void *ctx, const unsigned char *key,
                              size_t keylen );

    /** HMAC update function */
    void (*hmac_update_func)( void *ctx, const unsigned char *input,
                              size_t ilen );

    /** HMAC finalisation function */
    void (*hmac_finish_func)( void *ctx, unsigned char *output);

    /** HMAC context reset function */
    void (*hmac_reset_func)( void *ctx );

    /** Generic HMAC function */
    void (*hmac_func)( const unsigned char *key, size_t keylen,
                       const unsigned char *input, size_t ilen,
                       unsigned char *output );

    /** Allocate a new context */
    void * (*ctx_alloc_func)( void );

    /** Free the given context */
    void (*ctx_free_func)( void *ctx );

    /** Internal use only */
    void (*process_func)( void *ctx, const unsigned char *input );
} md_info_t;

/**
 * Generic message digest context.
 */
typedef struct {
    /** Information about the associated message digest */
    const md_info_t *md_info;

    /** Digest-specific context */
    void *md_ctx;
} md_context_t;

#define MD_CONTEXT_T_INIT { \
    NULL, /* md_info */ \
    NULL, /* md_ctx */ \
}

/**
 * \brief Returns the list of digests supported by the generic digest module.
 *
 * \return          a statically allocated array of digests, the last entry
 *                  is 0.
 */
const int *md_list( void );

/**
 * \brief           Returns the message digest information associated with the
 *                  given digest name.
 *
 * \param md_name   Name of the digest to search for.
 *
 * \return          The message digest information associated with md_name or
 *                  NULL if not found.
 */
const md_info_t *md_info_from_string( const char *md_name );

/**
 * \brief           Returns the message digest information associated with the
 *                  given digest type.
 *
 * \param md_type   type of digest to search for.
 *
 * \return          The message digest information associated with md_type or
 *                  NULL if not found.
 */
const md_info_t *md_info_from_type( md_type_t md_type );

/**
 * \brief               Initialize a md_context (as NONE)
 */
void md_init( md_context_t *ctx );

/**
 * \brief               Free and clear the message-specific context of ctx.
 *                      Freeing ctx itself remains the responsibility of the
 *                      caller.
 */
void md_free( md_context_t *ctx );

/**
 * \brief          Initialises and fills the message digest context structure
 *                 with the appropriate values.
 *
 * \note           Currently also clears structure. In future versions you
 *                 will be required to call md_init() on the structure
 *                 first.
 *
 * \param ctx      context to initialise. May not be NULL. The
 *                 digest-specific context (ctx->md_ctx) must be NULL. It will
 *                 be allocated, and must be freed using md_free_ctx() later.
 * \param md_info  message digest to use.
 *
 * \returns        \c 0 on success, \c POLARSSL_ERR_MD_BAD_INPUT_DATA on
 *                 parameter failure, \c POLARSSL_ERR_MD_ALLOC_FAILED if
 *                 allocation of the digest-specific context failed.
 */
int md_init_ctx( md_context_t *ctx, const md_info_t *md_info );

#if ! defined(POLARSSL_DEPRECATED_REMOVED)
#if defined(POLARSSL_DEPRECATED_WARNING)
#define DEPRECATED    __attribute__((deprecated))
#else
#define DEPRECATED
#endif
/**
 * \brief          Free the message-specific context of ctx. Freeing ctx itself
 *                 remains the responsibility of the caller.
 *
 * \deprecated     Use md_free() instead
 *
 * \param ctx      Free the message-specific context
 *
 * \returns        0
 */
int md_free_ctx( md_context_t *ctx ) DEPRECATED;
#undef DEPRECATED
#endif /* POLARSSL_DEPRECATED_REMOVED */

/**
 * \brief           Returns the size of the message digest output.
 *
 * \param md_info   message digest info
 *
 * \return          size of the message digest output.
 */
static inline unsigned char md_get_size( const md_info_t *md_info )
{
    if( md_info == NULL )
        return( 0 );

    return md_info->size;
}

/**
 * \brief           Returns the type of the message digest output.
 *
 * \param md_info   message digest info
 *
 * \return          type of the message digest output.
 */
static inline md_type_t md_get_type( const md_info_t *md_info )
{
    if( md_info == NULL )
        return( POLARSSL_MD_NONE );

    return md_info->type;
}

/**
 * \brief           Returns the name of the message digest output.
 *
 * \param md_info   message digest info
 *
 * \return          name of the message digest output.
 */
static inline const char *md_get_name( const md_info_t *md_info )
{
    if( md_info == NULL )
        return( NULL );

    return md_info->name;
}

/**
 * \brief          Set-up the given context for a new message digest
 *
 * \param ctx      generic message digest context.
 *
 * \returns        0 on success, POLARSSL_ERR_MD_BAD_INPUT_DATA if parameter
 *                 verification fails.
 */
int md_starts( md_context_t *ctx );

/**
 * \brief          Generic message digest process buffer
 *
 * \param ctx      Generic message digest context
 * \param input    buffer holding the  datal
 * \param ilen     length of the input data
 *
 * \returns        0 on success, POLARSSL_ERR_MD_BAD_INPUT_DATA if parameter
 *                 verification fails.
 */
int md_update( md_context_t *ctx, const unsigned char *input, size_t ilen );

/**
 * \brief          Generic message digest final digest
 *
 * \param ctx      Generic message digest context
 * \param output   Generic message digest checksum result
 *
 * \returns        0 on success, POLARSSL_ERR_MD_BAD_INPUT_DATA if parameter
 *                 verification fails.
 */
int md_finish( md_context_t *ctx, unsigned char *output );

/**
 * \brief          Output = message_digest( input buffer )
 *
 * \param md_info  message digest info
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   Generic message digest checksum result
 *
 * \returns        0 on success, POLARSSL_ERR_MD_BAD_INPUT_DATA if parameter
 *                 verification fails.
 */
int md( const md_info_t *md_info, const unsigned char *input, size_t ilen,
        unsigned char *output );

/**
 * \brief          Output = message_digest( file contents )
 *
 * \param md_info  message digest info
 * \param path     input file name
 * \param output   generic message digest checksum result
 *
 * \return         0 if successful, POLARSSL_ERR_MD_FILE_OPEN_FAILED if fopen
 *                 failed, POLARSSL_ERR_MD_FILE_READ_FAILED if fread failed,
 *                 POLARSSL_ERR_MD_BAD_INPUT_DATA if md_info was NULL.
 */
int md_file( const md_info_t *md_info, const char *path,
             unsigned char *output );

/**
 * \brief          Generic HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 *
 * \returns        0 on success, POLARSSL_ERR_MD_BAD_INPUT_DATA if parameter
 *                 verification fails.
 */
int md_hmac_starts( md_context_t *ctx, const unsigned char *key,
                    size_t keylen );

/**
 * \brief          Generic HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 *
 * \returns        0 on success, POLARSSL_ERR_MD_BAD_INPUT_DATA if parameter
 *                 verification fails.
 */
int md_hmac_update( md_context_t *ctx, const unsigned char *input,
                    size_t ilen );

/**
 * \brief          Generic HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   Generic HMAC checksum result
 *
 * \returns        0 on success, POLARSSL_ERR_MD_BAD_INPUT_DATA if parameter
 *                 verification fails.
 */
int md_hmac_finish( md_context_t *ctx, unsigned char *output);

/**
 * \brief          Generic HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 *
 * \returns        0 on success, POLARSSL_ERR_MD_BAD_INPUT_DATA if parameter
 *                 verification fails.
 */
int md_hmac_reset( md_context_t *ctx );

/**
 * \brief          Output = Generic_HMAC( hmac key, input buffer )
 *
 * \param md_info  message digest info
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   Generic HMAC-result
 *
 * \returns        0 on success, POLARSSL_ERR_MD_BAD_INPUT_DATA if parameter
 *                 verification fails.
 */
int md_hmac( const md_info_t *md_info, const unsigned char *key, size_t keylen,
                const unsigned char *input, size_t ilen,
                unsigned char *output );

/* Internal use */
int md_process( md_context_t *ctx, const unsigned char *data );

#ifdef __cplusplus
}
#endif

#endif /* POLARSSL_MD_H */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/md_wrap.h ************/


/**
 * \file md_wrap.h
 *
 * \brief Message digest wrappers.
 *
 * \author Adriaan de Jong <dejong@fox-it.com>
 *
 *  Copyright (C) 2006-2011, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_MD_WRAP_H
#define POLARSSL_MD_WRAP_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif



#ifdef __cplusplus
extern "C" {
#endif

#if defined(POLARSSL_MD2_C)
extern const md_info_t md2_info;
#endif
#if defined(POLARSSL_MD4_C)
extern const md_info_t md4_info;
#endif
#if defined(POLARSSL_MD5_C)
extern const md_info_t md5_info;
#endif
#if defined(POLARSSL_RIPEMD160_C)
extern const md_info_t ripemd160_info;
#endif
#if defined(POLARSSL_SHA1_C)
extern const md_info_t sha1_info;
#endif
#if defined(POLARSSL_SHA256_C)
extern const md_info_t sha224_info;
extern const md_info_t sha256_info;
#endif
#if defined(POLARSSL_SHA512_C)
extern const md_info_t sha384_info;
extern const md_info_t sha512_info;
#endif

#ifdef __cplusplus
}
#endif

#endif /* POLARSSL_MD_WRAP_H */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/md5.h ************/


/**
 * \file md5.h
 *
 * \brief MD5 message digest algorithm (hash function)
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_MD5_H
#define POLARSSL_MD5_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

#define POLARSSL_ERR_MD5_FILE_IO_ERROR                 -0x0074  /**< Read/write error in file. */

#if !defined(POLARSSL_MD5_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          MD5 context structure
 */
typedef struct
{
    uint32_t total[2];          /*!< number of bytes processed  */
    uint32_t state[4];          /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */

    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
}
md5_context;

/**
 * \brief          Initialize MD5 context
 *
 * \param ctx      MD5 context to be initialized
 */
void md5_init( md5_context *ctx );

/**
 * \brief          Clear MD5 context
 *
 * \param ctx      MD5 context to be cleared
 */
void md5_free( md5_context *ctx );

/**
 * \brief          MD5 context setup
 *
 * \param ctx      context to be initialized
 */
void md5_starts( md5_context *ctx );

/**
 * \brief          MD5 process buffer
 *
 * \param ctx      MD5 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void md5_update( md5_context *ctx, const unsigned char *input, size_t ilen );

/**
 * \brief          MD5 final digest
 *
 * \param ctx      MD5 context
 * \param output   MD5 checksum result
 */
void md5_finish( md5_context *ctx, unsigned char output[16] );

/* Internal use */
void md5_process( md5_context *ctx, const unsigned char data[64] );

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_MD5_ALT */

#endif /* POLARSSL_MD5_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Output = MD5( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   MD5 checksum result
 */
void md5( const unsigned char *input, size_t ilen, unsigned char output[16] );

/**
 * \brief          Output = MD5( file contents )
 *
 * \param path     input file name
 * \param output   MD5 checksum result
 *
 * \return         0 if successful, or POLARSSL_ERR_MD5_FILE_IO_ERROR
 */
int md5_file( const char *path, unsigned char output[16] );

/**
 * \brief          MD5 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void md5_hmac_starts( md5_context *ctx,
                      const unsigned char *key, size_t keylen );

/**
 * \brief          MD5 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void md5_hmac_update( md5_context *ctx,
                      const unsigned char *input, size_t ilen );

/**
 * \brief          MD5 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   MD5 HMAC checksum result
 */
void md5_hmac_finish( md5_context *ctx, unsigned char output[16] );

/**
 * \brief          MD5 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void md5_hmac_reset( md5_context *ctx );

/**
 * \brief          Output = HMAC-MD5( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-MD5 result
 */
void md5_hmac( const unsigned char *key, size_t keylen,
               const unsigned char *input, size_t ilen,
               unsigned char output[16] );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int md5_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* md5.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/md2.h ************/


/**
 * \file md2.h
 *
 * \brief MD2 message digest algorithm (hash function)
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_MD2_H
#define POLARSSL_MD2_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#define POLARSSL_ERR_MD2_FILE_IO_ERROR                 -0x0070  /**< Read/write error in file. */

#if !defined(POLARSSL_MD2_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          MD2 context structure
 */
typedef struct
{
    unsigned char cksum[16];    /*!< checksum of the data block */
    unsigned char state[48];    /*!< intermediate digest state  */
    unsigned char buffer[16];   /*!< data block being processed */

    unsigned char ipad[16];     /*!< HMAC: inner padding        */
    unsigned char opad[16];     /*!< HMAC: outer padding        */
    size_t left;                /*!< amount of data in buffer   */
}
md2_context;

/**
 * \brief          Initialize MD2 context
 *
 * \param ctx      MD2 context to be initialized
 */
void md2_init( md2_context *ctx );

/**
 * \brief          Clear MD2 context
 *
 * \param ctx      MD2 context to be cleared
 */
void md2_free( md2_context *ctx );

/**
 * \brief          MD2 context setup
 *
 * \param ctx      context to be initialized
 */
void md2_starts( md2_context *ctx );

/**
 * \brief          MD2 process buffer
 *
 * \param ctx      MD2 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void md2_update( md2_context *ctx, const unsigned char *input, size_t ilen );

/**
 * \brief          MD2 final digest
 *
 * \param ctx      MD2 context
 * \param output   MD2 checksum result
 */
void md2_finish( md2_context *ctx, unsigned char output[16] );

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_MD2_ALT */

#endif /* POLARSSL_MD2_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Output = MD2( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   MD2 checksum result
 */
void md2( const unsigned char *input, size_t ilen, unsigned char output[16] );

/**
 * \brief          Output = MD2( file contents )
 *
 * \param path     input file name
 * \param output   MD2 checksum result
 *
 * \return         0 if successful, or POLARSSL_ERR_MD2_FILE_IO_ERROR
 */
int md2_file( const char *path, unsigned char output[16] );

/**
 * \brief          MD2 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void md2_hmac_starts( md2_context *ctx, const unsigned char *key,
                      size_t keylen );

/**
 * \brief          MD2 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void md2_hmac_update( md2_context *ctx, const unsigned char *input,
                      size_t ilen );

/**
 * \brief          MD2 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   MD2 HMAC checksum result
 */
void md2_hmac_finish( md2_context *ctx, unsigned char output[16] );

/**
 * \brief          MD2 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void md2_hmac_reset( md2_context *ctx );

/**
 * \brief          Output = HMAC-MD2( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-MD2 result
 */
void md2_hmac( const unsigned char *key, size_t keylen,
               const unsigned char *input, size_t ilen,
               unsigned char output[16] );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int md2_self_test( int verbose );

/* Internal use */
void md2_process( md2_context *ctx );

#ifdef __cplusplus
}
#endif

#endif /* md2.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/md4.h ************/


/**
 * \file md4.h
 *
 * \brief MD4 message digest algorithm (hash function)
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_MD4_H
#define POLARSSL_MD4_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

#define POLARSSL_ERR_MD4_FILE_IO_ERROR                 -0x0072  /**< Read/write error in file. */

#if !defined(POLARSSL_MD4_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          MD4 context structure
 */
typedef struct
{
    uint32_t total[2];          /*!< number of bytes processed  */
    uint32_t state[4];          /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */

    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
}
md4_context;

/**
 * \brief          Initialize MD4 context
 *
 * \param ctx      MD4 context to be initialized
 */
void md4_init( md4_context *ctx );

/**
 * \brief          Clear MD4 context
 *
 * \param ctx      MD4 context to be cleared
 */
void md4_free( md4_context *ctx );

/**
 * \brief          MD4 context setup
 *
 * \param ctx      context to be initialized
 */
void md4_starts( md4_context *ctx );

/**
 * \brief          MD4 process buffer
 *
 * \param ctx      MD4 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void md4_update( md4_context *ctx, const unsigned char *input, size_t ilen );

/**
 * \brief          MD4 final digest
 *
 * \param ctx      MD4 context
 * \param output   MD4 checksum result
 */
void md4_finish( md4_context *ctx, unsigned char output[16] );

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_MD4_ALT */

#endif /* POLARSSL_MD4_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Output = MD4( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   MD4 checksum result
 */
void md4( const unsigned char *input, size_t ilen, unsigned char output[16] );

/**
 * \brief          Output = MD4( file contents )
 *
 * \param path     input file name
 * \param output   MD4 checksum result
 *
 * \return         0 if successful, or POLARSSL_ERR_MD4_FILE_IO_ERROR
 */
int md4_file( const char *path, unsigned char output[16] );

/**
 * \brief          MD4 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void md4_hmac_starts( md4_context *ctx, const unsigned char *key,
                      size_t keylen );

/**
 * \brief          MD4 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void md4_hmac_update( md4_context *ctx, const unsigned char *input,
                      size_t ilen );

/**
 * \brief          MD4 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   MD4 HMAC checksum result
 */
void md4_hmac_finish( md4_context *ctx, unsigned char output[16] );

/**
 * \brief          MD4 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void md4_hmac_reset( md4_context *ctx );

/**
 * \brief          Output = HMAC-MD4( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-MD4 result
 */
void md4_hmac( const unsigned char *key, size_t keylen,
               const unsigned char *input, size_t ilen,
               unsigned char output[16] );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int md4_self_test( int verbose );

/* Internal use */
void md4_process( md4_context *ctx, const unsigned char data[64] );

#ifdef __cplusplus
}
#endif

#endif /* md4.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/rsa.h ************/


/**
 * \file rsa.h
 *
 * \brief The RSA public-key cryptosystem
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_RSA_H
#define POLARSSL_RSA_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif




#if defined(POLARSSL_THREADING_C)

#endif

/*
 * RSA Error codes
 */
#define POLARSSL_ERR_RSA_BAD_INPUT_DATA                    -0x4080  /**< Bad input parameters to function. */
#define POLARSSL_ERR_RSA_INVALID_PADDING                   -0x4100  /**< Input data contains invalid padding and is rejected. */
#define POLARSSL_ERR_RSA_KEY_GEN_FAILED                    -0x4180  /**< Something failed during generation of a key. */
#define POLARSSL_ERR_RSA_KEY_CHECK_FAILED                  -0x4200  /**< Key failed to pass the library's validity check. */
#define POLARSSL_ERR_RSA_PUBLIC_FAILED                     -0x4280  /**< The public key operation failed. */
#define POLARSSL_ERR_RSA_PRIVATE_FAILED                    -0x4300  /**< The private key operation failed. */
#define POLARSSL_ERR_RSA_VERIFY_FAILED                     -0x4380  /**< The PKCS#1 verification failed. */
#define POLARSSL_ERR_RSA_OUTPUT_TOO_LARGE                  -0x4400  /**< The output buffer for decryption is not large enough. */
#define POLARSSL_ERR_RSA_RNG_FAILED                        -0x4480  /**< The random generator failed to generate non-zeros. */

/*
 * RSA constants
 */
#define RSA_PUBLIC      0
#define RSA_PRIVATE     1

#define RSA_PKCS_V15    0
#define RSA_PKCS_V21    1

#define RSA_SIGN        1
#define RSA_CRYPT       2

#define RSA_SALT_LEN_ANY    -1

/*
 * The above constants may be used even if the RSA module is compile out,
 * eg for alternative (PKCS#11) RSA implemenations in the PK layers.
 */
#if defined(POLARSSL_RSA_C)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          RSA context structure
 */
typedef struct
{
    int ver;                    /*!<  always 0          */
    size_t len;                 /*!<  size(N) in chars  */

    mpi N;                      /*!<  public modulus    */
    mpi E;                      /*!<  public exponent   */

    mpi D;                      /*!<  private exponent  */
    mpi P;                      /*!<  1st prime factor  */
    mpi Q;                      /*!<  2nd prime factor  */
    mpi DP;                     /*!<  D % (P - 1)       */
    mpi DQ;                     /*!<  D % (Q - 1)       */
    mpi QP;                     /*!<  1 / (Q % P)       */

    mpi RN;                     /*!<  cached R^2 mod N  */
    mpi RP;                     /*!<  cached R^2 mod P  */
    mpi RQ;                     /*!<  cached R^2 mod Q  */

    mpi Vi;                     /*!<  cached blinding value     */
    mpi Vf;                     /*!<  cached un-blinding value  */

    int padding;                /*!<  RSA_PKCS_V15 for 1.5 padding and
                                      RSA_PKCS_v21 for OAEP/PSS         */
    int hash_id;                /*!<  Hash identifier of md_type_t as
                                      specified in the md.h header file
                                      for the EME-OAEP and EMSA-PSS
                                      encoding                          */
#if defined(POLARSSL_THREADING_C)
    threading_mutex_t mutex;    /*!<  Thread-safety mutex       */
#endif
}
rsa_context;

/**
 * \brief          Initialize an RSA context
 *
 *                 Note: Set padding to RSA_PKCS_V21 for the RSAES-OAEP
 *                 encryption scheme and the RSASSA-PSS signature scheme.
 *
 * \param ctx      RSA context to be initialized
 * \param padding  RSA_PKCS_V15 or RSA_PKCS_V21
 * \param hash_id  RSA_PKCS_V21 hash identifier
 *
 * \note           The hash_id parameter is actually ignored
 *                 when using RSA_PKCS_V15 padding.
 *
 * \note           Choice of padding mode is strictly enforced for private key
 *                 operations, since there might be security concerns in
 *                 mixing padding modes. For public key operations it's merely
 *                 a default value, which can be overriden by calling specific
 *                 rsa_rsaes_xxx or rsa_rsassa_xxx functions.
 *
 * \note           The chosen hash is always used for OEAP encryption.
 *                 For PSS signatures, it's always used for making signatures,
 *                 but can be overriden (and always is, if set to
 *                 POLARSSL_MD_NONE) for verifying them.
 */
void rsa_init( rsa_context *ctx,
               int padding,
               int hash_id);

/**
 * \brief          Set padding for an already initialized RSA context
 *                 See \c rsa_init() for details.
 *
 * \param ctx      RSA context to be set
 * \param padding  RSA_PKCS_V15 or RSA_PKCS_V21
 * \param hash_id  RSA_PKCS_V21 hash identifier
 */
void rsa_set_padding( rsa_context *ctx, int padding, int hash_id);

/**
 * \brief          Generate an RSA keypair
 *
 * \param ctx      RSA context that will hold the key
 * \param f_rng    RNG function
 * \param p_rng    RNG parameter
 * \param nbits    size of the public key in bits
 * \param exponent public exponent (e.g., 65537)
 *
 * \note           rsa_init() must be called beforehand to setup
 *                 the RSA context.
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 */
int rsa_gen_key( rsa_context *ctx,
                 int (*f_rng)(void *, unsigned char *, size_t),
                 void *p_rng,
                 unsigned int nbits, int exponent );

/**
 * \brief          Check a public RSA key
 *
 * \param ctx      RSA context to be checked
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 */
int rsa_check_pubkey( const rsa_context *ctx );

/**
 * \brief          Check a private RSA key
 *
 * \param ctx      RSA context to be checked
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 */
int rsa_check_privkey( const rsa_context *ctx );

/**
 * \brief          Check a public-private RSA key pair.
 *                 Check each of the contexts, and make sure they match.
 *
 * \param pub      RSA context holding the public key
 * \param prv      RSA context holding the private key
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 */
int rsa_check_pub_priv( const rsa_context *pub, const rsa_context *prv );

/**
 * \brief          Do an RSA public key operation
 *
 * \param ctx      RSA context
 * \param input    input buffer
 * \param output   output buffer
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           This function does NOT take care of message
 *                 padding. Also, be sure to set input[0] = 0 or assure that
 *                 input is smaller than N.
 *
 * \note           The input and output buffers must be large
 *                 enough (eg. 128 bytes if RSA-1024 is used).
 */
int rsa_public( rsa_context *ctx,
                const unsigned char *input,
                unsigned char *output );

/**
 * \brief          Do an RSA private key operation
 *
 * \param ctx      RSA context
 * \param f_rng    RNG function (Needed for blinding)
 * \param p_rng    RNG parameter
 * \param input    input buffer
 * \param output   output buffer
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The input and output buffers must be large
 *                 enough (eg. 128 bytes if RSA-1024 is used).
 */
int rsa_private( rsa_context *ctx,
                 int (*f_rng)(void *, unsigned char *, size_t),
                 void *p_rng,
                 const unsigned char *input,
                 unsigned char *output );

/**
 * \brief          Generic wrapper to perform a PKCS#1 encryption using the
 *                 mode from the context. Add the message padding, then do an
 *                 RSA operation.
 *
 * \param ctx      RSA context
 * \param f_rng    RNG function (Needed for padding and PKCS#1 v2.1 encoding
 *                               and RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param ilen     contains the plaintext length
 * \param input    buffer holding the data to be encrypted
 * \param output   buffer that will hold the ciphertext
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The output buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 */
int rsa_pkcs1_encrypt( rsa_context *ctx,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng,
                       int mode, size_t ilen,
                       const unsigned char *input,
                       unsigned char *output );

/**
 * \brief          Perform a PKCS#1 v1.5 encryption (RSAES-PKCS1-v1_5-ENCRYPT)
 *
 * \param ctx      RSA context
 * \param f_rng    RNG function (Needed for padding and RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param ilen     contains the plaintext length
 * \param input    buffer holding the data to be encrypted
 * \param output   buffer that will hold the ciphertext
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The output buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 */
int rsa_rsaes_pkcs1_v15_encrypt( rsa_context *ctx,
                                 int (*f_rng)(void *, unsigned char *, size_t),
                                 void *p_rng,
                                 int mode, size_t ilen,
                                 const unsigned char *input,
                                 unsigned char *output );

/**
 * \brief          Perform a PKCS#1 v2.1 OAEP encryption (RSAES-OAEP-ENCRYPT)
 *
 * \param ctx      RSA context
 * \param f_rng    RNG function (Needed for padding and PKCS#1 v2.1 encoding
 *                               and RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param label    buffer holding the custom label to use
 * \param label_len contains the label length
 * \param ilen     contains the plaintext length
 * \param input    buffer holding the data to be encrypted
 * \param output   buffer that will hold the ciphertext
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The output buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 */
int rsa_rsaes_oaep_encrypt( rsa_context *ctx,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng,
                            int mode,
                            const unsigned char *label, size_t label_len,
                            size_t ilen,
                            const unsigned char *input,
                            unsigned char *output );

/**
 * \brief          Generic wrapper to perform a PKCS#1 decryption using the
 *                 mode from the context. Do an RSA operation, then remove
 *                 the message padding
 *
 * \param ctx      RSA context
 * \param f_rng    RNG function (Only needed for RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param olen     will contain the plaintext length
 * \param input    buffer holding the encrypted data
 * \param output   buffer that will hold the plaintext
 * \param output_max_len    maximum length of the output buffer
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The output buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used) otherwise
 *                 an error is thrown.
 */
int rsa_pkcs1_decrypt( rsa_context *ctx,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng,
                       int mode, size_t *olen,
                       const unsigned char *input,
                       unsigned char *output,
                       size_t output_max_len );

/**
 * \brief          Perform a PKCS#1 v1.5 decryption (RSAES-PKCS1-v1_5-DECRYPT)
 *
 * \param ctx      RSA context
 * \param f_rng    RNG function (Only needed for RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param olen     will contain the plaintext length
 * \param input    buffer holding the encrypted data
 * \param output   buffer that will hold the plaintext
 * \param output_max_len    maximum length of the output buffer
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The output buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used) otherwise
 *                 an error is thrown.
 */
int rsa_rsaes_pkcs1_v15_decrypt( rsa_context *ctx,
                                 int (*f_rng)(void *, unsigned char *, size_t),
                                 void *p_rng,
                                 int mode, size_t *olen,
                                 const unsigned char *input,
                                 unsigned char *output,
                                 size_t output_max_len );

/**
 * \brief          Perform a PKCS#1 v2.1 OAEP decryption (RSAES-OAEP-DECRYPT)
 *
 * \param ctx      RSA context
 * \param f_rng    RNG function (Only needed for RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param label    buffer holding the custom label to use
 * \param label_len contains the label length
 * \param olen     will contain the plaintext length
 * \param input    buffer holding the encrypted data
 * \param output   buffer that will hold the plaintext
 * \param output_max_len    maximum length of the output buffer
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The output buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used) otherwise
 *                 an error is thrown.
 */
int rsa_rsaes_oaep_decrypt( rsa_context *ctx,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng,
                            int mode,
                            const unsigned char *label, size_t label_len,
                            size_t *olen,
                            const unsigned char *input,
                            unsigned char *output,
                            size_t output_max_len );

/**
 * \brief          Generic wrapper to perform a PKCS#1 signature using the
 *                 mode from the context. Do a private RSA operation to sign
 *                 a message digest
 *
 * \param ctx      RSA context
 * \param f_rng    RNG function (Needed for PKCS#1 v2.1 encoding and for
 *                               RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param md_alg   a POLARSSL_MD_* (use POLARSSL_MD_NONE for signing raw data)
 * \param hashlen  message digest length (for POLARSSL_MD_NONE only)
 * \param hash     buffer holding the message digest
 * \param sig      buffer that will hold the ciphertext
 *
 * \return         0 if the signing operation was successful,
 *                 or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The "sig" buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 *
 * \note           In case of PKCS#1 v2.1 encoding, see comments on
 * \note           \c rsa_rsassa_pss_sign() for details on md_alg and hash_id.
 */
int rsa_pkcs1_sign( rsa_context *ctx,
                    int (*f_rng)(void *, unsigned char *, size_t),
                    void *p_rng,
                    int mode,
                    md_type_t md_alg,
                    unsigned int hashlen,
                    const unsigned char *hash,
                    unsigned char *sig );

/**
 * \brief          Perform a PKCS#1 v1.5 signature (RSASSA-PKCS1-v1_5-SIGN)
 *
 * \param ctx      RSA context
 * \param f_rng    RNG function (Only needed for RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param md_alg   a POLARSSL_MD_* (use POLARSSL_MD_NONE for signing raw data)
 * \param hashlen  message digest length (for POLARSSL_MD_NONE only)
 * \param hash     buffer holding the message digest
 * \param sig      buffer that will hold the ciphertext
 *
 * \return         0 if the signing operation was successful,
 *                 or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The "sig" buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 */
int rsa_rsassa_pkcs1_v15_sign( rsa_context *ctx,
                               int (*f_rng)(void *, unsigned char *, size_t),
                               void *p_rng,
                               int mode,
                               md_type_t md_alg,
                               unsigned int hashlen,
                               const unsigned char *hash,
                               unsigned char *sig );

/**
 * \brief          Perform a PKCS#1 v2.1 PSS signature (RSASSA-PSS-SIGN)
 *
 * \param ctx      RSA context
 * \param f_rng    RNG function (Needed for PKCS#1 v2.1 encoding and for
 *                               RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param md_alg   a POLARSSL_MD_* (use POLARSSL_MD_NONE for signing raw data)
 * \param hashlen  message digest length (for POLARSSL_MD_NONE only)
 * \param hash     buffer holding the message digest
 * \param sig      buffer that will hold the ciphertext
 *
 * \return         0 if the signing operation was successful,
 *                 or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The "sig" buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 *
 * \note           The hash_id in the RSA context is the one used for the
 *                 encoding. md_alg in the function call is the type of hash
 *                 that is encoded. According to RFC 3447 it is advised to
 *                 keep both hashes the same.
 */
int rsa_rsassa_pss_sign( rsa_context *ctx,
                         int (*f_rng)(void *, unsigned char *, size_t),
                         void *p_rng,
                         int mode,
                         md_type_t md_alg,
                         unsigned int hashlen,
                         const unsigned char *hash,
                         unsigned char *sig );

/**
 * \brief          Generic wrapper to perform a PKCS#1 verification using the
 *                 mode from the context. Do a public RSA operation and check
 *                 the message digest
 *
 * \param ctx      points to an RSA public key
 * \param f_rng    RNG function (Only needed for RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param md_alg   a POLARSSL_MD_* (use POLARSSL_MD_NONE for signing raw data)
 * \param hashlen  message digest length (for POLARSSL_MD_NONE only)
 * \param hash     buffer holding the message digest
 * \param sig      buffer holding the ciphertext
 *
 * \return         0 if the verify operation was successful,
 *                 or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The "sig" buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 *
 * \note           In case of PKCS#1 v2.1 encoding, see comments on
 *                 \c rsa_rsassa_pss_verify() about md_alg and hash_id.
 */
int rsa_pkcs1_verify( rsa_context *ctx,
                      int (*f_rng)(void *, unsigned char *, size_t),
                      void *p_rng,
                      int mode,
                      md_type_t md_alg,
                      unsigned int hashlen,
                      const unsigned char *hash,
                      const unsigned char *sig );

/**
 * \brief          Perform a PKCS#1 v1.5 verification (RSASSA-PKCS1-v1_5-VERIFY)
 *
 * \param ctx      points to an RSA public key
 * \param f_rng    RNG function (Only needed for RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param md_alg   a POLARSSL_MD_* (use POLARSSL_MD_NONE for signing raw data)
 * \param hashlen  message digest length (for POLARSSL_MD_NONE only)
 * \param hash     buffer holding the message digest
 * \param sig      buffer holding the ciphertext
 *
 * \return         0 if the verify operation was successful,
 *                 or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The "sig" buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 */
int rsa_rsassa_pkcs1_v15_verify( rsa_context *ctx,
                                 int (*f_rng)(void *, unsigned char *, size_t),
                                 void *p_rng,
                                 int mode,
                                 md_type_t md_alg,
                                 unsigned int hashlen,
                                 const unsigned char *hash,
                                 const unsigned char *sig );

/**
 * \brief          Perform a PKCS#1 v2.1 PSS verification (RSASSA-PSS-VERIFY)
 *                 (This is the "simple" version.)
 *
 * \param ctx      points to an RSA public key
 * \param f_rng    RNG function (Only needed for RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param md_alg   a POLARSSL_MD_* (use POLARSSL_MD_NONE for signing raw data)
 * \param hashlen  message digest length (for POLARSSL_MD_NONE only)
 * \param hash     buffer holding the message digest
 * \param sig      buffer holding the ciphertext
 *
 * \return         0 if the verify operation was successful,
 *                 or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The "sig" buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 *
 * \note           The hash_id in the RSA context is the one used for the
 *                 verification. md_alg in the function call is the type of
 *                 hash that is verified. According to RFC 3447 it is advised to
 *                 keep both hashes the same. If hash_id in the RSA context is
 *                 unset, the md_alg from the function call is used.
 */
int rsa_rsassa_pss_verify( rsa_context *ctx,
                           int (*f_rng)(void *, unsigned char *, size_t),
                           void *p_rng,
                           int mode,
                           md_type_t md_alg,
                           unsigned int hashlen,
                           const unsigned char *hash,
                           const unsigned char *sig );

/**
 * \brief          Perform a PKCS#1 v2.1 PSS verification (RSASSA-PSS-VERIFY)
 *                 (This is the version with "full" options.)
 *
 * \param ctx      points to an RSA public key
 * \param f_rng    RNG function (Only needed for RSA_PRIVATE)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param md_alg   a POLARSSL_MD_* (use POLARSSL_MD_NONE for signing raw data)
 * \param hashlen  message digest length (for POLARSSL_MD_NONE only)
 * \param hash     buffer holding the message digest
 * \param mgf1_hash_id message digest used for mask generation
 * \param expected_salt_len Length of the salt used in padding, use
 *                 RSA_SALT_LEN_ANY to accept any salt length
 * \param sig      buffer holding the ciphertext
 *
 * \return         0 if the verify operation was successful,
 *                 or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The "sig" buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 *
 * \note           The hash_id in the RSA context is ignored.
 */
int rsa_rsassa_pss_verify_ext( rsa_context *ctx,
                               int (*f_rng)(void *, unsigned char *, size_t),
                               void *p_rng,
                               int mode,
                               md_type_t md_alg,
                               unsigned int hashlen,
                               const unsigned char *hash,
                               md_type_t mgf1_hash_id,
                               int expected_salt_len,
                               const unsigned char *sig );

/**
 * \brief          Copy the components of an RSA context
 *
 * \param dst      Destination context
 * \param src      Source context
 *
 * \return         O on success,
 *                 POLARSSL_ERR_MPI_MALLOC_FAILED on memory allocation failure
 */
int rsa_copy( rsa_context *dst, const rsa_context *src );

/**
 * \brief          Free the components of an RSA key
 *
 * \param ctx      RSA Context to free
 */
void rsa_free( rsa_context *ctx );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int rsa_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* POLARSSL_RSA_C */

#endif /* rsa.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/asn1.h ************/


/**
 * \file asn1.h
 *
 * \brief Generic ASN.1 parsing
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_ASN1_H
#define POLARSSL_ASN1_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(POLARSSL_BIGNUM_C)

#endif

/**
 * \addtogroup asn1_module
 * \{
 */

/**
 * \name ASN1 Error codes
 * These error codes are OR'ed to X509 error codes for
 * higher error granularity.
 * ASN1 is a standard to specify data structures.
 * \{
 */
#define POLARSSL_ERR_ASN1_OUT_OF_DATA                      -0x0060  /**< Out of data when parsing an ASN1 data structure. */
#define POLARSSL_ERR_ASN1_UNEXPECTED_TAG                   -0x0062  /**< ASN1 tag was of an unexpected value. */
#define POLARSSL_ERR_ASN1_INVALID_LENGTH                   -0x0064  /**< Error when trying to determine the length or invalid length. */
#define POLARSSL_ERR_ASN1_LENGTH_MISMATCH                  -0x0066  /**< Actual length differs from expected length. */
#define POLARSSL_ERR_ASN1_INVALID_DATA                     -0x0068  /**< Data is invalid. (not used) */
#define POLARSSL_ERR_ASN1_MALLOC_FAILED                    -0x006A  /**< Memory allocation failed */
#define POLARSSL_ERR_ASN1_BUF_TOO_SMALL                    -0x006C  /**< Buffer too small when writing ASN.1 data structure. */

/* \} name */

/**
 * \name DER constants
 * These constants comply with DER encoded the ANS1 type tags.
 * DER encoding uses hexadecimal representation.
 * An example DER sequence is:\n
 * - 0x02 -- tag indicating INTEGER
 * - 0x01 -- length in octets
 * - 0x05 -- value
 * Such sequences are typically read into \c ::x509_buf.
 * \{
 */
#define ASN1_BOOLEAN                 0x01
#define ASN1_INTEGER                 0x02
#define ASN1_BIT_STRING              0x03
#define ASN1_OCTET_STRING            0x04
#define ASN1_NULL                    0x05
#define ASN1_OID                     0x06
#define ASN1_UTF8_STRING             0x0C
#define ASN1_SEQUENCE                0x10
#define ASN1_SET                     0x11
#define ASN1_PRINTABLE_STRING        0x13
#define ASN1_T61_STRING              0x14
#define ASN1_IA5_STRING              0x16
#define ASN1_UTC_TIME                0x17
#define ASN1_GENERALIZED_TIME        0x18
#define ASN1_UNIVERSAL_STRING        0x1C
#define ASN1_BMP_STRING              0x1E
#define ASN1_PRIMITIVE               0x00
#define ASN1_CONSTRUCTED             0x20
#define ASN1_CONTEXT_SPECIFIC        0x80
/* \} name */
/* \} addtogroup asn1_module */

/** Returns the size of the binary string, without the trailing \\0 */
#define OID_SIZE(x) (sizeof(x) - 1)

/**
 * Compares an asn1_buf structure to a reference OID.
 *
 * Only works for 'defined' oid_str values (OID_HMAC_SHA1), you cannot use a
 * 'unsigned char *oid' here!
 *
 * Warning: returns true when the OIDs are equal (unlike memcmp)!
 */
#define OID_CMP(oid_str, oid_buf)                                   \
        ( ( OID_SIZE(oid_str) == (oid_buf)->len ) &&                \
          memcmp( (oid_str), (oid_buf)->p, (oid_buf)->len) == 0 )

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \name Functions to parse ASN.1 data structures
 * \{
 */

/**
 * Type-length-value structure that allows for ASN1 using DER.
 */
typedef struct _asn1_buf
{
    int tag;                /**< ASN1 type, e.g. ASN1_UTF8_STRING. */
    size_t len;             /**< ASN1 length, e.g. in octets. */
    unsigned char *p;       /**< ASN1 data, e.g. in ASCII. */
}
asn1_buf;

/**
 * Container for ASN1 bit strings.
 */
typedef struct _asn1_bitstring
{
    size_t len;                 /**< ASN1 length, e.g. in octets. */
    unsigned char unused_bits;  /**< Number of unused bits at the end of the string */
    unsigned char *p;           /**< Raw ASN1 data for the bit string */
}
asn1_bitstring;

/**
 * Container for a sequence of ASN.1 items
 */
typedef struct _asn1_sequence
{
    asn1_buf buf;                   /**< Buffer containing the given ASN.1 item. */
    struct _asn1_sequence *next;    /**< The next entry in the sequence. */
}
asn1_sequence;

/**
 * Container for a sequence or list of 'named' ASN.1 data items
 */
typedef struct _asn1_named_data
{
    asn1_buf oid;                   /**< The object identifier. */
    asn1_buf val;                   /**< The named value. */
    struct _asn1_named_data *next;  /**< The next entry in the sequence. */
    unsigned char next_merged;      /**< Merge next item into the current one? */
}
asn1_named_data;

/**
 * \brief       Get the length of an ASN.1 element.
 *              Updates the pointer to immediately behind the length.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param len   The variable that will receive the value
 *
 * \return      0 if successful, POLARSSL_ERR_ASN1_OUT_OF_DATA on reaching
 *              end of data, POLARSSL_ERR_ASN1_INVALID_LENGTH if length is
 *              unparseable.
 */
int asn1_get_len( unsigned char **p,
                  const unsigned char *end,
                  size_t *len );

/**
 * \brief       Get the tag and length of the tag. Check for the requested tag.
 *              Updates the pointer to immediately behind the tag and length.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param len   The variable that will receive the length
 * \param tag   The expected tag
 *
 * \return      0 if successful, POLARSSL_ERR_ASN1_UNEXPECTED_TAG if tag did
 *              not match requested tag, or another specific ASN.1 error code.
 */
int asn1_get_tag( unsigned char **p,
                  const unsigned char *end,
                  size_t *len, int tag );

/**
 * \brief       Retrieve a boolean ASN.1 tag and its value.
 *              Updates the pointer to immediately behind the full tag.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param val   The variable that will receive the value
 *
 * \return      0 if successful or a specific ASN.1 error code.
 */
int asn1_get_bool( unsigned char **p,
                   const unsigned char *end,
                   int *val );

/**
 * \brief       Retrieve an integer ASN.1 tag and its value.
 *              Updates the pointer to immediately behind the full tag.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param val   The variable that will receive the value
 *
 * \return      0 if successful or a specific ASN.1 error code.
 */
int asn1_get_int( unsigned char **p,
                  const unsigned char *end,
                  int *val );

/**
 * \brief       Retrieve a bitstring ASN.1 tag and its value.
 *              Updates the pointer to immediately behind the full tag.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param bs    The variable that will receive the value
 *
 * \return      0 if successful or a specific ASN.1 error code.
 */
int asn1_get_bitstring( unsigned char **p, const unsigned char *end,
                        asn1_bitstring *bs);

/**
 * \brief       Retrieve a bitstring ASN.1 tag without unused bits and its
 *              value.
 *              Updates the pointer to the beginning of the bit/octet string.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param len   Length of the actual bit/octect string in bytes
 *
 * \return      0 if successful or a specific ASN.1 error code.
 */
int asn1_get_bitstring_null( unsigned char **p, const unsigned char *end,
                             size_t *len );

/**
 * \brief       Parses and splits an ASN.1 "SEQUENCE OF <tag>"
 *              Updated the pointer to immediately behind the full sequence tag.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param cur   First variable in the chain to fill
 * \param tag   Type of sequence
 *
 * \return      0 if successful or a specific ASN.1 error code.
 */
int asn1_get_sequence_of( unsigned char **p,
                          const unsigned char *end,
                          asn1_sequence *cur,
                          int tag);

#if defined(POLARSSL_BIGNUM_C)
/**
 * \brief       Retrieve a MPI value from an integer ASN.1 tag.
 *              Updates the pointer to immediately behind the full tag.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param X     The MPI that will receive the value
 *
 * \return      0 if successful or a specific ASN.1 or MPI error code.
 */
int asn1_get_mpi( unsigned char **p,
                  const unsigned char *end,
                  mpi *X );
#endif /* POLARSSL_BIGNUM_C */

/**
 * \brief       Retrieve an AlgorithmIdentifier ASN.1 sequence.
 *              Updates the pointer to immediately behind the full
 *              AlgorithmIdentifier.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param alg   The buffer to receive the OID
 * \param params The buffer to receive the params (if any)
 *
 * \return      0 if successful or a specific ASN.1 or MPI error code.
 */
int asn1_get_alg( unsigned char **p,
                  const unsigned char *end,
                  asn1_buf *alg, asn1_buf *params );

/**
 * \brief       Retrieve an AlgorithmIdentifier ASN.1 sequence with NULL or no
 *              params.
 *              Updates the pointer to immediately behind the full
 *              AlgorithmIdentifier.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param alg   The buffer to receive the OID
 *
 * \return      0 if successful or a specific ASN.1 or MPI error code.
 */
int asn1_get_alg_null( unsigned char **p,
                       const unsigned char *end,
                       asn1_buf *alg );

/**
 * \brief       Find a specific named_data entry in a sequence or list based on
 *              the OID.
 *
 * \param list  The list to seek through
 * \param oid   The OID to look for
 * \param len   Size of the OID
 *
 * \return      NULL if not found, or a pointer to the existing entry.
 */
asn1_named_data *asn1_find_named_data( asn1_named_data *list,
                                       const char *oid, size_t len );

/**
 * \brief       Free a asn1_named_data entry
 *
 * \param entry The named data entry to free
 */
void asn1_free_named_data( asn1_named_data *entry );

/**
 * \brief       Free all entries in a asn1_named_data list
 *              Head will be set to NULL
 *
 * \param head  Pointer to the head of the list of named data entries to free
 */
void asn1_free_named_data_list( asn1_named_data **head );

#ifdef __cplusplus
}
#endif

#endif /* asn1.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/pk.h ************/


/**
 * \file pk.h
 *
 * \brief Public Key abstraction layer
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef POLARSSL_PK_H
#define POLARSSL_PK_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif



#if defined(POLARSSL_RSA_C)

#endif

#if defined(POLARSSL_ECP_C)

#endif

#if defined(POLARSSL_ECDSA_C)

#endif

#define POLARSSL_ERR_PK_MALLOC_FAILED       -0x2F80  /**< Memory alloation failed. */
#define POLARSSL_ERR_PK_TYPE_MISMATCH       -0x2F00  /**< Type mismatch, eg attempt to encrypt with an ECDSA key */
#define POLARSSL_ERR_PK_BAD_INPUT_DATA      -0x2E80  /**< Bad input parameters to function. */
#define POLARSSL_ERR_PK_FILE_IO_ERROR       -0x2E00  /**< Read/write of file failed. */
#define POLARSSL_ERR_PK_KEY_INVALID_VERSION -0x2D80  /**< Unsupported key version */
#define POLARSSL_ERR_PK_KEY_INVALID_FORMAT  -0x2D00  /**< Invalid key tag or value. */
#define POLARSSL_ERR_PK_UNKNOWN_PK_ALG      -0x2C80  /**< Key algorithm is unsupported (only RSA and EC are supported). */
#define POLARSSL_ERR_PK_PASSWORD_REQUIRED   -0x2C00  /**< Private key password can't be empty. */
#define POLARSSL_ERR_PK_PASSWORD_MISMATCH   -0x2B80  /**< Given private key password does not allow for correct decryption. */
#define POLARSSL_ERR_PK_INVALID_PUBKEY      -0x2B00  /**< The pubkey tag or value is invalid (only RSA and EC are supported). */
#define POLARSSL_ERR_PK_INVALID_ALG         -0x2A80  /**< The algorithm tag or value is invalid. */
#define POLARSSL_ERR_PK_UNKNOWN_NAMED_CURVE -0x2A00  /**< Elliptic curve is unsupported (only NIST curves are supported). */
#define POLARSSL_ERR_PK_FEATURE_UNAVAILABLE -0x2980  /**< Unavailable feature, e.g. RSA disabled for RSA key. */
#define POLARSSL_ERR_PK_SIG_LEN_MISMATCH    -0x2000  /**< The signature is valid but its length is less than expected. */


#if defined(POLARSSL_RSA_C)
/**
 * Quick access to an RSA context inside a PK context.
 *
 * \warning You must make sure the PK context actually holds an RSA context
 * before using this macro!
 */
#define pk_rsa( pk )        ( (rsa_context *) (pk).pk_ctx )
#endif /* POLARSSL_RSA_C */

#if defined(POLARSSL_ECP_C)
/**
 * Quick access to an EC context inside a PK context.
 *
 * \warning You must make sure the PK context actually holds an EC context
 * before using this macro!
 */
#define pk_ec( pk )         ( (ecp_keypair *) (pk).pk_ctx )
#endif /* POLARSSL_ECP_C */


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Public key types
 */
typedef enum {
    POLARSSL_PK_NONE=0,
    POLARSSL_PK_RSA,
    POLARSSL_PK_ECKEY,
    POLARSSL_PK_ECKEY_DH,
    POLARSSL_PK_ECDSA,
    POLARSSL_PK_RSA_ALT,
    POLARSSL_PK_RSASSA_PSS,
} pk_type_t;

/**
 * \brief           Options for RSASSA-PSS signature verification.
 *                  See \c rsa_rsassa_pss_verify_ext()
 */
typedef struct
{
    md_type_t mgf1_hash_id;
    int expected_salt_len;

} pk_rsassa_pss_options;

/**
 * \brief           Types for interfacing with the debug module
 */
typedef enum
{
    POLARSSL_PK_DEBUG_NONE = 0,
    POLARSSL_PK_DEBUG_MPI,
    POLARSSL_PK_DEBUG_ECP,
} pk_debug_type;

/**
 * \brief           Item to send to the debug module
 */
typedef struct
{
    pk_debug_type type;
    const char *name;
    void *value;
} pk_debug_item;

/** Maximum number of item send for debugging, plus 1 */
#define POLARSSL_PK_DEBUG_MAX_ITEMS 3

/**
 * \brief           Public key information and operations
 */
typedef struct
{
    /** Public key type */
    pk_type_t type;

    /** Type name */
    const char *name;

    /** Get key size in bits */
    size_t (*get_size)( const void * );

    /** Tell if the context implements this type (e.g. ECKEY can do ECDSA) */
    int (*can_do)( pk_type_t type );

    /** Verify signature */
    int (*verify_func)( void *ctx, md_type_t md_alg,
                        const unsigned char *hash, size_t hash_len,
                        const unsigned char *sig, size_t sig_len );

    /** Make signature */
    int (*sign_func)( void *ctx, md_type_t md_alg,
                      const unsigned char *hash, size_t hash_len,
                      unsigned char *sig, size_t *sig_len,
                      int (*f_rng)(void *, unsigned char *, size_t),
                      void *p_rng );

    /** Decrypt message */
    int (*decrypt_func)( void *ctx, const unsigned char *input, size_t ilen,
                         unsigned char *output, size_t *olen, size_t osize,
                         int (*f_rng)(void *, unsigned char *, size_t),
                         void *p_rng );

    /** Encrypt message */
    int (*encrypt_func)( void *ctx, const unsigned char *input, size_t ilen,
                         unsigned char *output, size_t *olen, size_t osize,
                         int (*f_rng)(void *, unsigned char *, size_t),
                         void *p_rng );

    /** Check public-private key pair */
    int (*check_pair_func)( const void *pub, const void *prv );

    /** Allocate a new context */
    void * (*ctx_alloc_func)( void );

    /** Free the given context */
    void (*ctx_free_func)( void *ctx );

    /** Interface with the debug module */
    void (*debug_func)( const void *ctx, pk_debug_item *items );

} pk_info_t;

/**
 * \brief           Public key container
 */
typedef struct
{
    const pk_info_t *   pk_info;    /**< Public key informations        */
    void *              pk_ctx;     /**< Underlying public key context  */
} pk_context;

/**
 * \brief           Types for RSA-alt abstraction
 */
typedef int (*pk_rsa_alt_decrypt_func)( void *ctx, int mode, size_t *olen,
                    const unsigned char *input, unsigned char *output,
                    size_t output_max_len );
typedef int (*pk_rsa_alt_sign_func)( void *ctx,
                    int (*f_rng)(void *, unsigned char *, size_t), void *p_rng,
                    int mode, md_type_t md_alg, unsigned int hashlen,
                    const unsigned char *hash, unsigned char *sig );
typedef size_t (*pk_rsa_alt_key_len_func)( void *ctx );

/**
 * \brief           Return information associated with the given PK type
 *
 * \param pk_type   PK type to search for.
 *
 * \return          The PK info associated with the type or NULL if not found.
 */
const pk_info_t *pk_info_from_type( pk_type_t pk_type );

/**
 * \brief           Initialize a pk_context (as NONE)
 */
void pk_init( pk_context *ctx );

/**
 * \brief           Free a pk_context
 */
void pk_free( pk_context *ctx );

/**
 * \brief           Initialize a PK context with the information given
 *                  and allocates the type-specific PK subcontext.
 *
 * \param ctx       Context to initialize. Must be empty (type NONE).
 * \param info      Information to use
 *
 * \return          0 on success,
 *                  POLARSSL_ERR_PK_BAD_INPUT_DATA on invalid input,
 *                  POLARSSL_ERR_PK_MALLOC_FAILED on allocation failure.
 *
 * \note            For contexts holding an RSA-alt key, use
 *                  \c pk_init_ctx_rsa_alt() instead.
 */
int pk_init_ctx( pk_context *ctx, const pk_info_t *info );

/**
 * \brief           Initialize an RSA-alt context
 *
 * \param ctx       Context to initialize. Must be empty (type NONE).
 * \param key       RSA key pointer
 * \param decrypt_func  Decryption function
 * \param sign_func     Signing function
 * \param key_len_func  Function returning key length in bytes
 *
 * \return          0 on success, or POLARSSL_ERR_PK_BAD_INPUT_DATA if the
 *                  context wasn't already initialized as RSA_ALT.
 *
 * \note            This function replaces \c pk_init_ctx() for RSA-alt.
 */
int pk_init_ctx_rsa_alt( pk_context *ctx, void * key,
                         pk_rsa_alt_decrypt_func decrypt_func,
                         pk_rsa_alt_sign_func sign_func,
                         pk_rsa_alt_key_len_func key_len_func );

/**
 * \brief           Get the size in bits of the underlying key
 *
 * \param ctx       Context to use
 *
 * \return          Key size in bits, or 0 on error
 */
size_t pk_get_size( const pk_context *ctx );

/**
 * \brief           Get the length in bytes of the underlying key
 * \param ctx       Context to use
 *
 * \return          Key length in bytes, or 0 on error
 */
static inline size_t pk_get_len( const pk_context *ctx )
{
    return( ( pk_get_size( ctx ) + 7 ) / 8 );
}

/**
 * \brief           Tell if a context can do the operation given by type
 *
 * \param ctx       Context to test
 * \param type      Target type
 *
 * \return          0 if context can't do the operations,
 *                  1 otherwise.
 */
int pk_can_do( pk_context *ctx, pk_type_t type );

/**
 * \brief           Verify signature (including padding if relevant).
 *
 * \param ctx       PK context to use
 * \param md_alg    Hash algorithm used (see notes)
 * \param hash      Hash of the message to sign
 * \param hash_len  Hash length or 0 (see notes)
 * \param sig       Signature to verify
 * \param sig_len   Signature length
 *
 * \return          0 on success (signature is valid),
 *                  POLARSSL_ERR_PK_SIG_LEN_MISMATCH if the signature is
 *                  valid but its actual length is less than sig_len,
 *                  or a specific error code.
 *
 * \note            For RSA keys, the default padding type is PKCS#1 v1.5.
 *                  Use \c pk_verify_ext( POLARSSL_PK_RSASSA_PSS, ... )
 *                  to verify RSASSA_PSS signatures.
 *
 * \note            If hash_len is 0, then the length associated with md_alg
 *                  is used instead, or an error returned if it is invalid.
 *
 * \note            md_alg may be POLARSSL_MD_NONE, only if hash_len != 0
 */
int pk_verify( pk_context *ctx, md_type_t md_alg,
               const unsigned char *hash, size_t hash_len,
               const unsigned char *sig, size_t sig_len );

/**
 * \brief           Verify signature, with options.
 *                  (Includes verification of the padding depending on type.)
 *
 * \param type      Signature type (inc. possible padding type) to verify
 * \param options   Pointer to type-specific options, or NULL
 * \param ctx       PK context to use
 * \param md_alg    Hash algorithm used (see notes)
 * \param hash      Hash of the message to sign
 * \param hash_len  Hash length or 0 (see notes)
 * \param sig       Signature to verify
 * \param sig_len   Signature length
 *
 * \return          0 on success (signature is valid),
 *                  POLARSSL_ERR_PK_TYPE_MISMATCH if the PK context can't be
 *                  used for this type of signatures,
 *                  POLARSSL_ERR_PK_SIG_LEN_MISMATCH if the signature is
 *                  valid but its actual length is less than sig_len,
 *                  or a specific error code.
 *
 * \note            If hash_len is 0, then the length associated with md_alg
 *                  is used instead, or an error returned if it is invalid.
 *
 * \note            md_alg may be POLARSSL_MD_NONE, only if hash_len != 0
 *
 * \note            If type is POLARSSL_PK_RSASSA_PSS, then options must point
 *                  to a pk_rsassa_pss_options structure,
 *                  otherwise it must be NULL.
 */
int pk_verify_ext( pk_type_t type, const void *options,
                   pk_context *ctx, md_type_t md_alg,
                   const unsigned char *hash, size_t hash_len,
                   const unsigned char *sig, size_t sig_len );

/**
 * \brief           Make signature, including padding if relevant.
 *
 * \param ctx       PK context to use
 * \param md_alg    Hash algorithm used (see notes)
 * \param hash      Hash of the message to sign
 * \param hash_len  Hash length or 0 (see notes)
 * \param sig       Place to write the signature
 * \param sig_len   Number of bytes written
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 on success, or a specific error code.
 *
 * \note            For RSA keys, the default padding type is PKCS#1 v1.5.
 *                  There is no interface in the PK module to make RSASSA-PSS
 *                  signatures yet.
 *
 * \note            If hash_len is 0, then the length associated with md_alg
 *                  is used instead, or an error returned if it is invalid.
 *
 * \note            md_alg may be POLARSSL_MD_NONE, only if hash_len != 0
 */
int pk_sign( pk_context *ctx, md_type_t md_alg,
             const unsigned char *hash, size_t hash_len,
             unsigned char *sig, size_t *sig_len,
             int (*f_rng)(void *, unsigned char *, size_t), void *p_rng );

/**
 * \brief           Decrypt message (including padding if relevant).
 *
 * \param ctx       PK context to use
 * \param input     Input to decrypt
 * \param ilen      Input size
 * \param output    Decrypted output
 * \param olen      Decrypted message length
 * \param osize     Size of the output buffer
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \note            For RSA keys, the default padding type is PKCS#1 v1.5.
 *
 * \return          0 on success, or a specific error code.
 */
int pk_decrypt( pk_context *ctx,
                const unsigned char *input, size_t ilen,
                unsigned char *output, size_t *olen, size_t osize,
                int (*f_rng)(void *, unsigned char *, size_t), void *p_rng );

/**
 * \brief           Encrypt message (including padding if relevant).
 *
 * \param ctx       PK context to use
 * \param input     Message to encrypt
 * \param ilen      Message size
 * \param output    Encrypted output
 * \param olen      Encrypted output length
 * \param osize     Size of the output buffer
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \note            For RSA keys, the default padding type is PKCS#1 v1.5.
 *
 * \return          0 on success, or a specific error code.
 */
int pk_encrypt( pk_context *ctx,
                const unsigned char *input, size_t ilen,
                unsigned char *output, size_t *olen, size_t osize,
                int (*f_rng)(void *, unsigned char *, size_t), void *p_rng );

/**
 * \brief           Check if a public-private pair of keys matches.
 *
 * \param pub       Context holding a public key.
 * \param prv       Context holding a private (and public) key.
 *
 * \return          0 on success or POLARSSL_ERR_PK_BAD_INPUT_DATA
 */
int pk_check_pair( const pk_context *pub, const pk_context *prv );

/**
 * \brief           Export debug information
 *
 * \param ctx       Context to use
 * \param items     Place to write debug items
 *
 * \return          0 on success or POLARSSL_ERR_PK_BAD_INPUT_DATA
 */
int pk_debug( const pk_context *ctx, pk_debug_item *items );

/**
 * \brief           Access the type name
 *
 * \param ctx       Context to use
 *
 * \return          Type name on success, or "invalid PK"
 */
const char * pk_get_name( const pk_context *ctx );

/**
 * \brief           Get the key type
 *
 * \param ctx       Context to use
 *
 * \return          Type on success, or POLARSSL_PK_NONE
 */
pk_type_t pk_get_type( const pk_context *ctx );

#if defined(POLARSSL_PK_PARSE_C)
/** \ingroup pk_module */
/**
 * \brief           Parse a private key
 *
 * \param ctx       key to be initialized
 * \param key       input buffer
 * \param keylen    size of the buffer
 * \param pwd       password for decryption (optional)
 * \param pwdlen    size of the password
 *
 * \note            On entry, ctx must be empty, either freshly initialised
 *                  with pk_init() or reset with pk_free(). If you need a
 *                  specific key type, check the result with pk_can_do().
 *
 * \note            The key is also checked for correctness.
 *
 * \return          0 if successful, or a specific PK or PEM error code
 */
int pk_parse_key( pk_context *ctx,
                  const unsigned char *key, size_t keylen,
                  const unsigned char *pwd, size_t pwdlen );

/** \ingroup pk_module */
/**
 * \brief           Parse a public key
 *
 * \param ctx       key to be initialized
 * \param key       input buffer
 * \param keylen    size of the buffer
 *
 * \note            On entry, ctx must be empty, either freshly initialised
 *                  with pk_init() or reset with pk_free(). If you need a
 *                  specific key type, check the result with pk_can_do().
 *
 * \note            The key is also checked for correctness.
 *
 * \return          0 if successful, or a specific PK or PEM error code
 */
int pk_parse_public_key( pk_context *ctx,
                         const unsigned char *key, size_t keylen );

#if defined(POLARSSL_FS_IO)
/** \ingroup pk_module */
/**
 * \brief           Load and parse a private key
 *
 * \param ctx       key to be initialized
 * \param path      filename to read the private key from
 * \param password  password to decrypt the file (can be NULL)
 *
 * \note            On entry, ctx must be empty, either freshly initialised
 *                  with pk_init() or reset with pk_free(). If you need a
 *                  specific key type, check the result with pk_can_do().
 *
 * \note            The key is also checked for correctness.
 *
 * \return          0 if successful, or a specific PK or PEM error code
 */
int pk_parse_keyfile( pk_context *ctx,
                      const char *path, const char *password );

/** \ingroup pk_module */
/**
 * \brief           Load and parse a public key
 *
 * \param ctx       key to be initialized
 * \param path      filename to read the private key from
 *
 * \note            On entry, ctx must be empty, either freshly initialised
 *                  with pk_init() or reset with pk_free(). If you need a
 *                  specific key type, check the result with pk_can_do().
 *
 * \note            The key is also checked for correctness.
 *
 * \return          0 if successful, or a specific PK or PEM error code
 */
int pk_parse_public_keyfile( pk_context *ctx, const char *path );
#endif /* POLARSSL_FS_IO */
#endif /* POLARSSL_PK_PARSE_C */

#if defined(POLARSSL_PK_WRITE_C)
/**
 * \brief           Write a private key to a PKCS#1 or SEC1 DER structure
 *                  Note: data is written at the end of the buffer! Use the
 *                        return value to determine where you should start
 *                        using the buffer
 *
 * \param ctx       private to write away
 * \param buf       buffer to write to
 * \param size      size of the buffer
 *
 * \return          length of data written if successful, or a specific
 *                  error code
 */
int pk_write_key_der( pk_context *ctx, unsigned char *buf, size_t size );

/**
 * \brief           Write a public key to a SubjectPublicKeyInfo DER structure
 *                  Note: data is written at the end of the buffer! Use the
 *                        return value to determine where you should start
 *                        using the buffer
 *
 * \param ctx       public key to write away
 * \param buf       buffer to write to
 * \param size      size of the buffer
 *
 * \return          length of data written if successful, or a specific
 *                  error code
 */
int pk_write_pubkey_der( pk_context *ctx, unsigned char *buf, size_t size );

#if defined(POLARSSL_PEM_WRITE_C)
/**
 * \brief           Write a public key to a PEM string
 *
 * \param ctx       public key to write away
 * \param buf       buffer to write to
 * \param size      size of the buffer
 *
 * \return          0 successful, or a specific error code
 */
int pk_write_pubkey_pem( pk_context *ctx, unsigned char *buf, size_t size );

/**
 * \brief           Write a private key to a PKCS#1 or SEC1 PEM string
 *
 * \param ctx       private to write away
 * \param buf       buffer to write to
 * \param size      size of the buffer
 *
 * \return          0 successful, or a specific error code
 */
int pk_write_key_pem( pk_context *ctx, unsigned char *buf, size_t size );
#endif /* POLARSSL_PEM_WRITE_C */
#endif /* POLARSSL_PK_WRITE_C */

/*
 * WARNING: Low-level functions. You probably do not want to use these unless
 *          you are certain you do ;)
 */

#if defined(POLARSSL_PK_PARSE_C)
/**
 * \brief           Parse a SubjectPublicKeyInfo DER structure
 *
 * \param p         the position in the ASN.1 data
 * \param end       end of the buffer
 * \param pk        the key to fill
 *
 * \return          0 if successful, or a specific PK error code
 */
int pk_parse_subpubkey( unsigned char **p, const unsigned char *end,
                        pk_context *pk );
#endif /* POLARSSL_PK_PARSE_C */

#if defined(POLARSSL_PK_WRITE_C)
/**
 * \brief           Write a subjectPublicKey to ASN.1 data
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param key       public key to write away
 *
 * \return          the length written or a negative error code
 */
int pk_write_pubkey( unsigned char **p, unsigned char *start,
                     const pk_context *key );
#endif /* POLARSSL_PK_WRITE_C */

/*
 * Internal module functions. You probably do not want to use these unless you
 * know you do.
 */
#if defined(POLARSSL_FS_IO)
int pk_load_file( const char *path, unsigned char **buf, size_t *n );
#endif

#ifdef __cplusplus
}
#endif

#endif /* POLARSSL_PK_H */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/pk_wrap.h ************/


/**
 * \file pk.h
 *
 * \brief Public Key abstraction layer: wrapper functions
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef POLARSSL_PK_WRAP_H
#define POLARSSL_PK_WRAP_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif



/* Container for RSA-alt */
typedef struct
{
    void *key;
    pk_rsa_alt_decrypt_func decrypt_func;
    pk_rsa_alt_sign_func sign_func;
    pk_rsa_alt_key_len_func key_len_func;
} rsa_alt_context;

#if defined(POLARSSL_RSA_C)
extern const pk_info_t rsa_info;
#endif

#if defined(POLARSSL_ECP_C)
extern const pk_info_t eckey_info;
extern const pk_info_t eckeydh_info;
#endif

#if defined(POLARSSL_ECDSA_C)
extern const pk_info_t ecdsa_info;
#endif

extern const pk_info_t rsa_alt_info;

#endif /* POLARSSL_PK_WRAP_H */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/x509.h ************/


/**
 * \file x509.h
 *
 * \brief X.509 generic defines and structures
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_X509_H
#define POLARSSL_X509_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif




#if defined(POLARSSL_RSA_C)

#endif

/**
 * \addtogroup x509_module
 * \{
 */

#if !defined(POLARSSL_X509_MAX_INTERMEDIATE_CA)
/**
 * Maximum number of intermediate CAs in a verification chain.
 * That is, maximum length of the chain, excluding the end-entity certificate
 * and the trusted root certificate.
 *
 * Set this to a low value to prevent an adversary from making you waste
 * resources verifying an overlong certificate chain.
 */
#define POLARSSL_X509_MAX_INTERMEDIATE_CA   8
#endif

/**
 * \name X509 Error codes
 * \{
 */
#define POLARSSL_ERR_X509_FEATURE_UNAVAILABLE              -0x2080  /**< Unavailable feature, e.g. RSA hashing/encryption combination. */
#define POLARSSL_ERR_X509_UNKNOWN_OID                      -0x2100  /**< Requested OID is unknown. */
#define POLARSSL_ERR_X509_INVALID_FORMAT                   -0x2180  /**< The CRT/CRL/CSR format is invalid, e.g. different type expected. */
#define POLARSSL_ERR_X509_INVALID_VERSION                  -0x2200  /**< The CRT/CRL/CSR version element is invalid. */
#define POLARSSL_ERR_X509_INVALID_SERIAL                   -0x2280  /**< The serial tag or value is invalid. */
#define POLARSSL_ERR_X509_INVALID_ALG                      -0x2300  /**< The algorithm tag or value is invalid. */
#define POLARSSL_ERR_X509_INVALID_NAME                     -0x2380  /**< The name tag or value is invalid. */
#define POLARSSL_ERR_X509_INVALID_DATE                     -0x2400  /**< The date tag or value is invalid. */
#define POLARSSL_ERR_X509_INVALID_SIGNATURE                -0x2480  /**< The signature tag or value invalid. */
#define POLARSSL_ERR_X509_INVALID_EXTENSIONS               -0x2500  /**< The extension tag or value is invalid. */
#define POLARSSL_ERR_X509_UNKNOWN_VERSION                  -0x2580  /**< CRT/CRL/CSR has an unsupported version number. */
#define POLARSSL_ERR_X509_UNKNOWN_SIG_ALG                  -0x2600  /**< Signature algorithm (oid) is unsupported. */
#define POLARSSL_ERR_X509_SIG_MISMATCH                     -0x2680  /**< Signature algorithms do not match. (see \c ::x509_crt sig_oid) */
#define POLARSSL_ERR_X509_CERT_VERIFY_FAILED               -0x2700  /**< Certificate verification failed, e.g. CRL, CA or signature check failed. */
#define POLARSSL_ERR_X509_CERT_UNKNOWN_FORMAT              -0x2780  /**< Format not recognized as DER or PEM. */
#define POLARSSL_ERR_X509_BAD_INPUT_DATA                   -0x2800  /**< Input invalid. */
#define POLARSSL_ERR_X509_MALLOC_FAILED                    -0x2880  /**< Allocation of memory failed. */
#define POLARSSL_ERR_X509_FILE_IO_ERROR                    -0x2900  /**< Read/write of file failed. */
/* \} name */

/**
 * \name X509 Verify codes
 * \{
 */
/* Reminder: update x509_crt_verify_strings[] in library/x509_crt.c */
#define BADCERT_EXPIRED             0x01  /**< The certificate validity has expired. */
#define BADCERT_REVOKED             0x02  /**< The certificate has been revoked (is on a CRL). */
#define BADCERT_CN_MISMATCH         0x04  /**< The certificate Common Name (CN) does not match with the expected CN. */
#define BADCERT_NOT_TRUSTED         0x08  /**< The certificate is not correctly signed by the trusted CA. */
#define BADCRL_NOT_TRUSTED          0x10  /**< The CRL is not correctly signed by the trusted CA. */
#define BADCRL_EXPIRED              0x20  /**< The CRL is expired. */
#define BADCERT_MISSING             0x40  /**< Certificate was missing. */
#define BADCERT_SKIP_VERIFY         0x80  /**< Certificate verification was skipped. */
#define BADCERT_OTHER             0x0100  /**< Other reason (can be used by verify callback) */
#define BADCERT_FUTURE            0x0200  /**< The certificate validity starts in the future. */
#define BADCRL_FUTURE             0x0400  /**< The CRL is from the future */
#define BADCERT_KEY_USAGE         0x0800  /**< Usage does not match the keyUsage extension. */
#define BADCERT_EXT_KEY_USAGE     0x1000  /**< Usage does not match the extendedKeyUsage extension. */
#define BADCERT_NS_CERT_TYPE      0x2000  /**< Usage does not match the nsCertType extension. */
/* \} name */
/* \} addtogroup x509_module */

/*
 * X.509 v3 Key Usage Extension flags
 */
#define KU_DIGITAL_SIGNATURE            (0x80)  /* bit 0 */
#define KU_NON_REPUDIATION              (0x40)  /* bit 1 */
#define KU_KEY_ENCIPHERMENT             (0x20)  /* bit 2 */
#define KU_DATA_ENCIPHERMENT            (0x10)  /* bit 3 */
#define KU_KEY_AGREEMENT                (0x08)  /* bit 4 */
#define KU_KEY_CERT_SIGN                (0x04)  /* bit 5 */
#define KU_CRL_SIGN                     (0x02)  /* bit 6 */

/*
 * Netscape certificate types
 * (http://www.mozilla.org/projects/security/pki/nss/tech-notes/tn3.html)
 */

#define NS_CERT_TYPE_SSL_CLIENT         (0x80)  /* bit 0 */
#define NS_CERT_TYPE_SSL_SERVER         (0x40)  /* bit 1 */
#define NS_CERT_TYPE_EMAIL              (0x20)  /* bit 2 */
#define NS_CERT_TYPE_OBJECT_SIGNING     (0x10)  /* bit 3 */
#define NS_CERT_TYPE_RESERVED           (0x08)  /* bit 4 */
#define NS_CERT_TYPE_SSL_CA             (0x04)  /* bit 5 */
#define NS_CERT_TYPE_EMAIL_CA           (0x02)  /* bit 6 */
#define NS_CERT_TYPE_OBJECT_SIGNING_CA  (0x01)  /* bit 7 */

/*
 * X.509 extension types
 *
 * Comments refer to the status for using certificates. Status can be
 * different for writing certificates or reading CRLs or CSRs.
 */
#define EXT_AUTHORITY_KEY_IDENTIFIER    (1 << 0)
#define EXT_SUBJECT_KEY_IDENTIFIER      (1 << 1)
#define EXT_KEY_USAGE                   (1 << 2)    /* Parsed but not used */
#define EXT_CERTIFICATE_POLICIES        (1 << 3)
#define EXT_POLICY_MAPPINGS             (1 << 4)
#define EXT_SUBJECT_ALT_NAME            (1 << 5)    /* Supported (DNS) */
#define EXT_ISSUER_ALT_NAME             (1 << 6)
#define EXT_SUBJECT_DIRECTORY_ATTRS     (1 << 7)
#define EXT_BASIC_CONSTRAINTS           (1 << 8)    /* Supported */
#define EXT_NAME_CONSTRAINTS            (1 << 9)
#define EXT_POLICY_CONSTRAINTS          (1 << 10)
#define EXT_EXTENDED_KEY_USAGE          (1 << 11)   /* Parsed but not used */
#define EXT_CRL_DISTRIBUTION_POINTS     (1 << 12)
#define EXT_INIHIBIT_ANYPOLICY          (1 << 13)
#define EXT_FRESHEST_CRL                (1 << 14)

#define EXT_NS_CERT_TYPE                (1 << 16)   /* Parsed (and then ?) */

/*
 * Storage format identifiers
 * Recognized formats: PEM and DER
 */
#define X509_FORMAT_DER                 1
#define X509_FORMAT_PEM                 2

#define X509_MAX_DN_NAME_SIZE         256 /**< Maximum value size of a DN entry */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup x509_module
 * \{ */

/**
 * \name Structures for parsing X.509 certificates, CRLs and CSRs
 * \{
 */

/**
 * Type-length-value structure that allows for ASN1 using DER.
 */
typedef asn1_buf x509_buf;

/**
 * Container for ASN1 bit strings.
 */
typedef asn1_bitstring x509_bitstring;

/**
 * Container for ASN1 named information objects.
 * It allows for Relative Distinguished Names (e.g. cn=polarssl,ou=code,etc.).
 */
typedef asn1_named_data x509_name;

/**
 * Container for a sequence of ASN.1 items
 */
typedef asn1_sequence x509_sequence;

/** Container for date and time (precision in seconds). */
typedef struct _x509_time
{
    int year, mon, day;         /**< Date. */
    int hour, min, sec;         /**< Time. */
}
x509_time;

/** \} name Structures for parsing X.509 certificates, CRLs and CSRs */
/** \} addtogroup x509_module */

/**
 * \brief          Store the certificate DN in printable form into buf;
 *                 no more than size characters will be written.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param dn       The X509 name to represent
 *
 * \return         The amount of data written to the buffer, or -1 in
 *                 case of an error.
 */
int x509_dn_gets( char *buf, size_t size, const x509_name *dn );

/**
 * \brief          Store the certificate serial in printable form into buf;
 *                 no more than size characters will be written.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param serial   The X509 serial to represent
 *
 * \return         The amount of data written to the buffer, or -1 in
 *                 case of an error.
 */
int x509_serial_gets( char *buf, size_t size, const x509_buf *serial );

#if ! defined(POLARSSL_DEPRECATED_REMOVED)
#if defined(POLARSSL_DEPRECATED_WARNING)
#define DEPRECATED    __attribute__((deprecated))
#else
#define DEPRECATED
#endif
/**
 * \brief          Give an known OID, return its descriptive string.
 *
 * \deprecated     Use oid_get_extended_key_usage() instead.
 *
 * \warning        Only works for extended_key_usage OIDs!
 *
 * \param oid      buffer containing the oid
 *
 * \return         Return a string if the OID is known,
 *                 or NULL otherwise.
 */
const char *x509_oid_get_description( x509_buf *oid ) DEPRECATED;

/**
 * \brief          Give an OID, return a string version of its OID number.
 *
 * \deprecated     Use oid_get_numeric_string() instead.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param oid      Buffer containing the OID
 *
 * \return         Length of the string written (excluding final NULL) or
 *                 POLARSSL_ERR_OID_BUF_TO_SMALL in case of error
 */
int x509_oid_get_numeric_string( char *buf, size_t size, x509_buf *oid ) DEPRECATED;
#undef DEPRECATED
#endif /* POLARSSL_DEPRECATED_REMOVED */

/**
 * \brief          Check a given x509_time against the system time and check
 *                 if it is not expired.
 *
 * \param time     x509_time to check
 *
 * \return         0 if the x509_time is still valid,
 *                 1 otherwise.
 */
int x509_time_expired( const x509_time *time );

/**
 * \brief          Check a given x509_time against the system time and check
 *                 if it is not from the future.
 *
 * \param time     x509_time to check
 *
 * \return         0 if the x509_time is already valid,
 *                 1 otherwise.
 */
int x509_time_future( const x509_time *time );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int x509_self_test( int verbose );

/*
 * Internal module functions. You probably do not want to use these unless you
 * know you do.
 */
int x509_get_name( unsigned char **p, const unsigned char *end,
                   x509_name *cur );
int x509_get_alg_null( unsigned char **p, const unsigned char *end,
                       x509_buf *alg );
int x509_get_alg( unsigned char **p, const unsigned char *end,
                  x509_buf *alg, x509_buf *params );
#if defined(POLARSSL_X509_RSASSA_PSS_SUPPORT)
int x509_get_rsassa_pss_params( const x509_buf *params,
                                md_type_t *md_alg, md_type_t *mgf_md,
                                int *salt_len );
#endif
int x509_get_sig( unsigned char **p, const unsigned char *end, x509_buf *sig );
int x509_get_sig_alg( const x509_buf *sig_oid, const x509_buf *sig_params,
                      md_type_t *md_alg, pk_type_t *pk_alg,
                      void **sig_opts );
int x509_get_time( unsigned char **p, const unsigned char *end,
                   x509_time *time );
int x509_get_serial( unsigned char **p, const unsigned char *end,
                     x509_buf *serial );
int x509_get_ext( unsigned char **p, const unsigned char *end,
                  x509_buf *ext, int tag );
int x509_sig_alg_gets( char *buf, size_t size, const x509_buf *sig_oid,
                       pk_type_t pk_alg, md_type_t md_alg,
                       const void *sig_opts );
int x509_key_size_helper( char *buf, size_t size, const char *name );
int x509_string_to_names( asn1_named_data **head, const char *name );
int x509_set_extension( asn1_named_data **head, const char *oid, size_t oid_len,
                        int critical, const unsigned char *val,
                        size_t val_len );
int x509_write_extensions( unsigned char **p, unsigned char *start,
                           asn1_named_data *first );
int x509_write_names( unsigned char **p, unsigned char *start,
                      asn1_named_data *first );
int x509_write_sig( unsigned char **p, unsigned char *start,
                    const char *oid, size_t oid_len,
                    unsigned char *sig, size_t size );

#ifdef __cplusplus
}
#endif

#endif /* x509.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/ecp.h ************/


/**
 * \file ecp.h
 *
 * \brief Elliptic curves over GF(p)
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_ECP_H
#define POLARSSL_ECP_H



/*
 * ECP error codes
 */
#define POLARSSL_ERR_ECP_BAD_INPUT_DATA                    -0x4F80  /**< Bad input parameters to function. */
#define POLARSSL_ERR_ECP_BUFFER_TOO_SMALL                  -0x4F00  /**< The buffer is too small to write to. */
#define POLARSSL_ERR_ECP_FEATURE_UNAVAILABLE               -0x4E80  /**< Requested curve not available. */
#define POLARSSL_ERR_ECP_VERIFY_FAILED                     -0x4E00  /**< The signature is not valid. */
#define POLARSSL_ERR_ECP_MALLOC_FAILED                     -0x4D80  /**< Memory allocation failed. */
#define POLARSSL_ERR_ECP_RANDOM_FAILED                     -0x4D00  /**< Generation of random value, such as (ephemeral) key, failed. */
#define POLARSSL_ERR_ECP_INVALID_KEY                       -0x4C80  /**< Invalid private or public key. */
#define POLARSSL_ERR_ECP_SIG_LEN_MISMATCH                  -0x4C00  /**< Signature is valid but shorter than the user-supplied length. */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Domain parameters (curve, subgroup and generator) identifiers.
 *
 * Only curves over prime fields are supported.
 *
 * \warning This library does not support validation of arbitrary domain
 * parameters. Therefore, only well-known domain parameters from trusted
 * sources should be used. See ecp_use_known_dp().
 */
typedef enum
{
    POLARSSL_ECP_DP_NONE = 0,
    POLARSSL_ECP_DP_SECP192R1,      /*!< 192-bits NIST curve  */
    POLARSSL_ECP_DP_SECP224R1,      /*!< 224-bits NIST curve  */
    POLARSSL_ECP_DP_SECP256R1,      /*!< 256-bits NIST curve  */
    POLARSSL_ECP_DP_SECP384R1,      /*!< 384-bits NIST curve  */
    POLARSSL_ECP_DP_SECP521R1,      /*!< 521-bits NIST curve  */
    POLARSSL_ECP_DP_BP256R1,        /*!< 256-bits Brainpool curve */
    POLARSSL_ECP_DP_BP384R1,        /*!< 384-bits Brainpool curve */
    POLARSSL_ECP_DP_BP512R1,        /*!< 512-bits Brainpool curve */
    POLARSSL_ECP_DP_M221,           /*!< (not implemented yet)    */
    POLARSSL_ECP_DP_M255,           /*!< Curve25519               */
    POLARSSL_ECP_DP_M383,           /*!< (not implemented yet)    */
    POLARSSL_ECP_DP_M511,           /*!< (not implemented yet)    */
    POLARSSL_ECP_DP_SECP192K1,      /*!< 192-bits "Koblitz" curve */
    POLARSSL_ECP_DP_SECP224K1,      /*!< 224-bits "Koblitz" curve */
    POLARSSL_ECP_DP_SECP256K1,      /*!< 256-bits "Koblitz" curve */
} ecp_group_id;

/**
 * Number of supported curves (plus one for NONE).
 *
 * (Montgomery curves excluded for now.)
 */
#define POLARSSL_ECP_DP_MAX     12

/**
 * Curve information for use by other modules
 */
typedef struct
{
    ecp_group_id grp_id;    /*!< Internal identifier        */
    uint16_t tls_id;        /*!< TLS NamedCurve identifier  */
    uint16_t size;          /*!< Curve size in bits         */
    const char *name;       /*!< Human-friendly name        */
} ecp_curve_info;

/**
 * \brief           ECP point structure (jacobian coordinates)
 *
 * \note            All functions expect and return points satisfying
 *                  the following condition: Z == 0 or Z == 1. (Other
 *                  values of Z are used by internal functions only.)
 *                  The point is zero, or "at infinity", if Z == 0.
 *                  Otherwise, X and Y are its standard (affine) coordinates.
 */
typedef struct
{
    mpi X;          /*!<  the point's X coordinate  */
    mpi Y;          /*!<  the point's Y coordinate  */
    mpi Z;          /*!<  the point's Z coordinate  */
}
ecp_point;

/**
 * \brief           ECP group structure
 *
 * We consider two types of curves equations:
 * 1. Short Weierstrass y^2 = x^3 + A x + B     mod P   (SEC1 + RFC 4492)
 * 2. Montgomery,       y^2 = x^3 + A x^2 + x   mod P   (M255 + draft)
 * In both cases, a generator G for a prime-order subgroup is fixed. In the
 * short weierstrass, this subgroup is actually the whole curve, and its
 * cardinal is denoted by N.
 *
 * In the case of Short Weierstrass curves, our code requires that N is an odd
 * prime. (Use odd in ecp_mul() and prime in ecdsa_sign() for blinding.)
 *
 * In the case of Montgomery curves, we don't store A but (A + 2) / 4 which is
 * the quantity actually used in the formulas. Also, nbits is not the size of N
 * but the required size for private keys.
 *
 * If modp is NULL, reduction modulo P is done using a generic algorithm.
 * Otherwise, it must point to a function that takes an mpi in the range
 * 0..2^(2*pbits)-1 and transforms it in-place in an integer of little more
 * than pbits, so that the integer may be efficiently brought in the 0..P-1
 * range by a few additions or substractions. It must return 0 on success and
 * non-zero on failure.
 */
typedef struct
{
    ecp_group_id id;    /*!<  internal group identifier                     */
    mpi P;              /*!<  prime modulus of the base field               */
    mpi A;              /*!<  1. A in the equation, or 2. (A + 2) / 4       */
    mpi B;              /*!<  1. B in the equation, or 2. unused            */
    ecp_point G;        /*!<  generator of the (sub)group used              */
    mpi N;              /*!<  1. the order of G, or 2. unused               */
    size_t pbits;       /*!<  number of bits in P                           */
    size_t nbits;       /*!<  number of bits in 1. P, or 2. private keys    */
    unsigned int h;     /*!<  internal: 1 if the constants are static       */
    int (*modp)(mpi *); /*!<  function for fast reduction mod P             */
    int (*t_pre)(ecp_point *, void *);  /*!< unused                         */
    int (*t_post)(ecp_point *, void *); /*!< unused                         */
    void *t_data;                       /*!< unused                         */
    ecp_point *T;       /*!<  pre-computed points for ecp_mul_comb()        */
    size_t T_size;      /*!<  number for pre-computed points                */
}
ecp_group;

/**
 * \brief           ECP key pair structure
 *
 * A generic key pair that could be used for ECDSA, fixed ECDH, etc.
 *
 * \note Members purposefully in the same order as struc ecdsa_context.
 */
typedef struct
{
    ecp_group grp;      /*!<  Elliptic curve and base point     */
    mpi d;              /*!<  our secret value                  */
    ecp_point Q;        /*!<  our public value                  */
}
ecp_keypair;

/**
 * \name SECTION: Module settings
 *
 * The configuration options you can set for this module are in this section.
 * Either change them in config.h or define them on the compiler command line.
 * \{
 */

#if !defined(POLARSSL_ECP_MAX_BITS)
/**
 * Maximum size of the groups (that is, of N and P)
 */
#define POLARSSL_ECP_MAX_BITS     521   /**< Maximum bit size of groups */
#endif

#define POLARSSL_ECP_MAX_BYTES    ( ( POLARSSL_ECP_MAX_BITS + 7 ) / 8 )
#define POLARSSL_ECP_MAX_PT_LEN   ( 2 * POLARSSL_ECP_MAX_BYTES + 1 )

#if !defined(POLARSSL_ECP_WINDOW_SIZE)
/*
 * Maximum "window" size used for point multiplication.
 * Default: 6.
 * Minimum value: 2. Maximum value: 7.
 *
 * Result is an array of at most ( 1 << ( POLARSSL_ECP_WINDOW_SIZE - 1 ) )
 * points used for point multiplication. This value is directly tied to EC
 * peak memory usage, so decreasing it by one should roughly cut memory usage
 * by two (if large curves are in use).
 *
 * Reduction in size may reduce speed, but larger curves are impacted first.
 * Sample performances (in ECDHE handshakes/s, with FIXED_POINT_OPTIM = 1):
 *      w-size:     6       5       4       3       2
 *      521       145     141     135     120      97
 *      384       214     209     198     177     146
 *      256       320     320     303     262     226

 *      224       475     475     453     398     342
 *      192       640     640     633     587     476
 */
#define POLARSSL_ECP_WINDOW_SIZE    6   /**< Maximum window size used */
#endif /* POLARSSL_ECP_WINDOW_SIZE */

#if !defined(POLARSSL_ECP_FIXED_POINT_OPTIM)
/*
 * Trade memory for speed on fixed-point multiplication.
 *
 * This speeds up repeated multiplication of the generator (that is, the
 * multiplication in ECDSA signatures, and half of the multiplications in
 * ECDSA verification and ECDHE) by a factor roughly 3 to 4.
 *
 * The cost is increasing EC peak memory usage by a factor roughly 2.
 *
 * Change this value to 0 to reduce peak memory usage.
 */
#define POLARSSL_ECP_FIXED_POINT_OPTIM  1   /**< Enable fixed-point speed-up */
#endif /* POLARSSL_ECP_FIXED_POINT_OPTIM */

/* \} name SECTION: Module settings */

/*
 * Point formats, from RFC 4492's enum ECPointFormat
 */
#define POLARSSL_ECP_PF_UNCOMPRESSED    0   /**< Uncompressed point format */
#define POLARSSL_ECP_PF_COMPRESSED      1   /**< Compressed point format */

/*
 * Some other constants from RFC 4492
 */
#define POLARSSL_ECP_TLS_NAMED_CURVE    3   /**< ECCurveType's named_curve */

/**
 * \brief           Get the list of supported curves in order of preferrence
 *                  (full information)
 *
 * \return          A statically allocated array, the last entry is 0.
 */
const ecp_curve_info *ecp_curve_list( void );

/**
 * \brief           Get the list of supported curves in order of preferrence
 *                  (grp_id only)
 *
 * \return          A statically allocated array,
 *                  terminated with POLARSSL_ECP_DP_NONE.
 */
const ecp_group_id *ecp_grp_id_list( void );

/**
 * \brief           Get curve information from an internal group identifier
 *
 * \param grp_id    A POLARSSL_ECP_DP_XXX value
 *
 * \return          The associated curve information or NULL
 */
const ecp_curve_info *ecp_curve_info_from_grp_id( ecp_group_id grp_id );

/**
 * \brief           Get curve information from a TLS NamedCurve value
 *
 * \param tls_id    A POLARSSL_ECP_DP_XXX value
 *
 * \return          The associated curve information or NULL
 */
const ecp_curve_info *ecp_curve_info_from_tls_id( uint16_t tls_id );

/**
 * \brief           Get curve information from a human-readable name
 *
 * \param name      The name
 *
 * \return          The associated curve information or NULL
 */
const ecp_curve_info *ecp_curve_info_from_name( const char *name );

/**
 * \brief           Initialize a point (as zero)
 */
void ecp_point_init( ecp_point *pt );

/**
 * \brief           Initialize a group (to something meaningless)
 */
void ecp_group_init( ecp_group *grp );

/**
 * \brief           Initialize a key pair (as an invalid one)
 */
void ecp_keypair_init( ecp_keypair *key );

/**
 * \brief           Free the components of a point
 */
void ecp_point_free( ecp_point *pt );

/**
 * \brief           Free the components of an ECP group
 */
void ecp_group_free( ecp_group *grp );

/**
 * \brief           Free the components of a key pair
 */
void ecp_keypair_free( ecp_keypair *key );

/**
 * \brief           Copy the contents of point Q into P
 *
 * \param P         Destination point
 * \param Q         Source point
 *
 * \return          0 if successful,
 *                  POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int ecp_copy( ecp_point *P, const ecp_point *Q );

/**
 * \brief           Copy the contents of a group object
 *
 * \param dst       Destination group
 * \param src       Source group
 *
 * \return          0 if successful,
 *                  POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int ecp_group_copy( ecp_group *dst, const ecp_group *src );

/**
 * \brief           Set a point to zero
 *
 * \param pt        Destination point
 *
 * \return          0 if successful,
 *                  POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 */
int ecp_set_zero( ecp_point *pt );

/**
 * \brief           Tell if a point is zero
 *
 * \param pt        Point to test
 *
 * \return          1 if point is zero, 0 otherwise
 */
int ecp_is_zero( ecp_point *pt );

/**
 * \brief           Import a non-zero point from two ASCII strings
 *
 * \param P         Destination point
 * \param radix     Input numeric base
 * \param x         First affine coordinate as a null-terminated string
 * \param y         Second affine coordinate as a null-terminated string
 *
 * \return          0 if successful, or a POLARSSL_ERR_MPI_XXX error code
 */
int ecp_point_read_string( ecp_point *P, int radix,
                           const char *x, const char *y );

/**
 * \brief           Export a point into unsigned binary data
 *
 * \param grp       Group to which the point should belong
 * \param P         Point to export
 * \param format    Point format, should be a POLARSSL_ECP_PF_XXX macro
 * \param olen      Length of the actual output
 * \param buf       Output buffer
 * \param buflen    Length of the output buffer
 *
 * \return          0 if successful,
 *                  or POLARSSL_ERR_ECP_BAD_INPUT_DATA
 *                  or POLARSSL_ERR_ECP_BUFFER_TOO_SMALL
 */
int ecp_point_write_binary( const ecp_group *grp, const ecp_point *P,
                            int format, size_t *olen,
                            unsigned char *buf, size_t buflen );

/**
 * \brief           Import a point from unsigned binary data
 *
 * \param grp       Group to which the point should belong
 * \param P         Point to import
 * \param buf       Input buffer
 * \param ilen      Actual length of input
 *
 * \return          0 if successful,
 *                  POLARSSL_ERR_ECP_BAD_INPUT_DATA if input is invalid,
 *                  POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed,
 *                  POLARSSL_ERR_ECP_FEATURE_UNAVAILABLE if the point format
 *                  is not implemented.
 *
 * \note            This function does NOT check that the point actually
 *                  belongs to the given group, see ecp_check_pubkey() for
 *                  that.
 */
int ecp_point_read_binary( const ecp_group *grp, ecp_point *P,
                           const unsigned char *buf, size_t ilen );

/**
 * \brief           Import a point from a TLS ECPoint record
 *
 * \param grp       ECP group used
 * \param pt        Destination point
 * \param buf       $(Start of input buffer)
 * \param len       Buffer length
 *
 * \note            buf is updated to point right after the ECPoint on exit
 *
 * \return          0 if successful,
 *                  POLARSSL_ERR_MPI_XXX if initialization failed
 *                  POLARSSL_ERR_ECP_BAD_INPUT_DATA if input is invalid
 */
int ecp_tls_read_point( const ecp_group *grp, ecp_point *pt,
                        const unsigned char **buf, size_t len );

/**
 * \brief           Export a point as a TLS ECPoint record
 *
 * \param grp       ECP group used
 * \param pt        Point to export
 * \param format    Export format
 * \param olen      length of data written
 * \param buf       Buffer to write to
 * \param blen      Buffer length
 *
 * \return          0 if successful,
 *                  or POLARSSL_ERR_ECP_BAD_INPUT_DATA
 *                  or POLARSSL_ERR_ECP_BUFFER_TOO_SMALL
 */
int ecp_tls_write_point( const ecp_group *grp, const ecp_point *pt,
                         int format, size_t *olen,
                         unsigned char *buf, size_t blen );

/**
 * \brief           Import an ECP group from null-terminated ASCII strings
 *
 * \param grp       Destination group
 * \param radix     Input numeric base
 * \param p         Prime modulus of the base field
 * \param b         Constant term in the equation
 * \param gx        The generator's X coordinate
 * \param gy        The generator's Y coordinate
 * \param n         The generator's order
 *
 * \return          0 if successful, or a POLARSSL_ERR_MPI_XXX error code
 *
 * \note            Sets all fields except modp.
 */
int ecp_group_read_string( ecp_group *grp, int radix,
                           const char *p, const char *b,
                           const char *gx, const char *gy, const char *n);

/**
 * \brief           Set a group using well-known domain parameters
 *
 * \param grp       Destination group
 * \param index     Index in the list of well-known domain parameters
 *
 * \return          0 if successful,
 *                  POLARSSL_ERR_MPI_XXX if initialization failed
 *                  POLARSSL_ERR_ECP_FEATURE_UNAVAILABLE for unkownn groups
 *
 * \note            Index should be a value of RFC 4492's enum NamdeCurve,
 *                  possibly in the form of a POLARSSL_ECP_DP_XXX macro.
 */
int ecp_use_known_dp( ecp_group *grp, ecp_group_id index );

/**
 * \brief           Set a group from a TLS ECParameters record
 *
 * \param grp       Destination group
 * \param buf       &(Start of input buffer)
 * \param len       Buffer length
 *
 * \note            buf is updated to point right after ECParameters on exit
 *
 * \return          0 if successful,
 *                  POLARSSL_ERR_MPI_XXX if initialization failed
 *                  POLARSSL_ERR_ECP_BAD_INPUT_DATA if input is invalid
 */
int ecp_tls_read_group( ecp_group *grp, const unsigned char **buf, size_t len );

/**
 * \brief           Write the TLS ECParameters record for a group
 *
 * \param grp       ECP group used
 * \param olen      Number of bytes actually written
 * \param buf       Buffer to write to
 * \param blen      Buffer length
 *
 * \return          0 if successful,
 *                  or POLARSSL_ERR_ECP_BUFFER_TOO_SMALL
 */
int ecp_tls_write_group( const ecp_group *grp, size_t *olen,
                         unsigned char *buf, size_t blen );

/**
 * \brief           Addition: R = P + Q
 *
 * \param grp       ECP group
 * \param R         Destination point
 * \param P         Left-hand point
 * \param Q         Right-hand point
 *
 * \return          0 if successful,
 *                  POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 *
 * \note            This function does not support Montgomery curves, such as
 *                  Curve25519.
 */
int ecp_add( const ecp_group *grp, ecp_point *R,
             const ecp_point *P, const ecp_point *Q );

/**
 * \brief           Subtraction: R = P - Q
 *
 * \param grp       ECP group
 * \param R         Destination point
 * \param P         Left-hand point
 * \param Q         Right-hand point
 *
 * \return          0 if successful,
 *                  POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 *
 * \note            This function does not support Montgomery curves, such as
 *                  Curve25519.
 */
int ecp_sub( const ecp_group *grp, ecp_point *R,
             const ecp_point *P, const ecp_point *Q );

/**
 * \brief           Multiplication by an integer: R = m * P
 *                  (Not thread-safe to use same group in multiple threads)
 *
 * \param grp       ECP group
 * \param R         Destination point
 * \param m         Integer by which to multiply
 * \param P         Point to multiply
 * \param f_rng     RNG function (see notes)
 * \param p_rng     RNG parameter
 *
 * \return          0 if successful,
 *                  POLARSSL_ERR_ECP_INVALID_KEY if m is not a valid privkey
 *                  or P is not a valid pubkey,
 *                  POLARSSL_ERR_MPI_MALLOC_FAILED if memory allocation failed
 *
 * \note            In order to prevent timing attacks, this function
 *                  executes the exact same sequence of (base field)
 *                  operations for any valid m. It avoids any if-branch or
 *                  array index depending on the value of m.
 *
 * \note            If f_rng is not NULL, it is used to randomize intermediate
 *                  results in order to prevent potential timing attacks
 *                  targeting these results. It is recommended to always
 *                  provide a non-NULL f_rng (the overhead is negligible).
 */
int ecp_mul( ecp_group *grp, ecp_point *R,
             const mpi *m, const ecp_point *P,
             int (*f_rng)(void *, unsigned char *, size_t), void *p_rng );

/**
 * \brief           Check that a point is a valid public key on this curve
 *
 * \param grp       Curve/group the point should belong to
 * \param pt        Point to check
 *
 * \return          0 if point is a valid public key,
 *                  POLARSSL_ERR_ECP_INVALID_KEY otherwise.
 *
 * \note            This function only checks the point is non-zero, has valid
 *                  coordinates and lies on the curve, but not that it is
 *                  indeed a multiple of G. This is additional check is more
 *                  expensive, isn't required by standards, and shouldn't be
 *                  necessary if the group used has a small cofactor. In
 *                  particular, it is useless for the NIST groups which all
 *                  have a cofactor of 1.
 *
 * \note            Uses bare components rather than an ecp_keypair structure
 *                  in order to ease use with other structures such as
 *                  ecdh_context of ecdsa_context.
 */
int ecp_check_pubkey( const ecp_group *grp, const ecp_point *pt );

/**
 * \brief           Check that an mpi is a valid private key for this curve
 *
 * \param grp       Group used
 * \param d         Integer to check
 *
 * \return          0 if point is a valid private key,
 *                  POLARSSL_ERR_ECP_INVALID_KEY otherwise.
 *
 * \note            Uses bare components rather than an ecp_keypair structure
 *                  in order to ease use with other structures such as
 *                  ecdh_context of ecdsa_context.
 */
int ecp_check_privkey( const ecp_group *grp, const mpi *d );

/**
 * \brief           Generate a keypair
 *
 * \param grp       ECP group
 * \param d         Destination MPI (secret part)
 * \param Q         Destination point (public part)
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 if successful,
 *                  or a POLARSSL_ERR_ECP_XXX or POLARSSL_MPI_XXX error code
 *
 * \note            Uses bare components rather than an ecp_keypair structure
 *                  in order to ease use with other structures such as
 *                  ecdh_context of ecdsa_context.
 */
int ecp_gen_keypair( ecp_group *grp, mpi *d, ecp_point *Q,
                     int (*f_rng)(void *, unsigned char *, size_t),
                     void *p_rng );

/**
 * \brief           Generate a keypair
 *
 * \param grp_id    ECP group identifier
 * \param key       Destination keypair
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 if successful,
 *                  or a POLARSSL_ERR_ECP_XXX or POLARSSL_MPI_XXX error code
 */
int ecp_gen_key( ecp_group_id grp_id, ecp_keypair *key,
                int (*f_rng)(void *, unsigned char *, size_t), void *p_rng );

/**
 * \brief           Check a public-private key pair
 *
 * \param pub       Keypair structure holding a public key
 * \param prv       Keypair structure holding a private (plus public) key
 *
 * \return          0 if successful (keys are valid and match), or
 *                  POLARSSL_ERR_ECP_BAD_INPUT_DATA, or
 *                  a POLARSSL_ERR_ECP_XXX or POLARSSL_ERR_MPI_XXX code.
 */
int ecp_check_pub_priv( const ecp_keypair *pub, const ecp_keypair *prv );

#if defined(POLARSSL_SELF_TEST)
/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if a test failed
 */
int ecp_self_test( int verbose );
#endif

#ifdef __cplusplus
}
#endif

#endif /* ecp.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/ecdsa.h ************/


/**
 * \file ecdsa.h
 *
 * \brief Elliptic curve DSA
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_ECDSA_H
#define POLARSSL_ECDSA_H




/**
 * \brief           ECDSA context structure
 *
 * \note Purposefully begins with the same members as struct ecp_keypair.
 */
typedef struct
{
    ecp_group grp;      /*!<  elliptic curve used           */
    mpi d;              /*!<  secret signature key          */
    ecp_point Q;        /*!<  public signature key          */
    mpi r;              /*!<  first integer from signature  */
    mpi s;              /*!<  second integer from signature */
}
ecdsa_context;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief           Compute ECDSA signature of a previously hashed message
 *
 * \param grp       ECP group
 * \param r         First output integer
 * \param s         Second output integer
 * \param d         Private signing key
 * \param buf       Message hash
 * \param blen      Length of buf
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 if successful,
 *                  or a POLARSSL_ERR_ECP_XXX or POLARSSL_MPI_XXX error code
 */
int ecdsa_sign( ecp_group *grp, mpi *r, mpi *s,
                const mpi *d, const unsigned char *buf, size_t blen,
                int (*f_rng)(void *, unsigned char *, size_t), void *p_rng );

#if defined(POLARSSL_ECDSA_DETERMINISTIC)
/**
 * \brief           Compute ECDSA signature of a previously hashed message
 *                  (deterministic version)
 *
 * \param grp       ECP group
 * \param r         First output integer
 * \param s         Second output integer
 * \param d         Private signing key
 * \param buf       Message hash
 * \param blen      Length of buf
 * \param md_alg    MD algorithm used to hash the message
 *
 * \return          0 if successful,
 *                  or a POLARSSL_ERR_ECP_XXX or POLARSSL_MPI_XXX error code
 */
int ecdsa_sign_det( ecp_group *grp, mpi *r, mpi *s,
                    const mpi *d, const unsigned char *buf, size_t blen,
                    md_type_t md_alg );
#endif /* POLARSSL_ECDSA_DETERMINISTIC */

/**
 * \brief           Verify ECDSA signature of a previously hashed message
 *
 * \param grp       ECP group
 * \param buf       Message hash
 * \param blen      Length of buf
 * \param Q         Public key to use for verification
 * \param r         First integer of the signature
 * \param s         Second integer of the signature
 *
 * \return          0 if successful,
 *                  POLARSSL_ERR_ECP_BAD_INPUT_DATA if signature is invalid
 *                  or a POLARSSL_ERR_ECP_XXX or POLARSSL_MPI_XXX error code
 */
int ecdsa_verify( ecp_group *grp,
                  const unsigned char *buf, size_t blen,
                  const ecp_point *Q, const mpi *r, const mpi *s);

/**
 * \brief           Compute ECDSA signature and write it to buffer,
 *                  serialized as defined in RFC 4492 page 20.
 *                  (Not thread-safe to use same context in multiple threads)
 *
 * \param ctx       ECDSA context
 * \param hash      Message hash
 * \param hlen      Length of hash
 * \param sig       Buffer that will hold the signature
 * \param slen      Length of the signature written
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \note            The "sig" buffer must be at least as large as twice the
 *                  size of the curve used, plus 7 (eg. 71 bytes if a 256-bit
 *                  curve is used).
 *
 * \return          0 if successful,
 *                  or a POLARSSL_ERR_ECP, POLARSSL_ERR_MPI or
 *                  POLARSSL_ERR_ASN1 error code
 */
int ecdsa_write_signature( ecdsa_context *ctx,
                           const unsigned char *hash, size_t hlen,
                           unsigned char *sig, size_t *slen,
                           int (*f_rng)(void *, unsigned char *, size_t),
                           void *p_rng );

#if defined(POLARSSL_ECDSA_DETERMINISTIC)
/**
 * \brief           Compute ECDSA signature and write it to buffer,
 *                  serialized as defined in RFC 4492 page 20.
 *                  Deterministic version, RFC 6979.
 *                  (Not thread-safe to use same context in multiple threads)
 *
 * \param ctx       ECDSA context
 * \param hash      Message hash
 * \param hlen      Length of hash
 * \param sig       Buffer that will hold the signature
 * \param slen      Length of the signature written
 * \param md_alg    MD algorithm used to hash the message
 *
 * \note            The "sig" buffer must be at least as large as twice the
 *                  size of the curve used, plus 7 (eg. 71 bytes if a 256-bit
 *                  curve is used).
 *
 * \return          0 if successful,
 *                  or a POLARSSL_ERR_ECP, POLARSSL_ERR_MPI or
 *                  POLARSSL_ERR_ASN1 error code
 */
int ecdsa_write_signature_det( ecdsa_context *ctx,
                               const unsigned char *hash, size_t hlen,
                               unsigned char *sig, size_t *slen,
                               md_type_t md_alg );
#endif /* POLARSSL_ECDSA_DETERMINISTIC */

/**
 * \brief           Read and verify an ECDSA signature
 *
 * \param ctx       ECDSA context
 * \param hash      Message hash
 * \param hlen      Size of hash
 * \param sig       Signature to read and verify
 * \param slen      Size of sig
 *
 * \return          0 if successful,
 *                  POLARSSL_ERR_ECP_BAD_INPUT_DATA if signature is invalid,
 *                  POLARSSL_ERR_ECP_SIG_LEN_MISTMATCH if the signature is
 *                  valid but its actual length is less than siglen,
 *                  or a POLARSSL_ERR_ECP or POLARSSL_ERR_MPI error code
 */
int ecdsa_read_signature( ecdsa_context *ctx,
                          const unsigned char *hash, size_t hlen,
                          const unsigned char *sig, size_t slen );

/**
 * \brief           Generate an ECDSA keypair on the given curve
 *
 * \param ctx       ECDSA context in which the keypair should be stored
 * \param gid       Group (elliptic curve) to use. One of the various
 *                  POLARSSL_ECP_DP_XXX macros depending on configuration.
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 on success, or a POLARSSL_ERR_ECP code.
 */
int ecdsa_genkey( ecdsa_context *ctx, ecp_group_id gid,
                  int (*f_rng)(void *, unsigned char *, size_t), void *p_rng );

/**
 * \brief           Set an ECDSA context from an EC key pair
 *
 * \param ctx       ECDSA context to set
 * \param key       EC key to use
 *
 * \return          0 on success, or a POLARSSL_ERR_ECP code.
 */
int ecdsa_from_keypair( ecdsa_context *ctx, const ecp_keypair *key );

/**
 * \brief           Initialize context
 *
 * \param ctx       Context to initialize
 */
void ecdsa_init( ecdsa_context *ctx );

/**
 * \brief           Free context
 *
 * \param ctx       Context to free
 */
void ecdsa_free( ecdsa_context *ctx );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int ecdsa_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* ecdsa.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/x509_crl.h ************/


/**
 * \file x509_crl.h
 *
 * \brief X.509 certificate revocation list parsing
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_X509_CRL_H
#define POLARSSL_X509_CRL_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup x509_module
 * \{ */

/**
 * \name Structures and functions for parsing CRLs
 * \{
 */

/**
 * Certificate revocation list entry.
 * Contains the CA-specific serial numbers and revocation dates.
 */
typedef struct _x509_crl_entry
{
    x509_buf raw;

    x509_buf serial;

    x509_time revocation_date;

    x509_buf entry_ext;

    struct _x509_crl_entry *next;
}
x509_crl_entry;

/**
 * Certificate revocation list structure.
 * Every CRL may have multiple entries.
 */
typedef struct _x509_crl
{
    x509_buf raw;           /**< The raw certificate data (DER). */
    x509_buf tbs;           /**< The raw certificate body (DER). The part that is To Be Signed. */

    int version;            /**< CRL version (1=v1, 2=v2) */
    x509_buf sig_oid1;

    x509_buf issuer_raw;    /**< The raw issuer data (DER). */

    x509_name issuer;       /**< The parsed issuer data (named information object). */

    x509_time this_update;
    x509_time next_update;

    x509_crl_entry entry;   /**< The CRL entries containing the certificate revocation times for this CA. */

    x509_buf crl_ext;

    x509_buf sig_oid2;
    x509_buf sig;
    md_type_t sig_md;           /**< Internal representation of the MD algorithm of the signature algorithm, e.g. POLARSSL_MD_SHA256 */
    pk_type_t sig_pk;           /**< Internal representation of the Public Key algorithm of the signature algorithm, e.g. POLARSSL_PK_RSA */
    void *sig_opts;             /**< Signature options to be passed to pk_verify_ext(), e.g. for RSASSA-PSS */

    struct _x509_crl *next;
}
x509_crl;

/**
 * \brief          Parse a DER-encoded CRL and append it to the chained list
 *
 * \param chain    points to the start of the chain
 * \param buf      buffer holding the CRL data in DER format
 * \param buflen   size of the buffer
 *
 * \return         0 if successful, or a specific X509 or PEM error code
 */
int x509_crl_parse_der( x509_crl *chain,
                        const unsigned char *buf, size_t buflen );
/**
 * \brief          Parse one or more CRLs and append them to the chained list
 *
 * \note           Mutliple CRLs are accepted only if using PEM format
 *
 * \param chain    points to the start of the chain
 * \param buf      buffer holding the CRL data in PEM or DER format
 * \param buflen   size of the buffer
 *
 * \return         0 if successful, or a specific X509 or PEM error code
 */
int x509_crl_parse( x509_crl *chain, const unsigned char *buf, size_t buflen );

#if defined(POLARSSL_FS_IO)
/**
 * \brief          Load one or more CRLs and append them to the chained list
 *
 * \note           Mutliple CRLs are accepted only if using PEM format
 *
 * \param chain    points to the start of the chain
 * \param path     filename to read the CRLs from (in PEM or DER encoding)
 *
 * \return         0 if successful, or a specific X509 or PEM error code
 */
int x509_crl_parse_file( x509_crl *chain, const char *path );
#endif /* POLARSSL_FS_IO */

/**
 * \brief          Returns an informational string about the CRL.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param prefix   A line prefix
 * \param crl      The X509 CRL to represent
 *
 * \return         The amount of data written to the buffer, or -1 in
 *                 case of an error.
 */
int x509_crl_info( char *buf, size_t size, const char *prefix,
                   const x509_crl *crl );

/**
 * \brief          Initialize a CRL (chain)
 *
 * \param crl      CRL chain to initialize
 */
void x509_crl_init( x509_crl *crl );

/**
 * \brief          Unallocate all CRL data
 *
 * \param crl      CRL chain to free
 */
void x509_crl_free( x509_crl *crl );

/* \} name */
/* \} addtogroup x509_module */

#ifdef __cplusplus
}
#endif

#endif /* x509_crl.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/x509_crt.h ************/


/**
 * \file x509_crt.h
 *
 * \brief X.509 certificate parsing and writing
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_X509_CRT_H
#define POLARSSL_X509_CRT_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif




/**
 * \addtogroup x509_module
 * \{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \name Structures and functions for parsing and writing X.509 certificates
 * \{
 */

/**
 * Container for an X.509 certificate. The certificate may be chained.
 */
typedef struct _x509_crt
{
    x509_buf raw;               /**< The raw certificate data (DER). */
    x509_buf tbs;               /**< The raw certificate body (DER). The part that is To Be Signed. */

    int version;                /**< The X.509 version. (1=v1, 2=v2, 3=v3) */
    x509_buf serial;            /**< Unique id for certificate issued by a specific CA. */
    x509_buf sig_oid1;          /**< Signature algorithm, e.g. sha1RSA */

    x509_buf issuer_raw;        /**< The raw issuer data (DER). Used for quick comparison. */
    x509_buf subject_raw;       /**< The raw subject data (DER). Used for quick comparison. */

    x509_name issuer;           /**< The parsed issuer data (named information object). */
    x509_name subject;          /**< The parsed subject data (named information object). */

    x509_time valid_from;       /**< Start time of certificate validity. */
    x509_time valid_to;         /**< End time of certificate validity. */

    pk_context pk;              /**< Container for the public key context. */

    x509_buf issuer_id;         /**< Optional X.509 v2/v3 issuer unique identifier. */
    x509_buf subject_id;        /**< Optional X.509 v2/v3 subject unique identifier. */
    x509_buf v3_ext;            /**< Optional X.509 v3 extensions.  */
    x509_sequence subject_alt_names;    /**< Optional list of Subject Alternative Names (Only dNSName supported). */

    int ext_types;              /**< Bit string containing detected and parsed extensions */
    int ca_istrue;              /**< Optional Basic Constraint extension value: 1 if this certificate belongs to a CA, 0 otherwise. */
    int max_pathlen;            /**< Optional Basic Constraint extension value: The maximum path length to the root certificate. Path length is 1 higher than RFC 5280 'meaning', so 1+ */

    unsigned char key_usage;    /**< Optional key usage extension value: See the values in x509.h */

    x509_sequence ext_key_usage; /**< Optional list of extended key usage OIDs. */

    unsigned char ns_cert_type; /**< Optional Netscape certificate type extension value: See the values in x509.h */

    x509_buf sig_oid2;          /**< Signature algorithm. Must match sig_oid1. */
    x509_buf sig;               /**< Signature: hash of the tbs part signed with the private key. */
    md_type_t sig_md;           /**< Internal representation of the MD algorithm of the signature algorithm, e.g. POLARSSL_MD_SHA256 */
    pk_type_t sig_pk;           /**< Internal representation of the Public Key algorithm of the signature algorithm, e.g. POLARSSL_PK_RSA */
    void *sig_opts;             /**< Signature options to be passed to pk_verify_ext(), e.g. for RSASSA-PSS */

    struct _x509_crt *next;     /**< Next certificate in the CA-chain. */
}
x509_crt;

#define X509_CRT_VERSION_1              0
#define X509_CRT_VERSION_2              1
#define X509_CRT_VERSION_3              2

#define X509_RFC5280_MAX_SERIAL_LEN 32
#define X509_RFC5280_UTC_TIME_LEN   15

/**
 * Container for writing a certificate (CRT)
 */
typedef struct _x509write_cert
{
    int version;
    mpi serial;
    pk_context *subject_key;
    pk_context *issuer_key;
    asn1_named_data *subject;
    asn1_named_data *issuer;
    md_type_t md_alg;
    char not_before[X509_RFC5280_UTC_TIME_LEN + 1];
    char not_after[X509_RFC5280_UTC_TIME_LEN + 1];
    asn1_named_data *extensions;
}
x509write_cert;

#if defined(POLARSSL_X509_CRT_PARSE_C)
/**
 * \brief          Parse a single DER formatted certificate and add it
 *                 to the chained list.
 *
 * \param chain    points to the start of the chain
 * \param buf      buffer holding the certificate DER data
 * \param buflen   size of the buffer
 *
 * \return         0 if successful, or a specific X509 or PEM error code
 */
int x509_crt_parse_der( x509_crt *chain, const unsigned char *buf,
                        size_t buflen );

/**
 * \brief          Parse one or more certificates and add them
 *                 to the chained list. Parses permissively. If some
 *                 certificates can be parsed, the result is the number
 *                 of failed certificates it encountered. If none complete
 *                 correctly, the first error is returned.
 *
 * \param chain    points to the start of the chain
 * \param buf      buffer holding the certificate data
 * \param buflen   size of the buffer
 *
 * \return         0 if all certificates parsed successfully, a positive number
 *                 if partly successful or a specific X509 or PEM error code
 */
int x509_crt_parse( x509_crt *chain, const unsigned char *buf, size_t buflen );

#if defined(POLARSSL_FS_IO)
/**
 * \brief          Load one or more certificates and add them
 *                 to the chained list. Parses permissively. If some
 *                 certificates can be parsed, the result is the number
 *                 of failed certificates it encountered. If none complete
 *                 correctly, the first error is returned.
 *
 * \param chain    points to the start of the chain
 * \param path     filename to read the certificates from
 *
 * \return         0 if all certificates parsed successfully, a positive number
 *                 if partly successful or a specific X509 or PEM error code
 */
int x509_crt_parse_file( x509_crt *chain, const char *path );

/**
 * \brief          Load one or more certificate files from a path and add them
 *                 to the chained list. Parses permissively. If some
 *                 certificates can be parsed, the result is the number
 *                 of failed certificates it encountered. If none complete
 *                 correctly, the first error is returned.
 *
 * \warning        This function is NOT thread-safe unless
 *                 POLARSSL_THREADING_PTHREADS is defined. If you're using an
 *                 alternative threading implementation, you should either use
 *                 this function only in the main thread, or mutex it.
 *
 * \param chain    points to the start of the chain
 * \param path     directory / folder to read the certificate files from
 *
 * \return         0 if all certificates parsed successfully, a positive number
 *                 if partly successful or a specific X509 or PEM error code
 */
int x509_crt_parse_path( x509_crt *chain, const char *path );
#endif /* POLARSSL_FS_IO */

/**
 * \brief          Returns an informational string about the
 *                 certificate.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param prefix   A line prefix
 * \param crt      The X509 certificate to represent
 *
 * \return         The amount of data written to the buffer, or -1 in
 *                 case of an error.
 */
int x509_crt_info( char *buf, size_t size, const char *prefix,
                   const x509_crt *crt );

/**
 * \brief          Returns an informational string about the
 *                 verification status of a certificate.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param prefix   A line prefix
 * \param flags    Verification flags created by x509_crt_verify()
 *
 * \return         The amount of data written to the buffer, or -1 in
 *                 case of an error.
 */
int x509_crt_verify_info( char *buf, size_t size, const char *prefix,
                          int flags );

/**
 * \brief          Verify the certificate signature
 *
 *                 The verify callback is a user-supplied callback that
 *                 can clear / modify / add flags for a certificate. If set,
 *                 the verification callback is called for each
 *                 certificate in the chain (from the trust-ca down to the
 *                 presented crt). The parameters for the callback are:
 *                 (void *parameter, x509_crt *crt, int certificate_depth,
 *                 int *flags). With the flags representing current flags for
 *                 that specific certificate and the certificate depth from
 *                 the bottom (Peer cert depth = 0).
 *
 *                 All flags left after returning from the callback
 *                 are also returned to the application. The function should
 *                 return 0 for anything but a fatal error.
 *
 * \note           In case verification failed, the results can be displayed
 *                 using \c x509_crt_verify_info()
 *
 * \param crt      a certificate to be verified
 * \param trust_ca the trusted CA chain
 * \param ca_crl   the CRL chain for trusted CA's
 * \param cn       expected Common Name (can be set to
 *                 NULL if the CN must not be verified)
 * \param flags    result of the verification
 * \param f_vrfy   verification function
 * \param p_vrfy   verification parameter
 *
 * \return         0 if successful or POLARSSL_ERR_X509_SIG_VERIFY_FAILED,
 *                 in which case *flags will have one or more BADCERT_XXX or
 *                 BADCRL_XXX flags set,
 *                 or another error in case of a fatal error encountered
 *                 during the verification process.
 */
int x509_crt_verify( x509_crt *crt,
                     x509_crt *trust_ca,
                     x509_crl *ca_crl,
                     const char *cn, int *flags,
                     int (*f_vrfy)(void *, x509_crt *, int, int *),
                     void *p_vrfy );

#if defined(POLARSSL_X509_CHECK_KEY_USAGE)
/**
 * \brief          Check usage of certificate against keyUsage extension.
 *
 * \param crt      Leaf certificate used.
 * \param usage    Intended usage(s) (eg KU_KEY_ENCIPHERMENT before using the
 *                 certificate to perform an RSA key exchange).
 *
 * \return         0 is these uses of the certificate are allowed,
 *                 POLARSSL_ERR_X509_BAD_INPUT_DATA if the keyUsage extension
 *                 is present but does not contain all the bits set in the
 *                 usage argument.
 *
 * \note           You should only call this function on leaf certificates, on
 *                 (intermediate) CAs the keyUsage extension is automatically
 *                 checked by \c x509_crt_verify().
 */
int x509_crt_check_key_usage( const x509_crt *crt, int usage );
#endif /* POLARSSL_X509_CHECK_KEY_USAGE) */

#if defined(POLARSSL_X509_CHECK_EXTENDED_KEY_USAGE)
/**
 * \brief          Check usage of certificate against extentedJeyUsage.
 *
 * \param crt      Leaf certificate used.
 * \param usage_oid Intended usage (eg OID_SERVER_AUTH or OID_CLIENT_AUTH).
 * \param usage_len Length of usage_oid (eg given by OID_SIZE()).
 *
 * \return         0 is this use of the certificate is allowed,
 *                 POLARSSL_ERR_X509_BAD_INPUT_DATA if not.
 *
 * \note           Usually only makes sense on leaf certificates.
 */
int x509_crt_check_extended_key_usage( const x509_crt *crt,
                                       const char *usage_oid,
                                       size_t usage_len );
#endif /* POLARSSL_X509_CHECK_EXTENDED_KEY_USAGE) */

#if defined(POLARSSL_X509_CRL_PARSE_C)
/**
 * \brief          Verify the certificate revocation status
 *
 * \param crt      a certificate to be verified
 * \param crl      the CRL to verify against
 *
 * \return         1 if the certificate is revoked, 0 otherwise
 *
 */
int x509_crt_revoked( const x509_crt *crt, const x509_crl *crl );
#endif /* POLARSSL_X509_CRL_PARSE_C */

/**
 * \brief          Initialize a certificate (chain)
 *
 * \param crt      Certificate chain to initialize
 */
void x509_crt_init( x509_crt *crt );

/**
 * \brief          Unallocate all certificate data
 *
 * \param crt      Certificate chain to free
 */
void x509_crt_free( x509_crt *crt );
#endif /* POLARSSL_X509_CRT_PARSE_C */

/* \} name */
/* \} addtogroup x509_module */

#if defined(POLARSSL_X509_CRT_WRITE_C)
/**
 * \brief           Initialize a CRT writing context
 *
 * \param ctx       CRT context to initialize
 */
void x509write_crt_init( x509write_cert *ctx );

/**
 * \brief           Set the verion for a Certificate
 *                  Default: X509_CRT_VERSION_3
 *
 * \param ctx       CRT context to use
 * \param version   version to set (X509_CRT_VERSION_1, X509_CRT_VERSION_2 or
 *                                  X509_CRT_VERSION_3)
 */
void x509write_crt_set_version( x509write_cert *ctx, int version );

/**
 * \brief           Set the serial number for a Certificate.
 *
 * \param ctx       CRT context to use
 * \param serial    serial number to set
 *
 * \return          0 if successful
 */
int x509write_crt_set_serial( x509write_cert *ctx, const mpi *serial );

/**
 * \brief           Set the validity period for a Certificate
 *                  Timestamps should be in string format for UTC timezone
 *                  i.e. "YYYYMMDDhhmmss"
 *                  e.g. "20131231235959" for December 31st 2013
 *                       at 23:59:59
 *
 * \param ctx       CRT context to use
 * \param not_before    not_before timestamp
 * \param not_after     not_after timestamp
 *
 * \return          0 if timestamp was parsed successfully, or
 *                  a specific error code
 */
int x509write_crt_set_validity( x509write_cert *ctx, const char *not_before,
                                const char *not_after );

/**
 * \brief           Set the issuer name for a Certificate
 *                  Issuer names should contain a comma-separated list
 *                  of OID types and values:
 *                  e.g. "C=UK,O=ARM,CN=mbed TLS CA"
 *
 * \param ctx           CRT context to use
 * \param issuer_name   issuer name to set
 *
 * \return          0 if issuer name was parsed successfully, or
 *                  a specific error code
 */
int x509write_crt_set_issuer_name( x509write_cert *ctx,
                                   const char *issuer_name );

/**
 * \brief           Set the subject name for a Certificate
 *                  Subject names should contain a comma-separated list
 *                  of OID types and values:
 *                  e.g. "C=UK,O=ARM,CN=mbed TLS Server 1"
 *
 * \param ctx           CRT context to use
 * \param subject_name  subject name to set
 *
 * \return          0 if subject name was parsed successfully, or
 *                  a specific error code
 */
int x509write_crt_set_subject_name( x509write_cert *ctx,
                                    const char *subject_name );

/**
 * \brief           Set the subject public key for the certificate
 *
 * \param ctx       CRT context to use
 * \param key       public key to include
 */
void x509write_crt_set_subject_key( x509write_cert *ctx, pk_context *key );

/**
 * \brief           Set the issuer key used for signing the certificate
 *
 * \param ctx       CRT context to use
 * \param key       private key to sign with
 */
void x509write_crt_set_issuer_key( x509write_cert *ctx, pk_context *key );

/**
 * \brief           Set the MD algorithm to use for the signature
 *                  (e.g. POLARSSL_MD_SHA1)
 *
 * \param ctx       CRT context to use
 * \param md_alg    MD algorithm to use
 */
void x509write_crt_set_md_alg( x509write_cert *ctx, md_type_t md_alg );

/**
 * \brief           Generic function to add to or replace an extension in the
 *                  CRT
 *
 * \param ctx       CRT context to use
 * \param oid       OID of the extension
 * \param oid_len   length of the OID
 * \param critical  if the extension is critical (per the RFC's definition)
 * \param val       value of the extension OCTET STRING
 * \param val_len   length of the value data
 *
 * \return          0 if successful, or a POLARSSL_ERR_X509WRITE_MALLOC_FAILED
 */
int x509write_crt_set_extension( x509write_cert *ctx,
                                 const char *oid, size_t oid_len,
                                 int critical,
                                 const unsigned char *val, size_t val_len );

/**
 * \brief           Set the basicConstraints extension for a CRT
 *
 * \param ctx       CRT context to use
 * \param is_ca     is this a CA certificate
 * \param max_pathlen   maximum length of certificate chains below this
 *                      certificate (only for CA certificates, -1 is
 *                      inlimited)
 *
 * \return          0 if successful, or a POLARSSL_ERR_X509WRITE_MALLOC_FAILED
 */
int x509write_crt_set_basic_constraints( x509write_cert *ctx,
                                         int is_ca, int max_pathlen );

#if defined(POLARSSL_SHA1_C)
/**
 * \brief           Set the subjectKeyIdentifier extension for a CRT
 *                  Requires that x509write_crt_set_subject_key() has been
 *                  called before
 *
 * \param ctx       CRT context to use
 *
 * \return          0 if successful, or a POLARSSL_ERR_X509WRITE_MALLOC_FAILED
 */
int x509write_crt_set_subject_key_identifier( x509write_cert *ctx );

/**
 * \brief           Set the authorityKeyIdentifier extension for a CRT
 *                  Requires that x509write_crt_set_issuer_key() has been
 *                  called before
 *
 * \param ctx       CRT context to use
 *
 * \return          0 if successful, or a POLARSSL_ERR_X509WRITE_MALLOC_FAILED
 */
int x509write_crt_set_authority_key_identifier( x509write_cert *ctx );
#endif /* POLARSSL_SHA1_C */

/**
 * \brief           Set the Key Usage Extension flags
 *                  (e.g. KU_DIGITAL_SIGNATURE | KU_KEY_CERT_SIGN)
 *
 * \param ctx       CRT context to use
 * \param key_usage key usage flags to set
 *
 * \return          0 if successful, or POLARSSL_ERR_X509WRITE_MALLOC_FAILED
 */
int x509write_crt_set_key_usage( x509write_cert *ctx, unsigned char key_usage );

/**
 * \brief           Set the Netscape Cert Type flags
 *                  (e.g. NS_CERT_TYPE_SSL_CLIENT | NS_CERT_TYPE_EMAIL)
 *
 * \param ctx           CRT context to use
 * \param ns_cert_type  Netscape Cert Type flags to set
 *
 * \return          0 if successful, or POLARSSL_ERR_X509WRITE_MALLOC_FAILED
 */
int x509write_crt_set_ns_cert_type( x509write_cert *ctx,
                                    unsigned char ns_cert_type );

/**
 * \brief           Free the contents of a CRT write context
 *
 * \param ctx       CRT context to free
 */
void x509write_crt_free( x509write_cert *ctx );

/**
 * \brief           Write a built up certificate to a X509 DER structure
 *                  Note: data is written at the end of the buffer! Use the
 *                        return value to determine where you should start
 *                        using the buffer
 *
 * \param ctx       certificate to write away
 * \param buf       buffer to write to
 * \param size      size of the buffer
 * \param f_rng     RNG function (for signature, see note)
 * \param p_rng     RNG parameter
 *
 * \return          length of data written if successful, or a specific
 *                  error code
 *
 * \note            f_rng may be NULL if RSA is used for signature and the
 *                  signature is made offline (otherwise f_rng is desirable
 *                  for countermeasures against timing attacks).
 *                  ECDSA signatures always require a non-NULL f_rng.
 */
int x509write_crt_der( x509write_cert *ctx, unsigned char *buf, size_t size,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng );

#if defined(POLARSSL_PEM_WRITE_C)
/**
 * \brief           Write a built up certificate to a X509 PEM string
 *
 * \param ctx       certificate to write away
 * \param buf       buffer to write to
 * \param size      size of the buffer
 * \param f_rng     RNG function (for signature, see note)
 * \param p_rng     RNG parameter
 *
 * \return          0 successful, or a specific error code
 *
 * \note            f_rng may be NULL if RSA is used for signature and the
 *                  signature is made offline (otherwise f_rng is desirable
 *                  for countermeasures against timing attacks).
 *                  ECDSA signatures always require a non-NULL f_rng.
 */
int x509write_crt_pem( x509write_cert *ctx, unsigned char *buf, size_t size,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng );
#endif /* POLARSSL_PEM_WRITE_C */
#endif /* POLARSSL_X509_CRT_WRITE_C */

#ifdef __cplusplus
}
#endif

#endif /* x509_crt.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/x509_csr.h ************/


/**
 * \file x509_csr.h
 *
 * \brief X.509 certificate signing request parsing and writing
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_X509_CSR_H
#define POLARSSL_X509_CSR_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup x509_module
 * \{ */

/**
 * \name Structures and functions for X.509 Certificate Signing Requests (CSR)
 * \{
 */

/**
 * Certificate Signing Request (CSR) structure.
 */
typedef struct _x509_csr
{
    x509_buf raw;           /**< The raw CSR data (DER). */
    x509_buf cri;           /**< The raw CertificateRequestInfo body (DER). */

    int version;            /**< CSR version (1=v1). */

    x509_buf  subject_raw;  /**< The raw subject data (DER). */
    x509_name subject;      /**< The parsed subject data (named information object). */

    pk_context pk;          /**< Container for the public key context. */

    x509_buf sig_oid;
    x509_buf sig;
    md_type_t sig_md;       /**< Internal representation of the MD algorithm of the signature algorithm, e.g. POLARSSL_MD_SHA256 */
    pk_type_t sig_pk;       /**< Internal representation of the Public Key algorithm of the signature algorithm, e.g. POLARSSL_PK_RSA */
    void *sig_opts;         /**< Signature options to be passed to pk_verify_ext(), e.g. for RSASSA-PSS */
}
x509_csr;

/**
 * Container for writing a CSR
 */
typedef struct _x509write_csr
{
    pk_context *key;
    asn1_named_data *subject;
    md_type_t md_alg;
    asn1_named_data *extensions;
}
x509write_csr;

#if defined(POLARSSL_X509_CSR_PARSE_C)
/**
 * \brief          Load a Certificate Signing Request (CSR) in DER format
 *
 * \param csr      CSR context to fill
 * \param buf      buffer holding the CRL data
 * \param buflen   size of the buffer
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509_csr_parse_der( x509_csr *csr,
                        const unsigned char *buf, size_t buflen );

/**
 * \brief          Load a Certificate Signing Request (CSR), DER or PEM format
 *
 * \param csr      CSR context to fill
 * \param buf      buffer holding the CRL data
 * \param buflen   size of the buffer
 *
 * \return         0 if successful, or a specific X509 or PEM error code
 */
int x509_csr_parse( x509_csr *csr, const unsigned char *buf, size_t buflen );

#if defined(POLARSSL_FS_IO)
/**
 * \brief          Load a Certificate Signing Request (CSR)
 *
 * \param csr      CSR context to fill
 * \param path     filename to read the CSR from
 *
 * \return         0 if successful, or a specific X509 or PEM error code
 */
int x509_csr_parse_file( x509_csr *csr, const char *path );
#endif /* POLARSSL_FS_IO */

/**
 * \brief          Returns an informational string about the
 *                 CSR.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param prefix   A line prefix
 * \param csr      The X509 CSR to represent
 *
 * \return         The length of the string written (exluding the terminating
 *                 null byte), or a negative value in case of an error.
 */
int x509_csr_info( char *buf, size_t size, const char *prefix,
                   const x509_csr *csr );

/**
 * \brief          Initialize a CSR
 *
 * \param csr      CSR to initialize
 */
void x509_csr_init( x509_csr *csr );

/**
 * \brief          Unallocate all CSR data
 *
 * \param csr      CSR to free
 */
void x509_csr_free( x509_csr *csr );
#endif /* POLARSSL_X509_CSR_PARSE_C */

/* \} name */
/* \} addtogroup x509_module */

#if defined(POLARSSL_X509_CSR_WRITE_C)
/**
 * \brief           Initialize a CSR context
 *
 * \param ctx       CSR context to initialize
 */
void x509write_csr_init( x509write_csr *ctx );

/**
 * \brief           Set the subject name for a CSR
 *                  Subject names should contain a comma-separated list
 *                  of OID types and values:
 *                  e.g. "C=UK,O=ARM,CN=mbed TLS Server 1"
 *
 * \param ctx           CSR context to use
 * \param subject_name  subject name to set
 *
 * \return          0 if subject name was parsed successfully, or
 *                  a specific error code
 */
int x509write_csr_set_subject_name( x509write_csr *ctx,
                                    const char *subject_name );

/**
 * \brief           Set the key for a CSR (public key will be included,
 *                  private key used to sign the CSR when writing it)
 *
 * \param ctx       CSR context to use
 * \param key       Asymetric key to include
 */
void x509write_csr_set_key( x509write_csr *ctx, pk_context *key );

/**
 * \brief           Set the MD algorithm to use for the signature
 *                  (e.g. POLARSSL_MD_SHA1)
 *
 * \param ctx       CSR context to use
 * \param md_alg    MD algorithm to use
 */
void x509write_csr_set_md_alg( x509write_csr *ctx, md_type_t md_alg );

/**
 * \brief           Set the Key Usage Extension flags
 *                  (e.g. KU_DIGITAL_SIGNATURE | KU_KEY_CERT_SIGN)
 *
 * \param ctx       CSR context to use
 * \param key_usage key usage flags to set
 *
 * \return          0 if successful, or POLARSSL_ERR_X509WRITE_MALLOC_FAILED
 */
int x509write_csr_set_key_usage( x509write_csr *ctx, unsigned char key_usage );

/**
 * \brief           Set the Netscape Cert Type flags
 *                  (e.g. NS_CERT_TYPE_SSL_CLIENT | NS_CERT_TYPE_EMAIL)
 *
 * \param ctx           CSR context to use
 * \param ns_cert_type  Netscape Cert Type flags to set
 *
 * \return          0 if successful, or POLARSSL_ERR_X509WRITE_MALLOC_FAILED
 */
int x509write_csr_set_ns_cert_type( x509write_csr *ctx,
                                    unsigned char ns_cert_type );

/**
 * \brief           Generic function to add to or replace an extension in the
 *                  CSR
 *
 * \param ctx       CSR context to use
 * \param oid       OID of the extension
 * \param oid_len   length of the OID
 * \param val       value of the extension OCTET STRING
 * \param val_len   length of the value data
 *
 * \return          0 if successful, or a POLARSSL_ERR_X509WRITE_MALLOC_FAILED
 */
int x509write_csr_set_extension( x509write_csr *ctx,
                                 const char *oid, size_t oid_len,
                                 const unsigned char *val, size_t val_len );

/**
 * \brief           Free the contents of a CSR context
 *
 * \param ctx       CSR context to free
 */
void x509write_csr_free( x509write_csr *ctx );

/**
 * \brief           Write a CSR (Certificate Signing Request) to a
 *                  DER structure
 *                  Note: data is written at the end of the buffer! Use the
 *                        return value to determine where you should start
 *                        using the buffer
 *
 * \param ctx       CSR to write away
 * \param buf       buffer to write to
 * \param size      size of the buffer
 * \param f_rng     RNG function (for signature, see note)
 * \param p_rng     RNG parameter
 *
 * \return          length of data written if successful, or a specific
 *                  error code
 *
 * \note            f_rng may be NULL if RSA is used for signature and the
 *                  signature is made offline (otherwise f_rng is desirable
 *                  for countermeasures against timing attacks).
 *                  ECDSA signatures always require a non-NULL f_rng.
 */
int x509write_csr_der( x509write_csr *ctx, unsigned char *buf, size_t size,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng );

#if defined(POLARSSL_PEM_WRITE_C)
/**
 * \brief           Write a CSR (Certificate Signing Request) to a
 *                  PEM string
 *
 * \param ctx       CSR to write away
 * \param buf       buffer to write to
 * \param size      size of the buffer
 * \param f_rng     RNG function (for signature, see note)
 * \param p_rng     RNG parameter
 *
 * \return          0 successful, or a specific error code
 *
 * \note            f_rng may be NULL if RSA is used for signature and the
 *                  signature is made offline (otherwise f_rng is desirable
 *                  for couermeasures against timing attacks).
 *                  ECDSA signatures always require a non-NULL f_rng.
 */
int x509write_csr_pem( x509write_csr *ctx, unsigned char *buf, size_t size,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng );
#endif /* POLARSSL_PEM_WRITE_C */
#endif /* POLARSSL_X509_CSR_WRITE_C */

#ifdef __cplusplus
}
#endif

#endif /* x509_csr.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/cipher.h ************/


/**
 * \file cipher.h
 *
 * \brief Generic cipher wrapper.
 *
 * \author Adriaan de Jong <dejong@fox-it.com>
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef POLARSSL_CIPHER_H
#define POLARSSL_CIPHER_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(POLARSSL_GCM_C) || defined(POLARSSL_CCM_C)
#define POLARSSL_CIPHER_MODE_AEAD
#endif

#if defined(POLARSSL_CIPHER_MODE_CBC)
#define POLARSSL_CIPHER_MODE_WITH_PADDING
#endif

#if defined(POLARSSL_ARC4_C)
#define POLARSSL_CIPHER_MODE_STREAM
#endif

#if defined(_MSC_VER) && !defined(inline)
#define inline _inline
#else
#if defined(__ARMCC_VERSION) && !defined(inline)
#define inline __inline
#endif /* __ARMCC_VERSION */
#endif /*_MSC_VER */

#define POLARSSL_ERR_CIPHER_FEATURE_UNAVAILABLE            -0x6080  /**< The selected feature is not available. */
#define POLARSSL_ERR_CIPHER_BAD_INPUT_DATA                 -0x6100  /**< Bad input parameters to function. */
#define POLARSSL_ERR_CIPHER_ALLOC_FAILED                   -0x6180  /**< Failed to allocate memory. */
#define POLARSSL_ERR_CIPHER_INVALID_PADDING                -0x6200  /**< Input data contains invalid padding and is rejected. */
#define POLARSSL_ERR_CIPHER_FULL_BLOCK_EXPECTED            -0x6280  /**< Decryption of block requires a full block. */
#define POLARSSL_ERR_CIPHER_AUTH_FAILED                    -0x6300  /**< Authentication failed (for AEAD modes). */

#define POLARSSL_CIPHER_VARIABLE_IV_LEN     0x01    /**< Cipher accepts IVs of variable length */
#define POLARSSL_CIPHER_VARIABLE_KEY_LEN    0x02    /**< Cipher accepts keys of variable length */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    POLARSSL_CIPHER_ID_NONE = 0,
    POLARSSL_CIPHER_ID_NULL,
    POLARSSL_CIPHER_ID_AES,
    POLARSSL_CIPHER_ID_DES,
    POLARSSL_CIPHER_ID_3DES, /* Unused! */
    POLARSSL_CIPHER_ID_CAMELLIA,
    POLARSSL_CIPHER_ID_BLOWFISH,
    POLARSSL_CIPHER_ID_ARC4,
} cipher_id_t;

typedef enum {
    POLARSSL_CIPHER_NONE = 0,
    POLARSSL_CIPHER_NULL,
    POLARSSL_CIPHER_AES_128_ECB,
    POLARSSL_CIPHER_AES_192_ECB,
    POLARSSL_CIPHER_AES_256_ECB,
    POLARSSL_CIPHER_AES_128_CBC,
    POLARSSL_CIPHER_AES_192_CBC,
    POLARSSL_CIPHER_AES_256_CBC,
    POLARSSL_CIPHER_AES_128_CFB128,
    POLARSSL_CIPHER_AES_192_CFB128,
    POLARSSL_CIPHER_AES_256_CFB128,
    POLARSSL_CIPHER_AES_128_CTR,
    POLARSSL_CIPHER_AES_192_CTR,
    POLARSSL_CIPHER_AES_256_CTR,
    POLARSSL_CIPHER_AES_128_GCM,
    POLARSSL_CIPHER_AES_192_GCM,
    POLARSSL_CIPHER_AES_256_GCM,
    POLARSSL_CIPHER_CAMELLIA_128_ECB,
    POLARSSL_CIPHER_CAMELLIA_192_ECB,
    POLARSSL_CIPHER_CAMELLIA_256_ECB,
    POLARSSL_CIPHER_CAMELLIA_128_CBC,
    POLARSSL_CIPHER_CAMELLIA_192_CBC,
    POLARSSL_CIPHER_CAMELLIA_256_CBC,
    POLARSSL_CIPHER_CAMELLIA_128_CFB128,
    POLARSSL_CIPHER_CAMELLIA_192_CFB128,
    POLARSSL_CIPHER_CAMELLIA_256_CFB128,
    POLARSSL_CIPHER_CAMELLIA_128_CTR,
    POLARSSL_CIPHER_CAMELLIA_192_CTR,
    POLARSSL_CIPHER_CAMELLIA_256_CTR,
    POLARSSL_CIPHER_CAMELLIA_128_GCM,
    POLARSSL_CIPHER_CAMELLIA_192_GCM,
    POLARSSL_CIPHER_CAMELLIA_256_GCM,
    POLARSSL_CIPHER_DES_ECB,
    POLARSSL_CIPHER_DES_CBC,
    POLARSSL_CIPHER_DES_EDE_ECB,
    POLARSSL_CIPHER_DES_EDE_CBC,
    POLARSSL_CIPHER_DES_EDE3_ECB,
    POLARSSL_CIPHER_DES_EDE3_CBC,
    POLARSSL_CIPHER_BLOWFISH_ECB,
    POLARSSL_CIPHER_BLOWFISH_CBC,
    POLARSSL_CIPHER_BLOWFISH_CFB64,
    POLARSSL_CIPHER_BLOWFISH_CTR,
    POLARSSL_CIPHER_ARC4_128,
    POLARSSL_CIPHER_AES_128_CCM,
    POLARSSL_CIPHER_AES_192_CCM,
    POLARSSL_CIPHER_AES_256_CCM,
    POLARSSL_CIPHER_CAMELLIA_128_CCM,
    POLARSSL_CIPHER_CAMELLIA_192_CCM,
    POLARSSL_CIPHER_CAMELLIA_256_CCM,
} cipher_type_t;

typedef enum {
    POLARSSL_MODE_NONE = 0,
    POLARSSL_MODE_ECB,
    POLARSSL_MODE_CBC,
    POLARSSL_MODE_CFB,
    POLARSSL_MODE_OFB, /* Unused! */
    POLARSSL_MODE_CTR,
    POLARSSL_MODE_GCM,
    POLARSSL_MODE_STREAM,
    POLARSSL_MODE_CCM,
} cipher_mode_t;

typedef enum {
    POLARSSL_PADDING_PKCS7 = 0,     /**< PKCS7 padding (default)        */
    POLARSSL_PADDING_ONE_AND_ZEROS, /**< ISO/IEC 7816-4 padding         */
    POLARSSL_PADDING_ZEROS_AND_LEN, /**< ANSI X.923 padding             */
    POLARSSL_PADDING_ZEROS,         /**< zero padding (not reversible!) */
    POLARSSL_PADDING_NONE,          /**< never pad (full blocks only)   */
} cipher_padding_t;

typedef enum {
    POLARSSL_OPERATION_NONE = -1,
    POLARSSL_DECRYPT = 0,
    POLARSSL_ENCRYPT,
} operation_t;

enum {
    /** Undefined key length */
    POLARSSL_KEY_LENGTH_NONE = 0,
    /** Key length, in bits (including parity), for DES keys */
    POLARSSL_KEY_LENGTH_DES  = 64,
    /** Key length, in bits (including parity), for DES in two key EDE */
    POLARSSL_KEY_LENGTH_DES_EDE = 128,
    /** Key length, in bits (including parity), for DES in three-key EDE */
    POLARSSL_KEY_LENGTH_DES_EDE3 = 192,
};

/** Maximum length of any IV, in bytes */
#define POLARSSL_MAX_IV_LENGTH      16
/** Maximum block size of any cipher, in bytes */
#define POLARSSL_MAX_BLOCK_LENGTH   16

/**
 * Base cipher information. The non-mode specific functions and values.
 */
typedef struct {

    /** Base Cipher type (e.g. POLARSSL_CIPHER_ID_AES) */
    cipher_id_t cipher;

    /** Encrypt using ECB */
    int (*ecb_func)( void *ctx, operation_t mode,
                     const unsigned char *input, unsigned char *output );

#if defined(POLARSSL_CIPHER_MODE_CBC)
    /** Encrypt using CBC */
    int (*cbc_func)( void *ctx, operation_t mode, size_t length,
                     unsigned char *iv, const unsigned char *input,
                     unsigned char *output );
#endif

#if defined(POLARSSL_CIPHER_MODE_CFB)
    /** Encrypt using CFB (Full length) */
    int (*cfb_func)( void *ctx, operation_t mode, size_t length, size_t *iv_off,
                     unsigned char *iv, const unsigned char *input,
                     unsigned char *output );
#endif

#if defined(POLARSSL_CIPHER_MODE_CTR)
    /** Encrypt using CTR */
    int (*ctr_func)( void *ctx, size_t length, size_t *nc_off,
                     unsigned char *nonce_counter, unsigned char *stream_block,
                     const unsigned char *input, unsigned char *output );
#endif

#if defined(POLARSSL_CIPHER_MODE_STREAM)
    /** Encrypt using STREAM */
    int (*stream_func)( void *ctx, size_t length,
                        const unsigned char *input, unsigned char *output );
#endif

    /** Set key for encryption purposes */
    int (*setkey_enc_func)( void *ctx, const unsigned char *key,
                            unsigned int key_length );

    /** Set key for decryption purposes */
    int (*setkey_dec_func)( void *ctx, const unsigned char *key,
                            unsigned int key_length);

    /** Allocate a new context */
    void * (*ctx_alloc_func)( void );

    /** Free the given context */
    void (*ctx_free_func)( void *ctx );

} cipher_base_t;

/**
 * Cipher information. Allows cipher functions to be called in a generic way.
 */
typedef struct {
    /** Full cipher identifier (e.g. POLARSSL_CIPHER_AES_256_CBC) */
    cipher_type_t type;

    /** Cipher mode (e.g. POLARSSL_MODE_CBC) */
    cipher_mode_t mode;

    /** Cipher key length, in bits (default length for variable sized ciphers)
     *  (Includes parity bits for ciphers like DES) */
    unsigned int key_length;

    /** Name of the cipher */
    const char * name;

    /** IV/NONCE size, in bytes.
     *  For cipher that accept many sizes: recommended size */
    unsigned int iv_size;

    /** Flags for variable IV size, variable key size, etc. */
    int flags;

    /** block size, in bytes */
    unsigned int block_size;

    /** Base cipher information and functions */
    const cipher_base_t *base;

} cipher_info_t;

/**
 * Generic cipher context.
 */
typedef struct {
    /** Information about the associated cipher */
    const cipher_info_t *cipher_info;

    /** Key length to use */
    int key_length;

    /** Operation that the context's key has been initialised for */
    operation_t operation;

#if defined(POLARSSL_CIPHER_MODE_WITH_PADDING)
    /** Padding functions to use, if relevant for cipher mode */
    void (*add_padding)( unsigned char *output, size_t olen, size_t data_len );
    int (*get_padding)( unsigned char *input, size_t ilen, size_t *data_len );
#endif

    /** Buffer for data that hasn't been encrypted yet */
    unsigned char unprocessed_data[POLARSSL_MAX_BLOCK_LENGTH];

    /** Number of bytes that still need processing */
    size_t unprocessed_len;

    /** Current IV or NONCE_COUNTER for CTR-mode */
    unsigned char iv[POLARSSL_MAX_IV_LENGTH];

    /** IV size in bytes (for ciphers with variable-length IVs) */
    size_t iv_size;

    /** Cipher-specific context */
    void *cipher_ctx;
} cipher_context_t;

/**
 * \brief Returns the list of ciphers supported by the generic cipher module.
 *
 * \return              a statically allocated array of ciphers, the last entry
 *                      is 0.
 */
const int *cipher_list( void );

/**
 * \brief               Returns the cipher information structure associated
 *                      with the given cipher name.
 *
 * \param cipher_name   Name of the cipher to search for.
 *
 * \return              the cipher information structure associated with the
 *                      given cipher_name, or NULL if not found.
 */
const cipher_info_t *cipher_info_from_string( const char *cipher_name );

/**
 * \brief               Returns the cipher information structure associated
 *                      with the given cipher type.
 *
 * \param cipher_type   Type of the cipher to search for.
 *
 * \return              the cipher information structure associated with the
 *                      given cipher_type, or NULL if not found.
 */
const cipher_info_t *cipher_info_from_type( const cipher_type_t cipher_type );

/**
 * \brief               Returns the cipher information structure associated
 *                      with the given cipher id, key size and mode.
 *
 * \param cipher_id     Id of the cipher to search for
 *                      (e.g. POLARSSL_CIPHER_ID_AES)
 * \param key_length    Length of the key in bits
 * \param mode          Cipher mode (e.g. POLARSSL_MODE_CBC)
 *
 * \return              the cipher information structure associated with the
 *                      given cipher_type, or NULL if not found.
 */
const cipher_info_t *cipher_info_from_values( const cipher_id_t cipher_id,
                                              int key_length,
                                              const cipher_mode_t mode );

/**
 * \brief               Initialize a cipher_context (as NONE)
 */
void cipher_init( cipher_context_t *ctx );

/**
 * \brief               Free and clear the cipher-specific context of ctx.
 *                      Freeing ctx itself remains the responsibility of the
 *                      caller.
 */
void cipher_free( cipher_context_t *ctx );

/**
 * \brief               Initialises and fills the cipher context structure with
 *                      the appropriate values.
 *
 * \note                Currently also clears structure. In future versions you
 *                      will be required to call cipher_init() on the structure
 *                      first.
 *
 * \param ctx           context to initialise. May not be NULL.
 * \param cipher_info   cipher to use.
 *
 * \return              0 on success,
 *                      POLARSSL_ERR_CIPHER_BAD_INPUT_DATA on parameter failure,
 *                      POLARSSL_ERR_CIPHER_ALLOC_FAILED if allocation of the
 *                      cipher-specific context failed.
 */
int cipher_init_ctx( cipher_context_t *ctx, const cipher_info_t *cipher_info );

#if ! defined(POLARSSL_DEPRECATED_REMOVED)
#if defined(POLARSSL_DEPRECATED_WARNING)
#define DEPRECATED    __attribute__((deprecated))
#else
#define DEPRECATED
#endif
/**
 * \brief               Free the cipher-specific context of ctx. Freeing ctx
 *                      itself remains the responsibility of the caller.
 *
 * \deprecated          Use cipher_free() instead
 *
 * \param ctx           Free the cipher-specific context
 *
 * \returns             0
 */
int cipher_free_ctx( cipher_context_t *ctx ) DEPRECATED;
#undef DEPRECATED
#endif /* POLARSSL_DEPRECATED_REMOVED */

/**
 * \brief               Returns the block size of the given cipher.
 *
 * \param ctx           cipher's context. Must have been initialised.
 *
 * \return              size of the cipher's blocks, or 0 if ctx has not been
 *                      initialised.
 */
static inline unsigned int cipher_get_block_size( const cipher_context_t *ctx )
{
    if( NULL == ctx || NULL == ctx->cipher_info )
        return 0;

    return ctx->cipher_info->block_size;
}

/**
 * \brief               Returns the mode of operation for the cipher.
 *                      (e.g. POLARSSL_MODE_CBC)
 *
 * \param ctx           cipher's context. Must have been initialised.
 *
 * \return              mode of operation, or POLARSSL_MODE_NONE if ctx
 *                      has not been initialised.
 */
static inline cipher_mode_t cipher_get_cipher_mode( const cipher_context_t *ctx )
{
    if( NULL == ctx || NULL == ctx->cipher_info )
        return POLARSSL_MODE_NONE;

    return ctx->cipher_info->mode;
}

/**
 * \brief               Returns the size of the cipher's IV/NONCE in bytes.
 *
 * \param ctx           cipher's context. Must have been initialised.
 *
 * \return              If IV has not been set yet: (recommended) IV size
 *                      (0 for ciphers not using IV/NONCE).
 *                      If IV has already been set: actual size.
 */
static inline int cipher_get_iv_size( const cipher_context_t *ctx )
{
    if( NULL == ctx || NULL == ctx->cipher_info )
        return 0;

    if( ctx->iv_size != 0 )
        return (int) ctx->iv_size;

    return ctx->cipher_info->iv_size;
}

/**
 * \brief               Returns the type of the given cipher.
 *
 * \param ctx           cipher's context. Must have been initialised.
 *
 * \return              type of the cipher, or POLARSSL_CIPHER_NONE if ctx has
 *                      not been initialised.
 */
static inline cipher_type_t cipher_get_type( const cipher_context_t *ctx )
{
    if( NULL == ctx || NULL == ctx->cipher_info )
        return POLARSSL_CIPHER_NONE;

    return ctx->cipher_info->type;
}

/**
 * \brief               Returns the name of the given cipher, as a string.
 *
 * \param ctx           cipher's context. Must have been initialised.
 *
 * \return              name of the cipher, or NULL if ctx was not initialised.
 */
static inline const char *cipher_get_name( const cipher_context_t *ctx )
{
    if( NULL == ctx || NULL == ctx->cipher_info )
        return 0;

    return ctx->cipher_info->name;
}

/**
 * \brief               Returns the key length of the cipher.
 *
 * \param ctx           cipher's context. Must have been initialised.
 *
 * \return              cipher's key length, in bits, or
 *                      POLARSSL_KEY_LENGTH_NONE if ctx has not been
 *                      initialised.
 */
static inline int cipher_get_key_size( const cipher_context_t *ctx )
{
    if( NULL == ctx || NULL == ctx->cipher_info )
        return POLARSSL_KEY_LENGTH_NONE;

    return ctx->cipher_info->key_length;
}

/**
 * \brief               Returns the operation of the given cipher.
 *
 * \param ctx           cipher's context. Must have been initialised.
 *
 * \return              operation (POLARSSL_ENCRYPT or POLARSSL_DECRYPT),
 *                      or POLARSSL_OPERATION_NONE if ctx has not been
 *                      initialised.
 */
static inline operation_t cipher_get_operation( const cipher_context_t *ctx )
{
    if( NULL == ctx || NULL == ctx->cipher_info )
        return POLARSSL_OPERATION_NONE;

    return ctx->operation;
}

/**
 * \brief               Set the key to use with the given context.
 *
 * \param ctx           generic cipher context. May not be NULL. Must have been
 *                      initialised using cipher_context_from_type or
 *                      cipher_context_from_string.
 * \param key           The key to use.
 * \param key_length    key length to use, in bits.
 * \param operation     Operation that the key will be used for, either
 *                      POLARSSL_ENCRYPT or POLARSSL_DECRYPT.
 *
 * \returns             0 on success, POLARSSL_ERR_CIPHER_BAD_INPUT_DATA if
 *                      parameter verification fails or a cipher specific
 *                      error code.
 */
int cipher_setkey( cipher_context_t *ctx, const unsigned char *key,
                   int key_length, const operation_t operation );

#if defined(POLARSSL_CIPHER_MODE_WITH_PADDING)
/**
 * \brief               Set padding mode, for cipher modes that use padding.
 *                      (Default: PKCS7 padding.)
 *
 * \param ctx           generic cipher context
 * \param mode          padding mode
 *
 * \returns             0 on success, POLARSSL_ERR_CIPHER_FEATURE_UNAVAILABLE
 *                      if selected padding mode is not supported, or
 *                      POLARSSL_ERR_CIPHER_BAD_INPUT_DATA if the cipher mode
 *                      does not support padding.
 */
int cipher_set_padding_mode( cipher_context_t *ctx, cipher_padding_t mode );
#endif /* POLARSSL_CIPHER_MODE_WITH_PADDING */

/**
 * \brief               Set the initialization vector (IV) or nonce
 *
 * \param ctx           generic cipher context
 * \param iv            IV to use (or NONCE_COUNTER for CTR-mode ciphers)
 * \param iv_len        IV length for ciphers with variable-size IV;
 *                      discarded by ciphers with fixed-size IV.
 *
 * \returns             0 on success, or POLARSSL_ERR_CIPHER_BAD_INPUT_DATA
 *
 * \note                Some ciphers don't use IVs nor NONCE. For these
 *                      ciphers, this function has no effect.
 */
int cipher_set_iv( cipher_context_t *ctx,
                   const unsigned char *iv, size_t iv_len );

/**
 * \brief               Finish preparation of the given context
 *
 * \param ctx           generic cipher context
 *
 * \returns             0 on success, POLARSSL_ERR_CIPHER_BAD_INPUT_DATA
 *                      if parameter verification fails.
 */
int cipher_reset( cipher_context_t *ctx );

#if defined(POLARSSL_GCM_C)
/**
 * \brief               Add additional data (for AEAD ciphers).
 *                      Currently only supported with GCM.
 *                      Must be called exactly once, after cipher_reset().
 *
 * \param ctx           generic cipher context
 * \param ad            Additional data to use.
 * \param ad_len        Length of ad.
 *
 * \return              0 on success, or a specific error code.
 */
int cipher_update_ad( cipher_context_t *ctx,
                      const unsigned char *ad, size_t ad_len );
#endif /* POLARSSL_GCM_C */

/**
 * \brief               Generic cipher update function. Encrypts/decrypts
 *                      using the given cipher context. Writes as many block
 *                      size'd blocks of data as possible to output. Any data
 *                      that cannot be written immediately will either be added
 *                      to the next block, or flushed when cipher_final is
 *                      called.
 *                      Exception: for POLARSSL_MODE_ECB, expects single block
 *                                 in size (e.g. 16 bytes for AES)
 *
 * \param ctx           generic cipher context
 * \param input         buffer holding the input data
 * \param ilen          length of the input data
 * \param output        buffer for the output data. Should be able to hold at
 *                      least ilen + block_size. Cannot be the same buffer as
 *                      input!
 * \param olen          length of the output data, will be filled with the
 *                      actual number of bytes written.
 *
 * \returns             0 on success, POLARSSL_ERR_CIPHER_BAD_INPUT_DATA if
 *                      parameter verification fails,
 *                      POLARSSL_ERR_CIPHER_FEATURE_UNAVAILABLE on an
 *                      unsupported mode for a cipher or a cipher specific
 *                      error code.
 *
 * \note                If the underlying cipher is GCM, all calls to this
 *                      function, except the last one before cipher_finish(),
 *                      must have ilen a multiple of the block size.
 */
int cipher_update( cipher_context_t *ctx, const unsigned char *input,
                   size_t ilen, unsigned char *output, size_t *olen );

/**
 * \brief               Generic cipher finalisation function. If data still
 *                      needs to be flushed from an incomplete block, data
 *                      contained within it will be padded with the size of
 *                      the last block, and written to the output buffer.
 *
 * \param ctx           Generic cipher context
 * \param output        buffer to write data to. Needs block_size available.
 * \param olen          length of the data written to the output buffer.
 *
 * \returns             0 on success, POLARSSL_ERR_CIPHER_BAD_INPUT_DATA if
 *                      parameter verification fails,
 *                      POLARSSL_ERR_CIPHER_FULL_BLOCK_EXPECTED if decryption
 *                      expected a full block but was not provided one,
 *                      POLARSSL_ERR_CIPHER_INVALID_PADDING on invalid padding
 *                      while decrypting or a cipher specific error code.
 */
int cipher_finish( cipher_context_t *ctx,
                   unsigned char *output, size_t *olen );

#if defined(POLARSSL_GCM_C)
/**
 * \brief               Write tag for AEAD ciphers.
 *                      Currently only supported with GCM.
 *                      Must be called after cipher_finish().
 *
 * \param ctx           Generic cipher context
 * \param tag           buffer to write the tag
 * \param tag_len       Length of the tag to write
 *
 * \return              0 on success, or a specific error code.
 */
int cipher_write_tag( cipher_context_t *ctx,
                      unsigned char *tag, size_t tag_len );

/**
 * \brief               Check tag for AEAD ciphers.
 *                      Currently only supported with GCM.
 *                      Must be called after cipher_finish().
 *
 * \param ctx           Generic cipher context
 * \param tag           Buffer holding the tag
 * \param tag_len       Length of the tag to check
 *
 * \return              0 on success, or a specific error code.
 */
int cipher_check_tag( cipher_context_t *ctx,
                      const unsigned char *tag, size_t tag_len );
#endif /* POLARSSL_GCM_C */

/**
 * \brief               Generic all-in-one encryption/decryption
 *                      (for all ciphers except AEAD constructs).
 *
 * \param ctx           generic cipher context
 * \param iv            IV to use (or NONCE_COUNTER for CTR-mode ciphers)
 * \param iv_len        IV length for ciphers with variable-size IV;
 *                      discarded by ciphers with fixed-size IV.
 * \param input         buffer holding the input data
 * \param ilen          length of the input data
 * \param output        buffer for the output data. Should be able to hold at
 *                      least ilen + block_size. Cannot be the same buffer as
 *                      input!
 * \param olen          length of the output data, will be filled with the
 *                      actual number of bytes written.
 *
 * \note                Some ciphers don't use IVs nor NONCE. For these
 *                      ciphers, use iv = NULL and iv_len = 0.
 *
 * \returns             0 on success, or
 *                      POLARSSL_ERR_CIPHER_BAD_INPUT_DATA, or
 *                      POLARSSL_ERR_CIPHER_FULL_BLOCK_EXPECTED if decryption
 *                      expected a full block but was not provided one, or
 *                      POLARSSL_ERR_CIPHER_INVALID_PADDING on invalid padding
 *                      while decrypting, or
 *                      a cipher specific error code.
 */
int cipher_crypt( cipher_context_t *ctx,
                  const unsigned char *iv, size_t iv_len,
                  const unsigned char *input, size_t ilen,
                  unsigned char *output, size_t *olen );

#if defined(POLARSSL_CIPHER_MODE_AEAD)
/**
 * \brief               Generic autenticated encryption (AEAD ciphers).
 *
 * \param ctx           generic cipher context
 * \param iv            IV to use (or NONCE_COUNTER for CTR-mode ciphers)
 * \param iv_len        IV length for ciphers with variable-size IV;
 *                      discarded by ciphers with fixed-size IV.
 * \param ad            Additional data to authenticate.
 * \param ad_len        Length of ad.
 * \param input         buffer holding the input data
 * \param ilen          length of the input data
 * \param output        buffer for the output data.
 *                      Should be able to hold at least ilen.
 * \param olen          length of the output data, will be filled with the
 *                      actual number of bytes written.
 * \param tag           buffer for the authentication tag
 * \param tag_len       desired tag length
 *
 * \returns             0 on success, or
 *                      POLARSSL_ERR_CIPHER_BAD_INPUT_DATA, or
 *                      a cipher specific error code.
 */
int cipher_auth_encrypt( cipher_context_t *ctx,
                         const unsigned char *iv, size_t iv_len,
                         const unsigned char *ad, size_t ad_len,
                         const unsigned char *input, size_t ilen,
                         unsigned char *output, size_t *olen,
                         unsigned char *tag, size_t tag_len );

/**
 * \brief               Generic autenticated decryption (AEAD ciphers).
 *
 * \param ctx           generic cipher context
 * \param iv            IV to use (or NONCE_COUNTER for CTR-mode ciphers)
 * \param iv_len        IV length for ciphers with variable-size IV;
 *                      discarded by ciphers with fixed-size IV.
 * \param ad            Additional data to be authenticated.
 * \param ad_len        Length of ad.
 * \param input         buffer holding the input data
 * \param ilen          length of the input data
 * \param output        buffer for the output data.
 *                      Should be able to hold at least ilen.
 * \param olen          length of the output data, will be filled with the
 *                      actual number of bytes written.
 * \param tag           buffer holding the authentication tag
 * \param tag_len       length of the authentication tag
 *
 * \returns             0 on success, or
 *                      POLARSSL_ERR_CIPHER_BAD_INPUT_DATA, or
 *                      POLARSSL_ERR_CIPHER_AUTH_FAILED if data isn't authentic,
 *                      or a cipher specific error code.
 *
 * \note                If the data is not authentic, then the output buffer
 *                      is zeroed out to prevent the unauthentic plaintext to
 *                      be used by mistake, making this interface safer.
 */
int cipher_auth_decrypt( cipher_context_t *ctx,
                         const unsigned char *iv, size_t iv_len,
                         const unsigned char *ad, size_t ad_len,
                         const unsigned char *input, size_t ilen,
                         unsigned char *output, size_t *olen,
                         const unsigned char *tag, size_t tag_len );
#endif /* POLARSSL_CIPHER_MODE_AEAD */

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int cipher_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* POLARSSL_CIPHER_H */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/cipher_wrap.h ************/


/**
 * \file cipher_wrap.h
 *
 * \brief Cipher wrappers.
 *
 * \author Adriaan de Jong <dejong@fox-it.com>
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_CIPHER_WRAP_H
#define POLARSSL_CIPHER_WRAP_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif



#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    cipher_type_t type;
    const cipher_info_t *info;
} cipher_definition_t;

extern const cipher_definition_t cipher_definitions[];

extern int supported_ciphers[];

#ifdef __cplusplus
}
#endif

#endif /* POLARSSL_CIPHER_WRAP_H */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/ssl_ciphersuites.h ************/


/**
 * \file ssl_ciphersuites.h
 *
 * \brief SSL Ciphersuites for mbed TLS
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_SSL_CIPHERSUITES_H
#define POLARSSL_SSL_CIPHERSUITES_H





#ifdef __cplusplus
extern "C" {
#endif

/*
 * Supported ciphersuites (Official IANA names)
 */
#define TLS_RSA_WITH_NULL_MD5                    0x01   /**< Weak! */
#define TLS_RSA_WITH_NULL_SHA                    0x02   /**< Weak! */

#define TLS_RSA_WITH_RC4_128_MD5                 0x04
#define TLS_RSA_WITH_RC4_128_SHA                 0x05
#define TLS_RSA_WITH_DES_CBC_SHA                 0x09   /**< Weak! Not in TLS 1.2 */

#define TLS_RSA_WITH_3DES_EDE_CBC_SHA            0x0A

#define TLS_DHE_RSA_WITH_DES_CBC_SHA             0x15   /**< Weak! Not in TLS 1.2 */
#define TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA        0x16

#define TLS_PSK_WITH_NULL_SHA                    0x2C   /**< Weak! */
#define TLS_DHE_PSK_WITH_NULL_SHA                0x2D   /**< Weak! */
#define TLS_RSA_PSK_WITH_NULL_SHA                0x2E   /**< Weak! */
#define TLS_RSA_WITH_AES_128_CBC_SHA             0x2F

#define TLS_DHE_RSA_WITH_AES_128_CBC_SHA         0x33
#define TLS_RSA_WITH_AES_256_CBC_SHA             0x35
#define TLS_DHE_RSA_WITH_AES_256_CBC_SHA         0x39

#define TLS_RSA_WITH_NULL_SHA256                 0x3B   /**< Weak! */
#define TLS_RSA_WITH_AES_128_CBC_SHA256          0x3C   /**< TLS 1.2 */
#define TLS_RSA_WITH_AES_256_CBC_SHA256          0x3D   /**< TLS 1.2 */

#define TLS_RSA_WITH_CAMELLIA_128_CBC_SHA        0x41
#define TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA    0x45

#define TLS_DHE_RSA_WITH_AES_128_CBC_SHA256      0x67   /**< TLS 1.2 */
#define TLS_DHE_RSA_WITH_AES_256_CBC_SHA256      0x6B   /**< TLS 1.2 */

#define TLS_RSA_WITH_CAMELLIA_256_CBC_SHA        0x84
#define TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA    0x88

#define TLS_PSK_WITH_RC4_128_SHA                 0x8A
#define TLS_PSK_WITH_3DES_EDE_CBC_SHA            0x8B
#define TLS_PSK_WITH_AES_128_CBC_SHA             0x8C
#define TLS_PSK_WITH_AES_256_CBC_SHA             0x8D

#define TLS_DHE_PSK_WITH_RC4_128_SHA             0x8E
#define TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA        0x8F
#define TLS_DHE_PSK_WITH_AES_128_CBC_SHA         0x90
#define TLS_DHE_PSK_WITH_AES_256_CBC_SHA         0x91

#define TLS_RSA_PSK_WITH_RC4_128_SHA             0x92
#define TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA        0x93
#define TLS_RSA_PSK_WITH_AES_128_CBC_SHA         0x94
#define TLS_RSA_PSK_WITH_AES_256_CBC_SHA         0x95

#define TLS_RSA_WITH_AES_128_GCM_SHA256          0x9C   /**< TLS 1.2 */
#define TLS_RSA_WITH_AES_256_GCM_SHA384          0x9D   /**< TLS 1.2 */
#define TLS_DHE_RSA_WITH_AES_128_GCM_SHA256      0x9E   /**< TLS 1.2 */
#define TLS_DHE_RSA_WITH_AES_256_GCM_SHA384      0x9F   /**< TLS 1.2 */

#define TLS_PSK_WITH_AES_128_GCM_SHA256          0xA8   /**< TLS 1.2 */
#define TLS_PSK_WITH_AES_256_GCM_SHA384          0xA9   /**< TLS 1.2 */
#define TLS_DHE_PSK_WITH_AES_128_GCM_SHA256      0xAA   /**< TLS 1.2 */
#define TLS_DHE_PSK_WITH_AES_256_GCM_SHA384      0xAB   /**< TLS 1.2 */
#define TLS_RSA_PSK_WITH_AES_128_GCM_SHA256      0xAC   /**< TLS 1.2 */
#define TLS_RSA_PSK_WITH_AES_256_GCM_SHA384      0xAD   /**< TLS 1.2 */

#define TLS_PSK_WITH_AES_128_CBC_SHA256          0xAE
#define TLS_PSK_WITH_AES_256_CBC_SHA384          0xAF
#define TLS_PSK_WITH_NULL_SHA256                 0xB0   /**< Weak! */
#define TLS_PSK_WITH_NULL_SHA384                 0xB1   /**< Weak! */

#define TLS_DHE_PSK_WITH_AES_128_CBC_SHA256      0xB2
#define TLS_DHE_PSK_WITH_AES_256_CBC_SHA384      0xB3
#define TLS_DHE_PSK_WITH_NULL_SHA256             0xB4   /**< Weak! */
#define TLS_DHE_PSK_WITH_NULL_SHA384             0xB5   /**< Weak! */

#define TLS_RSA_PSK_WITH_AES_128_CBC_SHA256      0xB6
#define TLS_RSA_PSK_WITH_AES_256_CBC_SHA384      0xB7
#define TLS_RSA_PSK_WITH_NULL_SHA256             0xB8   /**< Weak! */
#define TLS_RSA_PSK_WITH_NULL_SHA384             0xB9   /**< Weak! */

#define TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256     0xBA   /**< TLS 1.2 */
#define TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256 0xBE   /**< TLS 1.2 */

#define TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256     0xC0   /**< TLS 1.2 */
#define TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256 0xC4   /**< TLS 1.2 */

#define TLS_ECDH_ECDSA_WITH_NULL_SHA             0xC001 /**< Weak! */
#define TLS_ECDH_ECDSA_WITH_RC4_128_SHA          0xC002 /**< Not in SSL3! */
#define TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA     0xC003 /**< Not in SSL3! */
#define TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA      0xC004 /**< Not in SSL3! */
#define TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA      0xC005 /**< Not in SSL3! */

#define TLS_ECDHE_ECDSA_WITH_NULL_SHA            0xC006 /**< Weak! */
#define TLS_ECDHE_ECDSA_WITH_RC4_128_SHA         0xC007 /**< Not in SSL3! */
#define TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA    0xC008 /**< Not in SSL3! */
#define TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA     0xC009 /**< Not in SSL3! */
#define TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA     0xC00A /**< Not in SSL3! */

#define TLS_ECDH_RSA_WITH_NULL_SHA               0xC00B /**< Weak! */
#define TLS_ECDH_RSA_WITH_RC4_128_SHA            0xC00C /**< Not in SSL3! */
#define TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA       0xC00D /**< Not in SSL3! */
#define TLS_ECDH_RSA_WITH_AES_128_CBC_SHA        0xC00E /**< Not in SSL3! */
#define TLS_ECDH_RSA_WITH_AES_256_CBC_SHA        0xC00F /**< Not in SSL3! */

#define TLS_ECDHE_RSA_WITH_NULL_SHA              0xC010 /**< Weak! */
#define TLS_ECDHE_RSA_WITH_RC4_128_SHA           0xC011 /**< Not in SSL3! */
#define TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA      0xC012 /**< Not in SSL3! */
#define TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA       0xC013 /**< Not in SSL3! */
#define TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA       0xC014 /**< Not in SSL3! */

#define TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256  0xC023 /**< TLS 1.2 */
#define TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384  0xC024 /**< TLS 1.2 */
#define TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256   0xC025 /**< TLS 1.2 */
#define TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384   0xC026 /**< TLS 1.2 */
#define TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256    0xC027 /**< TLS 1.2 */
#define TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384    0xC028 /**< TLS 1.2 */
#define TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256     0xC029 /**< TLS 1.2 */
#define TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384     0xC02A /**< TLS 1.2 */

#define TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256  0xC02B /**< TLS 1.2 */
#define TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384  0xC02C /**< TLS 1.2 */
#define TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256   0xC02D /**< TLS 1.2 */
#define TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384   0xC02E /**< TLS 1.2 */
#define TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256    0xC02F /**< TLS 1.2 */
#define TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384    0xC030 /**< TLS 1.2 */
#define TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256     0xC031 /**< TLS 1.2 */
#define TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384     0xC032 /**< TLS 1.2 */

#define TLS_ECDHE_PSK_WITH_RC4_128_SHA           0xC033 /**< Not in SSL3! */
#define TLS_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA      0xC034 /**< Not in SSL3! */
#define TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA       0xC035 /**< Not in SSL3! */
#define TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA       0xC036 /**< Not in SSL3! */
#define TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256    0xC037 /**< Not in SSL3! */
#define TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384    0xC038 /**< Not in SSL3! */
#define TLS_ECDHE_PSK_WITH_NULL_SHA              0xC039 /**< Weak! No SSL3! */
#define TLS_ECDHE_PSK_WITH_NULL_SHA256           0xC03A /**< Weak! No SSL3! */
#define TLS_ECDHE_PSK_WITH_NULL_SHA384           0xC03B /**< Weak! No SSL3! */

#define TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256 0xC072 /**< Not in SSL3! */
#define TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384 0xC073 /**< Not in SSL3! */
#define TLS_ECDH_ECDSA_WITH_CAMELLIA_128_CBC_SHA256  0xC074 /**< Not in SSL3! */
#define TLS_ECDH_ECDSA_WITH_CAMELLIA_256_CBC_SHA384  0xC075 /**< Not in SSL3! */
#define TLS_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256   0xC076 /**< Not in SSL3! */
#define TLS_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384   0xC077 /**< Not in SSL3! */
#define TLS_ECDH_RSA_WITH_CAMELLIA_128_CBC_SHA256    0xC078 /**< Not in SSL3! */
#define TLS_ECDH_RSA_WITH_CAMELLIA_256_CBC_SHA384    0xC079 /**< Not in SSL3! */

#define TLS_RSA_WITH_CAMELLIA_128_GCM_SHA256         0xC07A /**< TLS 1.2 */
#define TLS_RSA_WITH_CAMELLIA_256_GCM_SHA384         0xC07B /**< TLS 1.2 */
#define TLS_DHE_RSA_WITH_CAMELLIA_128_GCM_SHA256     0xC07C /**< TLS 1.2 */
#define TLS_DHE_RSA_WITH_CAMELLIA_256_GCM_SHA384     0xC07D /**< TLS 1.2 */
#define TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_GCM_SHA256 0xC086 /**< TLS 1.2 */
#define TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_GCM_SHA384 0xC087 /**< TLS 1.2 */
#define TLS_ECDH_ECDSA_WITH_CAMELLIA_128_GCM_SHA256  0xC088 /**< TLS 1.2 */
#define TLS_ECDH_ECDSA_WITH_CAMELLIA_256_GCM_SHA384  0xC089 /**< TLS 1.2 */
#define TLS_ECDHE_RSA_WITH_CAMELLIA_128_GCM_SHA256   0xC08A /**< TLS 1.2 */
#define TLS_ECDHE_RSA_WITH_CAMELLIA_256_GCM_SHA384   0xC08B /**< TLS 1.2 */
#define TLS_ECDH_RSA_WITH_CAMELLIA_128_GCM_SHA256    0xC08C /**< TLS 1.2 */
#define TLS_ECDH_RSA_WITH_CAMELLIA_256_GCM_SHA384    0xC08D /**< TLS 1.2 */

#define TLS_PSK_WITH_CAMELLIA_128_GCM_SHA256       0xC08E /**< TLS 1.2 */
#define TLS_PSK_WITH_CAMELLIA_256_GCM_SHA384       0xC08F /**< TLS 1.2 */
#define TLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256   0xC090 /**< TLS 1.2 */
#define TLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384   0xC091 /**< TLS 1.2 */
#define TLS_RSA_PSK_WITH_CAMELLIA_128_GCM_SHA256   0xC092 /**< TLS 1.2 */
#define TLS_RSA_PSK_WITH_CAMELLIA_256_GCM_SHA384   0xC093 /**< TLS 1.2 */

#define TLS_PSK_WITH_CAMELLIA_128_CBC_SHA256       0xC094
#define TLS_PSK_WITH_CAMELLIA_256_CBC_SHA384       0xC095
#define TLS_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256   0xC096
#define TLS_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384   0xC097
#define TLS_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256   0xC098
#define TLS_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384   0xC099
#define TLS_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256 0xC09A /**< Not in SSL3! */
#define TLS_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384 0xC09B /**< Not in SSL3! */

#define TLS_RSA_WITH_AES_128_CCM                0xC09C  /**< TLS 1.2 */
#define TLS_RSA_WITH_AES_256_CCM                0xC09D  /**< TLS 1.2 */
#define TLS_DHE_RSA_WITH_AES_128_CCM            0xC09E  /**< TLS 1.2 */
#define TLS_DHE_RSA_WITH_AES_256_CCM            0xC09F  /**< TLS 1.2 */
#define TLS_RSA_WITH_AES_128_CCM_8              0xC0A0  /**< TLS 1.2 */
#define TLS_RSA_WITH_AES_256_CCM_8              0xC0A1  /**< TLS 1.2 */
#define TLS_DHE_RSA_WITH_AES_128_CCM_8          0xC0A2  /**< TLS 1.2 */
#define TLS_DHE_RSA_WITH_AES_256_CCM_8          0xC0A3  /**< TLS 1.2 */
#define TLS_PSK_WITH_AES_128_CCM                0xC0A4  /**< TLS 1.2 */
#define TLS_PSK_WITH_AES_256_CCM                0xC0A5  /**< TLS 1.2 */
#define TLS_DHE_PSK_WITH_AES_128_CCM            0xC0A6  /**< TLS 1.2 */
#define TLS_DHE_PSK_WITH_AES_256_CCM            0xC0A7  /**< TLS 1.2 */
#define TLS_PSK_WITH_AES_128_CCM_8              0xC0A8  /**< TLS 1.2 */
#define TLS_PSK_WITH_AES_256_CCM_8              0xC0A9  /**< TLS 1.2 */
#define TLS_DHE_PSK_WITH_AES_128_CCM_8          0xC0AA  /**< TLS 1.2 */
#define TLS_DHE_PSK_WITH_AES_256_CCM_8          0xC0AB  /**< TLS 1.2 */
/* The last two are named with PSK_DHE in the RFC, which looks like a typo */

#define TLS_ECDHE_ECDSA_WITH_AES_128_CCM        0xC0AC  /**< TLS 1.2 */
#define TLS_ECDHE_ECDSA_WITH_AES_256_CCM        0xC0AD  /**< TLS 1.2 */
#define TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8      0xC0AE  /**< TLS 1.2 */
#define TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8      0xC0AF  /**< TLS 1.2 */

/* Reminder: update _ssl_premaster_secret when adding a new key exchange.
 * Reminder: update POLARSSL_KEY_EXCHANGE__WITH_CERT__ENABLED below.
 */
typedef enum {
    POLARSSL_KEY_EXCHANGE_NONE = 0,
    POLARSSL_KEY_EXCHANGE_RSA,
    POLARSSL_KEY_EXCHANGE_DHE_RSA,
    POLARSSL_KEY_EXCHANGE_ECDHE_RSA,
    POLARSSL_KEY_EXCHANGE_ECDHE_ECDSA,
    POLARSSL_KEY_EXCHANGE_PSK,
    POLARSSL_KEY_EXCHANGE_DHE_PSK,
    POLARSSL_KEY_EXCHANGE_RSA_PSK,
    POLARSSL_KEY_EXCHANGE_ECDHE_PSK,
    POLARSSL_KEY_EXCHANGE_ECDH_RSA,
    POLARSSL_KEY_EXCHANGE_ECDH_ECDSA,
} key_exchange_type_t;

#if defined(POLARSSL_KEY_EXCHANGE_RSA_ENABLED)          || \
    defined(POLARSSL_KEY_EXCHANGE_DHE_RSA_ENABLED)      || \
    defined(POLARSSL_KEY_EXCHANGE_ECDHE_RSA_ENABLED)    || \
    defined(POLARSSL_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED)  || \
    defined(POLARSSL_KEY_EXCHANGE_RSA_PSK_ENABLED)      || \
    defined(POLARSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED)    || \
    defined(POLARSSL_KEY_EXCHANGE_ECDH_RSA_ENABLED)     || \
    defined(POLARSSL_KEY_EXCHANGE_ECDH_ECDSA_ENABLED)
#define POLARSSL_KEY_EXCHANGE__WITH_CERT__ENABLED
#endif

typedef struct _ssl_ciphersuite_t ssl_ciphersuite_t;

#define POLARSSL_CIPHERSUITE_WEAK       0x01    /**< Weak ciphersuite flag  */
#define POLARSSL_CIPHERSUITE_SHORT_TAG  0x02    /**< Short authentication tag,
                                                     eg for CCM_8 */

/**
 * \brief   This structure is used for storing ciphersuite information
 */
struct _ssl_ciphersuite_t
{
    int id;
    const char * name;

    cipher_type_t cipher;
    md_type_t mac;
    key_exchange_type_t key_exchange;

    int min_major_ver;
    int min_minor_ver;
    int max_major_ver;
    int max_minor_ver;

    unsigned char flags;
};

const int *ssl_list_ciphersuites( void );

const ssl_ciphersuite_t *ssl_ciphersuite_from_string( const char *ciphersuite_name );
const ssl_ciphersuite_t *ssl_ciphersuite_from_id( int ciphersuite_id );

#if defined(POLARSSL_PK_C)
pk_type_t ssl_get_ciphersuite_sig_pk_alg( const ssl_ciphersuite_t *info );
#endif

int ssl_ciphersuite_uses_ec( const ssl_ciphersuite_t *info );
int ssl_ciphersuite_uses_psk( const ssl_ciphersuite_t *info );

#ifdef __cplusplus
}
#endif

#endif /* ssl_ciphersuites.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/ecdh.h ************/


/**
 * \file ecdh.h
 *
 * \brief Elliptic curve Diffie-Hellman
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_ECDH_H
#define POLARSSL_ECDH_H



#ifdef __cplusplus
extern "C" {
#endif

/**
 * When importing from an EC key, select if it is our key or the peer's key
 */
typedef enum
{
    POLARSSL_ECDH_OURS,
    POLARSSL_ECDH_THEIRS,
} ecdh_side;

/**
 * \brief           ECDH context structure
 */
typedef struct
{
    ecp_group grp;      /*!<  elliptic curve used                           */
    mpi d;              /*!<  our secret value (private key)                */
    ecp_point Q;        /*!<  our public value (public key)                 */
    ecp_point Qp;       /*!<  peer's public value (public key)              */
    mpi z;              /*!<  shared secret                                 */
    int point_format;   /*!<  format for point export in TLS messages       */
    ecp_point Vi;       /*!<  blinding value (for later)                    */
    ecp_point Vf;       /*!<  un-blinding value (for later)                 */
    mpi _d;             /*!<  previous d (for later)                        */
}
ecdh_context;

/**
 * \brief           Generate a public key.
 *                  Raw function that only does the core computation.
 *
 * \param grp       ECP group
 * \param d         Destination MPI (secret exponent, aka private key)
 * \param Q         Destination point (public key)
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 if successful,
 *                  or a POLARSSL_ERR_ECP_XXX or POLARSSL_MPI_XXX error code
 */
int ecdh_gen_public( ecp_group *grp, mpi *d, ecp_point *Q,
                     int (*f_rng)(void *, unsigned char *, size_t),
                     void *p_rng );

/**
 * \brief           Compute shared secret
 *                  Raw function that only does the core computation.
 *
 * \param grp       ECP group
 * \param z         Destination MPI (shared secret)
 * \param Q         Public key from other party
 * \param d         Our secret exponent (private key)
 * \param f_rng     RNG function (see notes)
 * \param p_rng     RNG parameter
 *
 * \return          0 if successful,
 *                  or a POLARSSL_ERR_ECP_XXX or POLARSSL_MPI_XXX error code
 *
 * \note            If f_rng is not NULL, it is used to implement
 *                  countermeasures against potential elaborate timing
 *                  attacks, see \c ecp_mul() for details.
 */
int ecdh_compute_shared( ecp_group *grp, mpi *z,
                         const ecp_point *Q, const mpi *d,
                         int (*f_rng)(void *, unsigned char *, size_t),
                         void *p_rng );

/**
 * \brief           Initialize context
 *
 * \param ctx       Context to initialize
 */
void ecdh_init( ecdh_context *ctx );

/**
 * \brief           Free context
 *
 * \param ctx       Context to free
 */
void ecdh_free( ecdh_context *ctx );

/**
 * \brief           Generate a public key and a TLS ServerKeyExchange payload.
 *                  (First function used by a TLS server for ECDHE.)
 *
 * \param ctx       ECDH context
 * \param olen      number of chars written
 * \param buf       destination buffer
 * \param blen      length of buffer
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \note            This function assumes that ctx->grp has already been
 *                  properly set (for example using ecp_use_known_dp).
 *
 * \return          0 if successful, or an POLARSSL_ERR_ECP_XXX error code
 */
int ecdh_make_params( ecdh_context *ctx, size_t *olen,
                      unsigned char *buf, size_t blen,
                      int (*f_rng)(void *, unsigned char *, size_t),
                      void *p_rng );

/**
 * \brief           Parse and procress a TLS ServerKeyExhange payload.
 *                  (First function used by a TLS client for ECDHE.)
 *
 * \param ctx       ECDH context
 * \param buf       pointer to start of input buffer
 * \param end       one past end of buffer
 *
 * \return          0 if successful, or an POLARSSL_ERR_ECP_XXX error code
 */
int ecdh_read_params( ecdh_context *ctx,
                      const unsigned char **buf, const unsigned char *end );

/**
 * \brief           Setup an ECDH context from an EC key.
 *                  (Used by clients and servers in place of the
 *                  ServerKeyEchange for static ECDH: import ECDH parameters
 *                  from a certificate's EC key information.)
 *
 * \param ctx       ECDH constext to set
 * \param key       EC key to use
 * \param side      Is it our key (1) or the peer's key (0) ?
 *
 * \return          0 if successful, or an POLARSSL_ERR_ECP_XXX error code
 */
int ecdh_get_params( ecdh_context *ctx, const ecp_keypair *key,
                     ecdh_side side );

/**
 * \brief           Generate a public key and a TLS ClientKeyExchange payload.
 *                  (Second function used by a TLS client for ECDH(E).)
 *
 * \param ctx       ECDH context
 * \param olen      number of bytes actually written
 * \param buf       destination buffer
 * \param blen      size of destination buffer
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 if successful, or an POLARSSL_ERR_ECP_XXX error code
 */
int ecdh_make_public( ecdh_context *ctx, size_t *olen,
                      unsigned char *buf, size_t blen,
                      int (*f_rng)(void *, unsigned char *, size_t),
                      void *p_rng );

/**
 * \brief           Parse and process a TLS ClientKeyExchange payload.
 *                  (Second function used by a TLS server for ECDH(E).)
 *
 * \param ctx       ECDH context
 * \param buf       start of input buffer
 * \param blen      length of input buffer
 *
 * \return          0 if successful, or an POLARSSL_ERR_ECP_XXX error code
 */
int ecdh_read_public( ecdh_context *ctx,
                      const unsigned char *buf, size_t blen );

/**
 * \brief           Derive and export the shared secret.
 *                  (Last function used by both TLS client en servers.)
 *
 * \param ctx       ECDH context
 * \param olen      number of bytes written
 * \param buf       destination buffer
 * \param blen      buffer length
 * \param f_rng     RNG function, see notes for \c ecdh_compute_shared()
 * \param p_rng     RNG parameter
 *
 * \return          0 if successful, or an POLARSSL_ERR_ECP_XXX error code
 */
int ecdh_calc_secret( ecdh_context *ctx, size_t *olen,
                      unsigned char *buf, size_t blen,
                      int (*f_rng)(void *, unsigned char *, size_t),
                      void *p_rng );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int ecdh_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* ecdh.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/sha1.h ************/


/**
 * \file sha1.h
 *
 * \brief SHA-1 cryptographic hash function
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_SHA1_H
#define POLARSSL_SHA1_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

#define POLARSSL_ERR_SHA1_FILE_IO_ERROR                -0x0076  /**< Read/write error in file. */

#if !defined(POLARSSL_SHA1_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          SHA-1 context structure
 */
typedef struct
{
    uint32_t total[2];          /*!< number of bytes processed  */
    uint32_t state[5];          /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */

    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
}
sha1_context;

/**
 * \brief          Initialize SHA-1 context
 *
 * \param ctx      SHA-1 context to be initialized
 */
void sha1_init( sha1_context *ctx );

/**
 * \brief          Clear SHA-1 context
 *
 * \param ctx      SHA-1 context to be cleared
 */
void sha1_free( sha1_context *ctx );

/**
 * \brief          SHA-1 context setup
 *
 * \param ctx      context to be initialized
 */
void sha1_starts( sha1_context *ctx );

/**
 * \brief          SHA-1 process buffer
 *
 * \param ctx      SHA-1 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha1_update( sha1_context *ctx, const unsigned char *input, size_t ilen );

/**
 * \brief          SHA-1 final digest
 *
 * \param ctx      SHA-1 context
 * \param output   SHA-1 checksum result
 */
void sha1_finish( sha1_context *ctx, unsigned char output[20] );

/* Internal use */
void sha1_process( sha1_context *ctx, const unsigned char data[64] );

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_SHA1_ALT */

#endif /* POLARSSL_SHA1_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Output = SHA-1( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-1 checksum result
 */
void sha1( const unsigned char *input, size_t ilen, unsigned char output[20] );

/**
 * \brief          Output = SHA-1( file contents )
 *
 * \param path     input file name
 * \param output   SHA-1 checksum result
 *
 * \return         0 if successful, or POLARSSL_ERR_SHA1_FILE_IO_ERROR
 */
int sha1_file( const char *path, unsigned char output[20] );

/**
 * \brief          SHA-1 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void sha1_hmac_starts( sha1_context *ctx, const unsigned char *key,
                       size_t keylen );

/**
 * \brief          SHA-1 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha1_hmac_update( sha1_context *ctx, const unsigned char *input,
                       size_t ilen );

/**
 * \brief          SHA-1 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   SHA-1 HMAC checksum result
 */
void sha1_hmac_finish( sha1_context *ctx, unsigned char output[20] );

/**
 * \brief          SHA-1 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void sha1_hmac_reset( sha1_context *ctx );

/**
 * \brief          Output = HMAC-SHA-1( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SHA-1 result
 */
void sha1_hmac( const unsigned char *key, size_t keylen,
                const unsigned char *input, size_t ilen,
                unsigned char output[20] );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int sha1_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* sha1.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/sha256.h ************/


/**
 * \file sha256.h
 *
 * \brief SHA-224 and SHA-256 cryptographic hash function
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_SHA256_H
#define POLARSSL_SHA256_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

#define POLARSSL_ERR_SHA256_FILE_IO_ERROR              -0x0078  /**< Read/write error in file. */

#if !defined(POLARSSL_SHA256_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          SHA-256 context structure
 */
typedef struct
{
    uint32_t total[2];          /*!< number of bytes processed  */
    uint32_t state[8];          /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */

    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
    int is224;                  /*!< 0 => SHA-256, else SHA-224 */
}
sha256_context;

/**
 * \brief          Initialize SHA-256 context
 *
 * \param ctx      SHA-256 context to be initialized
 */
void sha256_init( sha256_context *ctx );

/**
 * \brief          Clear SHA-256 context
 *
 * \param ctx      SHA-256 context to be cleared
 */
void sha256_free( sha256_context *ctx );

/**
 * \brief          SHA-256 context setup
 *
 * \param ctx      context to be initialized
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void sha256_starts( sha256_context *ctx, int is224 );

/**
 * \brief          SHA-256 process buffer
 *
 * \param ctx      SHA-256 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha256_update( sha256_context *ctx, const unsigned char *input,
                    size_t ilen );

/**
 * \brief          SHA-256 final digest
 *
 * \param ctx      SHA-256 context
 * \param output   SHA-224/256 checksum result
 */
void sha256_finish( sha256_context *ctx, unsigned char output[32] );

/* Internal use */
void sha256_process( sha256_context *ctx, const unsigned char data[64] );

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_SHA256_ALT */

#endif /* POLARSSL_SHA256_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Output = SHA-256( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-224/256 checksum result
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void sha256( const unsigned char *input, size_t ilen,
           unsigned char output[32], int is224 );

/**
 * \brief          Output = SHA-256( file contents )
 *
 * \param path     input file name
 * \param output   SHA-224/256 checksum result
 * \param is224    0 = use SHA256, 1 = use SHA224
 *
 * \return         0 if successful, or POLARSSL_ERR_SHA256_FILE_IO_ERROR
 */
int sha256_file( const char *path, unsigned char output[32], int is224 );

/**
 * \brief          SHA-256 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void sha256_hmac_starts( sha256_context *ctx, const unsigned char *key,
                         size_t keylen, int is224 );

/**
 * \brief          SHA-256 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha256_hmac_update( sha256_context *ctx, const unsigned char *input,
                         size_t ilen );

/**
 * \brief          SHA-256 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   SHA-224/256 HMAC checksum result
 */
void sha256_hmac_finish( sha256_context *ctx, unsigned char output[32] );

/**
 * \brief          SHA-256 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void sha256_hmac_reset( sha256_context *ctx );

/**
 * \brief          Output = HMAC-SHA-256( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SHA-224/256 result
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void sha256_hmac( const unsigned char *key, size_t keylen,
                  const unsigned char *input, size_t ilen,
                  unsigned char output[32], int is224 );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int sha256_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* sha256.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/sha512.h ************/


/**
 * \file sha512.h
 *
 * \brief SHA-384 and SHA-512 cryptographic hash function
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_SHA512_H
#define POLARSSL_SHA512_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(_MSC_VER) || defined(__WATCOMC__)
  typedef unsigned __int64 uint64_t;
#elif !VXWORKS
  #include <inttypes.h>
#endif

#define POLARSSL_ERR_SHA512_FILE_IO_ERROR              -0x007A  /**< Read/write error in file. */

#if !defined(POLARSSL_SHA512_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          SHA-512 context structure
 */
typedef struct
{
    uint64_t total[2];          /*!< number of bytes processed  */
    uint64_t state[8];          /*!< intermediate digest state  */
    unsigned char buffer[128];  /*!< data block being processed */

    unsigned char ipad[128];    /*!< HMAC: inner padding        */
    unsigned char opad[128];    /*!< HMAC: outer padding        */
    int is384;                  /*!< 0 => SHA-512, else SHA-384 */
}
sha512_context;

/**
 * \brief          Initialize SHA-512 context
 *
 * \param ctx      SHA-512 context to be initialized
 */
void sha512_init( sha512_context *ctx );

/**
 * \brief          Clear SHA-512 context
 *
 * \param ctx      SHA-512 context to be cleared
 */
void sha512_free( sha512_context *ctx );

/**
 * \brief          SHA-512 context setup
 *
 * \param ctx      context to be initialized
 * \param is384    0 = use SHA512, 1 = use SHA384
 */
void sha512_starts( sha512_context *ctx, int is384 );

/**
 * \brief          SHA-512 process buffer
 *
 * \param ctx      SHA-512 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha512_update( sha512_context *ctx, const unsigned char *input,
                    size_t ilen );

/**
 * \brief          SHA-512 final digest
 *
 * \param ctx      SHA-512 context
 * \param output   SHA-384/512 checksum result
 */
void sha512_finish( sha512_context *ctx, unsigned char output[64] );

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_SHA512_ALT */

#endif /* POLARSSL_SHA512_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Output = SHA-512( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-384/512 checksum result
 * \param is384    0 = use SHA512, 1 = use SHA384
 */
void sha512( const unsigned char *input, size_t ilen,
             unsigned char output[64], int is384 );

/**
 * \brief          Output = SHA-512( file contents )
 *
 * \param path     input file name
 * \param output   SHA-384/512 checksum result
 * \param is384    0 = use SHA512, 1 = use SHA384
 *
 * \return         0 if successful, or POLARSSL_ERR_SHA512_FILE_IO_ERROR
 */
int sha512_file( const char *path, unsigned char output[64], int is384 );

/**
 * \brief          SHA-512 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param is384    0 = use SHA512, 1 = use SHA384
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void sha512_hmac_starts( sha512_context *ctx, const unsigned char *key,
                         size_t keylen, int is384 );

/**
 * \brief          SHA-512 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha512_hmac_update( sha512_context *ctx, const unsigned char *input,
                         size_t ilen );

/**
 * \brief          SHA-512 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   SHA-384/512 HMAC checksum result
 */
void sha512_hmac_finish( sha512_context *ctx, unsigned char output[64] );

/**
 * \brief          SHA-512 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void sha512_hmac_reset( sha512_context *ctx );

/**
 * \brief          Output = HMAC-SHA-512( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SHA-384/512 result
 * \param is384    0 = use SHA512, 1 = use SHA384
 */
void sha512_hmac( const unsigned char *key, size_t keylen,
                const unsigned char *input, size_t ilen,
                unsigned char output[64], int is384 );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int sha512_self_test( int verbose );

/* Internal use */
void sha512_process( sha512_context *ctx, const unsigned char data[128] );

#ifdef __cplusplus
}
#endif

#endif /* sha512.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/aes.h ************/


/**
 * \file aes.h
 *
 * \brief AES block cipher
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_AES_H
#define POLARSSL_AES_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

/* padlock.c and aesni.c rely on these values! */
#define AES_ENCRYPT     1
#define AES_DECRYPT     0

#define POLARSSL_ERR_AES_INVALID_KEY_LENGTH                -0x0020  /**< Invalid key length. */
#define POLARSSL_ERR_AES_INVALID_INPUT_LENGTH              -0x0022  /**< Invalid data input length. */

#if !defined(POLARSSL_AES_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          AES context structure
 *
 * \note           buf is able to hold 32 extra bytes, which can be used:
 *                 - for alignment purposes if VIA padlock is used, and/or
 *                 - to simplify key expansion in the 256-bit case by
 *                 generating an extra round key
 */
typedef struct
{
    int nr;                     /*!<  number of rounds  */
    uint32_t *rk;               /*!<  AES round keys    */
    uint32_t buf[68];           /*!<  unaligned data    */
}
aes_context;

/**
 * \brief          Initialize AES context
 *
 * \param ctx      AES context to be initialized
 */
void aes_init( aes_context *ctx );

/**
 * \brief          Clear AES context
 *
 * \param ctx      AES context to be cleared
 */
void aes_free( aes_context *ctx );

/**
 * \brief          AES key schedule (encryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      encryption key
 * \param keysize  must be 128, 192 or 256
 *
 * \return         0 if successful, or POLARSSL_ERR_AES_INVALID_KEY_LENGTH
 */
int aes_setkey_enc( aes_context *ctx, const unsigned char *key,
                    unsigned int keysize );

/**
 * \brief          AES key schedule (decryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      decryption key
 * \param keysize  must be 128, 192 or 256
 *
 * \return         0 if successful, or POLARSSL_ERR_AES_INVALID_KEY_LENGTH
 */
int aes_setkey_dec( aes_context *ctx, const unsigned char *key,
                    unsigned int keysize );

/**
 * \brief          AES-ECB block encryption/decryption
 *
 * \param ctx      AES context
 * \param mode     AES_ENCRYPT or AES_DECRYPT
 * \param input    16-byte input block
 * \param output   16-byte output block
 *
 * \return         0 if successful
 */
int aes_crypt_ecb( aes_context *ctx,
                    int mode,
                    const unsigned char input[16],
                    unsigned char output[16] );

#if defined(POLARSSL_CIPHER_MODE_CBC)
/**
 * \brief          AES-CBC buffer encryption/decryption
 *                 Length should be a multiple of the block
 *                 size (16 bytes)
 *
 * \note           Upon exit, the content of the IV is updated so that you can
 *                 call the function same function again on the following
 *                 block(s) of data and get the same result as if it was
 *                 encrypted in one call. This allows a "streaming" usage.
 *                 If on the other hand you need to retain the contents of the
 *                 IV, you should either save it manually or use the cipher
 *                 module instead.
 *
 * \param ctx      AES context
 * \param mode     AES_ENCRYPT or AES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if successful, or POLARSSL_ERR_AES_INVALID_INPUT_LENGTH
 */
int aes_crypt_cbc( aes_context *ctx,
                    int mode,
                    size_t length,
                    unsigned char iv[16],
                    const unsigned char *input,
                    unsigned char *output );
#endif /* POLARSSL_CIPHER_MODE_CBC */

#if defined(POLARSSL_CIPHER_MODE_CFB)
/**
 * \brief          AES-CFB128 buffer encryption/decryption.
 *
 * Note: Due to the nature of CFB you should use the same key schedule for
 * both encryption and decryption. So a context initialized with
 * aes_setkey_enc() for both AES_ENCRYPT and AES_DECRYPT.
 *
 * \note           Upon exit, the content of the IV is updated so that you can
 *                 call the function same function again on the following
 *                 block(s) of data and get the same result as if it was
 *                 encrypted in one call. This allows a "streaming" usage.
 *                 If on the other hand you need to retain the contents of the
 *                 IV, you should either save it manually or use the cipher
 *                 module instead.
 *
 * \param ctx      AES context
 * \param mode     AES_ENCRYPT or AES_DECRYPT
 * \param length   length of the input data
 * \param iv_off   offset in IV (updated after use)
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if successful
 */
int aes_crypt_cfb128( aes_context *ctx,
                       int mode,
                       size_t length,
                       size_t *iv_off,
                       unsigned char iv[16],
                       const unsigned char *input,
                       unsigned char *output );

/**
 * \brief          AES-CFB8 buffer encryption/decryption.
 *
 * Note: Due to the nature of CFB you should use the same key schedule for
 * both encryption and decryption. So a context initialized with
 * aes_setkey_enc() for both AES_ENCRYPT and AES_DECRYPT.
 *
 * \note           Upon exit, the content of the IV is updated so that you can
 *                 call the function same function again on the following
 *                 block(s) of data and get the same result as if it was
 *                 encrypted in one call. This allows a "streaming" usage.
 *                 If on the other hand you need to retain the contents of the
 *                 IV, you should either save it manually or use the cipher
 *                 module instead.
 *
 * \param ctx      AES context
 * \param mode     AES_ENCRYPT or AES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if successful
 */
int aes_crypt_cfb8( aes_context *ctx,
                    int mode,
                    size_t length,
                    unsigned char iv[16],
                    const unsigned char *input,
                    unsigned char *output );
#endif /*POLARSSL_CIPHER_MODE_CFB */

#if defined(POLARSSL_CIPHER_MODE_CTR)
/**
 * \brief               AES-CTR buffer encryption/decryption
 *
 * Warning: You have to keep the maximum use of your counter in mind!
 *
 * Note: Due to the nature of CTR you should use the same key schedule for
 * both encryption and decryption. So a context initialized with
 * aes_setkey_enc() for both AES_ENCRYPT and AES_DECRYPT.
 *
 * \param ctx           AES context
 * \param length        The length of the data
 * \param nc_off        The offset in the current stream_block (for resuming
 *                      within current cipher stream). The offset pointer to
 *                      should be 0 at the start of a stream.
 * \param nonce_counter The 128-bit nonce and counter.
 * \param stream_block  The saved stream-block for resuming. Is overwritten
 *                      by the function.
 * \param input         The input data stream
 * \param output        The output data stream
 *
 * \return         0 if successful
 */
int aes_crypt_ctr( aes_context *ctx,
                       size_t length,
                       size_t *nc_off,
                       unsigned char nonce_counter[16],
                       unsigned char stream_block[16],
                       const unsigned char *input,
                       unsigned char *output );
#endif /* POLARSSL_CIPHER_MODE_CTR */

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_AES_ALT */

#endif /* POLARSSL_AES_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int aes_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* aes.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/arc4.h ************/


/**
 * \file arc4.h
 *
 * \brief The ARCFOUR stream cipher
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_ARC4_H
#define POLARSSL_ARC4_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if !defined(POLARSSL_ARC4_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          ARC4 context structure
 */
typedef struct
{
    int x;                      /*!< permutation index */
    int y;                      /*!< permutation index */
    unsigned char m[256];       /*!< permutation table */
}
arc4_context;

/**
 * \brief          Initialize ARC4 context
 *
 * \param ctx      ARC4 context to be initialized
 */
void arc4_init( arc4_context *ctx );

/**
 * \brief          Clear ARC4 context
 *
 * \param ctx      ARC4 context to be cleared
 */
void arc4_free( arc4_context *ctx );

/**
 * \brief          ARC4 key schedule
 *
 * \param ctx      ARC4 context to be setup
 * \param key      the secret key
 * \param keylen   length of the key, in bytes
 */
void arc4_setup( arc4_context *ctx, const unsigned char *key,
                 unsigned int keylen );

/**
 * \brief          ARC4 cipher function
 *
 * \param ctx      ARC4 context
 * \param length   length of the input data
 * \param input    buffer holding the input data
 * \param output   buffer for the output data
 *
 * \return         0 if successful
 */
int arc4_crypt( arc4_context *ctx, size_t length, const unsigned char *input,
                unsigned char *output );

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_ARC4_ALT */

#endif /* POLARSSL_ARC4_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int arc4_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* arc4.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/base64.h ************/


/**
 * \file base64.h
 *
 * \brief RFC 1521 base64 encoding/decoding
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_BASE64_H
#define POLARSSL_BASE64_H

#include <stddef.h>

#define POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL               -0x002A  /**< Output buffer too small. */
#define POLARSSL_ERR_BASE64_INVALID_CHARACTER              -0x002C  /**< Invalid character in input. */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Encode a buffer into base64 format
 *
 * \param dst      destination buffer
 * \param dlen     size of the buffer
 * \param src      source buffer
 * \param slen     amount of data to be encoded
 *
 * \return         0 if successful, or POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL.
 *                 *dlen is always updated to reflect the amount
 *                 of data that has (or would have) been written.
 *
 * \note           Call this function with *dlen = 0 to obtain the
 *                 required buffer size in *dlen
 */
int base64_encode( unsigned char *dst, size_t *dlen,
                   const unsigned char *src, size_t slen );

/**
 * \brief          Decode a base64-formatted buffer
 *
 * \param dst      destination buffer (can be NULL for checking size)
 * \param dlen     size of the buffer
 * \param src      source buffer
 * \param slen     amount of data to be decoded
 *
 * \return         0 if successful, POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL, or
 *                 POLARSSL_ERR_BASE64_INVALID_CHARACTER if the input data is
 *                 not correct. *dlen is always updated to reflect the amount
 *                 of data that has (or would have) been written.
 *
 * \note           Call this function with *dst = NULL or *dlen = 0 to obtain
 *                 the required buffer size in *dlen
 */
int base64_decode( unsigned char *dst, size_t *dlen,
                   const unsigned char *src, size_t slen );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int base64_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* base64.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/bn_mul.h ************/


/**
 * \file bn_mul.h
 *
 * \brief  Multi-precision integer library
 *
 *  Copyright (C) 2006-2010, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 *      Multiply source vector [s] with b, add result
 *       to destination vector [d] and set carry c.
 *
 *      Currently supports:
 *
 *         . IA-32 (386+)         . AMD64 / EM64T
 *         . IA-32 (SSE2)         . Motorola 68000
 *         . PowerPC, 32-bit      . MicroBlaze
 *         . PowerPC, 64-bit      . TriCore
 *         . SPARC v8             . ARM v3+
 *         . Alpha                . MIPS32
 *         . C, longlong          . C, generic
 */
#ifndef POLARSSL_BN_MUL_H
#define POLARSSL_BN_MUL_H



#if defined(POLARSSL_HAVE_ASM)

#if defined(__GNUC__)
#if defined(__i386__)

#define MULADDC_INIT                        \
    asm(                                    \
        "movl   %%ebx, %0           \n\t"   \
        "movl   %5, %%esi           \n\t"   \
        "movl   %6, %%edi           \n\t"   \
        "movl   %7, %%ecx           \n\t"   \
        "movl   %8, %%ebx           \n\t"

#define MULADDC_CORE                        \
        "lodsl                      \n\t"   \
        "mull   %%ebx               \n\t"   \
        "addl   %%ecx,   %%eax      \n\t"   \
        "adcl   $0,      %%edx      \n\t"   \
        "addl   (%%edi), %%eax      \n\t"   \
        "adcl   $0,      %%edx      \n\t"   \
        "movl   %%edx,   %%ecx      \n\t"   \
        "stosl                      \n\t"

#if defined(POLARSSL_HAVE_SSE2)

#define MULADDC_HUIT                            \
        "movd     %%ecx,     %%mm1      \n\t"   \
        "movd     %%ebx,     %%mm0      \n\t"   \
        "movd     (%%edi),   %%mm3      \n\t"   \
        "paddq    %%mm3,     %%mm1      \n\t"   \
        "movd     (%%esi),   %%mm2      \n\t"   \
        "pmuludq  %%mm0,     %%mm2      \n\t"   \
        "movd     4(%%esi),  %%mm4      \n\t"   \
        "pmuludq  %%mm0,     %%mm4      \n\t"   \
        "movd     8(%%esi),  %%mm6      \n\t"   \
        "pmuludq  %%mm0,     %%mm6      \n\t"   \
        "movd     12(%%esi), %%mm7      \n\t"   \
        "pmuludq  %%mm0,     %%mm7      \n\t"   \
        "paddq    %%mm2,     %%mm1      \n\t"   \
        "movd     4(%%edi),  %%mm3      \n\t"   \
        "paddq    %%mm4,     %%mm3      \n\t"   \
        "movd     8(%%edi),  %%mm5      \n\t"   \
        "paddq    %%mm6,     %%mm5      \n\t"   \
        "movd     12(%%edi), %%mm4      \n\t"   \
        "paddq    %%mm4,     %%mm7      \n\t"   \
        "movd     %%mm1,     (%%edi)    \n\t"   \
        "movd     16(%%esi), %%mm2      \n\t"   \
        "pmuludq  %%mm0,     %%mm2      \n\t"   \
        "psrlq    $32,       %%mm1      \n\t"   \
        "movd     20(%%esi), %%mm4      \n\t"   \
        "pmuludq  %%mm0,     %%mm4      \n\t"   \
        "paddq    %%mm3,     %%mm1      \n\t"   \
        "movd     24(%%esi), %%mm6      \n\t"   \
        "pmuludq  %%mm0,     %%mm6      \n\t"   \
        "movd     %%mm1,     4(%%edi)   \n\t"   \
        "psrlq    $32,       %%mm1      \n\t"   \
        "movd     28(%%esi), %%mm3      \n\t"   \
        "pmuludq  %%mm0,     %%mm3      \n\t"   \
        "paddq    %%mm5,     %%mm1      \n\t"   \
        "movd     16(%%edi), %%mm5      \n\t"   \
        "paddq    %%mm5,     %%mm2      \n\t"   \
        "movd     %%mm1,     8(%%edi)   \n\t"   \
        "psrlq    $32,       %%mm1      \n\t"   \
        "paddq    %%mm7,     %%mm1      \n\t"   \
        "movd     20(%%edi), %%mm5      \n\t"   \
        "paddq    %%mm5,     %%mm4      \n\t"   \
        "movd     %%mm1,     12(%%edi)  \n\t"   \
        "psrlq    $32,       %%mm1      \n\t"   \
        "paddq    %%mm2,     %%mm1      \n\t"   \
        "movd     24(%%edi), %%mm5      \n\t"   \
        "paddq    %%mm5,     %%mm6      \n\t"   \
        "movd     %%mm1,     16(%%edi)  \n\t"   \
        "psrlq    $32,       %%mm1      \n\t"   \
        "paddq    %%mm4,     %%mm1      \n\t"   \
        "movd     28(%%edi), %%mm5      \n\t"   \
        "paddq    %%mm5,     %%mm3      \n\t"   \
        "movd     %%mm1,     20(%%edi)  \n\t"   \
        "psrlq    $32,       %%mm1      \n\t"   \
        "paddq    %%mm6,     %%mm1      \n\t"   \
        "movd     %%mm1,     24(%%edi)  \n\t"   \
        "psrlq    $32,       %%mm1      \n\t"   \
        "paddq    %%mm3,     %%mm1      \n\t"   \
        "movd     %%mm1,     28(%%edi)  \n\t"   \
        "addl     $32,       %%edi      \n\t"   \
        "addl     $32,       %%esi      \n\t"   \
        "psrlq    $32,       %%mm1      \n\t"   \
        "movd     %%mm1,     %%ecx      \n\t"

#define MULADDC_STOP                    \
        "emms                   \n\t"   \
        "movl   %4, %%ebx       \n\t"   \
        "movl   %%ecx, %1       \n\t"   \
        "movl   %%edi, %2       \n\t"   \
        "movl   %%esi, %3       \n\t"   \
        : "=m" (t), "=m" (c), "=m" (d), "=m" (s)        \
        : "m" (t), "m" (s), "m" (d), "m" (c), "m" (b)   \
        : "eax", "ecx", "edx", "esi", "edi"             \
    );

#else

#define MULADDC_STOP                    \
        "movl   %4, %%ebx       \n\t"   \
        "movl   %%ecx, %1       \n\t"   \
        "movl   %%edi, %2       \n\t"   \
        "movl   %%esi, %3       \n\t"   \
        : "=m" (t), "=m" (c), "=m" (d), "=m" (s)        \
        : "m" (t), "m" (s), "m" (d), "m" (c), "m" (b)   \
        : "eax", "ecx", "edx", "esi", "edi"             \
    );
#endif /* SSE2 */
#endif /* i386 */

#if defined(__amd64__) || defined (__x86_64__)

#define MULADDC_INIT                        \
    asm(                                    \
        "movq   %3, %%rsi           \n\t"   \
        "movq   %4, %%rdi           \n\t"   \
        "movq   %5, %%rcx           \n\t"   \
        "movq   %6, %%rbx           \n\t"   \
        "xorq   %%r8, %%r8          \n\t"

#define MULADDC_CORE                        \
        "movq   (%%rsi), %%rax      \n\t"   \
        "mulq   %%rbx               \n\t"   \
        "addq   $8,      %%rsi      \n\t"   \
        "addq   %%rcx,   %%rax      \n\t"   \
        "movq   %%r8,    %%rcx      \n\t"   \
        "adcq   $0,      %%rdx      \n\t"   \
        "nop                        \n\t"   \
        "addq   %%rax,   (%%rdi)    \n\t"   \
        "adcq   %%rdx,   %%rcx      \n\t"   \
        "addq   $8,      %%rdi      \n\t"

#define MULADDC_STOP                        \
        "movq   %%rcx, %0           \n\t"   \
        "movq   %%rdi, %1           \n\t"   \
        "movq   %%rsi, %2           \n\t"   \
        : "=m" (c), "=m" (d), "=m" (s)                      \
        : "m" (s), "m" (d), "m" (c), "m" (b)                \
        : "rax", "rcx", "rdx", "rbx", "rsi", "rdi", "r8"    \
    );

#endif /* AMD64 */

#if defined(__mc68020__) || defined(__mcpu32__)

#define MULADDC_INIT                    \
    asm(                                \
        "movl   %3, %%a2        \n\t"   \
        "movl   %4, %%a3        \n\t"   \
        "movl   %5, %%d3        \n\t"   \
        "movl   %6, %%d2        \n\t"   \
        "moveq  #0, %%d0        \n\t"

#define MULADDC_CORE                    \
        "movel  %%a2@+, %%d1    \n\t"   \
        "mulul  %%d2, %%d4:%%d1 \n\t"   \
        "addl   %%d3, %%d1      \n\t"   \
        "addxl  %%d0, %%d4      \n\t"   \
        "moveq  #0,   %%d3      \n\t"   \
        "addl   %%d1, %%a3@+    \n\t"   \
        "addxl  %%d4, %%d3      \n\t"

#define MULADDC_STOP                    \
        "movl   %%d3, %0        \n\t"   \
        "movl   %%a3, %1        \n\t"   \
        "movl   %%a2, %2        \n\t"   \
        : "=m" (c), "=m" (d), "=m" (s)              \
        : "m" (s), "m" (d), "m" (c), "m" (b)        \
        : "d0", "d1", "d2", "d3", "d4", "a2", "a3"  \
    );

#define MULADDC_HUIT                        \
        "movel  %%a2@+,  %%d1       \n\t"   \
        "mulul  %%d2,    %%d4:%%d1  \n\t"   \
        "addxl  %%d3,    %%d1       \n\t"   \
        "addxl  %%d0,    %%d4       \n\t"   \
        "addl   %%d1,    %%a3@+     \n\t"   \
        "movel  %%a2@+,  %%d1       \n\t"   \
        "mulul  %%d2,    %%d3:%%d1  \n\t"   \
        "addxl  %%d4,    %%d1       \n\t"   \
        "addxl  %%d0,    %%d3       \n\t"   \
        "addl   %%d1,    %%a3@+     \n\t"   \
        "movel  %%a2@+,  %%d1       \n\t"   \
        "mulul  %%d2,    %%d4:%%d1  \n\t"   \
        "addxl  %%d3,    %%d1       \n\t"   \
        "addxl  %%d0,    %%d4       \n\t"   \
        "addl   %%d1,    %%a3@+     \n\t"   \
        "movel  %%a2@+,  %%d1       \n\t"   \
        "mulul  %%d2,    %%d3:%%d1  \n\t"   \
        "addxl  %%d4,    %%d1       \n\t"   \
        "addxl  %%d0,    %%d3       \n\t"   \
        "addl   %%d1,    %%a3@+     \n\t"   \
        "movel  %%a2@+,  %%d1       \n\t"   \
        "mulul  %%d2,    %%d4:%%d1  \n\t"   \
        "addxl  %%d3,    %%d1       \n\t"   \
        "addxl  %%d0,    %%d4       \n\t"   \
        "addl   %%d1,    %%a3@+     \n\t"   \
        "movel  %%a2@+,  %%d1       \n\t"   \
        "mulul  %%d2,    %%d3:%%d1  \n\t"   \
        "addxl  %%d4,    %%d1       \n\t"   \
        "addxl  %%d0,    %%d3       \n\t"   \
        "addl   %%d1,    %%a3@+     \n\t"   \
        "movel  %%a2@+,  %%d1       \n\t"   \
        "mulul  %%d2,    %%d4:%%d1  \n\t"   \
        "addxl  %%d3,    %%d1       \n\t"   \
        "addxl  %%d0,    %%d4       \n\t"   \
        "addl   %%d1,    %%a3@+     \n\t"   \
        "movel  %%a2@+,  %%d1       \n\t"   \
        "mulul  %%d2,    %%d3:%%d1  \n\t"   \
        "addxl  %%d4,    %%d1       \n\t"   \
        "addxl  %%d0,    %%d3       \n\t"   \
        "addl   %%d1,    %%a3@+     \n\t"   \
        "addxl  %%d0,    %%d3       \n\t"

#endif /* MC68000 */

#if defined(__powerpc64__) || defined(__ppc64__)

#if defined(__MACH__) && defined(__APPLE__)

#define MULADDC_INIT                        \
    asm(                                    \
        "ld     r3, %3              \n\t"   \
        "ld     r4, %4              \n\t"   \
        "ld     r5, %5              \n\t"   \
        "ld     r6, %6              \n\t"   \
        "addi   r3, r3, -8          \n\t"   \
        "addi   r4, r4, -8          \n\t"   \
        "addic  r5, r5,  0          \n\t"

#define MULADDC_CORE                        \
        "ldu    r7, 8(r3)           \n\t"   \
        "mulld  r8, r7, r6          \n\t"   \
        "mulhdu r9, r7, r6          \n\t"   \
        "adde   r8, r8, r5          \n\t"   \
        "ld     r7, 8(r4)           \n\t"   \
        "addze  r5, r9              \n\t"   \
        "addc   r8, r8, r7          \n\t"   \
        "stdu   r8, 8(r4)           \n\t"

#define MULADDC_STOP                        \
        "addze  r5, r5              \n\t"   \
        "addi   r4, r4, 8           \n\t"   \
        "addi   r3, r3, 8           \n\t"   \
        "std    r5, %0              \n\t"   \
        "std    r4, %1              \n\t"   \
        "std    r3, %2              \n\t"   \
        : "=m" (c), "=m" (d), "=m" (s)              \
        : "m" (s), "m" (d), "m" (c), "m" (b)        \
        : "r3", "r4", "r5", "r6", "r7", "r8", "r9"  \
    );


#else /* __MACH__ && __APPLE__ */

#define MULADDC_INIT                        \
    asm(                                    \
        "ld     %%r3, %3            \n\t"   \
        "ld     %%r4, %4            \n\t"   \
        "ld     %%r5, %5            \n\t"   \
        "ld     %%r6, %6            \n\t"   \
        "addi   %%r3, %%r3, -8      \n\t"   \
        "addi   %%r4, %%r4, -8      \n\t"   \
        "addic  %%r5, %%r5,  0      \n\t"

#define MULADDC_CORE                        \
        "ldu    %%r7, 8(%%r3)       \n\t"   \
        "mulld  %%r8, %%r7, %%r6    \n\t"   \
        "mulhdu %%r9, %%r7, %%r6    \n\t"   \
        "adde   %%r8, %%r8, %%r5    \n\t"   \
        "ld     %%r7, 8(%%r4)       \n\t"   \
        "addze  %%r5, %%r9          \n\t"   \
        "addc   %%r8, %%r8, %%r7    \n\t"   \
        "stdu   %%r8, 8(%%r4)       \n\t"

#define MULADDC_STOP                        \
        "addze  %%r5, %%r5          \n\t"   \
        "addi   %%r4, %%r4, 8       \n\t"   \
        "addi   %%r3, %%r3, 8       \n\t"   \
        "std    %%r5, %0            \n\t"   \
        "std    %%r4, %1            \n\t"   \
        "std    %%r3, %2            \n\t"   \
        : "=m" (c), "=m" (d), "=m" (s)              \
        : "m" (s), "m" (d), "m" (c), "m" (b)        \
        : "r3", "r4", "r5", "r6", "r7", "r8", "r9"  \
    );

#endif /* __MACH__ && __APPLE__ */

#elif defined(__powerpc__) || defined(__ppc__) /* end PPC64/begin PPC32  */

#if defined(__MACH__) && defined(__APPLE__)

#define MULADDC_INIT                    \
    asm(                                \
        "lwz    r3, %3          \n\t"   \
        "lwz    r4, %4          \n\t"   \
        "lwz    r5, %5          \n\t"   \
        "lwz    r6, %6          \n\t"   \
        "addi   r3, r3, -4      \n\t"   \
        "addi   r4, r4, -4      \n\t"   \
        "addic  r5, r5,  0      \n\t"

#define MULADDC_CORE                    \
        "lwzu   r7, 4(r3)       \n\t"   \
        "mullw  r8, r7, r6      \n\t"   \
        "mulhwu r9, r7, r6      \n\t"   \
        "adde   r8, r8, r5      \n\t"   \
        "lwz    r7, 4(r4)       \n\t"   \
        "addze  r5, r9          \n\t"   \
        "addc   r8, r8, r7      \n\t"   \
        "stwu   r8, 4(r4)       \n\t"

#define MULADDC_STOP                    \
        "addze  r5, r5          \n\t"   \
        "addi   r4, r4, 4       \n\t"   \
        "addi   r3, r3, 4       \n\t"   \
        "stw    r5, %0          \n\t"   \
        "stw    r4, %1          \n\t"   \
        "stw    r3, %2          \n\t"   \
        : "=m" (c), "=m" (d), "=m" (s)              \
        : "m" (s), "m" (d), "m" (c), "m" (b)        \
        : "r3", "r4", "r5", "r6", "r7", "r8", "r9"  \
    );

#else /* __MACH__ && __APPLE__ */

#define MULADDC_INIT                        \
    asm(                                    \
        "lwz    %%r3, %3            \n\t"   \
        "lwz    %%r4, %4            \n\t"   \
        "lwz    %%r5, %5            \n\t"   \
        "lwz    %%r6, %6            \n\t"   \
        "addi   %%r3, %%r3, -4      \n\t"   \
        "addi   %%r4, %%r4, -4      \n\t"   \
        "addic  %%r5, %%r5,  0      \n\t"

#define MULADDC_CORE                        \
        "lwzu   %%r7, 4(%%r3)       \n\t"   \
        "mullw  %%r8, %%r7, %%r6    \n\t"   \
        "mulhwu %%r9, %%r7, %%r6    \n\t"   \
        "adde   %%r8, %%r8, %%r5    \n\t"   \
        "lwz    %%r7, 4(%%r4)       \n\t"   \
        "addze  %%r5, %%r9          \n\t"   \
        "addc   %%r8, %%r8, %%r7    \n\t"   \
        "stwu   %%r8, 4(%%r4)       \n\t"

#define MULADDC_STOP                        \
        "addze  %%r5, %%r5          \n\t"   \
        "addi   %%r4, %%r4, 4       \n\t"   \
        "addi   %%r3, %%r3, 4       \n\t"   \
        "stw    %%r5, %0            \n\t"   \
        "stw    %%r4, %1            \n\t"   \
        "stw    %%r3, %2            \n\t"   \
        : "=m" (c), "=m" (d), "=m" (s)              \
        : "m" (s), "m" (d), "m" (c), "m" (b)        \
        : "r3", "r4", "r5", "r6", "r7", "r8", "r9"  \
    );

#endif /* __MACH__ && __APPLE__ */

#endif /* PPC32 */

/*
 * The Sparc64 assembly is reported to be broken.
 * Disable it for now, until we're able to fix it.
 */
#if 0 && defined(__sparc__) && defined(__sparc64__)

#define MULADDC_INIT                                    \
    asm(                                                \
                "ldx     %3, %%o0               \n\t"   \
                "ldx     %4, %%o1               \n\t"   \
                "ld      %5, %%o2               \n\t"   \
                "ld      %6, %%o3               \n\t"

#define MULADDC_CORE                                    \
                "ld      [%%o0], %%o4           \n\t"   \
                "inc     4, %%o0                \n\t"   \
                "ld      [%%o1], %%o5           \n\t"   \
                "umul    %%o3, %%o4, %%o4       \n\t"   \
                "addcc   %%o4, %%o2, %%o4       \n\t"   \
                "rd      %%y, %%g1              \n\t"   \
                "addx    %%g1, 0, %%g1          \n\t"   \
                "addcc   %%o4, %%o5, %%o4       \n\t"   \
                "st      %%o4, [%%o1]           \n\t"   \
                "addx    %%g1, 0, %%o2          \n\t"   \
                "inc     4, %%o1                \n\t"

        #define MULADDC_STOP                            \
                "st      %%o2, %0               \n\t"   \
                "stx     %%o1, %1               \n\t"   \
                "stx     %%o0, %2               \n\t"   \
        : "=m" (c), "=m" (d), "=m" (s)          \
        : "m" (s), "m" (d), "m" (c), "m" (b)    \
        : "g1", "o0", "o1", "o2", "o3", "o4",   \
          "o5"                                  \
        );
#endif /* SPARCv9 */

#if defined(__sparc__) && !defined(__sparc64__)

#define MULADDC_INIT                                    \
    asm(                                                \
                "ld      %3, %%o0               \n\t"   \
                "ld      %4, %%o1               \n\t"   \
                "ld      %5, %%o2               \n\t"   \
                "ld      %6, %%o3               \n\t"

#define MULADDC_CORE                                    \
                "ld      [%%o0], %%o4           \n\t"   \
                "inc     4, %%o0                \n\t"   \
                "ld      [%%o1], %%o5           \n\t"   \
                "umul    %%o3, %%o4, %%o4       \n\t"   \
                "addcc   %%o4, %%o2, %%o4       \n\t"   \
                "rd      %%y, %%g1              \n\t"   \
                "addx    %%g1, 0, %%g1          \n\t"   \
                "addcc   %%o4, %%o5, %%o4       \n\t"   \
                "st      %%o4, [%%o1]           \n\t"   \
                "addx    %%g1, 0, %%o2          \n\t"   \
                "inc     4, %%o1                \n\t"

#define MULADDC_STOP                                    \
                "st      %%o2, %0               \n\t"   \
                "st      %%o1, %1               \n\t"   \
                "st      %%o0, %2               \n\t"   \
        : "=m" (c), "=m" (d), "=m" (s)          \
        : "m" (s), "m" (d), "m" (c), "m" (b)    \
        : "g1", "o0", "o1", "o2", "o3", "o4",   \
          "o5"                                  \
        );

#endif /* SPARCv8 */

#if defined(__microblaze__) || defined(microblaze)

#define MULADDC_INIT                    \
    asm(                                \
        "lwi   r3,   %3         \n\t"   \
        "lwi   r4,   %4         \n\t"   \
        "lwi   r5,   %5         \n\t"   \
        "lwi   r6,   %6         \n\t"   \
        "andi  r7,   r6, 0xffff \n\t"   \
        "bsrli r6,   r6, 16     \n\t"

#define MULADDC_CORE                    \
        "lhui  r8,   r3,   0    \n\t"   \
        "addi  r3,   r3,   2    \n\t"   \
        "lhui  r9,   r3,   0    \n\t"   \
        "addi  r3,   r3,   2    \n\t"   \
        "mul   r10,  r9,  r6    \n\t"   \
        "mul   r11,  r8,  r7    \n\t"   \
        "mul   r12,  r9,  r7    \n\t"   \
        "mul   r13,  r8,  r6    \n\t"   \
        "bsrli  r8, r10,  16    \n\t"   \
        "bsrli  r9, r11,  16    \n\t"   \
        "add   r13, r13,  r8    \n\t"   \
        "add   r13, r13,  r9    \n\t"   \
        "bslli r10, r10,  16    \n\t"   \
        "bslli r11, r11,  16    \n\t"   \
        "add   r12, r12, r10    \n\t"   \
        "addc  r13, r13,  r0    \n\t"   \
        "add   r12, r12, r11    \n\t"   \
        "addc  r13, r13,  r0    \n\t"   \
        "lwi   r10,  r4,   0    \n\t"   \
        "add   r12, r12, r10    \n\t"   \
        "addc  r13, r13,  r0    \n\t"   \
        "add   r12, r12,  r5    \n\t"   \
        "addc   r5, r13,  r0    \n\t"   \
        "swi   r12,  r4,   0    \n\t"   \
        "addi   r4,  r4,   4    \n\t"

#define MULADDC_STOP                    \
        "swi   r5,   %0         \n\t"   \
        "swi   r4,   %1         \n\t"   \
        "swi   r3,   %2         \n\t"   \
        : "=m" (c), "=m" (d), "=m" (s)              \
        : "m" (s), "m" (d), "m" (c), "m" (b)        \
        : "r3", "r4"  "r5", "r6", "r7", "r8",       \
          "r9", "r10", "r11", "r12", "r13"          \
    );

#endif /* MicroBlaze */

#if defined(__tricore__)

#define MULADDC_INIT                            \
    asm(                                        \
        "ld.a   %%a2, %3                \n\t"   \
        "ld.a   %%a3, %4                \n\t"   \
        "ld.w   %%d4, %5                \n\t"   \
        "ld.w   %%d1, %6                \n\t"   \
        "xor    %%d5, %%d5              \n\t"

#define MULADDC_CORE                            \
        "ld.w   %%d0,   [%%a2+]         \n\t"   \
        "madd.u %%e2, %%e4, %%d0, %%d1  \n\t"   \
        "ld.w   %%d0,   [%%a3]          \n\t"   \
        "addx   %%d2,    %%d2,  %%d0    \n\t"   \
        "addc   %%d3,    %%d3,    0     \n\t"   \
        "mov    %%d4,    %%d3           \n\t"   \
        "st.w  [%%a3+],  %%d2           \n\t"

#define MULADDC_STOP                            \
        "st.w   %0, %%d4                \n\t"   \
        "st.a   %1, %%a3                \n\t"   \
        "st.a   %2, %%a2                \n\t"   \
        : "=m" (c), "=m" (d), "=m" (s)          \
        : "m" (s), "m" (d), "m" (c), "m" (b)    \
        : "d0", "d1", "e2", "d4", "a2", "a3"    \
    );

#endif /* TriCore */

#if defined(__arm__)

#if defined(__thumb__) && !defined(__thumb2__)

#define MULADDC_INIT                                    \
    asm(                                                \
            "ldr    r0, %3                      \n\t"   \
            "ldr    r1, %4                      \n\t"   \
            "ldr    r2, %5                      \n\t"   \
            "ldr    r3, %6                      \n\t"   \
            "lsr    r7, r3, #16                 \n\t"   \
            "mov    r9, r7                      \n\t"   \
            "lsl    r7, r3, #16                 \n\t"   \
            "lsr    r7, r7, #16                 \n\t"   \
            "mov    r8, r7                      \n\t"

#define MULADDC_CORE                                    \
            "ldmia  r0!, {r6}                   \n\t"   \
            "lsr    r7, r6, #16                 \n\t"   \
            "lsl    r6, r6, #16                 \n\t"   \
            "lsr    r6, r6, #16                 \n\t"   \
            "mov    r4, r8                      \n\t"   \
            "mul    r4, r6                      \n\t"   \
            "mov    r3, r9                      \n\t"   \
            "mul    r6, r3                      \n\t"   \
            "mov    r5, r9                      \n\t"   \
            "mul    r5, r7                      \n\t"   \
            "mov    r3, r8                      \n\t"   \
            "mul    r7, r3                      \n\t"   \
            "lsr    r3, r6, #16                 \n\t"   \
            "add    r5, r5, r3                  \n\t"   \
            "lsr    r3, r7, #16                 \n\t"   \
            "add    r5, r5, r3                  \n\t"   \
            "add    r4, r4, r2                  \n\t"   \
            "mov    r2, #0                      \n\t"   \
            "adc    r5, r2                      \n\t"   \
            "lsl    r3, r6, #16                 \n\t"   \
            "add    r4, r4, r3                  \n\t"   \
            "adc    r5, r2                      \n\t"   \
            "lsl    r3, r7, #16                 \n\t"   \
            "add    r4, r4, r3                  \n\t"   \
            "adc    r5, r2                      \n\t"   \
            "ldr    r3, [r1]                    \n\t"   \
            "add    r4, r4, r3                  \n\t"   \
            "adc    r2, r5                      \n\t"   \
            "stmia  r1!, {r4}                   \n\t"

#define MULADDC_STOP                                    \
            "str    r2, %0                      \n\t"   \
            "str    r1, %1                      \n\t"   \
            "str    r0, %2                      \n\t"   \
         : "=m" (c),  "=m" (d), "=m" (s)        \
         : "m" (s), "m" (d), "m" (c), "m" (b)   \
         : "r0", "r1", "r2", "r3", "r4", "r5",  \
           "r6", "r7", "r8", "r9", "cc"         \
         );

#else

#define MULADDC_INIT                                    \
    asm(                                                \
            "ldr    r0, %3                      \n\t"   \
            "ldr    r1, %4                      \n\t"   \
            "ldr    r2, %5                      \n\t"   \
            "ldr    r3, %6                      \n\t"

#define MULADDC_CORE                                    \
            "ldr    r4, [r0], #4                \n\t"   \
            "mov    r5, #0                      \n\t"   \
            "ldr    r6, [r1]                    \n\t"   \
            "umlal  r2, r5, r3, r4              \n\t"   \
            "adds   r7, r6, r2                  \n\t"   \
            "adc    r2, r5, #0                  \n\t"   \
            "str    r7, [r1], #4                \n\t"

#define MULADDC_STOP                                    \
            "str    r2, %0                      \n\t"   \
            "str    r1, %1                      \n\t"   \
            "str    r0, %2                      \n\t"   \
         : "=m" (c),  "=m" (d), "=m" (s)        \
         : "m" (s), "m" (d), "m" (c), "m" (b)   \
         : "r0", "r1", "r2", "r3", "r4", "r5",  \
           "r6", "r7", "cc"                     \
         );

#endif /* Thumb */

#endif /* ARMv3 */

#if defined(__alpha__)

#define MULADDC_INIT                    \
    asm(                                \
        "ldq    $1, %3          \n\t"   \
        "ldq    $2, %4          \n\t"   \
        "ldq    $3, %5          \n\t"   \
        "ldq    $4, %6          \n\t"

#define MULADDC_CORE                    \
        "ldq    $6,  0($1)      \n\t"   \
        "addq   $1,  8, $1      \n\t"   \
        "mulq   $6, $4, $7      \n\t"   \
        "umulh  $6, $4, $6      \n\t"   \
        "addq   $7, $3, $7      \n\t"   \
        "cmpult $7, $3, $3      \n\t"   \
        "ldq    $5,  0($2)      \n\t"   \
        "addq   $7, $5, $7      \n\t"   \
        "cmpult $7, $5, $5      \n\t"   \
        "stq    $7,  0($2)      \n\t"   \
        "addq   $2,  8, $2      \n\t"   \
        "addq   $6, $3, $3      \n\t"   \
        "addq   $5, $3, $3      \n\t"

#define MULADDC_STOP                                    \
        "stq    $3, %0          \n\t"   \
        "stq    $2, %1          \n\t"   \
        "stq    $1, %2          \n\t"   \
        : "=m" (c), "=m" (d), "=m" (s)              \
        : "m" (s), "m" (d), "m" (c), "m" (b)        \
        : "$1", "$2", "$3", "$4", "$5", "$6", "$7"  \
    );
#endif /* Alpha */

#if defined(__mips__) && !defined(__mips64)

#define MULADDC_INIT                    \
    asm(                                \
        "lw     $10, %3         \n\t"   \
        "lw     $11, %4         \n\t"   \
        "lw     $12, %5         \n\t"   \
        "lw     $13, %6         \n\t"

#define MULADDC_CORE                    \
        "lw     $14, 0($10)     \n\t"   \
        "multu  $13, $14        \n\t"   \
        "addi   $10, $10, 4     \n\t"   \
        "mflo   $14             \n\t"   \
        "mfhi   $9              \n\t"   \
        "addu   $14, $12, $14   \n\t"   \
        "lw     $15, 0($11)     \n\t"   \
        "sltu   $12, $14, $12   \n\t"   \
        "addu   $15, $14, $15   \n\t"   \
        "sltu   $14, $15, $14   \n\t"   \
        "addu   $12, $12, $9    \n\t"   \
        "sw     $15, 0($11)     \n\t"   \
        "addu   $12, $12, $14   \n\t"   \
        "addi   $11, $11, 4     \n\t"

#define MULADDC_STOP                    \
        "sw     $12, %0         \n\t"   \
        "sw     $11, %1         \n\t"   \
        "sw     $10, %2         \n\t"   \
        : "=m" (c), "=m" (d), "=m" (s)                      \
        : "m" (s), "m" (d), "m" (c), "m" (b)                \
        : "$9", "$10", "$11", "$12", "$13", "$14", "$15"    \
    );

#endif /* MIPS */
#endif /* GNUC */

#if (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)

#define MULADDC_INIT                            \
    __asm   mov     esi, s                      \
    __asm   mov     edi, d                      \
    __asm   mov     ecx, c                      \
    __asm   mov     ebx, b

#define MULADDC_CORE                            \
    __asm   lodsd                               \
    __asm   mul     ebx                         \
    __asm   add     eax, ecx                    \
    __asm   adc     edx, 0                      \
    __asm   add     eax, [edi]                  \
    __asm   adc     edx, 0                      \
    __asm   mov     ecx, edx                    \
    __asm   stosd

#if defined(POLARSSL_HAVE_SSE2)

#define EMIT __asm _emit

#define MULADDC_HUIT                            \
    EMIT 0x0F  EMIT 0x6E  EMIT 0xC9             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0xC3             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x1F             \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCB             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x16             \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xD0             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x66  EMIT 0x04  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xE0             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x76  EMIT 0x08  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xF0             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x7E  EMIT 0x0C  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xF8             \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCA             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x5F  EMIT 0x04  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xDC             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x6F  EMIT 0x08  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xEE             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x67  EMIT 0x0C  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xFC             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x0F             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x56  EMIT 0x10  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xD0             \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x66  EMIT 0x14  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xE0             \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCB             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x76  EMIT 0x18  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xF0             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x04  \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x5E  EMIT 0x1C  \
    EMIT 0x0F  EMIT 0xF4  EMIT 0xD8             \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCD             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x6F  EMIT 0x10  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xD5             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x08  \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCF             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x6F  EMIT 0x14  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xE5             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x0C  \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCA             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x6F  EMIT 0x18  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xF5             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x10  \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCC             \
    EMIT 0x0F  EMIT 0x6E  EMIT 0x6F  EMIT 0x1C  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xDD             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x14  \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCE             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x18  \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0xD4  EMIT 0xCB             \
    EMIT 0x0F  EMIT 0x7E  EMIT 0x4F  EMIT 0x1C  \
    EMIT 0x83  EMIT 0xC7  EMIT 0x20             \
    EMIT 0x83  EMIT 0xC6  EMIT 0x20             \
    EMIT 0x0F  EMIT 0x73  EMIT 0xD1  EMIT 0x20  \
    EMIT 0x0F  EMIT 0x7E  EMIT 0xC9

#define MULADDC_STOP                            \
    EMIT 0x0F  EMIT 0x77                        \
    __asm   mov     c, ecx                      \
    __asm   mov     d, edi                      \
    __asm   mov     s, esi                      \

#else

#define MULADDC_STOP                            \
    __asm   mov     c, ecx                      \
    __asm   mov     d, edi                      \
    __asm   mov     s, esi                      \

#endif /* SSE2 */
#endif /* MSVC */

#endif /* POLARSSL_HAVE_ASM */

#if !defined(MULADDC_CORE)
#if defined(POLARSSL_HAVE_UDBL)

#define MULADDC_INIT                    \
{                                       \
    t_udbl r;                           \
    t_uint r0, r1;

#define MULADDC_CORE                    \
    r   = *(s++) * (t_udbl) b;          \
    r0  = (t_uint) r;                   \
    r1  = (t_uint)( r >> biL );         \
    r0 += c;  r1 += (r0 <  c);          \
    r0 += *d; r1 += (r0 < *d);          \
    c = r1; *(d++) = r0;

#define MULADDC_STOP                    \
}

#else
#define MULADDC_INIT                    \
{                                       \
    t_uint s0, s1, b0, b1;              \
    t_uint r0, r1, rx, ry;              \
    b0 = ( b << biH ) >> biH;           \
    b1 = ( b >> biH );

#define MULADDC_CORE                    \
    s0 = ( *s << biH ) >> biH;          \
    s1 = ( *s >> biH ); s++;            \
    rx = s0 * b1; r0 = s0 * b0;         \
    ry = s1 * b0; r1 = s1 * b1;         \
    r1 += ( rx >> biH );                \
    r1 += ( ry >> biH );                \
    rx <<= biH; ry <<= biH;             \
    r0 += rx; r1 += (r0 < rx);          \
    r0 += ry; r1 += (r0 < ry);          \
    r0 +=  c; r1 += (r0 <  c);          \
    r0 += *d; r1 += (r0 < *d);          \
    c = r1; *(d++) = r0;

#define MULADDC_STOP                    \
}

#endif /* C (generic)  */
#endif /* C (longlong) */

#endif /* bn_mul.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/camellia.h ************/


/**
 * \file camellia.h
 *
 * \brief Camellia block cipher
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_CAMELLIA_H
#define POLARSSL_CAMELLIA_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

#define CAMELLIA_ENCRYPT     1
#define CAMELLIA_DECRYPT     0

#define POLARSSL_ERR_CAMELLIA_INVALID_KEY_LENGTH           -0x0024  /**< Invalid key length. */
#define POLARSSL_ERR_CAMELLIA_INVALID_INPUT_LENGTH         -0x0026  /**< Invalid data input length. */

#if !defined(POLARSSL_CAMELLIA_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          CAMELLIA context structure
 */
typedef struct
{
    int nr;                     /*!<  number of rounds  */
    uint32_t rk[68];            /*!<  CAMELLIA round keys    */
}
camellia_context;

/**
 * \brief          Initialize CAMELLIA context
 *
 * \param ctx      CAMELLIA context to be initialized
 */
void camellia_init( camellia_context *ctx );

/**
 * \brief          Clear CAMELLIA context
 *
 * \param ctx      CAMELLIA context to be cleared
 */
void camellia_free( camellia_context *ctx );

/**
 * \brief          CAMELLIA key schedule (encryption)
 *
 * \param ctx      CAMELLIA context to be initialized
 * \param key      encryption key
 * \param keysize  must be 128, 192 or 256
 *
 * \return         0 if successful, or POLARSSL_ERR_CAMELLIA_INVALID_KEY_LENGTH
 */
int camellia_setkey_enc( camellia_context *ctx, const unsigned char *key,
                         unsigned int keysize );

/**
 * \brief          CAMELLIA key schedule (decryption)
 *
 * \param ctx      CAMELLIA context to be initialized
 * \param key      decryption key
 * \param keysize  must be 128, 192 or 256
 *
 * \return         0 if successful, or POLARSSL_ERR_CAMELLIA_INVALID_KEY_LENGTH
 */
int camellia_setkey_dec( camellia_context *ctx, const unsigned char *key,
                         unsigned int keysize );

/**
 * \brief          CAMELLIA-ECB block encryption/decryption
 *
 * \param ctx      CAMELLIA context
 * \param mode     CAMELLIA_ENCRYPT or CAMELLIA_DECRYPT
 * \param input    16-byte input block
 * \param output   16-byte output block
 *
 * \return         0 if successful
 */
int camellia_crypt_ecb( camellia_context *ctx,
                    int mode,
                    const unsigned char input[16],
                    unsigned char output[16] );

#if defined(POLARSSL_CIPHER_MODE_CBC)
/**
 * \brief          CAMELLIA-CBC buffer encryption/decryption
 *                 Length should be a multiple of the block
 *                 size (16 bytes)
 *
 * \note           Upon exit, the content of the IV is updated so that you can
 *                 call the function same function again on the following
 *                 block(s) of data and get the same result as if it was
 *                 encrypted in one call. This allows a "streaming" usage.
 *                 If on the other hand you need to retain the contents of the
 *                 IV, you should either save it manually or use the cipher
 *                 module instead.
 *
 * \param ctx      CAMELLIA context
 * \param mode     CAMELLIA_ENCRYPT or CAMELLIA_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if successful, or
 *                 POLARSSL_ERR_CAMELLIA_INVALID_INPUT_LENGTH
 */
int camellia_crypt_cbc( camellia_context *ctx,
                    int mode,
                    size_t length,
                    unsigned char iv[16],
                    const unsigned char *input,
                    unsigned char *output );
#endif /* POLARSSL_CIPHER_MODE_CBC */

#if defined(POLARSSL_CIPHER_MODE_CFB)
/**
 * \brief          CAMELLIA-CFB128 buffer encryption/decryption
 *
 * Note: Due to the nature of CFB you should use the same key schedule for
 * both encryption and decryption. So a context initialized with
 * camellia_setkey_enc() for both CAMELLIA_ENCRYPT and CAMELLIE_DECRYPT.
 *
 * \note           Upon exit, the content of the IV is updated so that you can
 *                 call the function same function again on the following
 *                 block(s) of data and get the same result as if it was
 *                 encrypted in one call. This allows a "streaming" usage.
 *                 If on the other hand you need to retain the contents of the
 *                 IV, you should either save it manually or use the cipher
 *                 module instead.
 *
 * \param ctx      CAMELLIA context
 * \param mode     CAMELLIA_ENCRYPT or CAMELLIA_DECRYPT
 * \param length   length of the input data
 * \param iv_off   offset in IV (updated after use)
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if successful, or
 *                 POLARSSL_ERR_CAMELLIA_INVALID_INPUT_LENGTH
 */
int camellia_crypt_cfb128( camellia_context *ctx,
                       int mode,
                       size_t length,
                       size_t *iv_off,
                       unsigned char iv[16],
                       const unsigned char *input,
                       unsigned char *output );
#endif /* POLARSSL_CIPHER_MODE_CFB */

#if defined(POLARSSL_CIPHER_MODE_CTR)
/**
 * \brief               CAMELLIA-CTR buffer encryption/decryption
 *
 * Warning: You have to keep the maximum use of your counter in mind!
 *
 * Note: Due to the nature of CTR you should use the same key schedule for
 * both encryption and decryption. So a context initialized with
 * camellia_setkey_enc() for both CAMELLIA_ENCRYPT and CAMELLIA_DECRYPT.
 *
 * \param ctx           CAMELLIA context
 * \param length        The length of the data
 * \param nc_off        The offset in the current stream_block (for resuming
 *                      within current cipher stream). The offset pointer to
 *                      should be 0 at the start of a stream.
 * \param nonce_counter The 128-bit nonce and counter.
 * \param stream_block  The saved stream-block for resuming. Is overwritten
 *                      by the function.
 * \param input         The input data stream
 * \param output        The output data stream
 *
 * \return         0 if successful
 */
int camellia_crypt_ctr( camellia_context *ctx,
                       size_t length,
                       size_t *nc_off,
                       unsigned char nonce_counter[16],
                       unsigned char stream_block[16],
                       const unsigned char *input,
                       unsigned char *output );
#endif /* POLARSSL_CIPHER_MODE_CTR */

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_CAMELLIA_ALT */

#endif /* POLARSSL_CAMELLIA_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int camellia_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* camellia.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/ctr_drbg.h ************/


/**
 * \file ctr_drbg.h
 *
 * \brief CTR_DRBG based on AES-256 (NIST SP 800-90)
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_CTR_DRBG_H
#define POLARSSL_CTR_DRBG_H



#define POLARSSL_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED        -0x0034  /**< The entropy source failed. */
#define POLARSSL_ERR_CTR_DRBG_REQUEST_TOO_BIG              -0x0036  /**< Too many random requested in single call. */
#define POLARSSL_ERR_CTR_DRBG_INPUT_TOO_BIG                -0x0038  /**< Input too large (Entropy + additional). */
#define POLARSSL_ERR_CTR_DRBG_FILE_IO_ERROR                -0x003A  /**< Read/write error in file. */

#define CTR_DRBG_BLOCKSIZE          16      /**< Block size used by the cipher                  */
#define CTR_DRBG_KEYSIZE            32      /**< Key size used by the cipher                    */
#define CTR_DRBG_KEYBITS            ( CTR_DRBG_KEYSIZE * 8 )
#define CTR_DRBG_SEEDLEN            ( CTR_DRBG_KEYSIZE + CTR_DRBG_BLOCKSIZE )
                                            /**< The seed length (counter + AES key)            */

/**
 * \name SECTION: Module settings
 *
 * The configuration options you can set for this module are in this section.
 * Either change them in config.h or define them on the compiler command line.
 * \{
 */

#if !defined(CTR_DRBG_ENTROPY_LEN)
#if defined(POLARSSL_SHA512_C) && !defined(POLARSSL_ENTROPY_FORCE_SHA256)
#define CTR_DRBG_ENTROPY_LEN        48      /**< Amount of entropy used per seed by default (48 with SHA-512, 32 with SHA-256) */
#else
#define CTR_DRBG_ENTROPY_LEN        32      /**< Amount of entropy used per seed by default (48 with SHA-512, 32 with SHA-256) */
#endif
#endif

#if !defined(CTR_DRBG_RESEED_INTERVAL)
#define CTR_DRBG_RESEED_INTERVAL    10000   /**< Interval before reseed is performed by default */
#endif

#if !defined(CTR_DRBG_MAX_INPUT)
#define CTR_DRBG_MAX_INPUT          256     /**< Maximum number of additional input bytes */
#endif

#if !defined(CTR_DRBG_MAX_REQUEST)
#define CTR_DRBG_MAX_REQUEST        1024    /**< Maximum number of requested bytes per call */
#endif

#if !defined(CTR_DRBG_MAX_SEED_INPUT)
#define CTR_DRBG_MAX_SEED_INPUT     384     /**< Maximum size of (re)seed buffer */
#endif

/* \} name SECTION: Module settings */

#define CTR_DRBG_PR_OFF             0       /**< No prediction resistance       */
#define CTR_DRBG_PR_ON              1       /**< Prediction resistance enabled  */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          CTR_DRBG context structure
 */
typedef struct
{
    unsigned char counter[16];  /*!<  counter (V)       */
    int reseed_counter;         /*!<  reseed counter    */
    int prediction_resistance;  /*!<  enable prediction resistance (Automatic
                                      reseed before every random generation)  */
    size_t entropy_len;         /*!<  amount of entropy grabbed on each
                                      (re)seed          */
    int reseed_interval;        /*!<  reseed interval   */

    aes_context aes_ctx;        /*!<  AES context       */

    /*
     * Callbacks (Entropy)
     */
    int (*f_entropy)(void *, unsigned char *, size_t);

    void *p_entropy;            /*!<  context for the entropy function */
}
ctr_drbg_context;

/**
 * \brief               CTR_DRBG initialization
 *
 * Note: Personalization data can be provided in addition to the more generic
 *       entropy source to make this instantiation as unique as possible.
 *
 * \param ctx           CTR_DRBG context to be initialized
 * \param f_entropy     Entropy callback (p_entropy, buffer to fill, buffer
 *                      length)
 * \param p_entropy     Entropy context
 * \param custom        Personalization data (Device specific identifiers)
 *                      (Can be NULL)
 * \param len           Length of personalization data
 *
 * \return              0 if successful, or
 *                      POLARSSL_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED
 */
int ctr_drbg_init( ctr_drbg_context *ctx,
                   int (*f_entropy)(void *, unsigned char *, size_t),
                   void *p_entropy,
                   const unsigned char *custom,
                   size_t len );

/**
 * \brief               Clear CTR_CRBG context data
 *
 * \param ctx           CTR_DRBG context to clear
 */
void ctr_drbg_free( ctr_drbg_context *ctx );

/**
 * \brief               Enable / disable prediction resistance (Default: Off)
 *
 * Note: If enabled, entropy is used for ctx->entropy_len before each call!
 *       Only use this if you have ample supply of good entropy!
 *
 * \param ctx           CTR_DRBG context
 * \param resistance    CTR_DRBG_PR_ON or CTR_DRBG_PR_OFF
 */
void ctr_drbg_set_prediction_resistance( ctr_drbg_context *ctx,
                                         int resistance );

/**
 * \brief               Set the amount of entropy grabbed on each (re)seed
 *                      (Default: CTR_DRBG_ENTROPY_LEN)
 *
 * \param ctx           CTR_DRBG context
 * \param len           Amount of entropy to grab
 */
void ctr_drbg_set_entropy_len( ctr_drbg_context *ctx,
                               size_t len );

/**
 * \brief               Set the reseed interval
 *                      (Default: CTR_DRBG_RESEED_INTERVAL)
 *
 * \param ctx           CTR_DRBG context
 * \param interval      Reseed interval
 */
void ctr_drbg_set_reseed_interval( ctr_drbg_context *ctx,
                                   int interval );

/**
 * \brief               CTR_DRBG reseeding (extracts data from entropy source)
 *
 * \param ctx           CTR_DRBG context
 * \param additional    Additional data to add to state (Can be NULL)
 * \param len           Length of additional data
 *
 * \return              0 if successful, or
 *                      POLARSSL_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED
 */
int ctr_drbg_reseed( ctr_drbg_context *ctx,
                     const unsigned char *additional, size_t len );

/**
 * \brief               CTR_DRBG update state
 *
 * \param ctx           CTR_DRBG context
 * \param additional    Additional data to update state with
 * \param add_len       Length of additional data
 *
 * \note                If add_len is greater than CTR_DRBG_MAX_SEED_INPUT,
 *                      only the first CTR_DRBG_MAX_SEED_INPUT bytes are used,
 *                      the remaining ones are silently discarded.
 */
void ctr_drbg_update( ctr_drbg_context *ctx,
                      const unsigned char *additional, size_t add_len );

/**
 * \brief               CTR_DRBG generate random with additional update input
 *
 * Note: Automatically reseeds if reseed_counter is reached.
 *
 * \param p_rng         CTR_DRBG context
 * \param output        Buffer to fill
 * \param output_len    Length of the buffer
 * \param additional    Additional data to update with (Can be NULL)
 * \param add_len       Length of additional data
 *
 * \return              0 if successful, or
 *                      POLARSSL_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED, or
 *                      POLARSSL_ERR_CTR_DRBG_REQUEST_TOO_BIG
 */
int ctr_drbg_random_with_add( void *p_rng,
                              unsigned char *output, size_t output_len,
                              const unsigned char *additional, size_t add_len );

/**
 * \brief               CTR_DRBG generate random
 *
 * Note: Automatically reseeds if reseed_counter is reached.
 *
 * \param p_rng         CTR_DRBG context
 * \param output        Buffer to fill
 * \param output_len    Length of the buffer
 *
 * \return              0 if successful, or
 *                      POLARSSL_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED, or
 *                      POLARSSL_ERR_CTR_DRBG_REQUEST_TOO_BIG
 */
int ctr_drbg_random( void *p_rng,
                     unsigned char *output, size_t output_len );

#if defined(POLARSSL_FS_IO)
/**
 * \brief               Write a seed file
 *
 * \param ctx           CTR_DRBG context
 * \param path          Name of the file
 *
 * \return              0 if successful,
 *                      POLARSSL_ERR_CTR_DRBG_FILE_IO_ERROR on file error, or
 *                      POLARSSL_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED
 */
int ctr_drbg_write_seed_file( ctr_drbg_context *ctx, const char *path );

/**
 * \brief               Read and update a seed file. Seed is added to this
 *                      instance
 *
 * \param ctx           CTR_DRBG context
 * \param path          Name of the file
 *
 * \return              0 if successful,
 *                      POLARSSL_ERR_CTR_DRBG_FILE_IO_ERROR on file error,
 *                      POLARSSL_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED or
 *                      POLARSSL_ERR_CTR_DRBG_INPUT_TOO_BIG
 */
int ctr_drbg_update_seed_file( ctr_drbg_context *ctx, const char *path );
#endif /* POLARSSL_FS_IO */

/**
 * \brief               Checkup routine
 *
 * \return              0 if successful, or 1 if the test failed
 */
int ctr_drbg_self_test( int verbose );

/* Internal functions (do not call directly) */
int ctr_drbg_init_entropy_len( ctr_drbg_context *,
                               int (*)(void *, unsigned char *, size_t), void *,
                               const unsigned char *, size_t, size_t );

#ifdef __cplusplus
}
#endif

#endif /* ctr_drbg.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/des.h ************/


/**
 * \file des.h
 *
 * \brief DES block cipher
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_DES_H
#define POLARSSL_DES_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

#define DES_ENCRYPT     1
#define DES_DECRYPT     0

#define POLARSSL_ERR_DES_INVALID_INPUT_LENGTH              -0x0032  /**< The data input has an invalid length. */

#define DES_KEY_SIZE    8

#if !defined(POLARSSL_DES_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          DES context structure
 */
typedef struct
{
    int mode;                   /*!<  encrypt/decrypt   */
    uint32_t sk[32];            /*!<  DES subkeys       */
}
des_context;

/**
 * \brief          Triple-DES context structure
 */
typedef struct
{
    int mode;                   /*!<  encrypt/decrypt   */
    uint32_t sk[96];            /*!<  3DES subkeys      */
}
des3_context;

/**
 * \brief          Initialize DES context
 *
 * \param ctx      DES context to be initialized
 */
void des_init( des_context *ctx );

/**
 * \brief          Clear DES context
 *
 * \param ctx      DES context to be cleared
 */
void des_free( des_context *ctx );

/**
 * \brief          Initialize Triple-DES context
 *
 * \param ctx      DES3 context to be initialized
 */
void des3_init( des3_context *ctx );

/**
 * \brief          Clear Triple-DES context
 *
 * \param ctx      DES3 context to be cleared
 */
void des3_free( des3_context *ctx );

/**
 * \brief          Set key parity on the given key to odd.
 *
 *                 DES keys are 56 bits long, but each byte is padded with
 *                 a parity bit to allow verification.
 *
 * \param key      8-byte secret key
 */
void des_key_set_parity( unsigned char key[DES_KEY_SIZE] );

/**
 * \brief          Check that key parity on the given key is odd.
 *
 *                 DES keys are 56 bits long, but each byte is padded with
 *                 a parity bit to allow verification.
 *
 * \param key      8-byte secret key
 *
 * \return         0 is parity was ok, 1 if parity was not correct.
 */
int des_key_check_key_parity( const unsigned char key[DES_KEY_SIZE] );

/**
 * \brief          Check that key is not a weak or semi-weak DES key
 *
 * \param key      8-byte secret key
 *
 * \return         0 if no weak key was found, 1 if a weak key was identified.
 */
int des_key_check_weak( const unsigned char key[DES_KEY_SIZE] );

/**
 * \brief          DES key schedule (56-bit, encryption)
 *
 * \param ctx      DES context to be initialized
 * \param key      8-byte secret key
 *
 * \return         0
 */
int des_setkey_enc( des_context *ctx, const unsigned char key[DES_KEY_SIZE] );

/**
 * \brief          DES key schedule (56-bit, decryption)
 *
 * \param ctx      DES context to be initialized
 * \param key      8-byte secret key
 *
 * \return         0
 */
int des_setkey_dec( des_context *ctx, const unsigned char key[DES_KEY_SIZE] );

/**
 * \brief          Triple-DES key schedule (112-bit, encryption)
 *
 * \param ctx      3DES context to be initialized
 * \param key      16-byte secret key
 *
 * \return         0
 */
int des3_set2key_enc( des3_context *ctx,
                      const unsigned char key[DES_KEY_SIZE * 2] );

/**
 * \brief          Triple-DES key schedule (112-bit, decryption)
 *
 * \param ctx      3DES context to be initialized
 * \param key      16-byte secret key
 *
 * \return         0
 */
int des3_set2key_dec( des3_context *ctx,
                      const unsigned char key[DES_KEY_SIZE * 2] );

/**
 * \brief          Triple-DES key schedule (168-bit, encryption)
 *
 * \param ctx      3DES context to be initialized
 * \param key      24-byte secret key
 *
 * \return         0
 */
int des3_set3key_enc( des3_context *ctx,
                      const unsigned char key[DES_KEY_SIZE * 3] );

/**
 * \brief          Triple-DES key schedule (168-bit, decryption)
 *
 * \param ctx      3DES context to be initialized
 * \param key      24-byte secret key
 *
 * \return         0
 */
int des3_set3key_dec( des3_context *ctx,
                      const unsigned char key[DES_KEY_SIZE * 3] );

/**
 * \brief          DES-ECB block encryption/decryption
 *
 * \param ctx      DES context
 * \param input    64-bit input block
 * \param output   64-bit output block
 *
 * \return         0 if successful
 */
int des_crypt_ecb( des_context *ctx,
                    const unsigned char input[8],
                    unsigned char output[8] );

#if defined(POLARSSL_CIPHER_MODE_CBC)
/**
 * \brief          DES-CBC buffer encryption/decryption
 *
 * \note           Upon exit, the content of the IV is updated so that you can
 *                 call the function same function again on the following
 *                 block(s) of data and get the same result as if it was
 *                 encrypted in one call. This allows a "streaming" usage.
 *                 If on the other hand you need to retain the contents of the
 *                 IV, you should either save it manually or use the cipher
 *                 module instead.
 *
 * \param ctx      DES context
 * \param mode     DES_ENCRYPT or DES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 */
int des_crypt_cbc( des_context *ctx,
                    int mode,
                    size_t length,
                    unsigned char iv[8],
                    const unsigned char *input,
                    unsigned char *output );
#endif /* POLARSSL_CIPHER_MODE_CBC */

/**
 * \brief          3DES-ECB block encryption/decryption
 *
 * \param ctx      3DES context
 * \param input    64-bit input block
 * \param output   64-bit output block
 *
 * \return         0 if successful
 */
int des3_crypt_ecb( des3_context *ctx,
                     const unsigned char input[8],
                     unsigned char output[8] );

#if defined(POLARSSL_CIPHER_MODE_CBC)
/**
 * \brief          3DES-CBC buffer encryption/decryption
 *
 * \note           Upon exit, the content of the IV is updated so that you can
 *                 call the function same function again on the following
 *                 block(s) of data and get the same result as if it was
 *                 encrypted in one call. This allows a "streaming" usage.
 *                 If on the other hand you need to retain the contents of the
 *                 IV, you should either save it manually or use the cipher
 *                 module instead.
 *
 * \param ctx      3DES context
 * \param mode     DES_ENCRYPT or DES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if successful, or POLARSSL_ERR_DES_INVALID_INPUT_LENGTH
 */
int des3_crypt_cbc( des3_context *ctx,
                     int mode,
                     size_t length,
                     unsigned char iv[8],
                     const unsigned char *input,
                     unsigned char *output );
#endif /* POLARSSL_CIPHER_MODE_CBC */

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_DES_ALT */

#endif /* POLARSSL_DES_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int des_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* des.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/entropy.h ************/


/**
 * \file entropy.h
 *
 * \brief Entropy accumulator implementation
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_ENTROPY_H
#define POLARSSL_ENTROPY_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(POLARSSL_SHA512_C) && !defined(POLARSSL_ENTROPY_FORCE_SHA256)

#define POLARSSL_ENTROPY_SHA512_ACCUMULATOR
#else
#if defined(POLARSSL_SHA256_C)
#define POLARSSL_ENTROPY_SHA256_ACCUMULATOR

#endif
#endif

#if defined(POLARSSL_THREADING_C)

#endif

#if defined(POLARSSL_HAVEGE_C)

#endif

#define POLARSSL_ERR_ENTROPY_SOURCE_FAILED                 -0x003C  /**< Critical entropy source failure. */
#define POLARSSL_ERR_ENTROPY_MAX_SOURCES                   -0x003E  /**< No more sources can be added. */
#define POLARSSL_ERR_ENTROPY_NO_SOURCES_DEFINED            -0x0040  /**< No sources have been added to poll. */
#define POLARSSL_ERR_ENTROPY_FILE_IO_ERROR                 -0x0058  /**< Read/write error in file. */

/**
 * \name SECTION: Module settings
 *
 * The configuration options you can set for this module are in this section.
 * Either change them in config.h or define them on the compiler command line.
 * \{
 */

#if !defined(ENTROPY_MAX_SOURCES)
#define ENTROPY_MAX_SOURCES     20      /**< Maximum number of sources supported */
#endif

#if !defined(ENTROPY_MAX_GATHER)
#define ENTROPY_MAX_GATHER      128     /**< Maximum amount requested from entropy sources */
#endif

/* \} name SECTION: Module settings */

#if defined(POLARSSL_ENTROPY_SHA512_ACCUMULATOR)
#define ENTROPY_BLOCK_SIZE      64      /**< Block size of entropy accumulator (SHA-512) */
#else
#define ENTROPY_BLOCK_SIZE      32      /**< Block size of entropy accumulator (SHA-256) */
#endif

#define ENTROPY_MAX_SEED_SIZE   1024    /**< Maximum size of seed we read from seed file */
#define ENTROPY_SOURCE_MANUAL   ENTROPY_MAX_SOURCES

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief           Entropy poll callback pointer
 *
 * \param data      Callback-specific data pointer
 * \param output    Data to fill
 * \param len       Maximum size to provide
 * \param olen      The actual amount of bytes put into the buffer (Can be 0)
 *
 * \return          0 if no critical failures occurred,
 *                  POLARSSL_ERR_ENTROPY_SOURCE_FAILED otherwise
 */
typedef int (*f_source_ptr)(void *data, unsigned char *output, size_t len,
                            size_t *olen);

/**
 * \brief           Entropy source state
 */
typedef struct
{
    f_source_ptr    f_source;   /**< The entropy source callback */
    void *          p_source;   /**< The callback data pointer */
    size_t          size;       /**< Amount received */
    size_t          threshold;  /**< Minimum level required before release */
}
source_state;

/**
 * \brief           Entropy context structure
 */
typedef struct
{
#if defined(POLARSSL_ENTROPY_SHA512_ACCUMULATOR)
    sha512_context  accumulator;
#else
    sha256_context  accumulator;
#endif
    int             source_count;
    source_state    source[ENTROPY_MAX_SOURCES];
#if defined(POLARSSL_HAVEGE_C)
    havege_state    havege_data;
#endif
#if defined(POLARSSL_THREADING_C)
    threading_mutex_t mutex;    /*!< mutex                  */
#endif
}
entropy_context;

/**
 * \brief           Initialize the context
 *
 * \param ctx       Entropy context to initialize
 */
void entropy_init( entropy_context *ctx );

/**
 * \brief           Free the data in the context
 *
 * \param ctx       Entropy context to free
 */
void entropy_free( entropy_context *ctx );

/**
 * \brief           Adds an entropy source to poll
 *                  (Thread-safe if POLARSSL_THREADING_C is enabled)
 *
 * \param ctx       Entropy context
 * \param f_source  Entropy function
 * \param p_source  Function data
 * \param threshold Minimum required from source before entropy is released
 *                  ( with entropy_func() )
 *
 * \return          0 if successful or POLARSSL_ERR_ENTROPY_MAX_SOURCES
 */
int entropy_add_source( entropy_context *ctx,
                        f_source_ptr f_source, void *p_source,
                        size_t threshold );

/**
 * \brief           Trigger an extra gather poll for the accumulator
 *                  (Thread-safe if POLARSSL_THREADING_C is enabled)
 *
 * \param ctx       Entropy context
 *
 * \return          0 if successful, or POLARSSL_ERR_ENTROPY_SOURCE_FAILED
 */
int entropy_gather( entropy_context *ctx );

/**
 * \brief           Retrieve entropy from the accumulator
 *                  (Maximum length: ENTROPY_BLOCK_SIZE)
 *                  (Thread-safe if POLARSSL_THREADING_C is enabled)
 *
 * \param data      Entropy context
 * \param output    Buffer to fill
 * \param len       Number of bytes desired, must be at most ENTROPY_BLOCK_SIZE
 *
 * \return          0 if successful, or POLARSSL_ERR_ENTROPY_SOURCE_FAILED
 */
int entropy_func( void *data, unsigned char *output, size_t len );

/**
 * \brief           Add data to the accumulator manually
 *                  (Thread-safe if POLARSSL_THREADING_C is enabled)
 *
 * \param ctx       Entropy context
 * \param data      Data to add
 * \param len       Length of data
 *
 * \return          0 if successful
 */
int entropy_update_manual( entropy_context *ctx,
                           const unsigned char *data, size_t len );

#if defined(POLARSSL_FS_IO)
/**
 * \brief               Write a seed file
 *
 * \param ctx           Entropy context
 * \param path          Name of the file
 *
 * \return              0 if successful,
 *                      POLARSSL_ERR_ENTROPY_FILE_IO_ERROR on file error, or
 *                      POLARSSL_ERR_ENTROPY_SOURCE_FAILED
 */
int entropy_write_seed_file( entropy_context *ctx, const char *path );

/**
 * \brief               Read and update a seed file. Seed is added to this
 *                      instance. No more than ENTROPY_MAX_SEED_SIZE bytes are
 *                      read from the seed file. The rest is ignored.
 *
 * \param ctx           Entropy context
 * \param path          Name of the file
 *
 * \return              0 if successful,
 *                      POLARSSL_ERR_ENTROPY_FILE_IO_ERROR on file error,
 *                      POLARSSL_ERR_ENTROPY_SOURCE_FAILED
 */
int entropy_update_seed_file( entropy_context *ctx, const char *path );
#endif /* POLARSSL_FS_IO */

#if defined(POLARSSL_SELF_TEST)
/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if a test failed
 */
int entropy_self_test( int verbose );
#endif /* POLARSSL_SELF_TEST */

#ifdef __cplusplus
}
#endif

#endif /* entropy.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/entropy_poll.h ************/


/**
 * \file entropy_poll.h
 *
 * \brief Platform-specific and custom entropy polling functions
 *
 *  Copyright (C) 2006-2011, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_ENTROPY_POLL_H
#define POLARSSL_ENTROPY_POLL_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Default thresholds for built-in sources
 */
#define ENTROPY_MIN_PLATFORM    128     /**< Minimum for platform source    */
#define ENTROPY_MIN_HAVEGE      128     /**< Minimum for HAVEGE             */
#define ENTROPY_MIN_HARDCLOCK    32     /**< Minimum for hardclock()        */

#if !defined(POLARSSL_NO_PLATFORM_ENTROPY)
/**
 * \brief           Platform-specific entropy poll callback
 */
int platform_entropy_poll( void *data,
                           unsigned char *output, size_t len, size_t *olen );
#endif

#if defined(POLARSSL_HAVEGE_C)
/**
 * \brief           HAVEGE based entropy poll callback
 *
 * Requires an HAVEGE state as its data pointer.
 */
int havege_poll( void *data,
                 unsigned char *output, size_t len, size_t *olen );
#endif

#if defined(POLARSSL_TIMING_C)
/**
 * \brief           hardclock-based entropy poll callback
 */
int hardclock_poll( void *data,
                    unsigned char *output, size_t len, size_t *olen );
#endif

#ifdef __cplusplus
}
#endif

#endif /* entropy_poll.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/padlock.h ************/


/**
 * \file padlock.h
 *
 * \brief VIA PadLock ACE for HW encryption/decryption supported by some
 *        processors
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_PADLOCK_H
#define POLARSSL_PADLOCK_H



#define POLARSSL_ERR_PADLOCK_DATA_MISALIGNED               -0x0030  /**< Input data should be aligned. */

#if defined(POLARSSL_HAVE_ASM) && defined(__GNUC__) && defined(__i386__)

#ifndef POLARSSL_HAVE_X86
#define POLARSSL_HAVE_X86
#endif

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef INT32 int32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

#define PADLOCK_RNG 0x000C
#define PADLOCK_ACE 0x00C0
#define PADLOCK_PHE 0x0C00
#define PADLOCK_PMM 0x3000

#define PADLOCK_ALIGN16(x) (uint32_t *) (16 + ((int32_t) x & ~15))

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          PadLock detection routine
 *
 * \param feature  The feature to detect
 *
 * \return         1 if CPU has support for the feature, 0 otherwise
 */
int padlock_supports( int feature );

/**
 * \brief          PadLock AES-ECB block en(de)cryption
 *
 * \param ctx      AES context
 * \param mode     AES_ENCRYPT or AES_DECRYPT
 * \param input    16-byte input block
 * \param output   16-byte output block
 *
 * \return         0 if success, 1 if operation failed
 */
int padlock_xcryptecb( aes_context *ctx,
                       int mode,
                       const unsigned char input[16],
                       unsigned char output[16] );

/**
 * \brief          PadLock AES-CBC buffer en(de)cryption
 *
 * \param ctx      AES context
 * \param mode     AES_ENCRYPT or AES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if success, 1 if operation failed
 */
int padlock_xcryptcbc( aes_context *ctx,
                       int mode,
                       size_t length,
                       unsigned char iv[16],
                       const unsigned char *input,
                       unsigned char *output );

#ifdef __cplusplus
}
#endif

#endif /* HAVE_X86  */

#endif /* padlock.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/timing.h ************/


/**
 * \file timing.h
 *
 * \brief Portable interface to the CPU cycle counter
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_TIMING_H
#define POLARSSL_TIMING_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#if !defined(POLARSSL_TIMING_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          timer structure
 */
struct hr_time
{
    unsigned char opaque[32];
};

extern volatile int alarmed;

/**
 * \brief          Return the CPU cycle counter value
 */
unsigned long hardclock( void );

/**
 * \brief          Return the elapsed time in milliseconds
 *
 * \param val      points to a timer structure
 * \param reset    if set to 1, the timer is restarted
 */
unsigned long get_timer( struct hr_time *val, int reset );

/**
 * \brief          Setup an alarm clock
 *
 * \param seconds  delay before the "alarmed" flag is set
 *
 * \warning        Only one alarm at a time  is supported. In a threaded
 *                 context, this means one for the whole process, not one per
 *                 thread.
 */
void set_alarm( int seconds );

/**
 * \brief          Sleep for a certain amount of time
 *
 * \param milliseconds  delay in milliseconds
 */
void m_sleep( int milliseconds );

#if defined(POLARSSL_SELF_TEST)
/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if a test failed
 */
int timing_self_test( int verbose );
#endif

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_TIMING_ALT */

#endif /* POLARSSL_TIMING_ALT */

#endif /* timing.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/xtea.h ************/


/**
 * \file xtea.h
 *
 * \brief XTEA block cipher (32-bit)
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_XTEA_H
#define POLARSSL_XTEA_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

#define XTEA_ENCRYPT     1
#define XTEA_DECRYPT     0

#define POLARSSL_ERR_XTEA_INVALID_INPUT_LENGTH             -0x0028  /**< The data input has an invalid length. */

#if !defined(POLARSSL_XTEA_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          XTEA context structure
 */
typedef struct
{
    uint32_t k[4];       /*!< key */
}
xtea_context;

/**
 * \brief          Initialize XTEA context
 *
 * \param ctx      XTEA context to be initialized
 */
void xtea_init( xtea_context *ctx );

/**
 * \brief          Clear XTEA context
 *
 * \param ctx      XTEA context to be cleared
 */
void xtea_free( xtea_context *ctx );

/**
 * \brief          XTEA key schedule
 *
 * \param ctx      XTEA context to be initialized
 * \param key      the secret key
 */
void xtea_setup( xtea_context *ctx, const unsigned char key[16] );

/**
 * \brief          XTEA cipher function
 *
 * \param ctx      XTEA context
 * \param mode     XTEA_ENCRYPT or XTEA_DECRYPT
 * \param input    8-byte input block
 * \param output   8-byte output block
 *
 * \return         0 if successful
 */
int xtea_crypt_ecb( xtea_context *ctx,
                    int mode,
                    const unsigned char input[8],
                    unsigned char output[8] );

#if defined(POLARSSL_CIPHER_MODE_CBC)
/**
 * \brief          XTEA CBC cipher function
 *
 * \param ctx      XTEA context
 * \param mode     XTEA_ENCRYPT or XTEA_DECRYPT
 * \param length   the length of input, multiple of 8
 * \param iv       initialization vector for CBC mode
 * \param input    input block
 * \param output   output block
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_XTEA_INVALID_INPUT_LENGTH if the length % 8 != 0
 */
int xtea_crypt_cbc( xtea_context *ctx,
                    int mode,
                    size_t length,
                    unsigned char iv[8],
                    const unsigned char *input,
                    unsigned char *output);
#endif /* POLARSSL_CIPHER_MODE_CBC */

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_XTEA_ALT */

#endif /* POLARSSL_XTEA_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int xtea_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* xtea.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/ssl.h ************/


/**
 * \file ssl.h
 *
 * \brief SSL/TLS functions.
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_SSL_H
#define POLARSSL_SSL_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

/* Temporary compatibility trick for the current stable branch */
#if !defined(POLARSSL_SSL_DISABLE_RENEGOTIATION)
#define POLARSSL_SSL_RENEGOTIATION
#endif







#if defined(POLARSSL_MD5_C)

#endif

#if defined(POLARSSL_SHA1_C)

#endif

#if defined(POLARSSL_SHA256_C)

#endif

#if defined(POLARSSL_SHA512_C)

#endif

// for session tickets
#if defined(POLARSSL_AES_C)

#endif

#if defined(POLARSSL_X509_CRT_PARSE_C)


#endif

#if defined(POLARSSL_DHM_C)

#endif

#if defined(POLARSSL_ECDH_C)

#endif

#if defined(POLARSSL_ZLIB_SUPPORT)

#endif

#if defined(POLARSSL_HAVE_TIME)
#include <time.h>
#endif

/* For convenience below and in programs */
#if defined(POLARSSL_KEY_EXCHANGE_PSK_ENABLED) ||                           \
    defined(POLARSSL_KEY_EXCHANGE_RSA_PSK_ENABLED) ||                       \
    defined(POLARSSL_KEY_EXCHANGE_DHE_PSK_ENABLED) ||                       \
    defined(POLARSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED)
#define POLARSSL_KEY_EXCHANGE__SOME__PSK_ENABLED
#endif

#if defined(POLARSSL_KEY_EXCHANGE_ECDHE_RSA_ENABLED) ||                     \
    defined(POLARSSL_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED) ||                   \
    defined(POLARSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED)
#define POLARSSL_KEY_EXCHANGE__SOME__ECDHE_ENABLED
#endif

#if defined(_MSC_VER) && !defined(inline)
#define inline _inline
#else
#if defined(__ARMCC_VERSION) && !defined(inline)
#define inline __inline
#endif /* __ARMCC_VERSION */
#endif /*_MSC_VER */

/*
 * SSL Error codes
 */
#define POLARSSL_ERR_SSL_FEATURE_UNAVAILABLE               -0x7080  /**< The requested feature is not available. */
#define POLARSSL_ERR_SSL_BAD_INPUT_DATA                    -0x7100  /**< Bad input parameters to function. */
#define POLARSSL_ERR_SSL_INVALID_MAC                       -0x7180  /**< Verification of the message MAC failed. */
#define POLARSSL_ERR_SSL_INVALID_RECORD                    -0x7200  /**< An invalid SSL record was received. */
#define POLARSSL_ERR_SSL_CONN_EOF                          -0x7280  /**< The connection indicated an EOF. */
#define POLARSSL_ERR_SSL_UNKNOWN_CIPHER                    -0x7300  /**< An unknown cipher was received. */
#define POLARSSL_ERR_SSL_NO_CIPHER_CHOSEN                  -0x7380  /**< The server has no ciphersuites in common with the client. */
#define POLARSSL_ERR_SSL_NO_RNG                            -0x7400  /**< No RNG was provided to the SSL module. */
#define POLARSSL_ERR_SSL_NO_CLIENT_CERTIFICATE             -0x7480  /**< No client certification received from the client, but required by the authentication mode. */
#define POLARSSL_ERR_SSL_CERTIFICATE_TOO_LARGE             -0x7500  /**< Our own certificate(s) is/are too large to send in an SSL message. */
#define POLARSSL_ERR_SSL_CERTIFICATE_REQUIRED              -0x7580  /**< The own certificate is not set, but needed by the server. */
#define POLARSSL_ERR_SSL_PRIVATE_KEY_REQUIRED              -0x7600  /**< The own private key or pre-shared key is not set, but needed. */
#define POLARSSL_ERR_SSL_CA_CHAIN_REQUIRED                 -0x7680  /**< No CA Chain is set, but required to operate. */
#define POLARSSL_ERR_SSL_UNEXPECTED_MESSAGE                -0x7700  /**< An unexpected message was received from our peer. */
#define POLARSSL_ERR_SSL_FATAL_ALERT_MESSAGE               -0x7780  /**< A fatal alert message was received from our peer. */
#define POLARSSL_ERR_SSL_PEER_VERIFY_FAILED                -0x7800  /**< Verification of our peer failed. */
#define POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY                 -0x7880  /**< The peer notified us that the connection is going to be closed. */
#define POLARSSL_ERR_SSL_BAD_HS_CLIENT_HELLO               -0x7900  /**< Processing of the ClientHello handshake message failed. */
#define POLARSSL_ERR_SSL_BAD_HS_SERVER_HELLO               -0x7980  /**< Processing of the ServerHello handshake message failed. */
#define POLARSSL_ERR_SSL_BAD_HS_CERTIFICATE                -0x7A00  /**< Processing of the Certificate handshake message failed. */
#define POLARSSL_ERR_SSL_BAD_HS_CERTIFICATE_REQUEST        -0x7A80  /**< Processing of the CertificateRequest handshake message failed. */
#define POLARSSL_ERR_SSL_BAD_HS_SERVER_KEY_EXCHANGE        -0x7B00  /**< Processing of the ServerKeyExchange handshake message failed. */
#define POLARSSL_ERR_SSL_BAD_HS_SERVER_HELLO_DONE          -0x7B80  /**< Processing of the ServerHelloDone handshake message failed. */
#define POLARSSL_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE        -0x7C00  /**< Processing of the ClientKeyExchange handshake message failed. */
#define POLARSSL_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE_RP     -0x7C80  /**< Processing of the ClientKeyExchange handshake message failed in DHM / ECDH Read Public. */
#define POLARSSL_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE_CS     -0x7D00  /**< Processing of the ClientKeyExchange handshake message failed in DHM / ECDH Calculate Secret. */
#define POLARSSL_ERR_SSL_BAD_HS_CERTIFICATE_VERIFY         -0x7D80  /**< Processing of the CertificateVerify handshake message failed. */
#define POLARSSL_ERR_SSL_BAD_HS_CHANGE_CIPHER_SPEC         -0x7E00  /**< Processing of the ChangeCipherSpec handshake message failed. */
#define POLARSSL_ERR_SSL_BAD_HS_FINISHED                   -0x7E80  /**< Processing of the Finished handshake message failed. */
#define POLARSSL_ERR_SSL_MALLOC_FAILED                     -0x7F00  /**< Memory allocation failed */
#define POLARSSL_ERR_SSL_HW_ACCEL_FAILED                   -0x7F80  /**< Hardware acceleration function returned with error */
#define POLARSSL_ERR_SSL_HW_ACCEL_FALLTHROUGH              -0x6F80  /**< Hardware acceleration function skipped / left alone data */
#define POLARSSL_ERR_SSL_COMPRESSION_FAILED                -0x6F00  /**< Processing of the compression / decompression failed */
#define POLARSSL_ERR_SSL_BAD_HS_PROTOCOL_VERSION           -0x6E80  /**< Handshake protocol not within min/max boundaries */
#define POLARSSL_ERR_SSL_BAD_HS_NEW_SESSION_TICKET         -0x6E00  /**< Processing of the NewSessionTicket handshake message failed. */
#define POLARSSL_ERR_SSL_SESSION_TICKET_EXPIRED            -0x6D80  /**< Session ticket has expired. */
#define POLARSSL_ERR_SSL_PK_TYPE_MISMATCH                  -0x6D00  /**< Public key type mismatch (eg, asked for RSA key exchange and presented EC key) */
#define POLARSSL_ERR_SSL_UNKNOWN_IDENTITY                  -0x6C80  /**< Unknown identity received (eg, PSK identity) */
#define POLARSSL_ERR_SSL_INTERNAL_ERROR                    -0x6C00  /**< Internal error (eg, unexpected failure in lower-level module) */
#define POLARSSL_ERR_SSL_COUNTER_WRAPPING                  -0x6B80  /**< A counter would wrap (eg, too many messages exchanged). */
#define POLARSSL_ERR_SSL_WAITING_SERVER_HELLO_RENEGO       -0x6B00  /**< Unexpected message at ServerHello in renegotiation. */
#define POLARSSL_ERR_SSL_NO_USABLE_CIPHERSUITE             -0x6A80  /**< None of the common ciphersuites is usable (eg, no suitable certificate, see debug messages). */

/*
 * Various constants
 */
#define SSL_MAJOR_VERSION_3             3
#define SSL_MINOR_VERSION_0             0   /*!< SSL v3.0 */
#define SSL_MINOR_VERSION_1             1   /*!< TLS v1.0 */
#define SSL_MINOR_VERSION_2             2   /*!< TLS v1.1 */
#define SSL_MINOR_VERSION_3             3   /*!< TLS v1.2 */

/* Determine minimum supported version */
#define SSL_MIN_MAJOR_VERSION           SSL_MAJOR_VERSION_3

#if defined(POLARSSL_SSL_PROTO_SSL3)
#define SSL_MIN_MINOR_VERSION           SSL_MINOR_VERSION_0
#else
#if defined(POLARSSL_SSL_PROTO_TLS1)
#define SSL_MIN_MINOR_VERSION           SSL_MINOR_VERSION_1
#else
#if defined(POLARSSL_SSL_PROTO_TLS1_1)
#define SSL_MIN_MINOR_VERSION           SSL_MINOR_VERSION_2
#else
#if defined(POLARSSL_SSL_PROTO_TLS1_2)
#define SSL_MIN_MINOR_VERSION           SSL_MINOR_VERSION_3
#endif /* POLARSSL_SSL_PROTO_TLS1_2 */
#endif /* POLARSSL_SSL_PROTO_TLS1_1 */
#endif /* POLARSSL_SSL_PROTO_TLS1   */
#endif /* POLARSSL_SSL_PROTO_SSL3   */

/* Determine maximum supported version */
#define SSL_MAX_MAJOR_VERSION           SSL_MAJOR_VERSION_3

#if defined(POLARSSL_SSL_PROTO_TLS1_2)
#define SSL_MAX_MINOR_VERSION           SSL_MINOR_VERSION_3
#else
#if defined(POLARSSL_SSL_PROTO_TLS1_1)
#define SSL_MAX_MINOR_VERSION           SSL_MINOR_VERSION_2
#else
#if defined(POLARSSL_SSL_PROTO_TLS1)
#define SSL_MAX_MINOR_VERSION           SSL_MINOR_VERSION_1
#else
#if defined(POLARSSL_SSL_PROTO_SSL3)
#define SSL_MAX_MINOR_VERSION           SSL_MINOR_VERSION_0
#endif /* POLARSSL_SSL_PROTO_SSL3   */
#endif /* POLARSSL_SSL_PROTO_TLS1   */
#endif /* POLARSSL_SSL_PROTO_TLS1_1 */
#endif /* POLARSSL_SSL_PROTO_TLS1_2 */

/* RFC 6066 section 4, see also mfl_code_to_length in ssl_tls.c
 * NONE must be zero so that memset()ing structure to zero works */
#define SSL_MAX_FRAG_LEN_NONE           0   /*!< don't use this extension   */
#define SSL_MAX_FRAG_LEN_512            1   /*!< MaxFragmentLength 2^9      */
#define SSL_MAX_FRAG_LEN_1024           2   /*!< MaxFragmentLength 2^10     */
#define SSL_MAX_FRAG_LEN_2048           3   /*!< MaxFragmentLength 2^11     */
#define SSL_MAX_FRAG_LEN_4096           4   /*!< MaxFragmentLength 2^12     */
#define SSL_MAX_FRAG_LEN_INVALID        5   /*!< first invalid value        */

#define SSL_IS_CLIENT                   0
#define SSL_IS_SERVER                   1

#define SSL_IS_NOT_FALLBACK             0
#define SSL_IS_FALLBACK                 1

#define SSL_EXTENDED_MS_DISABLED        0
#define SSL_EXTENDED_MS_ENABLED         1

#define SSL_ETM_DISABLED                0
#define SSL_ETM_ENABLED                 1

#define SSL_COMPRESS_NULL               0
#define SSL_COMPRESS_DEFLATE            1

#define SSL_VERIFY_NONE                 0
#define SSL_VERIFY_OPTIONAL             1
#define SSL_VERIFY_REQUIRED             2

#define SSL_INITIAL_HANDSHAKE           0
#define SSL_RENEGOTIATION               1   /* In progress */
#define SSL_RENEGOTIATION_DONE          2   /* Done */
#define SSL_RENEGOTIATION_PENDING       3   /* Requested (server only) */

#define SSL_LEGACY_RENEGOTIATION        0
#define SSL_SECURE_RENEGOTIATION        1

#define SSL_RENEGOTIATION_DISABLED      0
#define SSL_RENEGOTIATION_ENABLED       1

#define SSL_RENEGOTIATION_NOT_ENFORCED  -1
#define SSL_RENEGO_MAX_RECORDS_DEFAULT  16

#define SSL_LEGACY_NO_RENEGOTIATION     0
#define SSL_LEGACY_ALLOW_RENEGOTIATION  1
#define SSL_LEGACY_BREAK_HANDSHAKE      2

#define SSL_TRUNC_HMAC_DISABLED         0
#define SSL_TRUNC_HMAC_ENABLED          1
#define SSL_TRUNCATED_HMAC_LEN          10  /* 80 bits, rfc 6066 section 7 */

#define SSL_SESSION_TICKETS_DISABLED     0
#define SSL_SESSION_TICKETS_ENABLED      1

#define SSL_CBC_RECORD_SPLITTING_DISABLED   -1
#define SSL_CBC_RECORD_SPLITTING_ENABLED     0

#define SSL_ARC4_ENABLED                0
#define SSL_ARC4_DISABLED               1

/**
 * \name SECTION: Module settings
 *
 * The configuration options you can set for this module are in this section.
 * Either change them in config.h or define them on the compiler command line.
 * \{
 */

#if !defined(SSL_DEFAULT_TICKET_LIFETIME)
#define SSL_DEFAULT_TICKET_LIFETIME     86400 /**< Lifetime of session tickets (if enabled) */
#endif

/*
 * Size of the input / output buffer.
 * Note: the RFC defines the default size of SSL / TLS messages. If you
 * change the value here, other clients / servers may not be able to
 * communicate with you anymore. Only change this value if you control
 * both sides of the connection and have it reduced at both sides, or
 * if you're using the Max Fragment Length extension and you know all your
 * peers are using it too!
 */
#if !defined(SSL_MAX_CONTENT_LEN)
#define SSL_MAX_CONTENT_LEN         16384   /**< Size of the input / output buffer */
#endif

/* \} name SECTION: Module settings */

/*
 * Allow extra bytes for record, authentication and encryption overhead:
 * counter (8) + header (5) + IV(16) + MAC (16-48) + padding (0-256)
 * and allow for a maximum of 1024 of compression expansion if
 * enabled.
 */
#if defined(POLARSSL_ZLIB_SUPPORT)
#define SSL_COMPRESSION_ADD          1024
#else
#define SSL_COMPRESSION_ADD             0
#endif

#if defined(POLARSSL_RC4_C) || defined(POLARSSL_CIPHER_MODE_CBC)
/* Ciphersuites using HMAC */
#if defined(POLARSSL_SHA512_C)
#define SSL_MAC_ADD                 48  /* SHA-384 used for HMAC */
#elif defined(POLARSSL_SHA256_C)
#define SSL_MAC_ADD                 32  /* SHA-256 used for HMAC */
#else
#define SSL_MAC_ADD                 20  /* SHA-1   used for HMAC */
#endif
#else
/* AEAD ciphersuites: GCM and CCM use a 128 bits tag */
#define SSL_MAC_ADD                 16
#endif

#if defined(POLARSSL_CIPHER_MODE_CBC)
#define SSL_PADDING_ADD            256
#else
#define SSL_PADDING_ADD              0
#endif

#define SSL_BUFFER_LEN  ( SSL_MAX_CONTENT_LEN               \
                        + SSL_COMPRESSION_ADD               \
                        + 29 /* counter + header + IV */    \
                        + SSL_MAC_ADD                       \
                        + SSL_PADDING_ADD                   \
                        )

/*
 * Length of the verify data for secure renegotiation
 */
#if defined(POLARSSL_SSL_PROTO_SSL3)
#define SSL_VERIFY_DATA_MAX_LEN 36
#else
#define SSL_VERIFY_DATA_MAX_LEN 12
#endif

/*
 * Signaling ciphersuite values (SCSV)
 */
#define SSL_EMPTY_RENEGOTIATION_INFO    0xFF   /**< renegotiation info ext */
#define SSL_FALLBACK_SCSV               0x5600 /**< draft-ietf-tls-downgrade-scsv-00 */

/*
 * Supported Signature and Hash algorithms (For TLS 1.2)
 * RFC 5246 section 7.4.1.4.1
 */
#define SSL_HASH_NONE                0
#define SSL_HASH_MD5                 1
#define SSL_HASH_SHA1                2
#define SSL_HASH_SHA224              3
#define SSL_HASH_SHA256              4
#define SSL_HASH_SHA384              5
#define SSL_HASH_SHA512              6

#define SSL_SIG_ANON                 0
#define SSL_SIG_RSA                  1
#define SSL_SIG_ECDSA                3

/*
 * Client Certificate Types
 * RFC 5246 section 7.4.4 plus RFC 4492 section 5.5
 */
#define SSL_CERT_TYPE_RSA_SIGN       1
#define SSL_CERT_TYPE_ECDSA_SIGN    64

/*
 * Message, alert and handshake types
 */
#define SSL_MSG_CHANGE_CIPHER_SPEC     20
#define SSL_MSG_ALERT                  21
#define SSL_MSG_HANDSHAKE              22
#define SSL_MSG_APPLICATION_DATA       23

#define SSL_ALERT_LEVEL_WARNING         1
#define SSL_ALERT_LEVEL_FATAL           2

#define SSL_ALERT_MSG_CLOSE_NOTIFY           0  /* 0x00 */
#define SSL_ALERT_MSG_UNEXPECTED_MESSAGE    10  /* 0x0A */
#define SSL_ALERT_MSG_BAD_RECORD_MAC        20  /* 0x14 */
#define SSL_ALERT_MSG_DECRYPTION_FAILED     21  /* 0x15 */
#define SSL_ALERT_MSG_RECORD_OVERFLOW       22  /* 0x16 */
#define SSL_ALERT_MSG_DECOMPRESSION_FAILURE 30  /* 0x1E */
#define SSL_ALERT_MSG_HANDSHAKE_FAILURE     40  /* 0x28 */
#define SSL_ALERT_MSG_NO_CERT               41  /* 0x29 */
#define SSL_ALERT_MSG_BAD_CERT              42  /* 0x2A */
#define SSL_ALERT_MSG_UNSUPPORTED_CERT      43  /* 0x2B */
#define SSL_ALERT_MSG_CERT_REVOKED          44  /* 0x2C */
#define SSL_ALERT_MSG_CERT_EXPIRED          45  /* 0x2D */
#define SSL_ALERT_MSG_CERT_UNKNOWN          46  /* 0x2E */
#define SSL_ALERT_MSG_ILLEGAL_PARAMETER     47  /* 0x2F */
#define SSL_ALERT_MSG_UNKNOWN_CA            48  /* 0x30 */
#define SSL_ALERT_MSG_ACCESS_DENIED         49  /* 0x31 */
#define SSL_ALERT_MSG_DECODE_ERROR          50  /* 0x32 */
#define SSL_ALERT_MSG_DECRYPT_ERROR         51  /* 0x33 */
#define SSL_ALERT_MSG_EXPORT_RESTRICTION    60  /* 0x3C */
#define SSL_ALERT_MSG_PROTOCOL_VERSION      70  /* 0x46 */
#define SSL_ALERT_MSG_INSUFFICIENT_SECURITY 71  /* 0x47 */
#define SSL_ALERT_MSG_INTERNAL_ERROR        80  /* 0x50 */
#define SSL_ALERT_MSG_INAPROPRIATE_FALLBACK 86  /* 0x56 */
#define SSL_ALERT_MSG_USER_CANCELED         90  /* 0x5A */
#define SSL_ALERT_MSG_NO_RENEGOTIATION     100  /* 0x64 */
#define SSL_ALERT_MSG_UNSUPPORTED_EXT      110  /* 0x6E */
#define SSL_ALERT_MSG_UNRECOGNIZED_NAME    112  /* 0x70 */
#define SSL_ALERT_MSG_UNKNOWN_PSK_IDENTITY 115  /* 0x73 */
#define SSL_ALERT_MSG_NO_APPLICATION_PROTOCOL 120 /* 0x78 */

#define SSL_HS_HELLO_REQUEST            0
#define SSL_HS_CLIENT_HELLO             1
#define SSL_HS_SERVER_HELLO             2
#define SSL_HS_NEW_SESSION_TICKET       4
#define SSL_HS_CERTIFICATE             11
#define SSL_HS_SERVER_KEY_EXCHANGE     12
#define SSL_HS_CERTIFICATE_REQUEST     13
#define SSL_HS_SERVER_HELLO_DONE       14
#define SSL_HS_CERTIFICATE_VERIFY      15
#define SSL_HS_CLIENT_KEY_EXCHANGE     16
#define SSL_HS_FINISHED                20

/*
 * TLS extensions
 */
#define TLS_EXT_SERVERNAME                   0
#define TLS_EXT_SERVERNAME_HOSTNAME          0

#define TLS_EXT_MAX_FRAGMENT_LENGTH          1

#define TLS_EXT_TRUNCATED_HMAC               4

#define TLS_EXT_SUPPORTED_ELLIPTIC_CURVES   10
#define TLS_EXT_SUPPORTED_POINT_FORMATS     11

#define TLS_EXT_SIG_ALG                     13

#define TLS_EXT_ALPN                        16

#define TLS_EXT_ENCRYPT_THEN_MAC            22 /* 0x16 */
#define TLS_EXT_EXTENDED_MASTER_SECRET  0x0017 /* 23 */

#define TLS_EXT_SESSION_TICKET              35

#define TLS_EXT_RENEGOTIATION_INFO      0xFF01

/*
 * TLS extension flags (for extensions with outgoing ServerHello content
 * that need it (e.g. for RENEGOTIATION_INFO the server already knows because
 * of state of the renegotiation flag, so no indicator is required)
 */
#define TLS_EXT_SUPPORTED_POINT_FORMATS_PRESENT (1 << 0)

/*
 * Size defines
 */
#if !defined(POLARSSL_PSK_MAX_LEN)
#define POLARSSL_PSK_MAX_LEN            32 /* 256 bits */
#endif

/* Dummy type used only for its size */
union _ssl_premaster_secret
{
#if defined(POLARSSL_KEY_EXCHANGE_RSA_ENABLED)
    unsigned char _pms_rsa[48];                         /* RFC 5246 8.1.1 */
#endif
#if defined(POLARSSL_KEY_EXCHANGE_DHE_RSA_ENABLED)
    unsigned char _pms_dhm[POLARSSL_MPI_MAX_SIZE];      /* RFC 5246 8.1.2 */
#endif
#if defined(POLARSSL_KEY_EXCHANGE_ECDHE_RSA_ENABLED)    || \
    defined(POLARSSL_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED)  || \
    defined(POLARSSL_KEY_EXCHANGE_ECDH_RSA_ENABLED)     || \
    defined(POLARSSL_KEY_EXCHANGE_ECDH_ECDSA_ENABLED)
    unsigned char _pms_ecdh[POLARSSL_ECP_MAX_BYTES];    /* RFC 4492 5.10 */
#endif
#if defined(POLARSSL_KEY_EXCHANGE_PSK_ENABLED)
    unsigned char _pms_psk[4 + 2 * POLARSSL_PSK_MAX_LEN];       /* RFC 4279 2 */
#endif
#if defined(POLARSSL_KEY_EXCHANGE_DHE_PSK_ENABLED)
    unsigned char _pms_dhe_psk[4 + POLARSSL_MPI_MAX_SIZE
                                 + POLARSSL_PSK_MAX_LEN];       /* RFC 4279 3 */
#endif
#if defined(POLARSSL_KEY_EXCHANGE_RSA_PSK_ENABLED)
    unsigned char _pms_rsa_psk[52 + POLARSSL_PSK_MAX_LEN];      /* RFC 4279 4 */
#endif
#if defined(POLARSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED)
    unsigned char _pms_ecdhe_psk[4 + POLARSSL_ECP_MAX_BYTES
                                   + POLARSSL_PSK_MAX_LEN];     /* RFC 5489 2 */
#endif
};

#define POLARSSL_PREMASTER_SIZE     sizeof( union _ssl_premaster_secret )

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Generic function pointers for allowing external RSA private key
 * implementations.
 */
typedef int (*rsa_decrypt_func)( void *ctx, int mode, size_t *olen,
                        const unsigned char *input, unsigned char *output,
                        size_t output_max_len );
typedef int (*rsa_sign_func)( void *ctx,
                     int (*f_rng)(void *, unsigned char *, size_t), void *p_rng,
                     int mode, md_type_t md_alg, unsigned int hashlen,
                     const unsigned char *hash, unsigned char *sig );
typedef size_t (*rsa_key_len_func)( void *ctx );

/*
 * SSL state machine
 */
typedef enum
{
    SSL_HELLO_REQUEST,
    SSL_CLIENT_HELLO,
    SSL_SERVER_HELLO,
    SSL_SERVER_CERTIFICATE,
    SSL_SERVER_KEY_EXCHANGE,
    SSL_CERTIFICATE_REQUEST,
    SSL_SERVER_HELLO_DONE,
    SSL_CLIENT_CERTIFICATE,
    SSL_CLIENT_KEY_EXCHANGE,
    SSL_CERTIFICATE_VERIFY,
    SSL_CLIENT_CHANGE_CIPHER_SPEC,
    SSL_CLIENT_FINISHED,
    SSL_SERVER_CHANGE_CIPHER_SPEC,
    SSL_SERVER_FINISHED,
    SSL_FLUSH_BUFFERS,
    SSL_HANDSHAKE_WRAPUP,
    SSL_HANDSHAKE_OVER,
    SSL_SERVER_NEW_SESSION_TICKET,
}
ssl_states;

typedef struct _ssl_session ssl_session;
typedef struct _ssl_context ssl_context;
typedef struct _ssl_transform ssl_transform;
typedef struct _ssl_handshake_params ssl_handshake_params;
#if defined(POLARSSL_SSL_SESSION_TICKETS)
typedef struct _ssl_ticket_keys ssl_ticket_keys;
#endif
#if defined(POLARSSL_X509_CRT_PARSE_C)
typedef struct _ssl_key_cert ssl_key_cert;
#endif

/*
 * This structure is used for storing current session data.
 */
struct _ssl_session
{
#if defined(POLARSSL_HAVE_TIME)
    time_t start;               /*!< starting time      */
#endif
    int ciphersuite;            /*!< chosen ciphersuite */
    int compression;            /*!< chosen compression */
    size_t length;              /*!< session id length  */
    unsigned char id[32];       /*!< session identifier */
    unsigned char master[48];   /*!< the master secret  */

#if defined(POLARSSL_X509_CRT_PARSE_C)
    x509_crt *peer_cert;        /*!< peer X.509 cert chain */
#endif /* POLARSSL_X509_CRT_PARSE_C */
    int verify_result;          /*!<  verification result     */

#if defined(POLARSSL_SSL_SESSION_TICKETS)
    unsigned char *ticket;      /*!< RFC 5077 session ticket */
    size_t ticket_len;          /*!< session ticket length   */
    uint32_t ticket_lifetime;   /*!< ticket lifetime hint    */
#endif /* POLARSSL_SSL_SESSION_TICKETS */

#if defined(POLARSSL_SSL_MAX_FRAGMENT_LENGTH)
    unsigned char mfl_code;     /*!< MaxFragmentLength negotiated by peer */
#endif /* POLARSSL_SSL_MAX_FRAGMENT_LENGTH */

#if defined(POLARSSL_SSL_TRUNCATED_HMAC)
    int trunc_hmac;             /*!< flag for truncated hmac activation   */
#endif /* POLARSSL_SSL_TRUNCATED_HMAC */

#if defined(POLARSSL_SSL_ENCRYPT_THEN_MAC)
    int encrypt_then_mac;       /*!< flag for EtM activation                */
#endif
};

/*
 * This structure contains a full set of runtime transform parameters
 * either in negotiation or active.
 */
struct _ssl_transform
{
    /*
     * Session specific crypto layer
     */
    const ssl_ciphersuite_t *ciphersuite_info;
                                        /*!<  Chosen cipersuite_info  */
    unsigned int keylen;                /*!<  symmetric key length    */
    size_t minlen;                      /*!<  min. ciphertext length  */
    size_t ivlen;                       /*!<  IV length               */
    size_t fixed_ivlen;                 /*!<  Fixed part of IV (AEAD) */
    size_t maclen;                      /*!<  MAC length              */

    unsigned char iv_enc[16];           /*!<  IV (encryption)         */
    unsigned char iv_dec[16];           /*!<  IV (decryption)         */

#if defined(POLARSSL_SSL_PROTO_SSL3)
    /* Needed only for SSL v3.0 secret */
    unsigned char mac_enc[20];          /*!<  SSL v3.0 secret (enc)   */
    unsigned char mac_dec[20];          /*!<  SSL v3.0 secret (dec)   */
#endif /* POLARSSL_SSL_PROTO_SSL3 */

    md_context_t md_ctx_enc;            /*!<  MAC (encryption)        */
    md_context_t md_ctx_dec;            /*!<  MAC (decryption)        */

    cipher_context_t cipher_ctx_enc;    /*!<  encryption context      */
    cipher_context_t cipher_ctx_dec;    /*!<  decryption context      */

    /*
     * Session specific compression layer
     */
#if defined(POLARSSL_ZLIB_SUPPORT)
    z_stream ctx_deflate;               /*!<  compression context     */
    z_stream ctx_inflate;               /*!<  decompression context   */
#endif
};

/*
 * This structure contains the parameters only needed during handshake.
 */
struct _ssl_handshake_params
{
    /*
     * Handshake specific crypto variables
     */
    int sig_alg;                        /*!<  Hash algorithm for signature   */
    int cert_type;                      /*!<  Requested cert type            */
    int verify_sig_alg;                 /*!<  Signature algorithm for verify */
#if defined(POLARSSL_DHM_C)
    dhm_context dhm_ctx;                /*!<  DHM key exchange        */
#endif
#if defined(POLARSSL_ECDH_C)
    ecdh_context ecdh_ctx;              /*!<  ECDH key exchange       */
#endif
#if defined(POLARSSL_ECDH_C) || defined(POLARSSL_ECDSA_C)
    const ecp_curve_info **curves;      /*!<  Supported elliptic curves */
#endif
#if defined(POLARSSL_X509_CRT_PARSE_C)
    /**
     * Current key/cert or key/cert list.
     * On client: pointer to ssl->key_cert, only the first entry used.
     * On server: starts as a pointer to ssl->key_cert, then becomes
     * a pointer to the chosen key from this list or the SNI list.
     */
    ssl_key_cert *key_cert;
#if defined(POLARSSL_SSL_SERVER_NAME_INDICATION)
    ssl_key_cert *sni_key_cert;         /*!<  key/cert list from SNI  */
#endif
#endif /* POLARSSL_X509_CRT_PARSE_C */

    /*
     * Checksum contexts
     */
#if defined(POLARSSL_SSL_PROTO_SSL3) || defined(POLARSSL_SSL_PROTO_TLS1) || \
    defined(POLARSSL_SSL_PROTO_TLS1_1)
       md5_context fin_md5;
      sha1_context fin_sha1;
#endif
#if defined(POLARSSL_SSL_PROTO_TLS1_2)
#if defined(POLARSSL_SHA256_C)
    sha256_context fin_sha256;
#endif
#if defined(POLARSSL_SHA512_C)
    sha512_context fin_sha512;
#endif
#endif /* POLARSSL_SSL_PROTO_TLS1_2 */

    void (*update_checksum)(ssl_context *, const unsigned char *, size_t);
    void (*calc_verify)(ssl_context *, unsigned char *);
    void (*calc_finished)(ssl_context *, unsigned char *, int);
    int  (*tls_prf)(const unsigned char *, size_t, const char *,
                    const unsigned char *, size_t,
                    unsigned char *, size_t);

    size_t pmslen;                      /*!<  premaster length        */

    unsigned char randbytes[64];        /*!<  random bytes            */
    unsigned char premaster[POLARSSL_PREMASTER_SIZE];
                                        /*!<  premaster secret        */

    int resume;                         /*!<  session resume indicator*/
    int max_major_ver;                  /*!< max. major version client*/
    int max_minor_ver;                  /*!< max. minor version client*/
    int cli_exts;                       /*!< client extension presence*/

#if defined(POLARSSL_SSL_SESSION_TICKETS)
    int new_session_ticket;             /*!< use NewSessionTicket?    */
#endif /* POLARSSL_SSL_SESSION_TICKETS */
#if defined(POLARSSL_SSL_EXTENDED_MASTER_SECRET)
    int extended_ms;                    /*!< use Extended Master Secret? */
#endif
};

#if defined(POLARSSL_SSL_SESSION_TICKETS)
/*
 * Parameters needed to secure session tickets
 */
struct _ssl_ticket_keys
{
    unsigned char key_name[16];     /*!< name to quickly discard bad tickets */
    aes_context enc;                /*!< encryption context                  */
    aes_context dec;                /*!< decryption context                  */
    unsigned char mac_key[16];      /*!< authentication key                  */
};
#endif /* POLARSSL_SSL_SESSION_TICKETS */

#if defined(POLARSSL_X509_CRT_PARSE_C)
/*
 * List of certificate + private key pairs
 */
struct _ssl_key_cert
{
    x509_crt *cert;                 /*!< cert                       */
    pk_context *key;                /*!< private key                */
    int key_own_alloc;              /*!< did we allocate key?       */
    ssl_key_cert *next;             /*!< next key/cert pair         */
};
#endif /* POLARSSL_X509_CRT_PARSE_C */

struct _ssl_context
{
    /*
     * Miscellaneous
     */
    int state;                  /*!< SSL handshake: current state     */
    int renegotiation;          /*!< Initial or renegotiation         */
#if defined(POLARSSL_SSL_RENEGOTIATION)
    int renego_records_seen;    /*!< Records since renego request     */
#endif

    int major_ver;              /*!< equal to  SSL_MAJOR_VERSION_3    */
    int minor_ver;              /*!< either 0 (SSL3) or 1 (TLS1.0)    */

    int max_major_ver;          /*!< max. major version used          */
    int max_minor_ver;          /*!< max. minor version used          */
    int min_major_ver;          /*!< min. major version used          */
    int min_minor_ver;          /*!< min. minor version used          */

#if defined(POLARSSL_SSL_FALLBACK_SCSV) && defined(POLARSSL_SSL_CLI_C)
    char fallback;              /*!< flag for fallback connections    */
#endif
#if defined(POLARSSL_SSL_ENCRYPT_THEN_MAC)
    char encrypt_then_mac;      /*!< flag for encrypt-then-mac        */
#endif
#if defined(POLARSSL_SSL_EXTENDED_MASTER_SECRET)
    char extended_ms;           /*!< flag for extended master secret  */
#endif
    char arc4_disabled;         /*!< flag for disabling RC4           */

    /*
     * Callbacks (RNG, debug, I/O, verification)
     */
    int  (*f_rng)(void *, unsigned char *, size_t);
    void (*f_dbg)(void *, int, const char *);
    int (*f_recv)(void *, unsigned char *, size_t);
    int (*f_send)(void *, const unsigned char *, size_t);
    int (*f_get_cache)(void *, ssl_session *);
    int (*f_set_cache)(void *, const ssl_session *);

    void *p_rng;                /*!< context for the RNG function     */
    void *p_dbg;                /*!< context for the debug function   */
    void *p_recv;               /*!< context for reading operations   */
    void *p_send;               /*!< context for writing operations   */
    void *p_get_cache;          /*!< context for cache retrieval      */
    void *p_set_cache;          /*!< context for cache store          */
    void *p_hw_data;            /*!< context for HW acceleration      */

#if defined(POLARSSL_SSL_SERVER_NAME_INDICATION)
    int (*f_sni)(void *, ssl_context *, const unsigned char *, size_t);
    void *p_sni;                /*!< context for SNI extension        */
#endif

#if defined(POLARSSL_X509_CRT_PARSE_C)
    int (*f_vrfy)(void *, x509_crt *, int, int *);
    void *p_vrfy;               /*!< context for verification         */
#endif

#if defined(POLARSSL_KEY_EXCHANGE__SOME__PSK_ENABLED)
    int (*f_psk)(void *, ssl_context *, const unsigned char *, size_t);
    void *p_psk;               /*!< context for PSK retrieval         */
#endif

    /*
     * Session layer
     */
    ssl_session *session_in;            /*!<  current session data (in)   */
    ssl_session *session_out;           /*!<  current session data (out)  */
    ssl_session *session;               /*!<  negotiated session data     */
    ssl_session *session_negotiate;     /*!<  session data in negotiation */

    ssl_handshake_params *handshake;    /*!<  params required only during
                                              the handshake process        */

    /*
     * Record layer transformations
     */
    ssl_transform *transform_in;        /*!<  current transform params (in)   */
    ssl_transform *transform_out;       /*!<  current transform params (in)   */
    ssl_transform *transform;           /*!<  negotiated transform params     */
    ssl_transform *transform_negotiate; /*!<  transform params in negotiation */

    /*
     * Record layer (incoming data)
     */
    unsigned char *in_ctr;      /*!< 64-bit incoming message counter  */
    unsigned char *in_hdr;      /*!< 5-byte record header (in_ctr+8)  */
    unsigned char *in_iv;       /*!< ivlen-byte IV (in_hdr+5)         */
    unsigned char *in_msg;      /*!< message contents (in_iv+ivlen)   */
    unsigned char *in_offt;     /*!< read offset in application data  */

    int in_msgtype;             /*!< record header: message type      */
    size_t in_msglen;           /*!< record header: message length    */
    size_t in_left;             /*!< amount of data read so far       */

    size_t in_hslen;            /*!< current handshake message length */
    int nb_zero;                /*!< # of 0-length encrypted messages */
    int record_read;            /*!< record is already present        */

    /*
     * Record layer (outgoing data)
     */
    unsigned char *out_ctr;     /*!< 64-bit outgoing message counter  */
    unsigned char *out_hdr;     /*!< 5-byte record header (out_ctr+8) */
    unsigned char *out_iv;      /*!< ivlen-byte IV (out_hdr+5)        */
    unsigned char *out_msg;     /*!< message contents (out_iv+ivlen)  */

    int out_msgtype;            /*!< record header: message type      */
    size_t out_msglen;          /*!< record header: message length    */
    size_t out_left;            /*!< amount of data not yet written   */

#if defined(POLARSSL_ZLIB_SUPPORT)
    unsigned char *compress_buf;        /*!<  zlib data buffer        */
#endif
#if defined(POLARSSL_SSL_MAX_FRAGMENT_LENGTH)
    unsigned char mfl_code;     /*!< MaxFragmentLength chosen by us   */
#endif /* POLARSSL_SSL_MAX_FRAGMENT_LENGTH */
#if defined(POLARSSL_SSL_CBC_RECORD_SPLITTING)
    signed char split_done;     /*!< flag for record splitting:
                                     -1 disabled, 0 todo, 1 done      */
#endif

    /*
     * PKI layer
     */
#if defined(POLARSSL_X509_CRT_PARSE_C)
    ssl_key_cert *key_cert;             /*!<  own certificate(s)/key(s) */

    x509_crt *ca_chain;                 /*!<  own trusted CA chain      */
    x509_crl *ca_crl;                   /*!<  trusted CA CRLs           */
    const char *peer_cn;                /*!<  expected peer CN          */
#endif /* POLARSSL_X509_CRT_PARSE_C */

    /*
     * Support for generating and checking session tickets
     */
#if defined(POLARSSL_SSL_SESSION_TICKETS)
    ssl_ticket_keys *ticket_keys;       /*!<  keys for ticket encryption */
#endif /* POLARSSL_SSL_SESSION_TICKETS */

    /*
     * User settings
     */
    int endpoint;                       /*!<  0: client, 1: server    */
    int authmode;                       /*!<  verification mode       */
    int client_auth;                    /*!<  flag for client auth.   */
    int verify_result;                  /*!<  verification result     */
#if defined(POLARSSL_SSL_RENEGOTIATION)
    int disable_renegotiation;          /*!<  enable/disable renegotiation   */
    int renego_max_records;             /*!<  grace period for renegotiation */
    unsigned char renego_period[8];     /*!<  value of the record counters
                                              that triggers renegotiation    */
#endif
    int allow_legacy_renegotiation;     /*!<  allow legacy renegotiation     */
    const int *ciphersuite_list[4];     /*!<  allowed ciphersuites / version */
#if defined(POLARSSL_SSL_SET_CURVES)
    const ecp_group_id *curve_list;     /*!<  allowed curves                 */
#endif
#if defined(POLARSSL_SSL_TRUNCATED_HMAC)
    int trunc_hmac;                     /*!<  negotiate truncated hmac?      */
#endif
#if defined(POLARSSL_SSL_SESSION_TICKETS)
    int session_tickets;                /*!<  use session tickets?    */
    int ticket_lifetime;                /*!<  session ticket lifetime */
#endif

#if defined(POLARSSL_DHM_C)
    mpi dhm_P;                          /*!<  prime modulus for DHM   */
    mpi dhm_G;                          /*!<  generator for DHM       */
#endif

#if defined(POLARSSL_KEY_EXCHANGE__SOME__PSK_ENABLED)
    /*
     * PSK values
     */
    unsigned char *psk;
    size_t         psk_len;
    unsigned char *psk_identity;
    size_t         psk_identity_len;
#endif

#if defined(POLARSSL_SSL_SERVER_NAME_INDICATION)
    /*
     * SNI extension
     */
    unsigned char *hostname;
    size_t         hostname_len;
#endif

#if defined(POLARSSL_SSL_ALPN)
    /*
     * ALPN extension
     */
    const char **alpn_list;     /*!<  ordered list of supported protocols   */
    const char *alpn_chosen;    /*!<  negotiated protocol                   */
#endif

    /*
     * Secure renegotiation
     */
    int secure_renegotiation;           /*!<  does peer support legacy or
                                              secure renegotiation           */
#if defined(POLARSSL_SSL_RENEGOTIATION)
    size_t verify_data_len;             /*!<  length of verify data stored   */
    char own_verify_data[SSL_VERIFY_DATA_MAX_LEN]; /*!<  previous handshake verify data */
    char peer_verify_data[SSL_VERIFY_DATA_MAX_LEN]; /*!<  previous handshake verify data */
#endif
};

#if defined(POLARSSL_SSL_HW_RECORD_ACCEL)

#define SSL_CHANNEL_OUTBOUND    0
#define SSL_CHANNEL_INBOUND     1

extern int (*ssl_hw_record_init)(ssl_context *ssl,
                const unsigned char *key_enc, const unsigned char *key_dec,
                size_t keylen,
                const unsigned char *iv_enc,  const unsigned char *iv_dec,
                size_t ivlen,
                const unsigned char *mac_enc, const unsigned char *mac_dec,
                size_t maclen);
extern int (*ssl_hw_record_activate)(ssl_context *ssl, int direction);
extern int (*ssl_hw_record_reset)(ssl_context *ssl);
extern int (*ssl_hw_record_write)(ssl_context *ssl);
extern int (*ssl_hw_record_read)(ssl_context *ssl);
extern int (*ssl_hw_record_finish)(ssl_context *ssl);
#endif /* POLARSSL_SSL_HW_RECORD_ACCEL */

/**
 * \brief Returns the list of ciphersuites supported by the SSL/TLS module.
 *
 * \return              a statically allocated array of ciphersuites, the last
 *                      entry is 0.
 */
const int *ssl_list_ciphersuites( void );

/**
 * \brief               Return the name of the ciphersuite associated with the
 *                      given ID
 *
 * \param ciphersuite_id SSL ciphersuite ID
 *
 * \return              a string containing the ciphersuite name
 */
const char *ssl_get_ciphersuite_name( const int ciphersuite_id );

/**
 * \brief               Return the ID of the ciphersuite associated with the
 *                      given name
 *
 * \param ciphersuite_name SSL ciphersuite name
 *
 * \return              the ID with the ciphersuite or 0 if not found
 */
int ssl_get_ciphersuite_id( const char *ciphersuite_name );

/**
 * \brief          Initialize an SSL context
 *                 (An individual SSL context is not thread-safe)
 *
 * \param ssl      SSL context
 *
 * \return         0 if successful, or POLARSSL_ERR_SSL_MALLOC_FAILED if
 *                 memory allocation failed
 */
int ssl_init( ssl_context *ssl );

/**
 * \brief          Reset an already initialized SSL context for re-use
 *                 while retaining application-set variables, function
 *                 pointers and data.
 *
 * \param ssl      SSL context
 * \return         0 if successful, or POLASSL_ERR_SSL_MALLOC_FAILED,
                   POLARSSL_ERR_SSL_HW_ACCEL_FAILED or
 *                 POLARSSL_ERR_SSL_COMPRESSION_FAILED
 */
int ssl_session_reset( ssl_context *ssl );

/**
 * \brief          Set the current endpoint type
 *
 * \param ssl      SSL context
 * \param endpoint must be SSL_IS_CLIENT or SSL_IS_SERVER
 *
 * \note           This function should be called right after ssl_init() since
 *                 some other ssl_set_foo() functions depend on it.
 */
void ssl_set_endpoint( ssl_context *ssl, int endpoint );

/**
 * \brief          Set the certificate verification mode
 *
 * \param ssl      SSL context
 * \param authmode can be:
 *
 *  SSL_VERIFY_NONE:      peer certificate is not checked (default),
 *                        this is insecure and SHOULD be avoided.
 *
 *  SSL_VERIFY_OPTIONAL:  peer certificate is checked, however the
 *                        handshake continues even if verification failed;
 *                        ssl_get_verify_result() can be called after the
 *                        handshake is complete.
 *
 *  SSL_VERIFY_REQUIRED:  peer *must* present a valid certificate,
 *                        handshake is aborted if verification failed.
 *
 * \note On client, SSL_VERIFY_REQUIRED is the recommended mode.
 * With SSL_VERIFY_OPTIONAL, the user needs to call ssl_get_verify_result() at
 * the right time(s), which may not be obvious, while REQUIRED always perform
 * the verification as soon as possible. For example, REQUIRED was protecting
 * against the "triple handshake" attack even before it was found.
 */
void ssl_set_authmode( ssl_context *ssl, int authmode );

#if defined(POLARSSL_X509_CRT_PARSE_C)
/**
 * \brief          Set the verification callback (Optional).
 *
 *                 If set, the verify callback is called for each
 *                 certificate in the chain. For implementation
 *                 information, please see \c x509parse_verify()
 *
 * \param ssl      SSL context
 * \param f_vrfy   verification function
 * \param p_vrfy   verification parameter
 */
void ssl_set_verify( ssl_context *ssl,
                     int (*f_vrfy)(void *, x509_crt *, int, int *),
                     void *p_vrfy );
#endif /* POLARSSL_X509_CRT_PARSE_C */

/**
 * \brief          Set the random number generator callback
 *
 * \param ssl      SSL context
 * \param f_rng    RNG function
 * \param p_rng    RNG parameter
 */
void ssl_set_rng( ssl_context *ssl,
                  int (*f_rng)(void *, unsigned char *, size_t),
                  void *p_rng );

/**
 * \brief          Set the debug callback
 *
 * \param ssl      SSL context
 * \param f_dbg    debug function
 * \param p_dbg    debug parameter
 */
void ssl_set_dbg( ssl_context *ssl,
                  void (*f_dbg)(void *, int, const char *),
                  void  *p_dbg );

/**
 * \brief          Set the underlying BIO read and write callbacks
 *
 * \param ssl      SSL context
 * \param f_recv   read callback
 * \param p_recv   read parameter
 * \param f_send   write callback
 * \param p_send   write parameter
 */
void ssl_set_bio( ssl_context *ssl,
        int (*f_recv)(void *, unsigned char *, size_t), void *p_recv,
        int (*f_send)(void *, const unsigned char *, size_t), void *p_send );

#if defined(POLARSSL_SSL_SRV_C)
/**
 * \brief          Set the session cache callbacks (server-side only)
 *                 If not set, no session resuming is done (except if session
 *                 tickets are enabled too).
 *
 *                 The session cache has the responsibility to check for stale
 *                 entries based on timeout. See RFC 5246 for recommendations.
 *
 *                 Warning: session.peer_cert is cleared by the SSL/TLS layer on
 *                 connection shutdown, so do not cache the pointer! Either set
 *                 it to NULL or make a full copy of the certificate.
 *
 *                 The get callback is called once during the initial handshake
 *                 to enable session resuming. The get function has the
 *                 following parameters: (void *parameter, ssl_session *session)
 *                 If a valid entry is found, it should fill the master of
 *                 the session object with the cached values and return 0,
 *                 return 1 otherwise. Optionally peer_cert can be set as well
 *                 if it is properly present in cache entry.
 *
 *                 The set callback is called once during the initial handshake
 *                 to enable session resuming after the entire handshake has
 *                 been finished. The set function has the following parameters:
 *                 (void *parameter, const ssl_session *session). The function
 *                 should create a cache entry for future retrieval based on
 *                 the data in the session structure and should keep in mind
 *                 that the ssl_session object presented (and all its referenced
 *                 data) is cleared by the SSL/TLS layer when the connection is
 *                 terminated. It is recommended to add metadata to determine if
 *                 an entry is still valid in the future. Return 0 if
 *                 successfully cached, return 1 otherwise.
 *
 * \param ssl            SSL context
 * \param f_get_cache    session get callback
 * \param p_get_cache    session get parameter
 * \param f_set_cache    session set callback
 * \param p_set_cache    session set parameter
 */
void ssl_set_session_cache( ssl_context *ssl,
        int (*f_get_cache)(void *, ssl_session *), void *p_get_cache,
        int (*f_set_cache)(void *, const ssl_session *), void *p_set_cache );
#endif /* POLARSSL_SSL_SRV_C */

#if defined(POLARSSL_SSL_CLI_C)
/**
 * \brief          Request resumption of session (client-side only)
 *                 Session data is copied from presented session structure.
 *
 * \param ssl      SSL context
 * \param session  session context
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_SSL_MALLOC_FAILED if memory allocation failed,
 *                 POLARSSL_ERR_SSL_BAD_INPUT_DATA if used server-side or
 *                 arguments are otherwise invalid
 *
 * \sa             ssl_get_session()
 */
int ssl_set_session( ssl_context *ssl, const ssl_session *session );
#endif /* POLARSSL_SSL_CLI_C */

/**
 * \brief               Set the list of allowed ciphersuites and the preference
 *                      order. First in the list has the highest preference.
 *                      (Overrides all version specific lists)
 *
 *                      The ciphersuites array is not copied, and must remain
 *                      valid for the lifetime of the ssl_context.
 *
 *                      Note: The server uses its own preferences
 *                      over the preference of the client unless
 *                      POLARSSL_SSL_SRV_RESPECT_CLIENT_PREFERENCE is defined!
 *
 * \param ssl           SSL context
 * \param ciphersuites  0-terminated list of allowed ciphersuites
 */
void ssl_set_ciphersuites( ssl_context *ssl, const int *ciphersuites );

/**
 * \brief               Set the list of allowed ciphersuites and the
 *                      preference order for a specific version of the protocol.
 *                      (Only useful on the server side)
 *
 * \param ssl           SSL context
 * \param ciphersuites  0-terminated list of allowed ciphersuites
 * \param major         Major version number (only SSL_MAJOR_VERSION_3
 *                      supported)
 * \param minor         Minor version number (SSL_MINOR_VERSION_0,
 *                      SSL_MINOR_VERSION_1 and SSL_MINOR_VERSION_2,
 *                      SSL_MINOR_VERSION_3 supported)
 */
void ssl_set_ciphersuites_for_version( ssl_context *ssl,
                                       const int *ciphersuites,
                                       int major, int minor );

#if defined(POLARSSL_X509_CRT_PARSE_C)
/**
 * \brief          Set the data required to verify peer certificate
 *
 * \param ssl      SSL context
 * \param ca_chain trusted CA chain (meaning all fully trusted top-level CAs)
 * \param ca_crl   trusted CA CRLs
 * \param peer_cn  expected peer CommonName (or NULL)
 */
void ssl_set_ca_chain( ssl_context *ssl, x509_crt *ca_chain,
                       x509_crl *ca_crl, const char *peer_cn );

/**
 * \brief          Set own certificate chain and private key
 *
 * \note           own_cert should contain in order from the bottom up your
 *                 certificate chain. The top certificate (self-signed)
 *                 can be omitted.
 *
 * \note           This function may be called more than once if you want to
 *                 support multiple certificates (eg, one using RSA and one
 *                 using ECDSA). However, on client, currently only the first
 *                 certificate is used (subsequent calls have no effect).
 *
 * \param ssl      SSL context
 * \param own_cert own public certificate chain
 * \param pk_key   own private key
 *
 * \return         0 on success or POLARSSL_ERR_SSL_MALLOC_FAILED
 */
int ssl_set_own_cert( ssl_context *ssl, x509_crt *own_cert,
                       pk_context *pk_key );

#if ! defined(POLARSSL_DEPRECATED_REMOVED)
#if defined(POLARSSL_DEPRECATED_WARNING)
#define DEPRECATED    __attribute__((deprecated))
#else
#define DEPRECATED
#endif
#if defined(POLARSSL_RSA_C)
/**
 * \brief          Set own certificate chain and private RSA key
 *
 *                 Note: own_cert should contain IN order from the bottom
 *                 up your certificate chain. The top certificate (self-signed)
 *                 can be omitted.
 *
 * \deprecated     Please use \c ssl_set_own_cert() instead.
 *
 * \param ssl      SSL context
 * \param own_cert own public certificate chain
 * \param rsa_key  own private RSA key
 *
 * \return          0 on success, or a specific error code.
 */
int ssl_set_own_cert_rsa( ssl_context *ssl, x509_crt *own_cert,
                          rsa_context *rsa_key ) DEPRECATED;
#endif /* POLARSSL_RSA_C */

/**
 * \brief          Set own certificate and external RSA private
 *                 key and handling callbacks, such as the PKCS#11 wrappers
 *                 or any other external private key handler.
 *                 (see the respective RSA functions in rsa.h for documentation
 *                 of the callback parameters, with the only change being
 *                 that the rsa_context * is a void * in the callbacks)
 *
 *                 Note: own_cert should contain IN order from the bottom
 *                 up your certificate chain. The top certificate (self-signed)
 *                 can be omitted.
 *
 * \deprecated     Please use \c pk_init_ctx_rsa_alt()
 *                 and \c ssl_set_own_cert() instead.
 *
 * \param ssl      SSL context
 * \param own_cert own public certificate chain
 * \param rsa_key  alternate implementation private RSA key
 * \param rsa_decrypt  alternate implementation of \c rsa_pkcs1_decrypt()
 * \param rsa_sign     alternate implementation of \c rsa_pkcs1_sign()
 * \param rsa_key_len  function returning length of RSA key in bytes
 *
 * \return          0 on success, or a specific error code.
 */
int ssl_set_own_cert_alt( ssl_context *ssl, x509_crt *own_cert,
                          void *rsa_key,
                          rsa_decrypt_func rsa_decrypt,
                          rsa_sign_func rsa_sign,
                          rsa_key_len_func rsa_key_len ) DEPRECATED;
#undef DEPRECATED
#endif /* POLARSSL_DEPRECATED_REMOVED */
#endif /* POLARSSL_X509_CRT_PARSE_C */

#if defined(POLARSSL_KEY_EXCHANGE__SOME__PSK_ENABLED)
/**
 * \brief          Set the Pre Shared Key (PSK) and the identity name connected
 *                 to it.
 *
 * \param ssl      SSL context
 * \param psk      pointer to the pre-shared key
 * \param psk_len  pre-shared key length
 * \param psk_identity      pointer to the pre-shared key identity
 * \param psk_identity_len  identity key length
 *
 * \return         0 if successful or POLARSSL_ERR_SSL_MALLOC_FAILED
 */
int ssl_set_psk( ssl_context *ssl, const unsigned char *psk, size_t psk_len,
                 const unsigned char *psk_identity, size_t psk_identity_len );

/**
 * \brief          Set the PSK callback (server-side only) (Optional).
 *
 *                 If set, the PSK callback is called for each
 *                 handshake where a PSK ciphersuite was negotiated.
 *                 The caller provides the identity received and wants to
 *                 receive the actual PSK data and length.
 *
 *                 The callback has the following parameters: (void *parameter,
 *                 ssl_context *ssl, const unsigned char *psk_identity,
 *                 size_t identity_len)
 *                 If a valid PSK identity is found, the callback should use
 *                 ssl_set_psk() on the ssl context to set the correct PSK and
 *                 identity and return 0.
 *                 Any other return value will result in a denied PSK identity.
 *
 * \param ssl      SSL context
 * \param f_psk    PSK identity function
 * \param p_psk    PSK identity parameter
 */
void ssl_set_psk_cb( ssl_context *ssl,
                     int (*f_psk)(void *, ssl_context *, const unsigned char *,
                                  size_t),
                     void *p_psk );
#endif /* POLARSSL_KEY_EXCHANGE__SOME__PSK_ENABLED */

#if defined(POLARSSL_DHM_C)
/**
 * \brief          Set the Diffie-Hellman public P and G values,
 *                 read as hexadecimal strings (server-side only)
 *                 (Default: POLARSSL_DHM_RFC5114_MODP_1024_[PG])
 *
 * \param ssl      SSL context
 * \param dhm_P    Diffie-Hellman-Merkle modulus
 * \param dhm_G    Diffie-Hellman-Merkle generator
 *
 * \return         0 if successful
 */
int ssl_set_dh_param( ssl_context *ssl, const char *dhm_P, const char *dhm_G );

/**
 * \brief          Set the Diffie-Hellman public P and G values,
 *                 read from existing context (server-side only)
 *
 * \param ssl      SSL context
 * \param dhm_ctx  Diffie-Hellman-Merkle context
 *
 * \return         0 if successful
 */
int ssl_set_dh_param_ctx( ssl_context *ssl, dhm_context *dhm_ctx );
#endif /* POLARSSL_DHM_C */

#if defined(POLARSSL_SSL_SET_CURVES)
/**
 * \brief          Set the allowed curves in order of preference.
 *                 (Default: all defined curves.)
 *
 *                 On server: this only affects selection of the ECDHE curve;
 *                 the curves used for ECDH and ECDSA are determined by the
 *                 list of available certificates instead.
 *
 *                 On client: this affects the list of curves offered for any
 *                 use. The server can override our preference order.
 *
 *                 Both sides: limits the set of curves used by peer to the
 *                 listed curves for any use (ECDH(E), certificates).
 *
 * \param ssl      SSL context
 * \param curves   Ordered list of allowed curves,
 *                 terminated by POLARSSL_ECP_DP_NONE.
 */
void ssl_set_curves( ssl_context *ssl, const ecp_group_id *curves );
#endif /* POLARSSL_SSL_SET_CURVES */

#if defined(POLARSSL_SSL_SERVER_NAME_INDICATION)
/**
 * \brief          Set hostname for ServerName TLS extension
 *                 (client-side only)
 *
 *
 * \param ssl      SSL context
 * \param hostname the server hostname
 *
 * \return         0 if successful or POLARSSL_ERR_SSL_MALLOC_FAILED
 */
int ssl_set_hostname( ssl_context *ssl, const char *hostname );

/**
 * \brief          Set server side ServerName TLS extension callback
 *                 (optional, server-side only).
 *
 *                 If set, the ServerName callback is called whenever the
 *                 server receives a ServerName TLS extension from the client
 *                 during a handshake. The ServerName callback has the
 *                 following parameters: (void *parameter, ssl_context *ssl,
 *                 const unsigned char *hostname, size_t len). If a suitable
 *                 certificate is found, the callback should set the
 *                 certificate and key to use with ssl_set_own_cert() (and
 *                 possibly adjust the CA chain as well) and return 0. The
 *                 callback should return -1 to abort the handshake at this
 *                 point.
 *
 * \param ssl      SSL context
 * \param f_sni    verification function
 * \param p_sni    verification parameter
 */
void ssl_set_sni( ssl_context *ssl,
                  int (*f_sni)(void *, ssl_context *, const unsigned char *,
                               size_t),
                  void *p_sni );
#endif /* POLARSSL_SSL_SERVER_NAME_INDICATION */

#if defined(POLARSSL_SSL_ALPN)
/**
 * \brief          Set the supported Application Layer Protocols.
 *
 * \param ssl      SSL context
 * \param protos   NULL-terminated list of supported protocols,
 *                 in decreasing preference order.
 *
 * \return         0 on success, or POLARSSL_ERR_SSL_BAD_INPUT_DATA.
 */
int ssl_set_alpn_protocols( ssl_context *ssl, const char **protos );

/**
 * \brief          Get the name of the negotiated Application Layer Protocol.
 *                 This function should be called after the handshake is
 *                 completed.
 *
 * \param ssl      SSL context
 *
 * \return         Protcol name, or NULL if no protocol was negotiated.
 */
const char *ssl_get_alpn_protocol( const ssl_context *ssl );
#endif /* POLARSSL_SSL_ALPN */

/**
 * \brief          Set the maximum supported version sent from the client side
 *                 and/or accepted at the server side
 *                 (Default: SSL_MAX_MAJOR_VERSION, SSL_MAX_MINOR_VERSION)
 *
 *                 Note: This ignores ciphersuites from 'higher' versions.
 *                 Note: Input outside of the SSL_MAX_XXXXX_VERSION and
 *                       SSL_MIN_XXXXX_VERSION range is ignored.
 *
 * \param ssl      SSL context
 * \param major    Major version number (only SSL_MAJOR_VERSION_3 supported)
 * \param minor    Minor version number (SSL_MINOR_VERSION_0,
 *                 SSL_MINOR_VERSION_1 and SSL_MINOR_VERSION_2,
 *                 SSL_MINOR_VERSION_3 supported)
 */
void ssl_set_max_version( ssl_context *ssl, int major, int minor );

/**
 * \brief          Set the minimum accepted SSL/TLS protocol version
 *                 (Default: SSL_MIN_MAJOR_VERSION, SSL_MIN_MINOR_VERSION)
 *
 * \note           Input outside of the SSL_MAX_XXXXX_VERSION and
 *                 SSL_MIN_XXXXX_VERSION range is ignored.
 *
 * \note           SSL_MINOR_VERSION_0 (SSL v3) should be avoided.
 *
 * \param ssl      SSL context
 * \param major    Major version number (only SSL_MAJOR_VERSION_3 supported)
 * \param minor    Minor version number (SSL_MINOR_VERSION_0,
 *                 SSL_MINOR_VERSION_1 and SSL_MINOR_VERSION_2,
 *                 SSL_MINOR_VERSION_3 supported)
 */
void ssl_set_min_version( ssl_context *ssl, int major, int minor );

#if defined(POLARSSL_SSL_FALLBACK_SCSV) && defined(POLARSSL_SSL_CLI_C)
/**
 * \brief          Set the fallback flag (client-side only).
 *                 (Default: SSL_IS_NOT_FALLBACK).
 *
 * \note           Set to SSL_IS_FALLBACK when preparing a fallback
 *                 connection, that is a connection with max_version set to a
 *                 lower value than the value you're willing to use. Such
 *                 fallback connections are not recommended but are sometimes
 *                 necessary to interoperate with buggy (version-intolerant)
 *                 servers.
 *
 * \warning        You should NOT set this to SSL_IS_FALLBACK for
 *                 non-fallback connections! This would appear to work for a
 *                 while, then cause failures when the server is upgraded to
 *                 support a newer TLS version.
 *
 * \param ssl      SSL context
 * \param fallback SSL_IS_NOT_FALLBACK or SSL_IS_FALLBACK
 */
void ssl_set_fallback( ssl_context *ssl, char fallback );
#endif /* POLARSSL_SSL_FALLBACK_SCSV && POLARSSL_SSL_CLI_C */

#if defined(POLARSSL_SSL_ENCRYPT_THEN_MAC)
/**
 * \brief           Enable or disable Encrypt-then-MAC
 *                  (Default: SSL_ETM_ENABLED)
 *
 * \note            This should always be enabled, it is a security
 *                  improvement, and should not cause any interoperability
 *                  issue (used only if the peer supports it too).
 *
 * \param ssl       SSL context
 * \param etm       SSL_ETM_ENABLED or SSL_ETM_DISABLED
 */
void ssl_set_encrypt_then_mac( ssl_context *ssl, char etm );
#endif /* POLARSSL_SSL_ENCRYPT_THEN_MAC */

#if defined(POLARSSL_SSL_EXTENDED_MASTER_SECRET)
/**
 * \brief           Enable or disable Extended Master Secret negotiation.
 *                  (Default: SSL_EXTENDED_MS_ENABLED)
 *
 * \note            This should always be enabled, it is a security fix to the
 *                  protocol, and should not cause any interoperability issue
 *                  (used only if the peer supports it too).
 *
 * \param ssl       SSL context
 * \param ems       SSL_EXTENDED_MS_ENABLED or SSL_EXTENDED_MS_DISABLED
 */
void ssl_set_extended_master_secret( ssl_context *ssl, char ems );
#endif /* POLARSSL_SSL_EXTENDED_MASTER_SECRET */

/**
 * \brief          Disable or enable support for RC4
 *                 (Default: SSL_ARC4_ENABLED)
 *
 * \note           Though the default is RC4 for compatibility reasons in the
 *                 1.3 branch, the recommended value is SSL_ARC4_DISABLED.
 *
 * \note           This function will likely be removed in future versions as
 *                 RC4 will then be disabled by default at compile time.
 *
 * \param ssl      SSL context
 * \param arc4     SSL_ARC4_ENABLED or SSL_ARC4_DISABLED
 */
void ssl_set_arc4_support( ssl_context *ssl, char arc4 );

#if defined(POLARSSL_SSL_MAX_FRAGMENT_LENGTH)
/**
 * \brief          Set the maximum fragment length to emit and/or negotiate
 *                 (Default: SSL_MAX_CONTENT_LEN, usually 2^14 bytes)
 *                 (Server: set maximum fragment length to emit,
 *                 usually negotiated by the client during handshake
 *                 (Client: set maximum fragment length to emit *and*
 *                 negotiate with the server during handshake)
 *
 * \param ssl      SSL context
 * \param mfl_code Code for maximum fragment length (allowed values:
 *                 SSL_MAX_FRAG_LEN_512,  SSL_MAX_FRAG_LEN_1024,
 *                 SSL_MAX_FRAG_LEN_2048, SSL_MAX_FRAG_LEN_4096)
 *
 * \return         0 if successful or POLARSSL_ERR_SSL_BAD_INPUT_DATA
 */
int ssl_set_max_frag_len( ssl_context *ssl, unsigned char mfl_code );
#endif /* POLARSSL_SSL_MAX_FRAGMENT_LENGTH */

#if defined(POLARSSL_SSL_TRUNCATED_HMAC)
/**
 * \brief          Activate negotiation of truncated HMAC
 *                 (Default: SSL_TRUNC_HMAC_DISABLED on client,
 *                           SSL_TRUNC_HMAC_ENABLED on server.)
 *
 * \param ssl      SSL context
 * \param truncate Enable or disable (SSL_TRUNC_HMAC_ENABLED or
 *                                    SSL_TRUNC_HMAC_DISABLED)
 *
 * \return         Always 0.
 */
int ssl_set_truncated_hmac( ssl_context *ssl, int truncate );
#endif /* POLARSSL_SSL_TRUNCATED_HMAC */

#if defined(POLARSSL_SSL_CBC_RECORD_SPLITTING)
/**
 * \brief          Enable / Disable 1/n-1 record splitting
 *                 (Default: SSL_CBC_RECORD_SPLITTING_ENABLED)
 *
 * \note           Only affects SSLv3 and TLS 1.0, not higher versions.
 *                 Does not affect non-CBC ciphersuites in any version.
 *
 * \param ssl      SSL context
 * \param split    SSL_CBC_RECORD_SPLITTING_ENABLED or
 *                 SSL_CBC_RECORD_SPLITTING_DISABLED
 */
void ssl_set_cbc_record_splitting( ssl_context *ssl, char split );
#endif /* POLARSSL_SSL_CBC_RECORD_SPLITTING */

#if defined(POLARSSL_SSL_SESSION_TICKETS)
/**
 * \brief          Enable / Disable session tickets
 *                 (Default: SSL_SESSION_TICKETS_ENABLED on client,
 *                           SSL_SESSION_TICKETS_DISABLED on server)
 *
 * \note           On server, ssl_set_rng() must be called before this function
 *                 to allow generating the ticket encryption and
 *                 authentication keys.
 *
 * \param ssl      SSL context
 * \param use_tickets   Enable or disable (SSL_SESSION_TICKETS_ENABLED or
 *                                         SSL_SESSION_TICKETS_DISABLED)
 *
 * \return         0 if successful,
 *                 or a specific error code (server only).
 */
int ssl_set_session_tickets( ssl_context *ssl, int use_tickets );

/**
 * \brief          Set session ticket lifetime (server only)
 *                 (Default: SSL_DEFAULT_TICKET_LIFETIME (86400 secs / 1 day))
 *
 * \param ssl      SSL context
 * \param lifetime session ticket lifetime
 */
void ssl_set_session_ticket_lifetime( ssl_context *ssl, int lifetime );
#endif /* POLARSSL_SSL_SESSION_TICKETS */

#if defined(POLARSSL_SSL_RENEGOTIATION)
/**
 * \brief          Enable / Disable renegotiation support for connection when
 *                 initiated by peer
 *                 (Default: SSL_RENEGOTIATION_DISABLED)
 *
 *                 Note: A server with support enabled is more vulnerable for a
 *                 resource DoS by a malicious client. You should enable this on
 *                 a client to enable server-initiated renegotiation.
 *
 * \param ssl      SSL context
 * \param renegotiation     Enable or disable (SSL_RENEGOTIATION_ENABLED or
 *                                             SSL_RENEGOTIATION_DISABLED)
 */
void ssl_set_renegotiation( ssl_context *ssl, int renegotiation );
#endif /* POLARSSL_SSL_RENEGOTIATION */

/**
 * \brief          Prevent or allow legacy renegotiation.
 *                 (Default: SSL_LEGACY_NO_RENEGOTIATION)
 *
 *                 SSL_LEGACY_NO_RENEGOTIATION allows connections to
 *                 be established even if the peer does not support
 *                 secure renegotiation, but does not allow renegotiation
 *                 to take place if not secure.
 *                 (Interoperable and secure option)
 *
 *                 SSL_LEGACY_ALLOW_RENEGOTIATION allows renegotiations
 *                 with non-upgraded peers. Allowing legacy renegotiation
 *                 makes the connection vulnerable to specific man in the
 *                 middle attacks. (See RFC 5746)
 *                 (Most interoperable and least secure option)
 *
 *                 SSL_LEGACY_BREAK_HANDSHAKE breaks off connections
 *                 if peer does not support secure renegotiation. Results
 *                 in interoperability issues with non-upgraded peers
 *                 that do not support renegotiation altogether.
 *                 (Most secure option, interoperability issues)
 *
 * \param ssl      SSL context
 * \param allow_legacy  Prevent or allow (SSL_NO_LEGACY_RENEGOTIATION,
 *                                        SSL_ALLOW_LEGACY_RENEGOTIATION or
 *                                        SSL_LEGACY_BREAK_HANDSHAKE)
 */
void ssl_legacy_renegotiation( ssl_context *ssl, int allow_legacy );

#if defined(POLARSSL_SSL_RENEGOTIATION)
/**
 * \brief          Enforce requested renegotiation.
 *                 (Default: enforced, max_records = 16)
 *
 *                 When we request a renegotiation, the peer can comply or
 *                 ignore the request. This function allows us to decide
 *                 whether to enforce our renegotiation requests by closing
 *                 the connection if the peer doesn't comply.
 *
 *                 However, records could already be in transit from the peer
 *                 when the request is emitted. In order to increase
 *                 reliability, we can accept a number of records before the
 *                 expected handshake records.
 *
 *                 The optimal value is highly dependent on the specific usage
 *                 scenario.
 *
 * \warning        On client, the grace period can only happen during
 *                 ssl_read(), as opposed to ssl_write() and ssl_renegotiate()
 *                 which always behave as if max_record was 0. The reason is,
 *                 if we receive application data from the server, we need a
 *                 place to write it, which only happens during ssl_read().
 *
 * \param ssl      SSL context
 * \param max_records Use SSL_RENEGOTIATION_NOT_ENFORCED if you don't want to
 *                 enforce renegotiation, or a non-negative value to enforce
 *                 it but allow for a grace period of max_records records.
 */
void ssl_set_renegotiation_enforced( ssl_context *ssl, int max_records );

/**
 * \brief          Set record counter threshold for periodic renegotiation.
 *                 (Default: 2^64 - 256.)
 *
 *                 Renegotiation is automatically triggered when a record
 *                 counter (outgoing or ingoing) crosses the defined
 *                 threshold. The default value is meant to prevent the
 *                 connection from being closed when the counter is about to
 *                 reached its maximal value (it is not allowed to wrap).
 *
 *                 Lower values can be used to enforce policies such as "keys
 *                 must be refreshed every N packets with cipher X".
 *
 * \param ssl      SSL context
 * \param period   The threshold value: a big-endian 64-bit number.
 *                 Set to 2^64 - 1 to disable periodic renegotiation
 */
void ssl_set_renegotiation_period( ssl_context *ssl,
                                   const unsigned char period[8] );
#endif /* POLARSSL_SSL_RENEGOTIATION */

/**
 * \brief          Return the number of data bytes available to read
 *
 * \param ssl      SSL context
 *
 * \return         how many bytes are available in the read buffer
 */
size_t ssl_get_bytes_avail( const ssl_context *ssl );

/**
 * \brief          Return the result of the certificate verification
 *
 * \param ssl      SSL context
 *
 * \return         0 if successful,
 *                 -1 if result is not available (eg because the handshake was
 *                 aborted too early), or
 *                 a combination of BADCERT_xxx and BADCRL_xxx flags, see
 *                 x509.h
 */
int ssl_get_verify_result( const ssl_context *ssl );

/**
 * \brief          Return the name of the current ciphersuite
 *
 * \param ssl      SSL context
 *
 * \return         a string containing the ciphersuite name
 */
const char *ssl_get_ciphersuite( const ssl_context *ssl );

/**
 * \brief          Return the current SSL version (SSLv3/TLSv1/etc)
 *
 * \param ssl      SSL context
 *
 * \return         a string containing the SSL version
 */
const char *ssl_get_version( const ssl_context *ssl );

#if defined(POLARSSL_X509_CRT_PARSE_C)
/**
 * \brief          Return the peer certificate from the current connection
 *
 *                 Note: Can be NULL in case no certificate was sent during
 *                 the handshake. Different calls for the same connection can
 *                 return the same or different pointers for the same
 *                 certificate and even a different certificate altogether.
 *                 The peer cert CAN change in a single connection if
 *                 renegotiation is performed.
 *
 * \param ssl      SSL context
 *
 * \return         the current peer certificate
 */
const x509_crt *ssl_get_peer_cert( const ssl_context *ssl );
#endif /* POLARSSL_X509_CRT_PARSE_C */

#if defined(POLARSSL_SSL_CLI_C)
/**
 * \brief          Save session in order to resume it later (client-side only)
 *                 Session data is copied to presented session structure.
 *
 * \warning        Currently, peer certificate is lost in the operation.
 *
 * \param ssl      SSL context
 * \param session  session context
 *
 * \return         0 if successful,
 *                 POLARSSL_ERR_SSL_MALLOC_FAILED if memory allocation failed,
 *                 POLARSSL_ERR_SSL_BAD_INPUT_DATA if used server-side or
 *                 arguments are otherwise invalid
 *
 * \sa             ssl_set_session()
 */
int ssl_get_session( const ssl_context *ssl, ssl_session *session );
#endif /* POLARSSL_SSL_CLI_C */

/**
 * \brief          Perform the SSL handshake
 *
 * \param ssl      SSL context
 *
 * \return         0 if successful, POLARSSL_ERR_NET_WANT_READ,
 *                 POLARSSL_ERR_NET_WANT_WRITE, or a specific SSL error code.
 */
int ssl_handshake( ssl_context *ssl );

/**
 * \brief          Perform a single step of the SSL handshake
 *
 *                 Note: the state of the context (ssl->state) will be at
 *                 the following state after execution of this function.
 *                 Do not call this function if state is SSL_HANDSHAKE_OVER.
 *
 * \param ssl      SSL context
 *
 * \return         0 if successful, POLARSSL_ERR_NET_WANT_READ,
 *                 POLARSSL_ERR_NET_WANT_WRITE, or a specific SSL error code.
 */
int ssl_handshake_step( ssl_context *ssl );

#if defined(POLARSSL_SSL_RENEGOTIATION)
/**
 * \brief          Initiate an SSL renegotiation on the running connection.
 *                 Client: perform the renegotiation right now.
 *                 Server: request renegotiation, which will be performed
 *                 during the next call to ssl_read() if honored by client.
 *
 * \param ssl      SSL context
 *
 * \return         0 if successful, or any ssl_handshake() return value.
 */
int ssl_renegotiate( ssl_context *ssl );
#endif /* POLARSSL_SSL_RENEGOTIATION */

/**
 * \brief          Read at most 'len' application data bytes
 *
 * \param ssl      SSL context
 * \param buf      buffer that will hold the data
 * \param len      maximum number of bytes to read
 *
 * \return         This function returns the number of bytes read, 0 for EOF,
 *                 or a negative error code.
 */
int ssl_read( ssl_context *ssl, unsigned char *buf, size_t len );

/**
 * \brief          Write exactly 'len' application data bytes
 *
 * \param ssl      SSL context
 * \param buf      buffer holding the data
 * \param len      how many bytes must be written
 *
 * \return         This function returns the number of bytes written,
 *                 or a negative error code.
 *
 * \note           When this function returns POLARSSL_ERR_NET_WANT_WRITE,
 *                 it must be called later with the *same* arguments,
 *                 until it returns a positive value.
 *
 * \note           This function may write less than the number of bytes
 *                 requested if len is greater than the maximum record length.
 *                 For arbitrary-sized messages, it should be called in a loop.
 */
int ssl_write( ssl_context *ssl, const unsigned char *buf, size_t len );

/**
 * \brief           Send an alert message
 *
 * \param ssl       SSL context
 * \param level     The alert level of the message
 *                  (SSL_ALERT_LEVEL_WARNING or SSL_ALERT_LEVEL_FATAL)
 * \param message   The alert message (SSL_ALERT_MSG_*)
 *
 * \return          0 if successful, or a specific SSL error code.
 */
int ssl_send_alert_message( ssl_context *ssl,
                            unsigned char level,
                            unsigned char message );
/**
 * \brief          Notify the peer that the connection is being closed
 *
 * \param ssl      SSL context
 */
int ssl_close_notify( ssl_context *ssl );

/**
 * \brief          Free referenced items in an SSL context and clear memory
 *
 * \param ssl      SSL context
 */
void ssl_free( ssl_context *ssl );

/**
 * \brief          Initialize SSL session structure
 *
 * \param session  SSL session
 */
void ssl_session_init( ssl_session *session );

/**
 * \brief          Free referenced items in an SSL session including the
 *                 peer certificate and clear memory
 *
 * \param session  SSL session
 */
void ssl_session_free( ssl_session *session );

/**
 * \brief           Free referenced items in an SSL transform context and clear
 *                  memory
 *
 * \param transform SSL transform context
 */
void ssl_transform_free( ssl_transform *transform );

/**
 * \brief           Free referenced items in an SSL handshake context and clear
 *                  memory
 *
 * \param handshake SSL handshake context
 */
void ssl_handshake_free( ssl_handshake_params *handshake );

/*
 * Internal functions (do not call directly)
 */
int ssl_handshake_client_step( ssl_context *ssl );
int ssl_handshake_server_step( ssl_context *ssl );
void ssl_handshake_wrapup( ssl_context *ssl );

int ssl_send_fatal_handshake_failure( ssl_context *ssl );

int ssl_derive_keys( ssl_context *ssl );

int ssl_read_record( ssl_context *ssl );
/**
 * \return         0 if successful, POLARSSL_ERR_SSL_CONN_EOF on EOF or
 *                 another negative error code.
 */
int ssl_fetch_input( ssl_context *ssl, size_t nb_want );

int ssl_write_record( ssl_context *ssl );
int ssl_flush_output( ssl_context *ssl );

int ssl_parse_certificate( ssl_context *ssl );
int ssl_write_certificate( ssl_context *ssl );

int ssl_parse_change_cipher_spec( ssl_context *ssl );
int ssl_write_change_cipher_spec( ssl_context *ssl );

int ssl_parse_finished( ssl_context *ssl );
int ssl_write_finished( ssl_context *ssl );

void ssl_optimize_checksum( ssl_context *ssl,
                            const ssl_ciphersuite_t *ciphersuite_info );

#if defined(POLARSSL_KEY_EXCHANGE__SOME__PSK_ENABLED)
int ssl_psk_derive_premaster( ssl_context *ssl, key_exchange_type_t key_ex );
#endif

#if defined(POLARSSL_PK_C)
unsigned char ssl_sig_from_pk( pk_context *pk );
pk_type_t ssl_pk_alg_from_sig( unsigned char sig );
#endif

md_type_t ssl_md_alg_from_hash( unsigned char hash );

#if defined(POLARSSL_SSL_SET_CURVES)
int ssl_curve_is_acceptable( const ssl_context *ssl, ecp_group_id grp_id );
#endif

#if defined(POLARSSL_X509_CRT_PARSE_C)
static inline pk_context *ssl_own_key( ssl_context *ssl )
{
    return( ssl->handshake->key_cert == NULL ? NULL
            : ssl->handshake->key_cert->key );
}

static inline x509_crt *ssl_own_cert( ssl_context *ssl )
{
    return( ssl->handshake->key_cert == NULL ? NULL
            : ssl->handshake->key_cert->cert );
}

/*
 * Check usage of a certificate wrt extensions:
 * keyUsage, extendedKeyUsage (later), and nSCertType (later).
 *
 * Warning: cert_endpoint is the endpoint of the cert (ie, of our peer when we
 * check a cert we received from them)!
 *
 * Return 0 if everything is OK, -1 if not.
 */
int ssl_check_cert_usage( const x509_crt *cert,
                          const ssl_ciphersuite_t *ciphersuite,
                          int cert_endpoint,
                          int *flags );
#endif /* POLARSSL_X509_CRT_PARSE_C */

/* constant-time buffer comparison */
static inline int safer_memcmp( const void *a, const void *b, size_t n )
{
    size_t i;
    const unsigned char *A = (const unsigned char *) a;
    const unsigned char *B = (const unsigned char *) b;
    unsigned char diff = 0;

    for( i = 0; i < n; i++ )
        diff |= A[i] ^ B[i];

    return( diff );
}

#ifdef __cplusplus
}
#endif

#endif /* ssl.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/ssl_cache.h ************/


/**
 * \file ssl_cache.h
 *
 * \brief SSL session cache implementation
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_SSL_CACHE_H
#define POLARSSL_SSL_CACHE_H



#if defined(POLARSSL_THREADING_C)

#endif

/**
 * \name SECTION: Module settings
 *
 * The configuration options you can set for this module are in this section.
 * Either change them in config.h or define them on the compiler command line.
 * \{
 */

#if !defined(SSL_CACHE_DEFAULT_TIMEOUT)
#define SSL_CACHE_DEFAULT_TIMEOUT       86400   /*!< 1 day  */
#endif

#if !defined(SSL_CACHE_DEFAULT_MAX_ENTRIES)
#define SSL_CACHE_DEFAULT_MAX_ENTRIES      50   /*!< Maximum entries in cache */
#endif

/* \} name SECTION: Module settings */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ssl_cache_context ssl_cache_context;
typedef struct _ssl_cache_entry ssl_cache_entry;

/**
 * \brief   This structure is used for storing cache entries
 */
struct _ssl_cache_entry
{
#if defined(POLARSSL_HAVE_TIME)
    time_t timestamp;           /*!< entry timestamp    */
#endif
    ssl_session session;        /*!< entry session      */
#if defined(POLARSSL_X509_CRT_PARSE_C)
    x509_buf peer_cert;         /*!< entry peer_cert    */
#endif
    ssl_cache_entry *next;      /*!< chain pointer      */
};

/**
 * \brief Cache context
 */
struct _ssl_cache_context
{
    ssl_cache_entry *chain;     /*!< start of the chain     */
    int timeout;                /*!< cache entry timeout    */
    int max_entries;            /*!< maximum entries        */
#if defined(POLARSSL_THREADING_C)
    threading_mutex_t mutex;    /*!< mutex                  */
#endif
};

/**
 * \brief          Initialize an SSL cache context
 *
 * \param cache    SSL cache context
 */
void ssl_cache_init( ssl_cache_context *cache );

/**
 * \brief          Cache get callback implementation
 *                 (Thread-safe if POLARSSL_THREADING_C is enabled)
 *
 * \param data     SSL cache context
 * \param session  session to retrieve entry for
 */
int ssl_cache_get( void *data, ssl_session *session );

/**
 * \brief          Cache set callback implementation
 *                 (Thread-safe if POLARSSL_THREADING_C is enabled)
 *
 * \param data     SSL cache context
 * \param session  session to store entry for
 */
int ssl_cache_set( void *data, const ssl_session *session );

#if defined(POLARSSL_HAVE_TIME)
/**
 * \brief          Set the cache timeout
 *                 (Default: SSL_CACHE_DEFAULT_TIMEOUT (1 day))
 *
 *                 A timeout of 0 indicates no timeout.
 *
 * \param cache    SSL cache context
 * \param timeout  cache entry timeout in seconds
 */
void ssl_cache_set_timeout( ssl_cache_context *cache, int timeout );
#endif /* POLARSSL_HAVE_TIME */

/**
 * \brief          Set the cache timeout
 *                 (Default: SSL_CACHE_DEFAULT_MAX_ENTRIES (50))
 *
 * \param cache    SSL cache context
 * \param max      cache entry maximum
 */
void ssl_cache_set_max_entries( ssl_cache_context *cache, int max );

/**
 * \brief          Free referenced items in a cache context and clear memory
 *
 * \param cache    SSL cache context
 */
void ssl_cache_free( ssl_cache_context *cache );

#ifdef __cplusplus
}
#endif

#endif /* ssl_cache.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/debug.h ************/


/**
 * \file debug.h
 *
 * \brief Debug functions
 *
 *  Copyright (C) 2006-2011, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_DEBUG_H
#define POLARSSL_DEBUG_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif



#if defined(POLARSSL_ECP_C)

#endif

#if defined(POLARSSL_DEBUG_C)

#define POLARSSL_DEBUG_LOG_FULL         0 /**< Include file:line in log lines */
#define POLARSSL_DEBUG_LOG_RAW          1 /**< Only log raw debug lines */

/**
 * \name SECTION: Module settings
 *
 * The configuration options you can set for this module are in this section.
 * Either change them in config.h or define them on the compiler command line.
 * \{
 */

#if !defined(POLARSSL_DEBUG_DFL_MODE)
#define POLARSSL_DEBUG_DFL_MODE POLARSSL_DEBUG_LOG_FULL /**< Default log: Full or Raw */
#endif

/* \} name SECTION: Module settings */


#define SSL_DEBUG_MSG( level, args )                    \
    debug_print_msg( ssl, level, __FILE__, __LINE__, debug_fmt args );

#define SSL_DEBUG_RET( level, text, ret )                \
    debug_print_ret( ssl, level, __FILE__, __LINE__, text, ret );

#define SSL_DEBUG_BUF( level, text, buf, len )           \
    debug_print_buf( ssl, level, __FILE__, __LINE__, text, buf, len );

#if defined(POLARSSL_BIGNUM_C)
#define SSL_DEBUG_MPI( level, text, X )                  \
    debug_print_mpi( ssl, level, __FILE__, __LINE__, text, X );
#endif

#if defined(POLARSSL_ECP_C)
#define SSL_DEBUG_ECP( level, text, X )                  \
    debug_print_ecp( ssl, level, __FILE__, __LINE__, text, X );
#endif

#if defined(POLARSSL_X509_CRT_PARSE_C)
#define SSL_DEBUG_CRT( level, text, crt )                \
    debug_print_crt( ssl, level, __FILE__, __LINE__, text, crt );
#endif

#else /* POLARSSL_DEBUG_C */

#define SSL_DEBUG_MSG( level, args )            do { } while( 0 )
#define SSL_DEBUG_RET( level, text, ret )       do { } while( 0 )
#define SSL_DEBUG_BUF( level, text, buf, len )  do { } while( 0 )
#define SSL_DEBUG_MPI( level, text, X )         do { } while( 0 )
#define SSL_DEBUG_ECP( level, text, X )         do { } while( 0 )
#define SSL_DEBUG_CRT( level, text, crt )       do { } while( 0 )

#endif /* POLARSSL_DEBUG_C */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief   Set the log mode for the debug functions globally
 *          (Default value: POLARSSL_DEBUG_DFL_MODE)
 *
 * \param log_mode      The log mode to use (POLARSSL_DEBUG_LOG_FULL or
 *                                           POLARSSL_DEBUG_LOG_RAW)
 */
void debug_set_log_mode( int log_mode );

/**
 * \brief   Set the level threshold to handle globally. Messages that have a
 *          level over the threshold value are ignored.
 *          (Default value: 0 (No debug))
 *
 * \param threshold     maximum level of messages to pass on
 */
void debug_set_threshold( int threshold );

char *debug_fmt( const char *format, ... );

void debug_print_msg( const ssl_context *ssl, int level,
                      const char *file, int line, const char *text );

void debug_print_ret( const ssl_context *ssl, int level,
                      const char *file, int line,
                      const char *text, int ret );

void debug_print_buf( const ssl_context *ssl, int level,
                      const char *file, int line, const char *text,
                      unsigned char *buf, size_t len );

#if defined(POLARSSL_BIGNUM_C)
void debug_print_mpi( const ssl_context *ssl, int level,
                      const char *file, int line,
                      const char *text, const mpi *X );
#endif

#if defined(POLARSSL_ECP_C)
void debug_print_ecp( const ssl_context *ssl, int level,
                      const char *file, int line,
                      const char *text, const ecp_point *X );
#endif

#if defined(POLARSSL_X509_CRT_PARSE_C)
void debug_print_crt( const ssl_context *ssl, int level,
                      const char *file, int line,
                      const char *text, const x509_crt *crt );
#endif

#ifdef __cplusplus
}
#endif

#endif /* debug.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/blowfish.h ************/


/**
 * \file blowfish.h
 *
 * \brief Blowfish block cipher
 *
 *  Copyright (C) 2012-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_BLOWFISH_H
#define POLARSSL_BLOWFISH_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

#define BLOWFISH_ENCRYPT     1
#define BLOWFISH_DECRYPT     0
#define BLOWFISH_MAX_KEY     448
#define BLOWFISH_MIN_KEY     32
#define BLOWFISH_ROUNDS      16         /**< Rounds to use. When increasing this value, make sure to extend the initialisation vectors */
#define BLOWFISH_BLOCKSIZE   8          /* Blowfish uses 64 bit blocks */

#define POLARSSL_ERR_BLOWFISH_INVALID_KEY_LENGTH                -0x0016  /**< Invalid key length. */
#define POLARSSL_ERR_BLOWFISH_INVALID_INPUT_LENGTH              -0x0018  /**< Invalid data input length. */

#if !defined(POLARSSL_BLOWFISH_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Blowfish context structure
 */
typedef struct
{
    uint32_t P[BLOWFISH_ROUNDS + 2];    /*!<  Blowfish round keys    */
    uint32_t S[4][256];                 /*!<  key dependent S-boxes  */
}
blowfish_context;

/**
 * \brief          Initialize Blowfish context
 *
 * \param ctx      Blowfish context to be initialized
 */
void blowfish_init( blowfish_context *ctx );

/**
 * \brief          Clear Blowfish context
 *
 * \param ctx      Blowfish context to be cleared
 */
void blowfish_free( blowfish_context *ctx );

/**
 * \brief          Blowfish key schedule
 *
 * \param ctx      Blowfish context to be initialized
 * \param key      encryption key
 * \param keysize  must be between 32 and 448 bits
 *
 * \return         0 if successful, or POLARSSL_ERR_BLOWFISH_INVALID_KEY_LENGTH
 */
int blowfish_setkey( blowfish_context *ctx, const unsigned char *key,
                     unsigned int keysize );

/**
 * \brief          Blowfish-ECB block encryption/decryption
 *
 * \param ctx      Blowfish context
 * \param mode     BLOWFISH_ENCRYPT or BLOWFISH_DECRYPT
 * \param input    8-byte input block
 * \param output   8-byte output block
 *
 * \return         0 if successful
 */
int blowfish_crypt_ecb( blowfish_context *ctx,
                        int mode,
                        const unsigned char input[BLOWFISH_BLOCKSIZE],
                        unsigned char output[BLOWFISH_BLOCKSIZE] );

#if defined(POLARSSL_CIPHER_MODE_CBC)
/**
 * \brief          Blowfish-CBC buffer encryption/decryption
 *                 Length should be a multiple of the block
 *                 size (8 bytes)
 *
 * \note           Upon exit, the content of the IV is updated so that you can
 *                 call the function same function again on the following
 *                 block(s) of data and get the same result as if it was
 *                 encrypted in one call. This allows a "streaming" usage.
 *                 If on the other hand you need to retain the contents of the
 *                 IV, you should either save it manually or use the cipher
 *                 module instead.
 *
 * \param ctx      Blowfish context
 * \param mode     BLOWFISH_ENCRYPT or BLOWFISH_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if successful, or
 *                 POLARSSL_ERR_BLOWFISH_INVALID_INPUT_LENGTH
 */
int blowfish_crypt_cbc( blowfish_context *ctx,
                        int mode,
                        size_t length,
                        unsigned char iv[BLOWFISH_BLOCKSIZE],
                        const unsigned char *input,
                        unsigned char *output );
#endif /* POLARSSL_CIPHER_MODE_CBC */

#if defined(POLARSSL_CIPHER_MODE_CFB)
/**
 * \brief          Blowfish CFB buffer encryption/decryption.
 *
 * \note           Upon exit, the content of the IV is updated so that you can
 *                 call the function same function again on the following
 *                 block(s) of data and get the same result as if it was
 *                 encrypted in one call. This allows a "streaming" usage.
 *                 If on the other hand you need to retain the contents of the
 *                 IV, you should either save it manually or use the cipher
 *                 module instead.
 *
 * \param ctx      Blowfish context
 * \param mode     BLOWFISH_ENCRYPT or BLOWFISH_DECRYPT
 * \param length   length of the input data
 * \param iv_off   offset in IV (updated after use)
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if successful
 */
int blowfish_crypt_cfb64( blowfish_context *ctx,
                          int mode,
                          size_t length,
                          size_t *iv_off,
                          unsigned char iv[BLOWFISH_BLOCKSIZE],
                          const unsigned char *input,
                          unsigned char *output );
#endif /*POLARSSL_CIPHER_MODE_CFB */

#if defined(POLARSSL_CIPHER_MODE_CTR)
/**
 * \brief               Blowfish-CTR buffer encryption/decryption
 *
 * Warning: You have to keep the maximum use of your counter in mind!
 *
 * \param ctx           Blowfish context
 * \param length        The length of the data
 * \param nc_off        The offset in the current stream_block (for resuming
 *                      within current cipher stream). The offset pointer to
 *                      should be 0 at the start of a stream.
 * \param nonce_counter The 64-bit nonce and counter.
 * \param stream_block  The saved stream-block for resuming. Is overwritten
 *                      by the function.
 * \param input         The input data stream
 * \param output        The output data stream
 *
 * \return         0 if successful
 */
int blowfish_crypt_ctr( blowfish_context *ctx,
                        size_t length,
                        size_t *nc_off,
                        unsigned char nonce_counter[BLOWFISH_BLOCKSIZE],
                        unsigned char stream_block[BLOWFISH_BLOCKSIZE],
                        const unsigned char *input,
                        unsigned char *output );
#endif /* POLARSSL_CIPHER_MODE_CTR */

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_BLOWFISH_ALT */

#endif /* POLARSSL_BLOWFISH_ALT */

#endif /* blowfish.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/ccm.h ************/


/**
 * \file ccm.h
 *
 * \brief Counter with CBC-MAC (CCM) for 128-bit block ciphers
 *
 *  Copyright (C) 2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_CCM_H
#define POLARSSL_CCM_H



#define POLARSSL_ERR_CCM_BAD_INPUT      -0x000D /**< Bad input parameters to function. */
#define POLARSSL_ERR_CCM_AUTH_FAILED    -0x000F /**< Authenticated decryption failed. */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          CCM context structure
 */
typedef struct {
    cipher_context_t cipher_ctx;    /*!< cipher context used */
}
ccm_context;

/**
 * \brief           CCM initialization (encryption and decryption)
 *
 * \param ctx       CCM context to be initialized
 * \param cipher    cipher to use (a 128-bit block cipher)
 * \param key       encryption key
 * \param keysize   key size in bits (must be acceptable by the cipher)
 *
 * \return          0 if successful, or a cipher specific error code
 */
int ccm_init( ccm_context *ctx, cipher_id_t cipher,
              const unsigned char *key, unsigned int keysize );

/**
 * \brief           Free a CCM context and underlying cipher sub-context
 *
 * \param ctx       CCM context to free
 */
void ccm_free( ccm_context *ctx );

/**
 * \brief           CCM buffer encryption
 *
 * \param ctx       CCM context
 * \param length    length of the input data in bytes
 * \param iv        nonce (initialization vector)
 * \param iv_len    length of IV in bytes
 *                  must be 2, 3, 4, 5, 6, 7 or 8
 * \param add       additional data
 * \param add_len   length of additional data in bytes
 *                  must be less than 2^16 - 2^8
 * \param input     buffer holding the input data
 * \param output    buffer for holding the output data
 *                  must be at least 'length' bytes wide
 * \param tag       buffer for holding the tag
 * \param tag_len   length of the tag to generate in bytes
 *                  must be 4, 6, 8, 10, 14 or 16
 *
 * \note            The tag is written to a separate buffer. To get the tag
 *                  concatenated with the output as in the CCM spec, use
 *                  tag = output + length and make sure the output buffer is
 *                  at least length + tag_len wide.
 *
 * \return          0 if successful
 */
int ccm_encrypt_and_tag( ccm_context *ctx, size_t length,
                         const unsigned char *iv, size_t iv_len,
                         const unsigned char *add, size_t add_len,
                         const unsigned char *input, unsigned char *output,
                         unsigned char *tag, size_t tag_len );

/**
 * \brief           CCM buffer authenticated decryption
 *
 * \param ctx       CCM context
 * \param length    length of the input data
 * \param iv        initialization vector
 * \param iv_len    length of IV
 * \param add       additional data
 * \param add_len   length of additional data
 * \param input     buffer holding the input data
 * \param output    buffer for holding the output data
 * \param tag       buffer holding the tag
 * \param tag_len   length of the tag
 *
 * \return         0 if successful and authenticated,
 *                 POLARSSL_ERR_CCM_AUTH_FAILED if tag does not match
 */
int ccm_auth_decrypt( ccm_context *ctx, size_t length,
                      const unsigned char *iv, size_t iv_len,
                      const unsigned char *add, size_t add_len,
                      const unsigned char *input, unsigned char *output,
                      const unsigned char *tag, size_t tag_len );

#if defined(POLARSSL_SELF_TEST) && defined(POLARSSL_AES_C)
/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int ccm_self_test( int verbose );
#endif /* POLARSSL_SELF_TEST && POLARSSL_AES_C */

#ifdef __cplusplus
}
#endif

#endif /* POLARSSL_CGM_H */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/gcm.h ************/


/**
 * \file gcm.h
 *
 * \brief Galois/Counter mode for 128-bit block ciphers
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_GCM_H
#define POLARSSL_GCM_H



#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
typedef UINT64 uint64_t;
#elif !VXWORKS
#include <stdint.h>
#endif

#define GCM_ENCRYPT     1
#define GCM_DECRYPT     0

#define POLARSSL_ERR_GCM_AUTH_FAILED                       -0x0012  /**< Authenticated decryption failed. */
#define POLARSSL_ERR_GCM_BAD_INPUT                         -0x0014  /**< Bad input parameters to function. */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          GCM context structure
 */
typedef struct {
    cipher_context_t cipher_ctx;/*!< cipher context used */
    uint64_t HL[16];            /*!< Precalculated HTable */
    uint64_t HH[16];            /*!< Precalculated HTable */
    uint64_t len;               /*!< Total data length */
    uint64_t add_len;           /*!< Total add length */
    unsigned char base_ectr[16];/*!< First ECTR for tag */
    unsigned char y[16];        /*!< Y working value */
    unsigned char buf[16];      /*!< buf working value */
    int mode;                   /*!< Encrypt or Decrypt */
}
gcm_context;

/**
 * \brief           GCM initialization (encryption)
 *
 * \param ctx       GCM context to be initialized
 * \param cipher    cipher to use (a 128-bit block cipher)
 * \param key       encryption key
 * \param keysize   must be 128, 192 or 256
 *
 * \return          0 if successful, or a cipher specific error code
 */
int gcm_init( gcm_context *ctx, cipher_id_t cipher, const unsigned char *key,
              unsigned int keysize );

/**
 * \brief           GCM buffer encryption/decryption using a block cipher
 *
 * \note On encryption, the output buffer can be the same as the input buffer.
 *       On decryption, the output buffer cannot be the same as input buffer.
 *       If buffers overlap, the output buffer must trail at least 8 bytes
 *       behind the input buffer.
 *
 * \param ctx       GCM context
 * \param mode      GCM_ENCRYPT or GCM_DECRYPT
 * \param length    length of the input data
 * \param iv        initialization vector
 * \param iv_len    length of IV
 * \param add       additional data
 * \param add_len   length of additional data
 * \param input     buffer holding the input data
 * \param output    buffer for holding the output data
 * \param tag_len   length of the tag to generate
 * \param tag       buffer for holding the tag
 *
 * \return         0 if successful
 */
int gcm_crypt_and_tag( gcm_context *ctx,
                       int mode,
                       size_t length,
                       const unsigned char *iv,
                       size_t iv_len,
                       const unsigned char *add,
                       size_t add_len,
                       const unsigned char *input,
                       unsigned char *output,
                       size_t tag_len,
                       unsigned char *tag );

/**
 * \brief           GCM buffer authenticated decryption using a block cipher
 *
 * \note On decryption, the output buffer cannot be the same as input buffer.
 *       If buffers overlap, the output buffer must trail at least 8 bytes
 *       behind the input buffer.
 *
 * \param ctx       GCM context
 * \param length    length of the input data
 * \param iv        initialization vector
 * \param iv_len    length of IV
 * \param add       additional data
 * \param add_len   length of additional data
 * \param tag       buffer holding the tag
 * \param tag_len   length of the tag
 * \param input     buffer holding the input data
 * \param output    buffer for holding the output data
 *
 * \return         0 if successful and authenticated,
 *                 POLARSSL_ERR_GCM_AUTH_FAILED if tag does not match
 */
int gcm_auth_decrypt( gcm_context *ctx,
                      size_t length,
                      const unsigned char *iv,
                      size_t iv_len,
                      const unsigned char *add,
                      size_t add_len,
                      const unsigned char *tag,
                      size_t tag_len,
                      const unsigned char *input,
                      unsigned char *output );

/**
 * \brief           Generic GCM stream start function
 *
 * \param ctx       GCM context
 * \param mode      GCM_ENCRYPT or GCM_DECRYPT
 * \param iv        initialization vector
 * \param iv_len    length of IV
 * \param add       additional data (or NULL if length is 0)
 * \param add_len   length of additional data
 *
 * \return         0 if successful
 */
int gcm_starts( gcm_context *ctx,
                int mode,
                const unsigned char *iv,
                size_t iv_len,
                const unsigned char *add,
                size_t add_len );

/**
 * \brief           Generic GCM update function. Encrypts/decrypts using the
 *                  given GCM context. Expects input to be a multiple of 16
 *                  bytes! Only the last call before gcm_finish() can be less
 *                  than 16 bytes!
 *
 * \note On decryption, the output buffer cannot be the same as input buffer.
 *       If buffers overlap, the output buffer must trail at least 8 bytes
 *       behind the input buffer.
 *
 * \param ctx       GCM context
 * \param length    length of the input data
 * \param input     buffer holding the input data
 * \param output    buffer for holding the output data
 *
 * \return         0 if successful or POLARSSL_ERR_GCM_BAD_INPUT
 */
int gcm_update( gcm_context *ctx,
                size_t length,
                const unsigned char *input,
                unsigned char *output );

/**
 * \brief           Generic GCM finalisation function. Wraps up the GCM stream
 *                  and generates the tag. The tag can have a maximum length of
 *                  16 bytes.
 *
 * \param ctx       GCM context
 * \param tag       buffer for holding the tag (may be NULL if tag_len is 0)
 * \param tag_len   length of the tag to generate
 *
 * \return          0 if successful or POLARSSL_ERR_GCM_BAD_INPUT
 */
int gcm_finish( gcm_context *ctx,
                unsigned char *tag,
                size_t tag_len );

/**
 * \brief           Free a GCM context and underlying cipher sub-context
 *
 * \param ctx       GCM context to free
 */
void gcm_free( gcm_context *ctx );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int gcm_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* gcm.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/pem.h ************/


/**
 * \file pem.h
 *
 * \brief Privacy Enhanced Mail (PEM) decoding
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_PEM_H
#define POLARSSL_PEM_H

#include <stddef.h>

/**
 * \name PEM Error codes
 * These error codes are returned in case of errors reading the
 * PEM data.
 * \{
 */
#define POLARSSL_ERR_PEM_NO_HEADER_FOOTER_PRESENT          -0x1080  /**< No PEM header or footer found. */
#define POLARSSL_ERR_PEM_INVALID_DATA                      -0x1100  /**< PEM string is not as expected. */
#define POLARSSL_ERR_PEM_MALLOC_FAILED                     -0x1180  /**< Failed to allocate memory. */
#define POLARSSL_ERR_PEM_INVALID_ENC_IV                    -0x1200  /**< RSA IV is not in hex-format. */
#define POLARSSL_ERR_PEM_UNKNOWN_ENC_ALG                   -0x1280  /**< Unsupported key encryption algorithm. */
#define POLARSSL_ERR_PEM_PASSWORD_REQUIRED                 -0x1300  /**< Private key password can't be empty. */
#define POLARSSL_ERR_PEM_PASSWORD_MISMATCH                 -0x1380  /**< Given private key password does not allow for correct decryption. */
#define POLARSSL_ERR_PEM_FEATURE_UNAVAILABLE               -0x1400  /**< Unavailable feature, e.g. hashing/encryption combination. */
#define POLARSSL_ERR_PEM_BAD_INPUT_DATA                    -0x1480  /**< Bad input parameters to function. */
/* \} name */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(POLARSSL_PEM_PARSE_C)
/**
 * \brief       PEM context structure
 */
typedef struct
{
    unsigned char *buf;     /*!< buffer for decoded data             */
    size_t buflen;          /*!< length of the buffer                */
    unsigned char *info;    /*!< buffer for extra header information */
}
pem_context;

/**
 * \brief       PEM context setup
 *
 * \param ctx   context to be initialized
 */
void pem_init( pem_context *ctx );

/**
 * \brief       Read a buffer for PEM information and store the resulting
 *              data into the specified context buffers.
 *
 * \param ctx       context to use
 * \param header    header string to seek and expect
 * \param footer    footer string to seek and expect
 * \param data      source data to look in
 * \param pwd       password for decryption (can be NULL)
 * \param pwdlen    length of password
 * \param use_len   destination for total length used (set after header is
 *                  correctly read, so unless you get
 *                  POLARSSL_ERR_PEM_BAD_INPUT_DATA or
 *                  POLARSSL_ERR_PEM_NO_HEADER_FOOTER_PRESENT, use_len is
 *                  the length to skip)
 *
 * \note            Attempts to check password correctness by verifying if
 *                  the decrypted text starts with an ASN.1 sequence of
 *                  appropriate length
 *
 * \return          0 on success, or a specific PEM error code
 */
int pem_read_buffer( pem_context *ctx, const char *header, const char *footer,
                     const unsigned char *data,
                     const unsigned char *pwd,
                     size_t pwdlen, size_t *use_len );

/**
 * \brief       PEM context memory freeing
 *
 * \param ctx   context to be freed
 */
void pem_free( pem_context *ctx );
#endif /* POLARSSL_PEM_PARSE_C */

#if defined(POLARSSL_PEM_WRITE_C)
/**
 * \brief           Write a buffer of PEM information from a DER encoded
 *                  buffer.
 *
 * \param header    header string to write
 * \param footer    footer string to write
 * \param der_data  DER data to write
 * \param der_len   length of the DER data
 * \param buf       buffer to write to
 * \param buf_len   length of output buffer
 * \param olen      total length written / required (if buf_len is not enough)
 *
 * \return          0 on success, or a specific PEM or BASE64 error code. On
 *                  POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL olen is the required
 *                  size.
 */
int pem_write_buffer( const char *header, const char *footer,
                      const unsigned char *der_data, size_t der_len,
                      unsigned char *buf, size_t buf_len, size_t *olen );
#endif /* POLARSSL_PEM_WRITE_C */

#ifdef __cplusplus
}
#endif

#endif /* pem.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/asn1write.h ************/


/**
 * \file asn1write.h
 *
 * \brief ASN.1 buffer writing functionality
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_ASN1_WRITE_H
#define POLARSSL_ASN1_WRITE_H



#define ASN1_CHK_ADD(g, f) do { if( ( ret = f ) < 0 ) return( ret ); else   \
                                g += ret; } while( 0 )

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief           Write a length field in ASN.1 format
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param len       the length to write
 *
 * \return          the length written or a negative error code
 */
int asn1_write_len( unsigned char **p, unsigned char *start, size_t len );

/**
 * \brief           Write a ASN.1 tag in ASN.1 format
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param tag       the tag to write
 *
 * \return          the length written or a negative error code
 */
int asn1_write_tag( unsigned char **p, unsigned char *start,
                    unsigned char tag );

/**
 * \brief           Write raw buffer data
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param buf       data buffer to write
 * \param size      length of the data buffer
 *
 * \return          the length written or a negative error code
 */
int asn1_write_raw_buffer( unsigned char **p, unsigned char *start,
                           const unsigned char *buf, size_t size );

#if defined(POLARSSL_BIGNUM_C)
/**
 * \brief           Write a big number (ASN1_INTEGER) in ASN.1 format
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param X         the MPI to write
 *
 * \return          the length written or a negative error code
 */
int asn1_write_mpi( unsigned char **p, unsigned char *start, mpi *X );
#endif /* POLARSSL_BIGNUM_C */

/**
 * \brief           Write a NULL tag (ASN1_NULL) with zero data in ASN.1 format
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 *
 * \return          the length written or a negative error code
 */
int asn1_write_null( unsigned char **p, unsigned char *start );

/**
 * \brief           Write an OID tag (ASN1_OID) and data in ASN.1 format
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param oid       the OID to write
 * \param oid_len   length of the OID
 *
 * \return          the length written or a negative error code
 */
int asn1_write_oid( unsigned char **p, unsigned char *start,
                    const char *oid, size_t oid_len );

/**
 * \brief           Write an AlgorithmIdentifier sequence in ASN.1 format
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param oid       the OID of the algorithm
 * \param oid_len   length of the OID
 * \param par_len   length of parameters, which must be already written.
 *                  If 0, NULL parameters are added
 *
 * \return          the length written or a negative error code
 */
int asn1_write_algorithm_identifier( unsigned char **p, unsigned char *start,
                                     const char *oid, size_t oid_len,
                                     size_t par_len );

/**
 * \brief           Write a boolean tag (ASN1_BOOLEAN) and value in ASN.1 format
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param boolean   0 or 1
 *
 * \return          the length written or a negative error code
 */
int asn1_write_bool( unsigned char **p, unsigned char *start, int boolean );

/**
 * \brief           Write an int tag (ASN1_INTEGER) and value in ASN.1 format
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param val       the integer value
 *
 * \return          the length written or a negative error code
 */
int asn1_write_int( unsigned char **p, unsigned char *start, int val );

/**
 * \brief           Write a printable string tag (ASN1_PRINTABLE_STRING) and
 *                  value in ASN.1 format
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param text      the text to write
 * \param text_len  length of the text
 *
 * \return          the length written or a negative error code
 */
int asn1_write_printable_string( unsigned char **p, unsigned char *start,
                                 const char *text, size_t text_len );

/**
 * \brief           Write an IA5 string tag (ASN1_IA5_STRING) and
 *                  value in ASN.1 format
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param text      the text to write
 * \param text_len  length of the text
 *
 * \return          the length written or a negative error code
 */
int asn1_write_ia5_string( unsigned char **p, unsigned char *start,
                           const char *text, size_t text_len );

/**
 * \brief           Write a bitstring tag (ASN1_BIT_STRING) and
 *                  value in ASN.1 format
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param buf       the bitstring
 * \param bits      the total number of bits in the bitstring
 *
 * \return          the length written or a negative error code
 */
int asn1_write_bitstring( unsigned char **p, unsigned char *start,
                          const unsigned char *buf, size_t bits );

/**
 * \brief           Write an octet string tag (ASN1_OCTET_STRING) and
 *                  value in ASN.1 format
 *                  Note: function works backwards in data buffer
 *
 * \param p         reference to current position pointer
 * \param start     start of the buffer (for bounds-checking)
 * \param buf       data buffer to write
 * \param size      length of the data buffer
 *
 * \return          the length written or a negative error code
 */
int asn1_write_octet_string( unsigned char **p, unsigned char *start,
                             const unsigned char *buf, size_t size );

/**
 * \brief           Create or find a specific named_data entry for writing in a
 *                  sequence or list based on the OID. If not already in there,
 *                  a new entry is added to the head of the list.
 *                  Warning: Destructive behaviour for the val data!
 *
 * \param list      Pointer to the location of the head of the list to seek
 *                  through (will be updated in case of a new entry)
 * \param oid       The OID to look for
 * \param oid_len   Size of the OID
 * \param val       Data to store (can be NULL if you want to fill it by hand)
 * \param val_len   Minimum length of the data buffer needed
 *
 * \return      NULL if if there was a memory allocation error, or a pointer
 *              to the new / existing entry.
 */
asn1_named_data *asn1_store_named_data( asn1_named_data **list,
                                        const char *oid, size_t oid_len,
                                        const unsigned char *val,
                                        size_t val_len );

#ifdef __cplusplus
}
#endif

#endif /* POLARSSL_ASN1_WRITE_H */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/hmac_drbg.h ************/


/**
 * \file hmac_drbg.h
 *
 * \brief HMAC_DRBG (NIST SP 800-90A)
 *
 *  Copyright (C) 2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_HMAC_DRBG_H
#define POLARSSL_HMAC_DRBG_H



/*
 * Error codes
 */
#define POLARSSL_ERR_HMAC_DRBG_REQUEST_TOO_BIG              -0x0003  /**< Too many random requested in single call. */
#define POLARSSL_ERR_HMAC_DRBG_INPUT_TOO_BIG                -0x0005  /**< Input too large (Entropy + additional). */
#define POLARSSL_ERR_HMAC_DRBG_FILE_IO_ERROR                -0x0007  /**< Read/write error in file. */
#define POLARSSL_ERR_HMAC_DRBG_ENTROPY_SOURCE_FAILED        -0x0009  /**< The entropy source failed. */

/**
 * \name SECTION: Module settings
 *
 * The configuration options you can set for this module are in this section.
 * Either change them in config.h or define them on the compiler command line.
 * \{
 */

#if !defined(POLARSSL_HMAC_DRBG_RESEED_INTERVAL)
#define POLARSSL_HMAC_DRBG_RESEED_INTERVAL   10000   /**< Interval before reseed is performed by default */
#endif

#if !defined(POLARSSL_HMAC_DRBG_MAX_INPUT)
#define POLARSSL_HMAC_DRBG_MAX_INPUT         256     /**< Maximum number of additional input bytes */
#endif

#if !defined(POLARSSL_HMAC_DRBG_MAX_REQUEST)
#define POLARSSL_HMAC_DRBG_MAX_REQUEST       1024    /**< Maximum number of requested bytes per call */
#endif

#if !defined(POLARSSL_HMAC_DRBG_MAX_SEED_INPUT)
#define POLARSSL_HMAC_DRBG_MAX_SEED_INPUT    384     /**< Maximum size of (re)seed buffer */
#endif

/* \} name SECTION: Module settings */

#define POLARSSL_HMAC_DRBG_PR_OFF   0   /**< No prediction resistance       */
#define POLARSSL_HMAC_DRBG_PR_ON    1   /**< Prediction resistance enabled  */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * HMAC_DRBG context.
 */
typedef struct
{
    /* Working state: the key K is not stored explicitely,
     * but is implied by the HMAC context */
    md_context_t md_ctx;                    /*!< HMAC context (inc. K)  */
    unsigned char V[POLARSSL_MD_MAX_SIZE];  /*!< V in the spec          */
    int reseed_counter;                     /*!< reseed counter         */

    /* Administrative state */
    size_t entropy_len;         /*!< entropy bytes grabbed on each (re)seed */
    int prediction_resistance;  /*!< enable prediction resistance (Automatic
                                     reseed before every random generation) */
    int reseed_interval;        /*!< reseed interval   */

    /* Callbacks */
    int (*f_entropy)(void *, unsigned char *, size_t); /*!< entropy function */
    void *p_entropy;            /*!< context for the entropy function        */
} hmac_drbg_context;

/**
 * \brief               HMAC_DRBG initialisation
 *
 * \param ctx           HMAC_DRBG context to be initialised
 * \param md_info       MD algorithm to use for HMAC_DRBG
 * \param f_entropy     Entropy callback (p_entropy, buffer to fill, buffer
 *                      length)
 * \param p_entropy     Entropy context
 * \param custom        Personalization data (Device specific identifiers)
 *                      (Can be NULL)
 * \param len           Length of personalization data
 *
 * \note                The "security strength" as defined by NIST is set to:
 *                      128 bits if md_alg is SHA-1,
 *                      192 bits if md_alg is SHA-224,
 *                      256 bits if md_alg is SHA-256 or higher.
 *                      Note that SHA-256 is just as efficient as SHA-224.
 *
 * \return              0 if successful, or
 *                      POLARSSL_ERR_MD_BAD_INPUT_DATA, or
 *                      POLARSSL_ERR_MD_ALLOC_FAILED, or
 *                      POLARSSL_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED.
 */
int hmac_drbg_init( hmac_drbg_context *ctx,
                    const md_info_t * md_info,
                    int (*f_entropy)(void *, unsigned char *, size_t),
                    void *p_entropy,
                    const unsigned char *custom,
                    size_t len );

/**
 * \brief               Initilisation of simpified HMAC_DRBG (never reseeds).
 *                      (For use with deterministic ECDSA.)
 *
 * \param ctx           HMAC_DRBG context to be initialised
 * \param md_info       MD algorithm to use for HMAC_DRBG
 * \param data          Concatenation of entropy string and additional data
 * \param data_len      Length of data in bytes
 *
 * \return              0 if successful, or
 *                      POLARSSL_ERR_MD_BAD_INPUT_DATA, or
 *                      POLARSSL_ERR_MD_ALLOC_FAILED.
 */
int hmac_drbg_init_buf( hmac_drbg_context *ctx,
                        const md_info_t * md_info,
                        const unsigned char *data, size_t data_len );

/**
 * \brief               Enable / disable prediction resistance (Default: Off)
 *
 * Note: If enabled, entropy is used for ctx->entropy_len before each call!
 *       Only use this if you have ample supply of good entropy!
 *
 * \param ctx           HMAC_DRBG context
 * \param resistance    POLARSSL_HMAC_DRBG_PR_ON or POLARSSL_HMAC_DRBG_PR_OFF
 */
void hmac_drbg_set_prediction_resistance( hmac_drbg_context *ctx,
                                          int resistance );

/**
 * \brief               Set the amount of entropy grabbed on each reseed
 *                      (Default: given by the security strength, which
 *                      depends on the hash used, see \c hmac_drbg_init() )
 *
 * \param ctx           HMAC_DRBG context
 * \param len           Amount of entropy to grab, in bytes
 */
void hmac_drbg_set_entropy_len( hmac_drbg_context *ctx,
                                size_t len );

/**
 * \brief               Set the reseed interval
 *                      (Default: POLARSSL_HMAC_DRBG_RESEED_INTERVAL)
 *
 * \param ctx           HMAC_DRBG context
 * \param interval      Reseed interval
 */
void hmac_drbg_set_reseed_interval( hmac_drbg_context *ctx,
                                    int interval );

/**
 * \brief               HMAC_DRBG update state
 *
 * \param ctx           HMAC_DRBG context
 * \param additional    Additional data to update state with, or NULL
 * \param add_len       Length of additional data, or 0
 *
 * \note                Additional data is optional, pass NULL and 0 as second
 *                      third argument if no additional data is being used.
 */
void hmac_drbg_update( hmac_drbg_context *ctx,
                       const unsigned char *additional, size_t add_len );

/**
 * \brief               HMAC_DRBG reseeding (extracts data from entropy source)
 *
 * \param ctx           HMAC_DRBG context
 * \param additional    Additional data to add to state (Can be NULL)
 * \param len           Length of additional data
 *
 * \return              0 if successful, or
 *                      POLARSSL_ERR_HMAC_DRBG_ENTROPY_SOURCE_FAILED
 */
int hmac_drbg_reseed( hmac_drbg_context *ctx,
                      const unsigned char *additional, size_t len );

/**
 * \brief               HMAC_DRBG generate random with additional update input
 *
 * Note: Automatically reseeds if reseed_counter is reached or PR is enabled.
 *
 * \param p_rng         HMAC_DRBG context
 * \param output        Buffer to fill
 * \param output_len    Length of the buffer
 * \param additional    Additional data to update with (can be NULL)
 * \param add_len       Length of additional data (can be 0)
 *
 * \return              0 if successful, or
 *                      POLARSSL_ERR_HMAC_DRBG_ENTROPY_SOURCE_FAILED, or
 *                      POLARSSL_ERR_HMAC_DRBG_REQUEST_TOO_BIG, or
 *                      POLARSSL_ERR_HMAC_DRBG_INPUT_TOO_BIG.
 */
int hmac_drbg_random_with_add( void *p_rng,
                               unsigned char *output, size_t output_len,
                               const unsigned char *additional,
                               size_t add_len );

/**
 * \brief               HMAC_DRBG generate random
 *
 * Note: Automatically reseeds if reseed_counter is reached or PR is enabled.
 *
 * \param p_rng         HMAC_DRBG context
 * \param output        Buffer to fill
 * \param out_len       Length of the buffer
 *
 * \return              0 if successful, or
 *                      POLARSSL_ERR_HMAC_DRBG_ENTROPY_SOURCE_FAILED, or
 *                      POLARSSL_ERR_HMAC_DRBG_REQUEST_TOO_BIG
 */
int hmac_drbg_random( void *p_rng, unsigned char *output, size_t out_len );

/**
 * \brief               Free an HMAC_DRBG context
 *
 * \param ctx           HMAC_DRBG context to free.
 */
void hmac_drbg_free( hmac_drbg_context *ctx );

#if defined(POLARSSL_FS_IO)
/**
 * \brief               Write a seed file
 *
 * \param ctx           HMAC_DRBG context
 * \param path          Name of the file
 *
 * \return              0 if successful, 1 on file error, or
 *                      POLARSSL_ERR_HMAC_DRBG_ENTROPY_SOURCE_FAILED
 */
int hmac_drbg_write_seed_file( hmac_drbg_context *ctx, const char *path );

/**
 * \brief               Read and update a seed file. Seed is added to this
 *                      instance
 *
 * \param ctx           HMAC_DRBG context
 * \param path          Name of the file
 *
 * \return              0 if successful, 1 on file error,
 *                      POLARSSL_ERR_HMAC_DRBG_ENTROPY_SOURCE_FAILED or
 *                      POLARSSL_ERR_HMAC_DRBG_INPUT_TOO_BIG
 */
int hmac_drbg_update_seed_file( hmac_drbg_context *ctx, const char *path );
#endif /* POLARSSL_FS_IO */


#if defined(POLARSSL_SELF_TEST)
/**
 * \brief               Checkup routine
 *
 * \return              0 if successful, or 1 if the test failed
 */
int hmac_drbg_self_test( int verbose );
#endif

#ifdef __cplusplus
}
#endif

#endif /* hmac_drbg.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/pkcs12.h ************/


/**
 * \file pkcs12.h
 *
 * \brief PKCS#12 Personal Information Exchange Syntax
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_PKCS12_H
#define POLARSSL_PKCS12_H





#include <stddef.h>

#define POLARSSL_ERR_PKCS12_BAD_INPUT_DATA                 -0x1F80  /**< Bad input parameters to function. */
#define POLARSSL_ERR_PKCS12_FEATURE_UNAVAILABLE            -0x1F00  /**< Feature not available, e.g. unsupported encryption scheme. */
#define POLARSSL_ERR_PKCS12_PBE_INVALID_FORMAT             -0x1E80  /**< PBE ASN.1 data not as expected. */
#define POLARSSL_ERR_PKCS12_PASSWORD_MISMATCH              -0x1E00  /**< Given private key password does not allow for correct decryption. */

#define PKCS12_DERIVE_KEY       1   /**< encryption/decryption key */
#define PKCS12_DERIVE_IV        2   /**< initialization vector     */
#define PKCS12_DERIVE_MAC_KEY   3   /**< integrity / MAC key       */

#define PKCS12_PBE_DECRYPT      0
#define PKCS12_PBE_ENCRYPT      1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief            PKCS12 Password Based function (encryption / decryption)
 *                   for pbeWithSHAAnd128BitRC4
 *
 * \param pbe_params an ASN1 buffer containing the pkcs-12PbeParams structure
 * \param mode       either PKCS12_PBE_ENCRYPT or PKCS12_PBE_DECRYPT
 * \param pwd        the password used (may be NULL if no password is used)
 * \param pwdlen     length of the password (may be 0)
 * \param input      the input data
 * \param len        data length
 * \param output     the output buffer
 *
 * \return           0 if successful, or a POLARSSL_ERR_xxx code
 */
int pkcs12_pbe_sha1_rc4_128( asn1_buf *pbe_params, int mode,
                             const unsigned char *pwd,  size_t pwdlen,
                             const unsigned char *input, size_t len,
                             unsigned char *output );

/**
 * \brief            PKCS12 Password Based function (encryption / decryption)
 *                   for cipher-based and md-based PBE's
 *
 * \param pbe_params an ASN1 buffer containing the pkcs-12PbeParams structure
 * \param mode       either PKCS12_PBE_ENCRYPT or PKCS12_PBE_DECRYPT
 * \param cipher_type the cipher used
 * \param md_type     the md used
 * \param pwd        the password used (may be NULL if no password is used)
 * \param pwdlen     length of the password (may be 0)
 * \param input      the input data
 * \param len        data length
 * \param output     the output buffer
 *
 * \return           0 if successful, or a POLARSSL_ERR_xxx code
 */
int pkcs12_pbe( asn1_buf *pbe_params, int mode,
                cipher_type_t cipher_type, md_type_t md_type,
                const unsigned char *pwd,  size_t pwdlen,
                const unsigned char *input, size_t len,
                unsigned char *output );

/**
 * \brief            The PKCS#12 derivation function uses a password and a salt
 *                   to produce pseudo-random bits for a particular "purpose".
 *
 *                   Depending on the given id, this function can produce an
 *                   encryption/decryption key, an nitialization vector or an
 *                   integrity key.
 *
 * \param data       buffer to store the derived data in
 * \param datalen    length to fill
 * \param pwd        password to use (may be NULL if no password is used)
 * \param pwdlen     length of the password (may be 0)
 * \param salt       salt buffer to use
 * \param saltlen    length of the salt
 * \param md         md type to use during the derivation
 * \param id         id that describes the purpose (can be PKCS12_DERIVE_KEY,
 *                   PKCS12_DERIVE_IV or PKCS12_DERIVE_MAC_KEY)
 * \param iterations number of iterations
 *
 * \return          0 if successful, or a MD, BIGNUM type error.
 */
int pkcs12_derivation( unsigned char *data, size_t datalen,
                       const unsigned char *pwd, size_t pwdlen,
                       const unsigned char *salt, size_t saltlen,
                       md_type_t md, int id, int iterations );

#ifdef __cplusplus
}
#endif

#endif /* pkcs12.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/pkcs5.h ************/


/**
 * \file pkcs5.h
 *
 * \brief PKCS#5 functions
 *
 * \author Mathias Olsson <mathias@kompetensum.com>
 *
 *  Copyright (C) 2006-2013, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_PKCS5_H
#define POLARSSL_PKCS5_H




#include <stddef.h>

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

#define POLARSSL_ERR_PKCS5_BAD_INPUT_DATA                  -0x3f80  /**< Bad input parameters to function. */
#define POLARSSL_ERR_PKCS5_INVALID_FORMAT                  -0x3f00  /**< Unexpected ASN.1 data. */
#define POLARSSL_ERR_PKCS5_FEATURE_UNAVAILABLE             -0x3e80  /**< Requested encryption or digest alg not available. */
#define POLARSSL_ERR_PKCS5_PASSWORD_MISMATCH               -0x3e00  /**< Given private key password does not allow for correct decryption. */

#define PKCS5_DECRYPT      0
#define PKCS5_ENCRYPT      1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          PKCS#5 PBES2 function
 *
 * \param pbe_params the ASN.1 algorithm parameters
 * \param mode       either PKCS5_DECRYPT or PKCS5_ENCRYPT
 * \param pwd        password to use when generating key
 * \param pwdlen     length of password
 * \param data       data to process
 * \param datalen    length of data
 * \param output     output buffer
 *
 * \returns        0 on success, or a POLARSSL_ERR_xxx code if verification fails.
 */
int pkcs5_pbes2( asn1_buf *pbe_params, int mode,
                 const unsigned char *pwd,  size_t pwdlen,
                 const unsigned char *data, size_t datalen,
                 unsigned char *output );

/**
 * \brief          PKCS#5 PBKDF2 using HMAC
 *
 * \param ctx      Generic HMAC context
 * \param password Password to use when generating key
 * \param plen     Length of password
 * \param salt     Salt to use when generating key
 * \param slen     Length of salt
 * \param iteration_count       Iteration count
 * \param key_length            Length of generated key
 * \param output   Generated key. Must be at least as big as key_length
 *
 * \returns        0 on success, or a POLARSSL_ERR_xxx code if verification fails.
 */
int pkcs5_pbkdf2_hmac( md_context_t *ctx, const unsigned char *password,
                       size_t plen, const unsigned char *salt, size_t slen,
                       unsigned int iteration_count,
                       uint32_t key_length, unsigned char *output );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int pkcs5_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* pkcs5.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/oid.h ************/


/**
 * \file oid.h
 *
 * \brief Object Identifier (OID) database
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_OID_H
#define POLARSSL_OID_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif




#include <stddef.h>

#if defined(POLARSSL_CIPHER_C)

#endif

#if defined(POLARSSL_MD_C)

#endif

#if defined(POLARSSL_X509_USE_C) || defined(POLARSSL_X509_CREATE_C)

#endif

#define POLARSSL_ERR_OID_NOT_FOUND                         -0x002E  /**< OID is not found. */
#define POLARSSL_ERR_OID_BUF_TOO_SMALL                     -0x000B  /**< output buffer is too small */

/*
 * Top level OID tuples
 */
#define OID_ISO_MEMBER_BODIES           "\x2a"          /* {iso(1) member-body(2)} */
#define OID_ISO_IDENTIFIED_ORG          "\x2b"          /* {iso(1) identified-organization(3)} */
#define OID_ISO_CCITT_DS                "\x55"          /* {joint-iso-ccitt(2) ds(5)} */
#define OID_ISO_ITU_COUNTRY             "\x60"          /* {joint-iso-itu-t(2) country(16)} */

/*
 * ISO Member bodies OID parts
 */
#define OID_COUNTRY_US                  "\x86\x48"      /* {us(840)} */
#define OID_ORG_RSA_DATA_SECURITY       "\x86\xf7\x0d"  /* {rsadsi(113549)} */
#define OID_RSA_COMPANY                 OID_ISO_MEMBER_BODIES OID_COUNTRY_US \
                                        OID_ORG_RSA_DATA_SECURITY /* {iso(1) member-body(2) us(840) rsadsi(113549)} */
#define OID_ORG_ANSI_X9_62              "\xce\x3d" /* ansi-X9-62(10045) */
#define OID_ANSI_X9_62                  OID_ISO_MEMBER_BODIES OID_COUNTRY_US \
                                        OID_ORG_ANSI_X9_62

/*
 * ISO Identified organization OID parts
 */
#define OID_ORG_DOD                     "\x06"          /* {dod(6)} */
#define OID_ORG_OIW                     "\x0e"
#define OID_OIW_SECSIG                  OID_ORG_OIW "\x03"
#define OID_OIW_SECSIG_ALG              OID_OIW_SECSIG "\x02"
#define OID_OIW_SECSIG_SHA1             OID_OIW_SECSIG_ALG "\x1a"
#define OID_ORG_CERTICOM                "\x81\x04"  /* certicom(132) */
#define OID_CERTICOM                    OID_ISO_IDENTIFIED_ORG OID_ORG_CERTICOM
#define OID_ORG_TELETRUST               "\x24" /* teletrust(36) */
#define OID_TELETRUST                   OID_ISO_IDENTIFIED_ORG OID_ORG_TELETRUST

/*
 * ISO ITU OID parts
 */
#define OID_ORGANIZATION                "\x01"          /* {organization(1)} */
#define OID_ISO_ITU_US_ORG              OID_ISO_ITU_COUNTRY OID_COUNTRY_US OID_ORGANIZATION /* {joint-iso-itu-t(2) country(16) us(840) organization(1)} */

#define OID_ORG_GOV                     "\x65"          /* {gov(101)} */
#define OID_GOV                         OID_ISO_ITU_US_ORG OID_ORG_GOV /* {joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101)} */

#define OID_ORG_NETSCAPE                "\x86\xF8\x42"  /* {netscape(113730)} */
#define OID_NETSCAPE                    OID_ISO_ITU_US_ORG OID_ORG_NETSCAPE /* Netscape OID {joint-iso-itu-t(2) country(16) us(840) organization(1) netscape(113730)} */

/* ISO arc for standard certificate and CRL extensions */
#define OID_ID_CE                       OID_ISO_CCITT_DS "\x1D" /**< id-ce OBJECT IDENTIFIER  ::=  {joint-iso-ccitt(2) ds(5) 29} */

/**
 * Private Internet Extensions
 * { iso(1) identified-organization(3) dod(6) internet(1)
 *                      security(5) mechanisms(5) pkix(7) }
 */
#define OID_PKIX                        OID_ISO_IDENTIFIED_ORG OID_ORG_DOD "\x01\x05\x05\x07"

/*
 * Arc for standard naming attributes
 */
#define OID_AT                          OID_ISO_CCITT_DS "\x04" /**< id-at OBJECT IDENTIFIER ::= {joint-iso-ccitt(2) ds(5) 4} */
#define OID_AT_CN                       OID_AT "\x03" /**< id-at-commonName AttributeType:= {id-at 3} */
#define OID_AT_SUR_NAME                 OID_AT "\x04" /**< id-at-surName AttributeType:= {id-at 4} */
#define OID_AT_SERIAL_NUMBER            OID_AT "\x05" /**< id-at-serialNumber AttributeType:= {id-at 5} */
#define OID_AT_COUNTRY                  OID_AT "\x06" /**< id-at-countryName AttributeType:= {id-at 6} */
#define OID_AT_LOCALITY                 OID_AT "\x07" /**< id-at-locality AttributeType:= {id-at 7} */
#define OID_AT_STATE                    OID_AT "\x08" /**< id-at-state AttributeType:= {id-at 8} */
#define OID_AT_ORGANIZATION             OID_AT "\x0A" /**< id-at-organizationName AttributeType:= {id-at 10} */
#define OID_AT_ORG_UNIT                 OID_AT "\x0B" /**< id-at-organizationalUnitName AttributeType:= {id-at 11} */
#define OID_AT_TITLE                    OID_AT "\x0C" /**< id-at-title AttributeType:= {id-at 12} */
#define OID_AT_POSTAL_ADDRESS           OID_AT "\x10" /**< id-at-postalAddress AttributeType:= {id-at 16} */
#define OID_AT_POSTAL_CODE              OID_AT "\x11" /**< id-at-postalCode AttributeType:= {id-at 17} */
#define OID_AT_GIVEN_NAME               OID_AT "\x2A" /**< id-at-givenName AttributeType:= {id-at 42} */
#define OID_AT_INITIALS                 OID_AT "\x2B" /**< id-at-initials AttributeType:= {id-at 43} */
#define OID_AT_GENERATION_QUALIFIER     OID_AT "\x2C" /**< id-at-generationQualifier AttributeType:= {id-at 44} */
#define OID_AT_UNIQUE_IDENTIFIER        OID_AT "\x2D" /**< id-at-uniqueIdentifier AttributType:= {id-at 45} */
#define OID_AT_DN_QUALIFIER             OID_AT "\x2E" /**< id-at-dnQualifier AttributeType:= {id-at 46} */
#define OID_AT_PSEUDONYM                OID_AT "\x41" /**< id-at-pseudonym AttributeType:= {id-at 65} */

#define OID_DOMAIN_COMPONENT            "\x09\x92\x26\x89\x93\xF2\x2C\x64\x01\x19" /** id-domainComponent AttributeType:= {itu-t(0) data(9) pss(2342) ucl(19200300) pilot(100) pilotAttributeType(1) domainComponent(25)} */

/*
 * OIDs for standard certificate extensions
 */
#define OID_AUTHORITY_KEY_IDENTIFIER    OID_ID_CE "\x23" /**< id-ce-authorityKeyIdentifier OBJECT IDENTIFIER ::=  { id-ce 35 } */
#define OID_SUBJECT_KEY_IDENTIFIER      OID_ID_CE "\x0E" /**< id-ce-subjectKeyIdentifier OBJECT IDENTIFIER ::=  { id-ce 14 } */
#define OID_KEY_USAGE                   OID_ID_CE "\x0F" /**< id-ce-keyUsage OBJECT IDENTIFIER ::=  { id-ce 15 } */
#define OID_CERTIFICATE_POLICIES        OID_ID_CE "\x20" /**< id-ce-certificatePolicies OBJECT IDENTIFIER ::=  { id-ce 32 } */
#define OID_POLICY_MAPPINGS             OID_ID_CE "\x21" /**< id-ce-policyMappings OBJECT IDENTIFIER ::=  { id-ce 33 } */
#define OID_SUBJECT_ALT_NAME            OID_ID_CE "\x11" /**< id-ce-subjectAltName OBJECT IDENTIFIER ::=  { id-ce 17 } */
#define OID_ISSUER_ALT_NAME             OID_ID_CE "\x12" /**< id-ce-issuerAltName OBJECT IDENTIFIER ::=  { id-ce 18 } */
#define OID_SUBJECT_DIRECTORY_ATTRS     OID_ID_CE "\x09" /**< id-ce-subjectDirectoryAttributes OBJECT IDENTIFIER ::=  { id-ce 9 } */
#define OID_BASIC_CONSTRAINTS           OID_ID_CE "\x13" /**< id-ce-basicConstraints OBJECT IDENTIFIER ::=  { id-ce 19 } */
#define OID_NAME_CONSTRAINTS            OID_ID_CE "\x1E" /**< id-ce-nameConstraints OBJECT IDENTIFIER ::=  { id-ce 30 } */
#define OID_POLICY_CONSTRAINTS          OID_ID_CE "\x24" /**< id-ce-policyConstraints OBJECT IDENTIFIER ::=  { id-ce 36 } */
#define OID_EXTENDED_KEY_USAGE          OID_ID_CE "\x25" /**< id-ce-extKeyUsage OBJECT IDENTIFIER ::= { id-ce 37 } */
#define OID_CRL_DISTRIBUTION_POINTS     OID_ID_CE "\x1F" /**< id-ce-cRLDistributionPoints OBJECT IDENTIFIER ::=  { id-ce 31 } */
#define OID_INIHIBIT_ANYPOLICY          OID_ID_CE "\x36" /**< id-ce-inhibitAnyPolicy OBJECT IDENTIFIER ::=  { id-ce 54 } */
#define OID_FRESHEST_CRL                OID_ID_CE "\x2E" /**< id-ce-freshestCRL OBJECT IDENTIFIER ::=  { id-ce 46 } */

/*
 * Netscape certificate extensions
 */
#define OID_NS_CERT                 OID_NETSCAPE "\x01"
#define OID_NS_CERT_TYPE            OID_NS_CERT  "\x01"
#define OID_NS_BASE_URL             OID_NS_CERT  "\x02"
#define OID_NS_REVOCATION_URL       OID_NS_CERT  "\x03"
#define OID_NS_CA_REVOCATION_URL    OID_NS_CERT  "\x04"
#define OID_NS_RENEWAL_URL          OID_NS_CERT  "\x07"
#define OID_NS_CA_POLICY_URL        OID_NS_CERT  "\x08"
#define OID_NS_SSL_SERVER_NAME      OID_NS_CERT  "\x0C"
#define OID_NS_COMMENT              OID_NS_CERT  "\x0D"
#define OID_NS_DATA_TYPE            OID_NETSCAPE "\x02"
#define OID_NS_CERT_SEQUENCE        OID_NS_DATA_TYPE "\x05"

/*
 * OIDs for CRL extensions
 */
#define OID_PRIVATE_KEY_USAGE_PERIOD    OID_ID_CE "\x10"
#define OID_CRL_NUMBER                  OID_ID_CE "\x14" /**< id-ce-cRLNumber OBJECT IDENTIFIER ::= { id-ce 20 } */

/*
 * X.509 v3 Extended key usage OIDs
 */
#define OID_ANY_EXTENDED_KEY_USAGE      OID_EXTENDED_KEY_USAGE "\x00" /**< anyExtendedKeyUsage OBJECT IDENTIFIER ::= { id-ce-extKeyUsage 0 } */

#define OID_KP                          OID_PKIX "\x03" /**< id-kp OBJECT IDENTIFIER ::= { id-pkix 3 } */
#define OID_SERVER_AUTH                 OID_KP "\x01" /**< id-kp-serverAuth OBJECT IDENTIFIER ::= { id-kp 1 } */
#define OID_CLIENT_AUTH                 OID_KP "\x02" /**< id-kp-clientAuth OBJECT IDENTIFIER ::= { id-kp 2 } */
#define OID_CODE_SIGNING                OID_KP "\x03" /**< id-kp-codeSigning OBJECT IDENTIFIER ::= { id-kp 3 } */
#define OID_EMAIL_PROTECTION            OID_KP "\x04" /**< id-kp-emailProtection OBJECT IDENTIFIER ::= { id-kp 4 } */
#define OID_TIME_STAMPING               OID_KP "\x08" /**< id-kp-timeStamping OBJECT IDENTIFIER ::= { id-kp 8 } */
#define OID_OCSP_SIGNING                OID_KP "\x09" /**< id-kp-OCSPSigning OBJECT IDENTIFIER ::= { id-kp 9 } */

/*
 * PKCS definition OIDs
 */

#define OID_PKCS                OID_RSA_COMPANY "\x01" /**< pkcs OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) 1 } */
#define OID_PKCS1               OID_PKCS "\x01" /**< pkcs-1 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) 1 } */
#define OID_PKCS5               OID_PKCS "\x05" /**< pkcs-5 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) 5 } */
#define OID_PKCS9               OID_PKCS "\x09" /**< pkcs-9 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) 9 } */
#define OID_PKCS12              OID_PKCS "\x0c" /**< pkcs-12 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) 12 } */

/*
 * PKCS#1 OIDs
 */
#define OID_PKCS1_RSA           OID_PKCS1 "\x01" /**< rsaEncryption OBJECT IDENTIFIER ::= { pkcs-1 1 } */
#define OID_PKCS1_MD2           OID_PKCS1 "\x02" /**< md2WithRSAEncryption ::= { pkcs-1 2 } */
#define OID_PKCS1_MD4           OID_PKCS1 "\x03" /**< md4WithRSAEncryption ::= { pkcs-1 3 } */
#define OID_PKCS1_MD5           OID_PKCS1 "\x04" /**< md5WithRSAEncryption ::= { pkcs-1 4 } */
#define OID_PKCS1_SHA1          OID_PKCS1 "\x05" /**< sha1WithRSAEncryption ::= { pkcs-1 5 } */
#define OID_PKCS1_SHA224        OID_PKCS1 "\x0e" /**< sha224WithRSAEncryption ::= { pkcs-1 14 } */
#define OID_PKCS1_SHA256        OID_PKCS1 "\x0b" /**< sha256WithRSAEncryption ::= { pkcs-1 11 } */
#define OID_PKCS1_SHA384        OID_PKCS1 "\x0c" /**< sha384WithRSAEncryption ::= { pkcs-1 12 } */
#define OID_PKCS1_SHA512        OID_PKCS1 "\x0d" /**< sha512WithRSAEncryption ::= { pkcs-1 13 } */

#define OID_RSA_SHA_OBS         "\x2B\x0E\x03\x02\x1D"

#define OID_PKCS9_EMAIL         OID_PKCS9 "\x01" /**< emailAddress AttributeType ::= { pkcs-9 1 } */

/* RFC 4055 */
#define OID_RSASSA_PSS          OID_PKCS1 "\x0a" /**< id-RSASSA-PSS ::= { pkcs-1 10 } */
#define OID_MGF1                OID_PKCS1 "\x08" /**< id-mgf1 ::= { pkcs-1 8 } */

/*
 * Digest algorithms
 */
#define OID_DIGEST_ALG_MD2              OID_RSA_COMPANY "\x02\x02" /**< id-md2 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) 2 } */
#define OID_DIGEST_ALG_MD4              OID_RSA_COMPANY "\x02\x04" /**< id-md4 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) 4 } */
#define OID_DIGEST_ALG_MD5              OID_RSA_COMPANY "\x02\x05" /**< id-md5 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) 5 } */
#define OID_DIGEST_ALG_SHA1             OID_ISO_IDENTIFIED_ORG OID_OIW_SECSIG_SHA1 /**< id-sha1 OBJECT IDENTIFIER ::= { iso(1) identified-organization(3) oiw(14) secsig(3) algorithms(2) 26 } */
#define OID_DIGEST_ALG_SHA224           OID_GOV "\x03\x04\x02\x04" /**< id-sha224 OBJECT IDENTIFIER ::= { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3) nistalgorithm(4) hashalgs(2) 4 } */
#define OID_DIGEST_ALG_SHA256           OID_GOV "\x03\x04\x02\x01" /**< id-sha256 OBJECT IDENTIFIER ::= { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3) nistalgorithm(4) hashalgs(2) 1 } */

#define OID_DIGEST_ALG_SHA384           OID_GOV "\x03\x04\x02\x02" /**< id-sha384 OBJECT IDENTIFIER ::= { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3) nistalgorithm(4) hashalgs(2) 2 } */

#define OID_DIGEST_ALG_SHA512           OID_GOV "\x03\x04\x02\x03" /**< id-sha512 OBJECT IDENTIFIER ::= { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3) nistalgorithm(4) hashalgs(2) 3 } */

#define OID_HMAC_SHA1                   OID_RSA_COMPANY "\x02\x07" /**< id-hmacWithSHA1 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) 7 } */

/*
 * Encryption algorithms
 */
#define OID_DES_CBC                     OID_ISO_IDENTIFIED_ORG OID_OIW_SECSIG_ALG "\x07" /**< desCBC OBJECT IDENTIFIER ::= { iso(1) identified-organization(3) oiw(14) secsig(3) algorithms(2) 7 } */
#define OID_DES_EDE3_CBC                OID_RSA_COMPANY "\x03\x07" /**< des-ede3-cbc OBJECT IDENTIFIER ::= { iso(1) member-body(2) -- us(840) rsadsi(113549) encryptionAlgorithm(3) 7 } */

/*
 * PKCS#5 OIDs
 */
#define OID_PKCS5_PBKDF2                OID_PKCS5 "\x0c" /**< id-PBKDF2 OBJECT IDENTIFIER ::= {pkcs-5 12} */
#define OID_PKCS5_PBES2                 OID_PKCS5 "\x0d" /**< id-PBES2 OBJECT IDENTIFIER ::= {pkcs-5 13} */
#define OID_PKCS5_PBMAC1                OID_PKCS5 "\x0e" /**< id-PBMAC1 OBJECT IDENTIFIER ::= {pkcs-5 14} */

/*
 * PKCS#5 PBES1 algorithms
 */
#define OID_PKCS5_PBE_MD2_DES_CBC       OID_PKCS5 "\x01" /**< pbeWithMD2AndDES-CBC OBJECT IDENTIFIER ::= {pkcs-5 1} */
#define OID_PKCS5_PBE_MD2_RC2_CBC       OID_PKCS5 "\x04" /**< pbeWithMD2AndRC2-CBC OBJECT IDENTIFIER ::= {pkcs-5 4} */
#define OID_PKCS5_PBE_MD5_DES_CBC       OID_PKCS5 "\x03" /**< pbeWithMD5AndDES-CBC OBJECT IDENTIFIER ::= {pkcs-5 3} */
#define OID_PKCS5_PBE_MD5_RC2_CBC       OID_PKCS5 "\x06" /**< pbeWithMD5AndRC2-CBC OBJECT IDENTIFIER ::= {pkcs-5 6} */
#define OID_PKCS5_PBE_SHA1_DES_CBC      OID_PKCS5 "\x0a" /**< pbeWithSHA1AndDES-CBC OBJECT IDENTIFIER ::= {pkcs-5 10} */
#define OID_PKCS5_PBE_SHA1_RC2_CBC      OID_PKCS5 "\x0b" /**< pbeWithSHA1AndRC2-CBC OBJECT IDENTIFIER ::= {pkcs-5 11} */

/*
 * PKCS#8 OIDs
 */
#define OID_PKCS9_CSR_EXT_REQ           OID_PKCS9 "\x0e" /**< extensionRequest OBJECT IDENTIFIER ::= {pkcs-9 14} */

/*
 * PKCS#12 PBE OIDs
 */
#define OID_PKCS12_PBE                      OID_PKCS12 "\x01" /**< pkcs-12PbeIds OBJECT IDENTIFIER ::= {pkcs-12 1} */

#define OID_PKCS12_PBE_SHA1_RC4_128         OID_PKCS12_PBE "\x01" /**< pbeWithSHAAnd128BitRC4 OBJECT IDENTIFIER ::= {pkcs-12PbeIds 1} */
#define OID_PKCS12_PBE_SHA1_RC4_40          OID_PKCS12_PBE "\x02" /**< pbeWithSHAAnd40BitRC4 OBJECT IDENTIFIER ::= {pkcs-12PbeIds 2} */
#define OID_PKCS12_PBE_SHA1_DES3_EDE_CBC    OID_PKCS12_PBE "\x03" /**< pbeWithSHAAnd3-KeyTripleDES-CBC OBJECT IDENTIFIER ::= {pkcs-12PbeIds 3} */
#define OID_PKCS12_PBE_SHA1_DES2_EDE_CBC    OID_PKCS12_PBE "\x04" /**< pbeWithSHAAnd2-KeyTripleDES-CBC OBJECT IDENTIFIER ::= {pkcs-12PbeIds 4} */
#define OID_PKCS12_PBE_SHA1_RC2_128_CBC     OID_PKCS12_PBE "\x05" /**< pbeWithSHAAnd128BitRC2-CBC OBJECT IDENTIFIER ::= {pkcs-12PbeIds 5} */
#define OID_PKCS12_PBE_SHA1_RC2_40_CBC      OID_PKCS12_PBE "\x06" /**< pbeWithSHAAnd40BitRC2-CBC OBJECT IDENTIFIER ::= {pkcs-12PbeIds 6} */

/*
 * EC key algorithms from RFC 5480
 */

/* id-ecPublicKey OBJECT IDENTIFIER ::= {
 *       iso(1) member-body(2) us(840) ansi-X9-62(10045) keyType(2) 1 } */
#define OID_EC_ALG_UNRESTRICTED         OID_ANSI_X9_62 "\x02\01"

/*   id-ecDH OBJECT IDENTIFIER ::= {
 *     iso(1) identified-organization(3) certicom(132)
 *     schemes(1) ecdh(12) } */
#define OID_EC_ALG_ECDH                 OID_CERTICOM "\x01\x0c"

/*
 * ECParameters namedCurve identifiers, from RFC 5480, RFC 5639, and SEC2
 */

/* secp192r1 OBJECT IDENTIFIER ::= {
 *   iso(1) member-body(2) us(840) ansi-X9-62(10045) curves(3) prime(1) 1 } */
#define OID_EC_GRP_SECP192R1        OID_ANSI_X9_62 "\x03\x01\x01"

/* secp224r1 OBJECT IDENTIFIER ::= {
 *   iso(1) identified-organization(3) certicom(132) curve(0) 33 } */
#define OID_EC_GRP_SECP224R1        OID_CERTICOM "\x00\x21"

/* secp256r1 OBJECT IDENTIFIER ::= {
 *   iso(1) member-body(2) us(840) ansi-X9-62(10045) curves(3) prime(1) 7 } */
#define OID_EC_GRP_SECP256R1        OID_ANSI_X9_62 "\x03\x01\x07"

/* secp384r1 OBJECT IDENTIFIER ::= {
 *   iso(1) identified-organization(3) certicom(132) curve(0) 34 } */
#define OID_EC_GRP_SECP384R1        OID_CERTICOM "\x00\x22"

/* secp521r1 OBJECT IDENTIFIER ::= {
 *   iso(1) identified-organization(3) certicom(132) curve(0) 35 } */
#define OID_EC_GRP_SECP521R1        OID_CERTICOM "\x00\x23"

/* secp192k1 OBJECT IDENTIFIER ::= {
 *   iso(1) identified-organization(3) certicom(132) curve(0) 31 } */
#define OID_EC_GRP_SECP192K1        OID_CERTICOM "\x00\x1f"

/* secp224k1 OBJECT IDENTIFIER ::= {
 *   iso(1) identified-organization(3) certicom(132) curve(0) 32 } */
#define OID_EC_GRP_SECP224K1        OID_CERTICOM "\x00\x20"

/* secp256k1 OBJECT IDENTIFIER ::= {
 *   iso(1) identified-organization(3) certicom(132) curve(0) 10 } */
#define OID_EC_GRP_SECP256K1        OID_CERTICOM "\x00\x0a"

/* RFC 5639 4.1
 * ecStdCurvesAndGeneration OBJECT IDENTIFIER::= {iso(1)
 * identified-organization(3) teletrust(36) algorithm(3) signature-
 * algorithm(3) ecSign(2) 8}
 * ellipticCurve OBJECT IDENTIFIER ::= {ecStdCurvesAndGeneration 1}
 * versionOne OBJECT IDENTIFIER ::= {ellipticCurve 1} */
#define OID_EC_BRAINPOOL_V1         OID_TELETRUST "\x03\x03\x02\x08\x01\x01"

/* brainpoolP256r1 OBJECT IDENTIFIER ::= {versionOne 7} */
#define OID_EC_GRP_BP256R1          OID_EC_BRAINPOOL_V1 "\x07"

/* brainpoolP384r1 OBJECT IDENTIFIER ::= {versionOne 11} */
#define OID_EC_GRP_BP384R1          OID_EC_BRAINPOOL_V1 "\x0B"

/* brainpoolP512r1 OBJECT IDENTIFIER ::= {versionOne 13} */
#define OID_EC_GRP_BP512R1          OID_EC_BRAINPOOL_V1 "\x0D"

/*
 * SEC1 C.1
 *
 * prime-field OBJECT IDENTIFIER ::= { id-fieldType 1 }
 * id-fieldType OBJECT IDENTIFIER ::= { ansi-X9-62 fieldType(1)}
 */
#define OID_ANSI_X9_62_FIELD_TYPE   OID_ANSI_X9_62 "\x01"
#define OID_ANSI_X9_62_PRIME_FIELD  OID_ANSI_X9_62_FIELD_TYPE "\x01"

/*
 * ECDSA signature identifiers, from RFC 5480
 */
#define OID_ANSI_X9_62_SIG          OID_ANSI_X9_62 "\x04" /* signatures(4) */
#define OID_ANSI_X9_62_SIG_SHA2     OID_ANSI_X9_62_SIG "\x03" /* ecdsa-with-SHA2(3) */

/* ecdsa-with-SHA1 OBJECT IDENTIFIER ::= {
 *   iso(1) member-body(2) us(840) ansi-X9-62(10045) signatures(4) 1 } */
#define OID_ECDSA_SHA1              OID_ANSI_X9_62_SIG "\x01"

/* ecdsa-with-SHA224 OBJECT IDENTIFIER ::= {
 *   iso(1) member-body(2) us(840) ansi-X9-62(10045) signatures(4)
 *   ecdsa-with-SHA2(3) 1 } */
#define OID_ECDSA_SHA224            OID_ANSI_X9_62_SIG_SHA2 "\x01"

/* ecdsa-with-SHA256 OBJECT IDENTIFIER ::= {
 *   iso(1) member-body(2) us(840) ansi-X9-62(10045) signatures(4)
 *   ecdsa-with-SHA2(3) 2 } */
#define OID_ECDSA_SHA256            OID_ANSI_X9_62_SIG_SHA2 "\x02"

/* ecdsa-with-SHA384 OBJECT IDENTIFIER ::= {
 *   iso(1) member-body(2) us(840) ansi-X9-62(10045) signatures(4)
 *   ecdsa-with-SHA2(3) 3 } */
#define OID_ECDSA_SHA384            OID_ANSI_X9_62_SIG_SHA2 "\x03"

/* ecdsa-with-SHA512 OBJECT IDENTIFIER ::= {
 *   iso(1) member-body(2) us(840) ansi-X9-62(10045) signatures(4)
 *   ecdsa-with-SHA2(3) 4 } */
#define OID_ECDSA_SHA512            OID_ANSI_X9_62_SIG_SHA2 "\x04"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Base OID descriptor structure
 */
typedef struct {
    const char *asn1;               /*!< OID ASN.1 representation       */
    size_t asn1_len;                /*!< length of asn1                 */
    const char *name;               /*!< official name (e.g. from RFC)  */
    const char *description;        /*!< human friendly description     */
} oid_descriptor_t;

/**
 * \brief           Translate an ASN.1 OID into its numeric representation
 *                  (e.g. "\x2A\x86\x48\x86\xF7\x0D" into "1.2.840.113549")
 *
 * \param buf       buffer to put representation in
 * \param size      size of the buffer
 * \param oid       OID to translate
 *
 * \return          Length of the string written (excluding final NULL) or
 *                  POLARSSL_ERR_OID_BUF_TOO_SMALL in case of error
 */
int oid_get_numeric_string( char *buf, size_t size, const asn1_buf *oid );

#if defined(POLARSSL_X509_USE_C) || defined(POLARSSL_X509_CREATE_C)
/**
 * \brief          Translate an X.509 extension OID into local values
 *
 * \param oid      OID to use
 * \param ext_type place to store the extension type
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_x509_ext_type( const asn1_buf *oid, int *ext_type );
#endif

/**
 * \brief          Translate an X.509 attribute type OID into the short name
 *                 (e.g. the OID for an X520 Common Name into "CN")
 *
 * \param oid      OID to use
 * \param short_name    place to store the string pointer
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_attr_short_name( const asn1_buf *oid, const char **short_name );

/**
 * \brief          Translate PublicKeyAlgorithm OID into pk_type
 *
 * \param oid      OID to use
 * \param pk_alg   place to store public key algorithm
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_pk_alg( const asn1_buf *oid, pk_type_t *pk_alg );

/**
 * \brief          Translate pk_type into PublicKeyAlgorithm OID
 *
 * \param pk_alg   Public key type to look for
 * \param oid      place to store ASN.1 OID string pointer
 * \param olen     length of the OID
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_oid_by_pk_alg( pk_type_t pk_alg,
                           const char **oid, size_t *olen );

#if defined(POLARSSL_ECP_C)
/**
 * \brief          Translate NamedCurve OID into an EC group identifier
 *
 * \param oid      OID to use
 * \param grp_id   place to store group id
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_ec_grp( const asn1_buf *oid, ecp_group_id *grp_id );

/**
 * \brief          Translate EC group identifier into NamedCurve OID
 *
 * \param grp_id   EC group identifier
 * \param oid      place to store ASN.1 OID string pointer
 * \param olen     length of the OID
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_oid_by_ec_grp( ecp_group_id grp_id,
                           const char **oid, size_t *olen );
#endif /* POLARSSL_ECP_C */

#if defined(POLARSSL_MD_C)
/**
 * \brief          Translate SignatureAlgorithm OID into md_type and pk_type
 *
 * \param oid      OID to use
 * \param md_alg   place to store message digest algorithm
 * \param pk_alg   place to store public key algorithm
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_sig_alg( const asn1_buf *oid,
                     md_type_t *md_alg, pk_type_t *pk_alg );

/**
 * \brief          Translate SignatureAlgorithm OID into description
 *
 * \param oid      OID to use
 * \param desc     place to store string pointer
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_sig_alg_desc( const asn1_buf *oid, const char **desc );

/**
 * \brief          Translate md_type and pk_type into SignatureAlgorithm OID
 *
 * \param md_alg   message digest algorithm
 * \param pk_alg   public key algorithm
 * \param oid      place to store ASN.1 OID string pointer
 * \param olen     length of the OID
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_oid_by_sig_alg( pk_type_t pk_alg, md_type_t md_alg,
                            const char **oid, size_t *olen );

/**
 * \brief          Translate hash algorithm OID into md_type
 *
 * \param oid      OID to use
 * \param md_alg   place to store message digest algorithm
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_md_alg( const asn1_buf *oid, md_type_t *md_alg );
#endif /* POLARSSL_MD_C */

/**
 * \brief          Translate Extended Key Usage OID into description
 *
 * \param oid      OID to use
 * \param desc     place to store string pointer
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_extended_key_usage( const asn1_buf *oid, const char **desc );

/**
 * \brief          Translate md_type into hash algorithm OID
 *
 * \param md_alg   message digest algorithm
 * \param oid      place to store ASN.1 OID string pointer
 * \param olen     length of the OID
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_oid_by_md( md_type_t md_alg, const char **oid, size_t *olen );

#if defined(POLARSSL_CIPHER_C)
/**
 * \brief          Translate encryption algorithm OID into cipher_type
 *
 * \param oid           OID to use
 * \param cipher_alg    place to store cipher algorithm
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_cipher_alg( const asn1_buf *oid, cipher_type_t *cipher_alg );
#endif /* POLARSSL_CIPHER_C */

#if defined(POLARSSL_PKCS12_C)
/**
 * \brief          Translate PKCS#12 PBE algorithm OID into md_type and
 *                 cipher_type
 *
 * \param oid           OID to use
 * \param md_alg        place to store message digest algorithm
 * \param cipher_alg    place to store cipher algorithm
 *
 * \return         0 if successful, or POLARSSL_ERR_OID_NOT_FOUND
 */
int oid_get_pkcs12_pbe_alg( const asn1_buf *oid, md_type_t *md_alg,
                            cipher_type_t *cipher_alg );
#endif /* POLARSSL_PKCS12_C */

#ifdef __cplusplus
}
#endif

#endif /* oid.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/pbkdf2.h ************/


/**
 * \file pbkdf2.h
 *
 * \brief Password-Based Key Derivation Function 2 (from PKCS#5)
 *
 * \deprecated Use pkcs5.h instead.
 *
 * \author Mathias Olsson <mathias@kompetensum.com>
 *
 *  Copyright (C) 2006-2012, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_PBKDF2_H
#define POLARSSL_PBKDF2_H



#include <stddef.h>

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

#define POLARSSL_ERR_PBKDF2_BAD_INPUT_DATA                 -0x007C  /**< Bad input parameters to function. */

#ifdef __cplusplus
extern "C" {
#endif

#if ! defined(POLARSSL_DEPRECATED_REMOVED)
#if defined(POLARSSL_DEPRECATED_WARNING)
#define DEPRECATED    __attribute__((deprecated))
#else
#define DEPRECATED
#endif
/**
 * \brief          PKCS#5 PBKDF2 using HMAC
 *
 * \deprecated     Use pkcs5_pbkdf2_hmac() instead
 *
 * \param ctx      Generic HMAC context
 * \param password Password to use when generating key
 * \param plen     Length of password
 * \param salt     Salt to use when generating key
 * \param slen     Length of salt
 * \param iteration_count       Iteration count
 * \param key_length            Length of generated key
 * \param output   Generated key. Must be at least as big as key_length
 *
 * \returns        0 on success, or a POLARSSL_ERR_xxx code if verification fails.
 */
int pbkdf2_hmac( md_context_t *ctx, const unsigned char *password,
                 size_t plen, const unsigned char *salt, size_t slen,
                 unsigned int iteration_count,
                 uint32_t key_length, unsigned char *output ) DEPRECATED;

/**
 * \brief          Checkup routine
 *
 * \deprecated     Use pkcs5_self_test() instead
 *
 * \return         0 if successful, or 1 if the test failed
 */
int pbkdf2_self_test( int verbose ) DEPRECATED;
#undef DEPRECATED
#endif /* POLARSSL_DEPRECATED_REMOVED */

#ifdef __cplusplus
}
#endif

#endif /* pbkdf2.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/ripemd160.h ************/


/**
 * \file ripemd160.h
 *
 * \brief RIPE MD-160 message digest
 *
 *  Copyright (C) 2014-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_RIPEMD160_H
#define POLARSSL_RIPEMD160_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

#include <stddef.h>

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#elif !VXWORKS
#include <inttypes.h>
#endif

#define POLARSSL_ERR_RIPEMD160_FILE_IO_ERROR              -0x007E  /**< Read/write error in file. */

#if !defined(POLARSSL_RIPEMD160_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          RIPEMD-160 context structure
 */
typedef struct
{
    uint32_t total[2];          /*!< number of bytes processed  */
    uint32_t state[5];          /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */

    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
}
ripemd160_context;

/**
 * \brief          Initialize RIPEMD-160 context
 *
 * \param ctx      RIPEMD-160 context to be initialized
 */
void ripemd160_init( ripemd160_context *ctx );

/**
 * \brief          Clear RIPEMD-160 context
 *
 * \param ctx      RIPEMD-160 context to be cleared
 */
void ripemd160_free( ripemd160_context *ctx );

/**
 * \brief          RIPEMD-160 context setup
 *
 * \param ctx      context to be initialized
 */
void ripemd160_starts( ripemd160_context *ctx );

/**
 * \brief          RIPEMD-160 process buffer
 *
 * \param ctx      RIPEMD-160 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void ripemd160_update( ripemd160_context *ctx,
                       const unsigned char *input, size_t ilen );

/**
 * \brief          RIPEMD-160 final digest
 *
 * \param ctx      RIPEMD-160 context
 * \param output   RIPEMD-160 checksum result
 */
void ripemd160_finish( ripemd160_context *ctx, unsigned char output[20] );

/* Internal use */
void ripemd160_process( ripemd160_context *ctx, const unsigned char data[64] );

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_RIPEMD160_ALT */

#endif /* POLARSSL_RIPEMD160_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Output = RIPEMD-160( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   RIPEMD-160 checksum result
 */
void ripemd160( const unsigned char *input, size_t ilen,
                unsigned char output[20] );

#if defined(POLARSSL_FS_IO)
/**
 * \brief          Output = RIPEMD-160( file contents )
 *
 * \param path     input file name
 * \param output   RIPEMD-160 checksum result
 *
 * \return         0 if successful, or POLARSSL_ERR_RIPEMD160_FILE_IO_ERROR
 */
int ripemd160_file( const char *path, unsigned char output[20] );
#endif /* POLARSSL_FS_IO */

/**
 * \brief          RIPEMD-160 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void ripemd160_hmac_starts( ripemd160_context *ctx,
                            const unsigned char *key, size_t keylen );

/**
 * \brief          RIPEMD-160 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void ripemd160_hmac_update( ripemd160_context *ctx,
                            const unsigned char *input, size_t ilen );

/**
 * \brief          RIPEMD-160 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   RIPEMD-160 HMAC checksum result
 */
void ripemd160_hmac_finish( ripemd160_context *ctx, unsigned char output[20] );

/**
 * \brief          RIPEMD-160 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void ripemd160_hmac_reset( ripemd160_context *ctx );

/**
 * \brief          Output = HMAC-RIPEMD-160( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-RIPEMD-160 result
 */
void ripemd160_hmac( const unsigned char *key, size_t keylen,
                     const unsigned char *input, size_t ilen,
                     unsigned char output[20] );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int ripemd160_self_test( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* ripemd160.h */



/********* Start of file /Users/mob/.paks/mbedtls/ARMmbed/1.3.11/include/polarssl/version.h ************/


/**
 * \file version.h
 *
 * \brief Run-time version information
 *
 *  Copyright (C) 2006-2014, ARM Limited, All Rights Reserved
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * This set of compile-time defines and run-time variables can be used to
 * determine the version number of the mbed TLS library used.
 */
#ifndef POLARSSL_VERSION_H
#define POLARSSL_VERSION_H

#if !defined(POLARSSL_CONFIG_FILE)

#else
#include POLARSSL_CONFIG_FILE
#endif

/**
 * The version number x.y.z is split into three parts.
 * Major, Minor, Patchlevel
 */
#define POLARSSL_VERSION_MAJOR  1
#define POLARSSL_VERSION_MINOR  3
#define POLARSSL_VERSION_PATCH  11

/**
 * The single version number has the following structure:
 *    MMNNPP00
 *    Major version | Minor version | Patch version
 */
#define POLARSSL_VERSION_NUMBER         0x01030B00
#define POLARSSL_VERSION_STRING         "1.3.11"
#define POLARSSL_VERSION_STRING_FULL    "mbed TLS 1.3.11"

#if defined(POLARSSL_VERSION_C)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the version number.
 *
 * \return          The constructed version number in the format
 *                  MMNNPP00 (Major, Minor, Patch).
 */
unsigned int version_get_number( void );

/**
 * Get the version string ("x.y.z").
 *
 * \param string    The string that will receive the value.
 *                  (Should be at least 9 bytes in size)
 */
void version_get_string( char *string );

/**
 * Get the full version string ("mbed TLS x.y.z").
 *
 * \param string    The string that will receive the value. The mbed TLS version
 *                  string will use 18 bytes AT MOST including a terminating
 *                  null byte.
 *                  (So the buffer should be at least 18 bytes to receive this
 *                  version string).
 */
void version_get_string_full( char *string );

/**
 * \brief           Check if support for a feature was compiled into this
 *                  mbed TLS binary. This allows you to see at runtime if the
 *                  library was for instance compiled with or without
 *                  Multi-threading support.
 *
 *                  Note: only checks against defines in the sections "System
 *                        support", "mbed TLS modules" and "mbed TLS feature
 *                        support" in config.h
 *
 * \param feature   The string for the define to check (e.g. "POLARSSL_AES_C")
 *
 * \return          0 if the feature is present, -1 if the feature is not
 *                  present and -2 if support for feature checking as a whole
 *                  was not compiled in.
 */
int version_check_feature( const char *feature );

#ifdef __cplusplus
}
#endif

#endif /* POLARSSL_VERSION_C */

#endif /* version.h */

#endif /* ME_COM_MBEDTLS */
