// Copyright 2009-2022 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2022, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// of the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#ifndef _H_SST_STRIDE_PREFETCH
#define _H_SST_STRIDE_PREFETCH

#include <vector>

#include <sst/core/event.h>
#include <sst/core/sst_types.h>
#include <sst/core/component.h>
#include <sst/core/link.h>
#include <sst/core/timeConverter.h>
#include <sst/elements/memHierarchy/memEvent.h>
#include <sst/elements/memHierarchy/cacheListener.h>

#include <sst/core/output.h>

#include <bitset>

namespace WorkingSetSignatureConstants {
    constexpr uint signature_len = 1024;
    constexpr uint log2_signature_len = (int)(31 - __builtin_clz(signature_len | 1));
}
using bitvec = bitset<WorkingSetSignatureConstants::signature_len>;

using namespace SST;
using namespace SST::MemHierarchy;
using namespace std;

namespace SST {
namespace PhaseDetection {

class WorkingSetSignature : public SST::MemHierarchy::CacheListener {
public:
    WorkingSetSignature(ComponentId_t id, Params& params);
    ~WorkingSetSignature();

    void notifyAccess(const CacheListenerNotification& notify);
    void registerResponseCallback(Event::HandlerBase *handler);
    void printStats(Output &out);

    SST_ELI_REGISTER_SUBCOMPONENT_DERIVED(
        WorkingSetSignature,
            "PhaseDetection",
            "WorkingSetSignature",
            SST_ELI_ELEMENT_VERSION(1,0,0),
            "Working set based phase detection",
            SST::MemHierarchy::CacheListener
    )

    SST_ELI_DOCUMENT_PARAMS(
        { "verbose", "Controls the verbosity of the PhaseDetection component", "0" },
        /* To be removed */
        { "cache_line_size", "Size of the cache line the prefetcher is attached to", "64" },
        { "history", "Number of entries to keep for historical comparison", "16" },
        { "reach", "Reach (how far forward the prefetcher should fetch lines)", "2" },
        { "detect_range", "Range to detect addresses over in request counts", "4" },
        { "address_count", "Number of addresses to keep in prefetch table", "64" },
        { "page_size", "Page size for this controller", "4096" },
        { "overrun_page_boundaries", "Allow 0 is no, 1 is yes", "0" },
        /* PhaseDetector stuff */
        { "interval_len", "Number of accesses in an interval", "10000" },
        { "stable_min", "Number of intervals with similar signatures required to reach stability", "4"},
        { "threshold", "How similar two intervals must be to be declared similar" "0.5" },
        /*{ "bits_log2", "Log 2 of the size of the bit vector used for the signature", "10" },*/
        { "drop_bits", "The number of bits to drop from the right side of an Addr before hashing" "3" }
    )

    SST_ELI_DOCUMENT_STATISTICS(
        { "num_intervals", "The number of intervals seen by this phase detector", "intervals", 1},
        { "num_phases", "The number of different phases seen by this phase detector", "phases", 1}

    )

private:
    /* To be removed */
    std::deque<uint64_t>* prefetchHistory;
    uint32_t prefetchHistoryCount;
    uint64_t blockSize;
    bool overrunPageBoundary;
    uint64_t pageSize;
    uint32_t recentAddrListCount;
    uint32_t nextRecentAddressIndex;
    uint32_t strideDetectionRange;
    uint32_t strideReach;
    /* PhaseDetector Stuff */
    void DetectPhase();
    uint64_t hash_address(Addr);
    double diff_sigs(bitvec sig1, bitvec sig2);
    std::deque<uint64_t>* phase_history;
    Output* output;
    std::vector<Event::HandlerBase*> registeredCallbacks;
    Addr* recentAddrList;
    uint32_t verbosity;
    /* WorkingSetSignature specific */
    uint64_t interval_len;
    uint32_t stable_min;
    double threshold;
    /*uint32_t bits_log2;*/
    uint32_t drop_bits;
    uint64_t nextAddrListEntry;
    uint32_t stable_count;
    int32_t  phase;

    vector<bitvec> phase_table;
    bitvec current_signature;
    bitvec last_signature;

    Statistic<uint64_t>* statNumIntervals;
    Statistic<uint64_t>* statNumPhases;
};

} //namespace PhaseDetection
} //namespace SST

#endif
