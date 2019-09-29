#include "ut_cl_rng.h"

namespace UtCl
{

    RNG::RNG(
        _In_ unsigned int Min,
        _In_ unsigned int Max
    ) :
        m_dist(Min, Max)
    {
        m_rng.seed(std::random_device()());
    }

    unsigned int RNG::GetNextRandom()
    {
        return m_dist(m_rng);
    }

    RNG& RNG::GetInstance()
    {
        if (m_instance == NULL)
        {
            m_instance = new RNG;
        }

        return *m_instance;
    }

    RNG* RNG::m_instance = NULL;
}

