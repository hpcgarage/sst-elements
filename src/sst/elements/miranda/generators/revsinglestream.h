// Copyright 2009-2021 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2021, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#ifndef _H_SST_MIRANDA_REV_SINGLE_STREAM_GEN
#define _H_SST_MIRANDA_REV_SINGLE_STREAM_GEN

#include <sst/elements/miranda/mirandaGenerator.h>
#include <sst/core/output.h>

#include <queue>

namespace SST {
namespace Miranda {

class ReverseSingleStreamGenerator : public RequestGenerator {

public:
      ReverseSingleStreamGenerator( ComponentId_t id, Params& params );
      void build(Params& params);
      ~ReverseSingleStreamGenerator();
      void generate(MirandaRequestQueue<GeneratorRequest*>* q);
      bool isFinished();
      void completed();

      SST_ELI_REGISTER_SUBCOMPONENT_DERIVED(
            ReverseSingleStreamGenerator,
            "miranda",
            "ReverseSingleStreamGenerator",
            SST_ELI_ELEMENT_VERSION(1,0,0),
            "Creates a single reverse ordering stream of accesses to/from memory",
            SST::Miranda::RequestGenerator
         )

      SST_ELI_DOCUMENT_PARAMS(
            { "startat",          "Sets the start *index* for this generator", "0" },
            { "stopat",           "Sets the stop *index* for this generator", "524228" },
            { "verbose",          "Sets the verbosity of the output", "0" },
            { "stride",           "Sets the stride", "1" },
            { "gapLocations",           "Index for the jump in the buffer array", "0" },
            { "gapJump",           "amount for the gap that occurs at the gap location", "1" },
            { "arrGap",           "gap between each pattern", "100" },
            { "issueCount",           "number of requests", "1000" },
            { "byteAmount",     "number of bytes per request", "8" },
            { "length",       "Sets the length of the pattern", "8" },

         )

   private:
      uint64_t startIndex;
      uint64_t stopIndex;
      uint64_t datawidth;
      uint64_t nextIndex;
      uint64_t stride;
      uint64_t gapLocations;
      uint64_t gapJump;
      uint64_t arrgap;
      uint64_t issueCount;
      uint64_t reqLength;
      uint64_t reqSize;

      ReqOperation memOp;
      Output*  out;

};

}
}

#endif
