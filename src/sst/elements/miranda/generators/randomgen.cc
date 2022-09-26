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
#include <sst/core/rng/marsaglia.h>
#include <sst/elements/miranda/generators/randomgen.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;
using namespace SST::Miranda;


RandomGenerator::RandomGenerator( ComponentId_t id, Params& params ) :
	RequestGenerator(id, params) {
            build(params);
        }

void RandomGenerator::build(Params& params) {
	const uint32_t verbose = params.find<uint32_t>("verbose", 0);
	out = new Output("RandomGenerator[@p:@l]: ", verbose, 0, Output::STDOUT);
	patternType = params.find<uint64_t>("patternType", 1);
	reqSize = params.find<uint64_t>("reqSize", 8);
    nextAddr = 0;
    maxAddr = 100000;
    

	ifstream myfile;
    myfile.open("spatterpatterns.txt", ios::in);
    if ( myfile.fail()) {
            out->verbose(CALL_INFO, 1, 0, "Will issue %s operations\n", strerror(errno));
    }
    string myline;
    if ( myfile.is_open() ) {
        for (int j = 1; j < patternType; j++) {
            getline (myfile, myline);
        }
        getline (myfile, myline, ',');
        if (myline.compare("Gather") == 0) {
			memOp = READ;
        } else if (myline.compare("Scatter") == 0) {
			memOp = WRITE;
        }
        for (int i = 0; i < 16; i++) {
            getline (myfile, myline, ',');
            vect.push_back(stoi(myline));
        }
        getline (myfile, myline, ':');

        getline (myfile, myline, ',');
        arrGap = stoi(myline); // this should populate the delta or arrgap param
        getline (myfile, myline, ':');
        getline (myfile, myline);
        issueCount = stoi(myline); // this should populate the issue count param
    } else {
    }
    out->verbose(CALL_INFO, 1, 0, "Will issue %" PRIu64 " operations\n", vect.size());
	out->verbose(CALL_INFO, 1, 0, "Request lengths: %" PRIu64 " bytes\n", reqSize);
	out->verbose(CALL_INFO, 1, 0, "Maximum address: %" PRIu64 "\n", maxAddr);

}

RandomGenerator::~RandomGenerator() {
	delete out;
}

void RandomGenerator::generate(MirandaRequestQueue<GeneratorRequest*>* q) {
	out->verbose(CALL_INFO, 4, 0, "Generating next request number: %" PRIu64 "\n", issueCount);
 	for (int i = 0; i < vect.size(); i++) {
        if (issueCount == 0) {
            break;
        }
        if (nextAddr >= maxAddr) {
            nextAddr = 0;
        }
        if (vect[i] < 0 && nextAddr < abs(vect[i]) ) {
            nextAddr = 0;
        } else {
            nextAddr = nextAddr + vect[i];
        }
        out->verbose(CALL_INFO, 4, 0, "ADDR: %" PRIu64 "\n", nextAddr);
        q->push_back(new MemoryOpRequest(nextAddr, reqSize, memOp));
		issueCount--;
    }
	// What is the next address?
	nextAddr = (nextAddr + arrGap);
}

bool RandomGenerator::isFinished() {
	return (issueCount == 0);
}

void RandomGenerator::completed() {

}