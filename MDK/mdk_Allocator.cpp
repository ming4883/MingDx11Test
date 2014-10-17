#include "mdk_Allocator.h"

namespace mdk
{

CrtAllocator& CrtAllocator::get()
{
    static CrtAllocator _;
    return _;
}

}   // namespace
