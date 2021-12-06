#include <string_conversions.h>

using namespace spl;

thread_local char StringConversions::_numBuf[__NUMBER_BUFFER_SIZE];
thread_local char StringConversions::_expBuf[__NUMBER_BUFFER_SIZE];
