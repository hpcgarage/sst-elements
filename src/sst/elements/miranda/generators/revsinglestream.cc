/// Copyright 2009-2021 NTESS. Under the terms
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
#include <sst/elements/miranda/generators/revsinglestream.h>

using namespace SST::Miranda;


ReverseSingleStreamGenerator::ReverseSingleStreamGenerator( ComponentId_t id, Params& params ) :
	RequestGenerator(id, params) {
            build(params);
        }

void ReverseSingleStreamGenerator::build(Params& params) {

	const uint32_t verbose = params.find<uint32_t>("verbose", 0);

	//out = new Output("ReverseSingleStreamGenerator[@p:@l]: ", verbose, 0, Output::STDOUT);
	out = new Output("ReverseSingleStreamGenerator[@p]: ", verbose, 0, Output::STDOUT);

	issueCount = params.find<uint64_t>("issueCount", 1000);
	stopIndex   = params.find<uint64_t>("stopat", 524228);
	startIndex  = params.find<uint64_t>("startat", 0);
	gapLocations = params.find<uint64_t>("gapLocations", 0);
	gapJump = params.find<uint64_t>("gapJumps", 5);
	arrgap = params.find<uint64_t>("arrGap", 100);
	stride = params.find<uint64_t>("stride", 1);
	reqLength = params.find<uint64_t>("requestLength", 8);
	reqSize = params.find<uint64_t>("byteAmount", 8);


	//does not support multiple gap locations or multiple jumps

	if(startIndex > stopIndex) {
		out->fatal(CALL_INFO, -1, "Start address (%" PRIu64 ") must be less than stop address (%" PRIu64 ") in ms-1",
			startIndex, stopIndex);
	}

	out->verbose(CALL_INFO, 1, 0, "Start Address:         %" PRIu64 "\n", startIndex);
	out->verbose(CALL_INFO, 1, 0, "Stop Address:          %" PRIu64 "\n", stopIndex);
	// out->verbose(CALL_INFO, 1, 0, "Data width:            %" PRIu64 "\n", datawidth);
	// out->verbose(CALL_INFO, 1, 0, "Stride:                %" PRIu64 "\n", stride);

	nextIndex = startIndex;
}

ReverseSingleStreamGenerator::~ReverseSingleStreamGenerator() {
	delete out;
}

void ReverseSingleStreamGenerator::generate(MirandaRequestQueue<GeneratorRequest*>* q) {
	out->verbose(CALL_INFO, 4, 0, "Generating next request at address: %" PRIu64 "\n", nextIndex);
	int* buff = (int*) malloc(64*sizeof(int));
	for (int i = 0; i < reqLength; i++) {
		if (i == 0) {
			buff[i] = 0;
		} else if (i == gapLocations) {
			buff[i] = buff[i - 1] + gapJump;
		} else {
			buff[i] = buff[i - 1] + stride;
		}
	}
	for (int i = 0; i < reqLength; i++) {
		nextIndex = nextIndex + buff[i];
		q->push_back(new MemoryOpRequest(nextIndex, reqSize, READ));
		issueCount--;
	}
// issue count decrements after reqlength requests to simulate a request of length 8 (customizeable length coming soon)
	nextIndex = nextIndex + arrgap;
}

bool ReverseSingleStreamGenerator::isFinished() {
	return (issueCount == 0);
}

void ReverseSingleStreamGenerator::completed() {

}