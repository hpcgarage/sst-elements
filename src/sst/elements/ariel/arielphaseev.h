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


#ifndef _H_SST_ARIEL_PHASE_EVENT
#define _H_SST_ARIEL_PHASE_EVENT

#include "arielevent.h"

using namespace SST;

namespace SST {
namespace ArielComponent {

class ArielPhaseEvent : public ArielEvent {

    public:
        ArielPhaseEvent(int phase, long int timenano) :
                phase(phase), timenano(timenano) {
        }

        ~ArielPhaseEvent() {
        }

        ArielEventType getEventType() const {
                return PHASE_NOTIF;
        }

        long int getTimenano() const {
                return timenano;
        }

        int getPhase() const {
                return phase;
        }


    private:
        const long int timenano;
        const int phase;

};

}
}

#endif
