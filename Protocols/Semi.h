/*
 * Semi2k.h
 *
 */

#ifndef PROTOCOLS_SEMI_H_
#define PROTOCOLS_SEMI_H_

#include "SPDZ.h"
#include "Processor/TruncPrTuple.h"

/**
 * Dishonest-majority protocol for computation modulo a power of two
 */
template<class T>
class Semi : public SPDZ<T>
{
    SeededPRNG G;

public:
    Semi(Player& P) :
            SPDZ<T>(P)
    {
    }

    void randoms(T& res, int n_bits)
    {
        res.randomize_part(G, n_bits);
    }

    void trunc_pr(const vector<int>& regs, int size,
            SubProcessor<T>& proc)
    {
        trunc_pr(regs, size, proc, T::clear::characteristic_two);
    }

    template<int = 0>
    void trunc_pr(const vector<int>&, int, SubProcessor<T>&, true_type)
    {
        throw not_implemented();
    }

    template<int = 0>
    void trunc_pr(const vector<int>& regs, int size,
            SubProcessor<T>& proc, false_type)
    {
        if (this->P.num_players() > 2)
            throw runtime_error("probabilistic truncation "
                    "only implemented for two players");

        assert(regs.size() % 4 == 0);
        this->trunc_pr_counter += size * regs.size() / 4;
        typedef typename T::open_type open_type;

        vector<TruncPrTupleWithGap<open_type>> infos;
        for (size_t i = 0; i < regs.size(); i += 4)
            infos.push_back({regs, i});

        for (auto& info : infos)
        {
            if (not info.big_gap())
                throw runtime_error("bit length too large");
            if (this->P.my_num())
                for (int i = 0; i < size; i++)
                    proc.get_S_ref(info.dest_base + i) = -open_type(
                            -open_type(proc.get_S()[info.source_base + i])
                                    >> info.m);
            else
                for (int i = 0; i < size; i++)
                    proc.get_S_ref(info.dest_base + i) =
                            proc.get_S()[info.source_base + i] >> info.m;
        }
    }
};

#endif /* PROTOCOLS_SEMI_H_ */
