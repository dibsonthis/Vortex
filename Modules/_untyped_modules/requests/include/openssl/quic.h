/*
 * Copyright 2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OPENSSL_QUIC_H
# define OPENSSL_QUIC_H
# pragma once

# include "macros.h"
# include "ssl.h"

# ifndef OPENSSL_NO_QUIC

#  ifdef __cplusplus
extern "C" {
#  endif

/*
 * Method used for non-thread-assisted QUIC client operation.
 */
__owur const SSL_METHOD *OSSL_QUIC_client_method(void);
/*
 * Method used for thread-assisted QUIC client operation.
 */
__owur const SSL_METHOD *OSSL_QUIC_client_thread_method(void);
/*
 * This method does not implement full server operation,
 * is used for testing, and is subject to change in later
 * releases.
 */
__owur const SSL_METHOD *OSSL_QUIC_server_method(void);

#  ifdef __cplusplus
}
#  endif

# endif /* OPENSSL_NO_QUIC */
#endif
