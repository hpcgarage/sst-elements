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


#include <sst_config.h>
#include <sst/core/params.h>
#include <sst/elements/miranda/generators/streambench.h>
#include <vector>

using namespace SST::Miranda;
using namespace SST::Miranda;



STREAMBenchGenerator::STREAMBenchGenerator( ComponentId_t id, Params& params ) :
	RequestGenerator(id, params) {
            build(params);
        }

void STREAMBenchGenerator::build(Params& params) {
	const uint32_t verbose = params.find<uint32_t>("verbose", 0);
	reqSize = params.find<uint64_t>("reqSize", 8);
	out = new Output("RandomGenerator[@p:@l]: ", verbose, 0, Output::STDOUT);
	v = {0, 32, 32, -96, 160, 32, 32, -96, 160, 32, 32, -96, 160, 32, 32, -96};
	arrGap = 32;
	maxAddr = 12400;
	maxIssueCount = 81006;
	issueCount = maxIssueCount;
	reqLength = 16;
    out->verbose(CALL_INFO, 1, 0, "Will issue %" PRIu64 " operations\n", issueCount);
	out->verbose(CALL_INFO, 1, 0, "Request lengths: %" PRIu64 " bytes\n", reqSize);

}


STREAMBenchGenerator::~STREAMBenchGenerator() {
	delete out;
}

void STREAMBenchGenerator::generate(MirandaRequestQueue<GeneratorRequest*>* q) {
	out->verbose(CALL_INFO, 4, 0, "Generating next request number: %" PRIu64 "\n", issueCount);

 	for (int i = 0; i < v.size(); i++) {
        nextAddr = nextAddr + v[i];
		if (nextAddr >= maxAddr || nextAddr < 0) {
			nextAddr = 0;
		}
		out->verbose(CALL_INFO, 4, 0, "addr: %" PRIu64 "\n", nextAddr);
        q->push_back(new MemoryOpRequest(nextAddr, reqSize, READ));
		issueCount--;
    }
	// What is the next address?
	nextAddr = (nextAddr + arrGap);
}

bool STREAMBenchGenerator::isFinished() {
	return (issueCount <= 0 || issueCount > maxIssueCount);
}

void STREAMBenchGenerator::completed() {

}
