#pragma once
#include "Serialize/SerializeUtility.h"

/*
Some random generator timings:
MacBook Pro w/ Core 2 Duo 2.4GHz. Times are for gcc 4.0.1 (OS X 10.6.2) / VS2008 SP1 (Win XP SP3),
in milliseconds for this loop (4915200 calls):

 for (int j = 0; j < 100; ++j)
   for (int i = 0; i < 128*128*3; ++i)
     data[i] = (rnd.get() & 0x3) << 6;

                  gcc   vs2008    Size
C's rand():       57.0  109.3 ms     1
Mersenne Twister: 56.0   37.4 ms  2500
Unity 2.x LCG:    11.1    9.2 ms     4
Xorshift 128:     15.0   17.8 ms    16
Xorshift 32:      20.6   10.7 ms     4
WELL 512:         43.6   55.1 ms    68
*/

struct RandState
{
    UInt32 x, y, z, w;

    template<class TransferFunc>
    void Transfer(TransferFunc& transfer)
    {
        TRANSFER(x);
        TRANSFER(y);
        TRANSFER(z);
        TRANSFER(w);
    }
};


// Xorshift 128 implementation
// Xorshift paper: http://www.jstatsoft.org/v08/i14/paper
// Wikipedia: http://en.wikipedia.org/wiki/Xorshift
class EXPORT_COREMODULE Rand
{
public:
    Rand(UInt32 seed = 0)
    {
        SetSeed(seed);
    }

    UInt32 Get()
    {
        UInt32 t;
        t = m_State.x ^ (m_State.x << 11);
        m_State.x = m_State.y; m_State.y = m_State.z; m_State.z = m_State.w;
        return m_State.w = (m_State.w ^ (m_State.w >> 19)) ^ (t ^ (t >> 8));
    }

    UInt64 Get64()
    {
        UInt64 hi = Get();
        UInt64 lo = Get();
        return (hi << 32) | lo;
    }

    inline static float GetFloatFromInt(UInt32 value)
    {
        // take 23 bits of integer, and divide by 2^23-1
        return float(value & 0x007FFFFF) * (1.0f / 8388607.0f);
    }

    inline static UInt8 GetByteFromInt(UInt32 value)
    {
        // take the most significant byte from the 23-bit value
        return UInt8(value >> (23 - 8));
    }

    // random number between 0.0 and 1.0
    float GetFloat()
    {
        return GetFloatFromInt(Get());
    }

    // random number between -1.0 and 1.0
    float GetSignedFloat()
    {
        return GetFloat() * 2.0f - 1.0f;
    }

    void SetSeed(UInt32 seed)
    {
        // std::mt19937::initialization_multiplier = 1812433253U
        m_State.x = seed;
        m_State.y = m_State.x * 1812433253U + 1;
        m_State.z = m_State.y * 1812433253U + 1;
        m_State.w = m_State.z * 1812433253U + 1;
    }

    UInt32 GetSeed() const { return m_State.x; }

    const RandState& GetState() const { return m_State; }
    void SetState(const RandState& value) { m_State = value; }

    void RandomizeState();

    template<class TransferFunc>
    void TransferState(TransferFunc& transfer)
    {
        transfer.SetVersion(2);

        if (transfer.IsVersionSmallerOrEqual(1))
        {
            transfer.Transfer(m_State.x, "x");
            if (transfer.IsReading())
                SetSeed(m_State.x);
        }
        else
        {
            TRANSFER_WITH_NAME(m_State, "state");
        }
    }

    static Rand GetUniqueGenerator()
    {
        Rand result;
        result.RandomizeState();
        return result;
    }

    inline bool operator<(const Rand& rhs) const
    {
        return (memcmp(this, &rhs, sizeof(rhs)) < 0);
    }

    inline bool operator==(const Rand& rhs) const
    {
        return (memcmp(this, &rhs, sizeof(rhs)) == 0);
    }

private:
    RandState m_State;    // RNG state (128 bit)
};


EXPORT_COREMODULE Rand& GetScriptingRand();
