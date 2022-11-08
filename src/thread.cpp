/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <thread.h>

using namespace spl;

thread_local Thread::Context *Thread::__ctx = nullptr;
