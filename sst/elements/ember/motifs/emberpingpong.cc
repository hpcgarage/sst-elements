// Copyright 2009-2014 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2014, Sandia Corporation
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#include <sst_config.h>
#include "embergettimeev.h"
#include "emberpingpong.h"

using namespace SST::Ember;

EmberPingPongGenerator::EmberPingPongGenerator(SST::Component* owner, Params& params) :
	EmberMessagePassingGenerator(owner, params) {

	messageSize = (uint32_t) params.find_integer("messageSize", 1024);
	iterations = (uint32_t) params.find_integer("iterations", 1);
}

void EmberPingPongGenerator::configureEnvironment(const SST::Output* output, uint32_t pRank, uint32_t worldSize) {
	rank = pRank;
    if(0 == rank) {
        output->output("PingPong, rank=%d size=%d msgSize=%d iter=%d\n",
                                rank,worldSize,messageSize,iterations);
    }

}

void EmberPingPongGenerator::generate(const SST::Output* output, const uint32_t phase,
	std::queue<EmberEvent*>* evQ) {

	if(phase < iterations) {
		if(0 == rank) {

            if ( 0 == phase ) {
                evQ->push( new EmberGetTimeEvent( &m_startTime ) );
            }

            evQ->push( new EmberSendEvent((uint32_t) 1, messageSize, 
                                                0, (Communicator) 0) );
			evQ->push( new EmberRecvEvent((uint32_t) 1, messageSize,
                                                0, (Communicator) 0) );

            if ( phase + 1 == iterations ) {
                evQ->push( new EmberGetTimeEvent( &m_stopTime ) );
            }

		} else if (1 == rank) {
			evQ->push( new EmberRecvEvent((uint32_t) 0, messageSize, 
                                                0, (Communicator) 0) );
			evQ->push( new EmberSendEvent((uint32_t) 0, messageSize,
                                                0, (Communicator) 0) );

		} else {
			EmberFinalizeEvent* endSimFinalize = new EmberFinalizeEvent();
			evQ->push(endSimFinalize);
		}
	} else {
        if ( 0 == rank) {
            double totalTime = (double)(m_stopTime - m_startTime)/1000000000.0;

            double latency = ((totalTime/iterations)/2);
            double bandwidth = (double) messageSize / latency;

            output->output("total time %.3f us, loop %d, bufLen %d, "
                    "latency %.3f us. bandwidth %f GB/s\n",
                                totalTime * 1000000.0, iterations,
                                messageSize,
                                latency * 1000000.0,
                                bandwidth / 1000000000.0 );

        }

		EmberFinalizeEvent* finalize = new EmberFinalizeEvent();
		evQ->push(finalize);
	}
}
