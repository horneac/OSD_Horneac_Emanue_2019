#pragma once

#include <random>

namespace UtCl
{
    class RNG
    {
    public:
        RNG(
            _In_ unsigned int Min = 0,
            _In_ unsigned int Max = UINT32_MAX
        );

        unsigned int GetNextRandom();

        static RNG& GetInstance();


    private:
        std::mt19937 m_rng;
        const std::uniform_int_distribution<std::mt19937::result_type> m_dist;

        static RNG* m_instance;

    };
}
