/*
 * Copyright (c) 2009, IETR/INSA of Rennes
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *   * Neither the name of the IETR/INSA of Rennes nor the names of its
 *     contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
@brief Implementation of class IRLinker
@author Jerome Gorin
@file IRLinker.cpp
@version 1.0
@date 15/11/2010
*/

//------------------------------
#include <iostream>

#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"


#include "lib/RVCEngine/Decoder.h"
#include "lib/IRCore/Port.h"
#include "lib/IRJit//LLVMExecution.h"
#include "lib/IRSerialize/IRLinker.h"

#include "IRConstant.h"
//------------------------------

using namespace std;
using namespace llvm;

IRLinker::IRLinker(Decoder* decoder){
    this->decoder = decoder;

}

IRLinker::~IRLinker(){

}

int IRLinker::link(list<pair<Instance*, Instance*> >* instances){
    list<pair<Instance*, Instance*> >::iterator it;

    //Iterate though instances to link
    for (it = instances->begin(); it != instances->end(); it++){
        linkInstance(it->first, it->second);
    }

    return 0;
}

void IRLinker::linkInstance(Instance* refinstance, Instance* instance){

    //Link ports of the instance
    linkPorts(refinstance->getInputs(), instance->getInputs());
    linkPorts(refinstance->getOutputs(), instance->getOutputs());

    instance->setActions(refinstance->getActions());
    instance->setStateVars(refinstance->getStateVars());
    instance->setParameters(refinstance->getParameters());
    instance->setProcs(refinstance->getProcs());
    instance->setInitializes(refinstance->getInitializes());
    instance->setActionScheduler(refinstance->getActionScheduler());

    // Solve parameters of the linked instance
    instance->solveParameters();
}

void IRLinker::linkPorts(map<string, Port*>* refPorts, map<string, Port*>* ports){
    map<string, Port*>::iterator itRef;

    for (itRef = refPorts->begin(); itRef != refPorts->end(); itRef++){
        map<string, Port*>::iterator it;

        //Find the port occurence in reference ports
        it = ports->find(itRef->first);

        if (it == ports->end()){
            ports->insert(*itRef);
        }else{
            //Link variables
            Port* port = it->second;
            Port* refPort = itRef->second;

            port->setPtrVar(refPort->getPtrVar());
            port->setFifoVar(refPort->getFifoVar());
        }

    }
}

