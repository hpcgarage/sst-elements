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


//This will be the AMG Spatter Pattern Implementation

#ifndef _H_SST_MIRANDA_RANDOM_GEN
#define _H_SST_MIRANDA_RANDOM_GEN

#include <sst/elements/miranda/mirandaGenerator.h>
#include <sst/core/output.h>
#include <sst/core/rng/sstrng.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <queue>
using namespace std;
using namespace SST::RNG;

namespace SST {
namespace Miranda {

class RandomGenerator : public RequestGenerator {

public:
	RandomGenerator( ComponentId_t id, Params& params );
        void build(Params& params);
	~RandomGenerator();
	void generate(MirandaRequestQueue<GeneratorRequest*>* q);
	bool isFinished();
	void completed();

	SST_ELI_REGISTER_SUBCOMPONENT_DERIVED(
                RandomGenerator,
                "miranda",
                "RandomGenerator",
                SST_ELI_ELEMENT_VERSION(1,0,0),
                "Creates a random stream of accesses to/from memory",
                SST::Miranda::RequestGenerator
        )

	SST_ELI_DOCUMENT_PARAMS(
		{ "patternType",          "Determines what pattern is used", "1" },
		{ "reqSize",          "determines memory size of request", "8" },

    	
        )
private:
	uint64_t reqLength;
	uint64_t patternType;
	uint64_t reqSize;
	uint64_t maxAddr;
	uint64_t nextAddr;
	uint64_t arrGap;
	uint64_t issueCount;
	bool issueOpFences;
	vector<int> vect;
	Output*  out;
	ReqOperation memOp;


};

}
}

#endif