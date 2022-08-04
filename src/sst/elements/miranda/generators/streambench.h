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


#ifndef _H_SST_MIRANDA_STREAM_BENCH_GEN
#define _H_SST_MIRANDA_STREAM_BENCH_GEN

#include <sst/elements/miranda/mirandaGenerator.h>
#include <sst/core/output.h>
#include <vector>
#include <queue>
using namespace std;
namespace SST {
namespace Miranda {

class STREAMBenchGenerator : public RequestGenerator {

public:
	STREAMBenchGenerator( ComponentId_t id, Params& params );
        void build(Params& params);
	~STREAMBenchGenerator();
	void generate(MirandaRequestQueue<GeneratorRequest*>* q);
	bool isFinished();
	void completed();

	SST_ELI_REGISTER_SUBCOMPONENT_DERIVED(
                STREAMBenchGenerator,
                "miranda",
                "STREAMBenchGenerator",
                SST_ELI_ELEMENT_VERSION(1,0,0),
		"Creates a representation of the STREAM benchmark",
                SST::Miranda::RequestGenerator
        )

	SST_ELI_DOCUMENT_PARAMS(
		{ "verbose",          "Sets the verbosity output of the generator", "0" },
       	{ "reqSize",          "determines memory size of request", "8" },

		)

private:
	uint64_t reqLength;
	uint64_t arrGap;
	int64_t issueCount;
	uint64_t nextAddr;
	uint64_t maxAddr;
	uint64_t reqSize;
	uint64_t maxIssueCount;
	vector<int> v;
	Output*  out;

};

}
}

#endif
