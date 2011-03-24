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
@brief Implementation of class SuperInstance
@author Jerome Gorin
@file SuperInstance.cpp
@version 1.0
@date 24/12/2010
*/

//------------------------------
#include <map>
#include <list>

#include "Jade/Merger/SuperInstance.h"
//------------------------------

using namespace std;

SuperInstance::SuperInstance(std::string id, Instance* srcInstance, list<Port*>* intSrcPorts, int srcFactor, Instance* dstInstance, list<Port*>* intDstPorts, int dstFactor) : Instance(id, NULL){
	this->srcInstance = srcInstance;
	this->dstInstance = dstInstance;
	this->srcFactor = srcFactor;
	this->dstFactor = dstFactor;
	this->intSrcPorts = intSrcPorts;
	this->intDstPorts = intDstPorts;
	this->actor = createCompositeActor();
}

Actor* SuperInstance::createCompositeActor(){
	map<string, StateVar*>* stateVars = new map<string, StateVar*>();
	map<string, Variable*>* parameters = new map<string, Variable*>();
	map<string, Procedure*>* procedures = new map<string, Procedure*>();
	list<Action*>* initializes = new list<Action*>();
	list<Action*>* actions = new list<Action*>();
	MoC* moc = NULL;
	
	// Add stateVars
	map<string, StateVar*>::iterator itVar;


	return new Actor(id, NULL, "", new map<string, Port*>(),  new map<string, Port*>(),
		stateVars, parameters, procedures,initializes, actions, NULL, moc);
}