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

#include "sst_config.h"
#include "WorkingSetSignature.h"

#include <vector>
#include "stdlib.h"

#include "sst/core/params.h"


#define PHASE_DETECTION_MIN(a,b) (((a)<(b)) ? a : b)

using namespace SST;
using namespace SST::PhaseDetection;

double WorkingSetSignature::diff_sigs(bitvec sig1, bitvec sig2) {
    // Number of bits in common divided by total number of bits set
    return static_cast<double>((sig1 ^ sig2).count()) / (sig1 | sig2).count();
}

uint64_t WorkingSetSignature::hash_address(Addr address) {
    //TODO Find out what's going on with the 64 and 1024 here.

    //drop the bottom {drop_bits} bits of the signature
    //hash it then return the top {log2_signature_len} bits of the hash (the number of bits determined by the length of the signature)
    //use this to then index into a bitvec that represents the current signature to set a specific bit to 1
    /*sizeof(uint32_t)*/ /* likely 32 if 32-bit MT or 64 if 64-bit MT or other hash */
    return ((uint32_t) std::hash<bitset<64>>{}(address >> drop_bits)) >> (32  - WorkingSetSignatureConstants::log2_signature_len);

    // return hashed_randomized_address >> (32 /*sizeof(uint32_t)*/ /* likely 32 if 32-bit MT or 64 if 64-bit MT or other hash */ - log2_signature_len);
    
    //old: hash<bitset<1024>>()(address_minus_bottom_drop_bits) - time test on big XS: 18.539s
    //alt: hash<bitset<64>>()(address_minus_bottom_drop_bits) - time test on big XS: 7.962s
    //not really a hash: hash<uint64_t>()(address_minus_bottom_drop_bits) - time test on big XS: 6.178s
}



void WorkingSetSignature::notifyAccess(const CacheListenerNotification& notify) {
    const NotifyAccessType notifyType = notify.getAccessType();
    const NotifyResultType notifyResType = notify.getResultType();
    const Addr addr = notify.getPhysicalAddress();

    // Ignore EVICT and PREFETCH types
    if (notifyType != READ && notifyType != WRITE)
        return;

    // Store recieved address
    if ( UNLIKELY(nextAddrListEntry >= interval_len) ) {
       //Unreachable
       output->fatal(CALL_INFO, -1, "Error: This should be unreachable.\n");
    }

    // Store recieved addr
    recentAddrList[nextAddrListEntry] = addr;
    nextAddrListEntry += 1;

    // Once we reach the end of an interval, classify the phase, notify listners and clear
    // the recentAddrList
    if( nextAddrListEntry == interval_len ) {
        DetectPhase();
        nextAddrListEntry = 0;
    }
}


