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


#include <sst_config.h>
#include <sst/core/params.h>
#include <sst/elements/miranda/generators/singlestream.h>

using namespace SST::Miranda;
int buff[64] = {};

SingleStreamGenerator::SingleStreamGenerator( ComponentId_t id, Params& params ) :
	RequestGenerator(id, params) {
            build(params);
        }

void SingleStreamGenerator::build(Params& params) {

	const uint32_t verbose = params.find<uint32_t>("verbose", 0);

	out = new Output("SingleStreamGenerator[@p:@l]: ", verbose, 0, Output::STDOUT);

	issueCount = params.find<uint64_t>("count", 1000);
	reqLength  = params.find<uint64_t>("length", 8);
	startAddr  = params.find<uint64_t>("startat", 0);
	maxAddr    = params.find<uint64_t>("max_address", 524288);
    // NEW, if this doesnt work I may have to hardcode the gap value (Length param also changed to 1 above)
    arrgap = params.find<uint64_t>("arrGap", 100);
	stride = params.find<uint64_t>("stride", 1);
	reqSize = params.find<uint64_t>("byteAmount", 8);

	nextAddr   = startAddr;

	std::string op = params.find<std::string>( "memOp", "Read" );
	if ( ! op.compare( "Read" ) ) {
		memOp = READ;
	} else if ( ! op.compare( "Write" ) ) {
		memOp = WRITE;
	} else {
		assert( 0 );
	}

	out->verbose(CALL_INFO, 1, 0, "Will issue %" PRIu64 " %s operations\n",
				issueCount, memOp == READ ? "Read": "Write");
	out->verbose(CALL_INFO, 1, 0, "Request lengths: %" PRIu64 " bytes\n", reqLength);
	out->verbose(CALL_INFO, 1, 0, "Maximum address: %" PRIx64 "\n", maxAddr);
	out->verbose(CALL_INFO, 1, 0, "First address: %" PRIx64 "\n", nextAddr);
}

SingleStreamGenerator::~SingleStreamGenerator() {
	delete out;
}

void SingleStreamGenerator::generate(MirandaRequestQueue<GeneratorRequest*>* q) {
	out->verbose(CALL_INFO, 4, 0, "Generating next request number: %" PRIu64 "\n", issueCount);
    //For loop is new, used to implement pattern buffer (need to implement stride variable to make UNIFORM jump customizeable instead of 2)
	for (int i = 0; i < reqLength; i++) {
		if (i == 0) {
			buff[i] = 0;
			continue;
		} else {
			buff[i] = buff[i - 1] + stride;
		}
	}
    for (int i = 0; i < reqLength; i++) {
        nextAddr = nextAddr + buff[i];
        q->push_back(new MemoryOpRequest(nextAddr, reqSize, memOp));
		issueCount--;
    }
	// What is the next address?
	nextAddr = (nextAddr + arrgap);
	if( nextAddr >= maxAddr){
		nextAddr = startAddr;
	}

}

bool SingleStreamGenerator::isFinished() {
	return (issueCount == 0);
}

void SingleStreamGenerator::completed() {

}