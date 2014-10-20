#ifndef MDK_BITMASK_H_INCLUDED
#define MDK_BITMASK_H_INCLUDED

#include "mdk_Config.h"

namespace mdk
{

template<typename EnumType, typename MaskType = unsigned int>
struct BitMask
{
    MaskType mask;

    BitMask ()
        : mask (MaskType (0))
    {
    }

    BitMask& turnOn (EnumType whichBit)
    {
        m_assert (whichBit < sizeof (MaskType));
        m_assert (whichBit >= 0);

        mask = mask | (1 << whichBit);
        return *this;
    }

    BitMask& turnOff (EnumType whichBit)
    {
        m_assert (whichBit < sizeof (MaskType));
        m_assert (whichBit >= 0);

        mask = mask & ~(1 << whichBit);
        return *this;
    }

    bool isOn (EnumType whichBit) const
    {
        m_assert (whichBit < sizeof (MaskType));
        m_assert (whichBit >= 0);

        return (mask & (1 << whichBit)) > 0;
    }

    bool isOff (EnumType whichBit) const
    {
        m_assert (whichBit < sizeof (MaskType));
        m_assert (whichBit >= 0);

        return (mask & (1 << whichBit)) == 0;
    }
};

}

#endif  // MDK_BITMASK_H_INCLUDED
