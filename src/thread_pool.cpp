/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <thread_pool.h>

using namespace spl;

thread_local void * ExecutionContext::_task;
thread_local ExecutionContext * ExecutionContext::_this;