void WorkingSetSignature::DetectPhase() {
    /*  Needs to be updated with current MemHierarchy Commands/States, MemHierarchyInterface */
    uint32_t stride;
    bool foundStride = true;
    Addr targetAddress = 0;
    uint32_t strideIndex;

    // Classify phase

    //first, check if the phase is stable since the difference measure is acceptably low
    if (diff_sigs(current_signature, last_signature) < threshold) {
        stable_count += 1;
        if (stable_count >= stable_min && phase == -1) {
            //add the current signature to the phase table and make the phase #/phase id to its index
            phase_table.push_back(current_signature);
            phase = phase_table.size() - 1; // or indexof curr_sig?
            //line 194 in the python
        }
    } else { //line 196 in python
        //if difference too high then it's not stable and we might now know the phase
        stable_count = 0;
        phase = -1;

        //see if we've entered a phase we have seen before
        if (!phase_table.empty()) { //line 201 python
            double best_diff = threshold;
            for (auto phase_table_iterator = phase_table.begin(); phase_table_iterator != phase_table.end(); phase_table_iterator++) {
                const auto s = *phase_table_iterator;
                const auto diff = diff_sigs(current_signature, s);
                // difference_scores_from_phase_table.push_back(diff);
                if (diff < threshold && diff < best_diff) {
                    phase = std::distance(phase_table.begin(), phase_table_iterator);
                    best_diff = diff;
                    //set current phase to the phase of the one with the lowest difference from current (which is the index in the phase table)
                }
            }
        }
    }
    //whether or not the phase is stable, we need to update last phase and whatnot
    last_signature = current_signature;
    current_signature.reset();
    
    //add the current phase ID to the phase trace - from line 209 in python
    phase_history->push_back(phase);

    // Create memevent
    MemEvent* ev = NULL;

    // Notify listeners

    /*
    for (auto f : listeners) {
        f(phase);
    }
    */


    // OLD STUFF
    for(uint32_t i = 0; i < recentAddrListCount - 1; ++i) {
        for(uint32_t j = i + 1; j < recentAddrListCount; ++j) {
            stride = j - i;
            strideIndex = 1;
            foundStride = true;

            if(foundStride) {
                Addr targetPrefetchAddress = targetAddress + (strideReach * stride);
                targetPrefetchAddress = targetPrefetchAddress - (targetPrefetchAddress % blockSize);

                if(overrunPageBoundary) {
                        output->verbose(CALL_INFO, 2, 0,
                            "Issue prefetch, target address: %" PRIx64 ", prefetch address: %" PRIx64 " (reach out: %" PRIu32 ", stride=%" PRIu32 "), prefetchAddress=%" PRIu64 "\n",
                            targetAddress, targetAddress + (strideReach * stride),
                            (strideReach * stride), stride, targetPrefetchAddress);

                        // Check next address is aligned to a cache line boundary
                        assert((targetAddress + (strideReach * stride)) % blockSize == 0);

                        ev = new MemEvent(getName(), targetAddress + (strideReach * stride), targetAddress + (strideReach * stride), Command::GetS);
                } else {
                        const Addr targetAddressPhysPage = targetAddress / pageSize;
                        const Addr targetPrefetchAddressPage = targetPrefetchAddress / pageSize;

                        // Check next address is aligned to a cache line boundary
                        assert(targetPrefetchAddress % blockSize == 0);

                        // if the address we found and the next prefetch address are on the same
                        // we can safely prefetch without causing a page fault, otherwise we
                        // choose to not prefetch the address
                        if(targetAddressPhysPage == targetPrefetchAddressPage) {
                            output->verbose(CALL_INFO, 2, 0, "Issue prefetch, target address: %" PRIx64 ", prefetch address: %" PRIx64 " (reach out: %" PRIu32 ", stride=%" PRIu32 ")\n",
                                    targetAddress, targetPrefetchAddress, (strideReach * stride), stride);
                            ev = new MemEvent(getName(), targetPrefetchAddress, targetPrefetchAddress, Command::GetS);
                        } else {
                            output->verbose(CALL_INFO, 2, 0, "Cancel prefetch issue, request exceeds physical page limit\n");
                            output->verbose(CALL_INFO, 4, 0, "Target address: %" PRIx64 ", page=%" PRIx64 ", Prefetch address: %" PRIx64 ", page=%" PRIx64 "\n", targetAddress, targetAddressPhysPage, targetPrefetchAddress, targetPrefetchAddressPage);

                            ev = NULL;
                        }
                }

                break;
            }
        }

        if(ev != NULL) {
                break;
        }
    }

    if(ev != NULL) {
        std::vector<Event::HandlerBase*>::iterator callbackItr;

        Addr prefetchCacheLineBase = ev->getAddr() - (ev->getAddr() % blockSize);
        bool inHistory = false;
        const uint32_t currentHistCount = prefetchHistory->size();

        output->verbose(CALL_INFO, 2, 0, "Checking prefetch history for cache line at base %" PRIx64 ", valid prefetch history entries=%" PRIu32 "\n", prefetchCacheLineBase,
            currentHistCount);

        for(uint32_t i = 0; i < currentHistCount; ++i) {
            if(prefetchHistory->at(i) == prefetchCacheLineBase) {
                    inHistory = true;
                    break;
            }
        }

        if(! inHistory) {

            // Remove the oldest cache line
            if(currentHistCount == prefetchHistoryCount) {
                    prefetchHistory->pop_front();
            }

            // Put the cache line at the back of the queue
            prefetchHistory->push_back(prefetchCacheLineBase);

            assert((ev->getAddr() % blockSize) == 0);

                // Cycle over each registered call back and notify them that we want to issue a prefet$
                for(callbackItr = registeredCallbacks.begin(); callbackItr != registeredCallbacks.end(); callbackItr++) {
                    // Create a new read request, we cannot issue a write because the data will get
                    // overwritten and corrupt memory (even if we really do want to do a write)
                    MemEvent* newEv = new MemEvent(getName(), ev->getAddr(), ev->getAddr(), Command::GetS);
                        newEv->setSize(blockSize);
                        newEv->setPrefetchFlag(true);

                (*(*callbackItr))(newEv);
                }

            delete ev;
        } else {
            output->verbose(CALL_INFO, 2, 0, "Prefetch canceled - same cache line is found in the recent prefetch history.\n");
                delete ev;
        }
    }
}


