#include <thread_pool.h>

using namespace spl;

thread_local void * ExecutionContext::_task;
thread_local ExecutionContext * ExecutionContext::_this;
