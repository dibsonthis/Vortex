/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OPENSSL_COMPERR_H
# define OPENSSL_COMPERR_H
# pragma once

# include "opensslconf.h"
# include "symhacks.h"
# include "cryptoerr_legacy.h"


# ifndef OPENSSL_NO_COMP


/*
 * COMP reason codes.
 */
#  define COMP_R_BROTLI_DECODE_ERROR                       102
#  define COMP_R_BROTLI_ENCODE_ERROR                       103
#  define COMP_R_BROTLI_NOT_SUPPORTED                      104
#  define COMP_R_ZLIB_DEFLATE_ERROR                        99
#  define COMP_R_ZLIB_INFLATE_ERROR                        100
#  define COMP_R_ZLIB_NOT_SUPPORTED                        101
#  define COMP_R_ZSTD_COMPRESS_ERROR                       105
#  define COMP_R_ZSTD_DECODE_ERROR                         106
#  define COMP_R_ZSTD_DECOMPRESS_ERROR                     107
#  define COMP_R_ZSTD_NOT_SUPPORTED                        108

# endif
#endif
