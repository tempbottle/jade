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
@brief Implementation of class Connection
@author Jerome Gorin
@file Connection.cpp
@version 1.0
@date 15/11/2010
*/

//------------------------------
#include <iostream>

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"

#include "lib/IRCore/Network/Connection.h"
#include "lib/IRCore/Expression.h"
#include "lib/IRCore/Attribute/ValueAttribute.h"
#include "lib/IRCore/Network/Vertex.h"
#include "lib/Graph/HDAGGraph.h"
//------------------------------

using namespace std;
using namespace llvm;

extern cl::opt<int> FifoSize;

Connection::Connection(HDAGGraph* graph, Vertex* source, Port* srcPort, Vertex* target, Port* tgtPort, std::map<std::string, IRAttribute*>* attributes): HDAGEdge()
{   
    this->parent = graph;
    this->attributes = attributes;
    this->srcPort = srcPort;
    this->tgtPort = tgtPort;
    this->fifo = NULL;
    this->source = source;
    this->target = target;

    // Update graph
    graph->addEdge(source, target, this);

    // Set properties of the ports
    srcPort->setAccess(false, true);
    tgtPort->setAccess(true, false);

    // Bound connection to the port
    srcPort->addConnection(this);
    tgtPort->addConnection(this);
}


int Connection::getSize(){
    IRAttribute* attribute = getAttribute("bufferSize");

    if (attribute == NULL){
        return FifoSize;
    }

    if (attribute->isValue()){
        Expr* expr = ((ValueAttribute*)attribute)->getValue();
        return expr->evaluateAsInteger();
    }

    cerr<< "Error when parsing type of a connection";
    exit(0);
}

IRAttribute* Connection::getAttribute(std::string name){
    map<string, IRAttribute*>::iterator it;

    it = attributes->find(name);

    if(it == attributes->end()){
        return NULL;
    }

    return it->second;
}

void Connection::unsetFifo(){
    //Get GV of the port
    GlobalVariable* srcVar = srcPort->getFifoVar();
    GlobalVariable* dstVar = tgtPort->getFifoVar();

    //Remove GV initializer
    srcVar->setInitializer(NULL);
    dstVar->setInitializer(NULL);

    //Delete the fifo
    if (fifo != NULL){
        delete fifo;
        fifo = NULL;
    }
}