WorkingSetSignature::WorkingSetSignature(ComponentId_t id, Params& params) : CacheListener(id, params) {
    requireLibrary("memHierarchy");

    verbosity = params.find<int>("verbose", 0);

    char* new_prefix = (char*) malloc(sizeof(char) * 128);
    sprintf(new_prefix, "WorkingSetSignature[%s | @f:@p:@l] ", getName().c_str());
    output = new Output(new_prefix, verbosity, 0, Output::STDOUT);
    free(new_prefix);

    /* Algorithm Parameters */
    interval_len = params.find<uint64_t>("interval_len", 10000);
    stable_min   = params.find<uint32_t>("stable_min",   4);
    threshold    = params.find<float>("threshold",    0.5);
    /*bits_log2   = params.find<uint32_t>("bits_log2",    10);*/
    drop_bits    = params.find<uint32_t>("drop_bits",    3);

    blockSize = params.find<uint64_t>("cache_line_size", 64);

    prefetchHistoryCount = params.find<uint32_t>("history", 16);
    prefetchHistory = new std::deque<uint64_t>();

    strideReach = params.find<uint32_t>("reach", 2);
    strideDetectionRange = params.find<uint64_t>("detect_range", 4);
    pageSize = params.find<uint64_t>("page_size", 4096);

    uint32_t overrunPB = params.find<uint32_t>("overrun_page_boundaries", 0);
    overrunPageBoundary = (overrunPB == 0) ? false : true;

    // Variable initialization
    current_signature.reset();
    last_signature.reset();
    phase_table.clear();
    phase_history->clear();
    stable_count = 0;
    nextAddrListEntry = 0;
    recentAddrList = (Addr*) malloc(sizeof(Addr) * interval_len);
    phase = -1;

    for(uint32_t i = 0; i < interval_len; ++i) {
        recentAddrList[i] = (Addr) 0;
    }


    output->verbose(CALL_INFO, 1, 0, "WorkingSetSignature created, cache line: %" PRIu64 ", page size: %" PRIu64 "\n",
        blockSize, pageSize);


    statNumIntervals = registerStatistic<uint64_t>("num_intervals");
    statNumPhases = registerStatistic<uint64_t>("num_phases");
}

WorkingSetSignature::~WorkingSetSignature() {
    free(recentAddrList);
}

void WorkingSetSignature::registerResponseCallback(Event::HandlerBase* handler) {
    registeredCallbacks.push_back(handler);
}

void WorkingSetSignature::printStats(Output &out) {
}
