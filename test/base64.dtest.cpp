/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <base64.h>

module("base64")
.dependsOn({
    "exception"
});

using namespace spl;

unit("base64", "encode")
.body([] {
    static const char *data = "this is a test";

    size_t len;
    char *encoded = Base64::encode(data, strlen(data) + 1, len);
    assert(encoded != nullptr);
    assert(len == strlen(encoded));
    free(encoded);
});

unit("base64", "decode")
.body([] {
    static const char *data = "this is a test";

    size_t len, len2;
    char *encoded = Base64::encode(data, strlen(data) + 1, len);
    char *decoded = (char *) Base64::decode(encoded, len, len2);
    assert(decoded != nullptr);
    assert(len2 == strlen(data) + 1);
    assert(strcmp(data, decoded) == 0);
    free(encoded);
    free(decoded);
});
