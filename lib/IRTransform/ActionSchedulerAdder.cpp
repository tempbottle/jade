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
@brief Implementation of class ActionSchedulerAdder
@author Jerome Gorin
@file ActionSchedulerAdder.cpp
@version 0.1
@date 2010/04/12
*/

//------------------------------
#include "Jade/Decoder.h"
#include "Jade/Core/Actor/Action.h"
#include "Jade/Core/Actor/ActionScheduler.h"
#include "Jade/Core/Actor/ActionTag.h"
#include "Jade/Core/Actor.h"
#include "Jade/Core/Port.h"
#include "Jade/Core/Actor/Procedure.h"
#include "Jade/Core/Instance.h"
#include "Jade/Fifo/AbstractFifo.h"
#include "Jade/Transform/ActionSchedulerAdder.h"

#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
//------------------------------

using namespace llvm;
using namespace std;

ActionSchedulerAdder::ActionSchedulerAdder(llvm::LLVMContext& C, Decoder* decoder) : Context(C) {
	this->decoder = decoder;
}

void ActionSchedulerAdder::transform() {
	map<string, Instance*>::iterator it;
	map<string, Instance*>* instances = decoder->getInstances();
	
	for (it = instances->begin(); it != instances->end(); ++it){
		BBTransitions.clear();
		this->outsideSchedulerFn = NULL;

		Instance* instance = it->second;

		createScheduler(instance);

		if (instance->hasInitializes()){
			createInitialize(instance);
		}
	}

}

void ActionSchedulerAdder::createScheduler(Instance* instance){
	//Get properties of the instance
	Module* module = decoder->getModule();
	ActionScheduler* actionScheduler = instance->getActionScheduler();
	string name = instance->getId();
	name.append("_scheduler");

	Function* scheduler = cast<Function>(module->getOrInsertFunction(name, Type::getInt32Ty(Context),
										  (Type *)0));
	actionScheduler->setSchedulerFunction(scheduler);

	//Create values
	Value *Zero = ConstantInt::get(Type::getInt32Ty(Context), 0);
	Value *One = ConstantInt::get(Type::getInt32Ty(Context), 1);

	
	// Add a basic block entry to the scheduler.
	BasicBlock* BBEntry = BasicBlock::Create(Context, "entry", scheduler);

	//Create alloca on i and store 0
	AllocaInst* iVar = new AllocaInst(Type::getInt32Ty(Context), "i", BBEntry);
	StoreInst* storeInst = new StoreInst(Zero, iVar, BBEntry);

	// Add a basic block to bb and branch entry to bb.
	BasicBlock* BB = BasicBlock::Create(Context, "bb", scheduler);
	BranchInst::Create(BB, BBEntry);
	
	// Add a basic block return that return %i
	BasicBlock* returnBB = BasicBlock::Create(Context, "return", scheduler);
	LoadInst* loadIRet = new LoadInst(iVar, "i_ret", returnBB);
	ReturnInst::Create(Context, loadIRet, returnBB);


	// Add a basic block inc that return %i and branch to bb
	BasicBlock* incBB = BasicBlock::Create(Context, "inc_i", scheduler);
	LoadInst* loadIInc = new LoadInst(iVar, "i_load", incBB);
	BinaryOperator* iAdd = BinaryOperator::CreateNSWAdd(loadIInc, One, "i_add", incBB);
	new StoreInst(iAdd, iVar, incBB);
	BranchInst::Create(BB, incBB);
	
	if (actionScheduler->hasFsm()){
		BB = createSchedulerFSM(instance, BB, incBB, returnBB , scheduler);
	}else{
		BB = createSchedulerNoFSM(instance, BB, incBB, returnBB, scheduler);
	}
}

void ActionSchedulerAdder::createInitialize(Instance* instance){
	
	//Get properties of the instance
	Module* module = decoder->getModule();
	ActionScheduler* actionScheduler = instance->getActionScheduler();
	list<Action*>* initializes = instance->getInitializes();
	string name = instance->getId();
	name.append("_initialize");

	//Create initialize function
	Function* initialize = cast<Function>(module->getOrInsertFunction(name, Type::getVoidTy(Context),
											  (Type *)0));
	//Set function to the ActionScheduler
	actionScheduler->setInitializeFunction(initialize);

	// Add a basic block entry to the scheduler.
	BasicBlock* BB = BasicBlock::Create(Context, "entry", initialize);

	// Add a basic block return to the scheduler.
	BasicBlock* returnBB = BasicBlock::Create(Context, "return", initialize);
	ReturnInst::Create(Context, returnBB);

	//Test initialize function
	list<Action*>::iterator it;

	for ( it=initializes->begin() ; it != initializes->end(); it++ ){
		BB = createActionTest(*it, BB, returnBB, initialize);
	}

	//Create branch from skip to return
	BranchInst::Create(returnBB, BB);
}

BasicBlock* ActionSchedulerAdder::createSchedulerFSM(Instance* instance, BasicBlock* BB, BasicBlock* incBB, BasicBlock* returnBB, Function* function){
	//Create a variable that store the current state of the FSM
	Module* module = decoder->getModule();
	ActionScheduler* actionScheduler = instance->getActionScheduler();
	FSM* fsm = actionScheduler->getFsm();
	string name = instance->getId();
	name.append("_FSM_state");
	stateVar = cast<GlobalVariable>(module->getOrInsertGlobal(name, Type::getInt32Ty(Context)));
	
	//Set initial state to the state variable
	FSM::State* state = fsm->getInitialState();
	stateVar->setInitializer(ConstantInt::get(Type::getInt32Ty(Context), state->getIndex()));

	//Load state variable
	LoadInst* loadStateVar = new LoadInst(stateVar, "", BB);

	//Create action outside fsm
	std::list<Action*>* actions = actionScheduler->getActions();
	if (!actions->empty()){
		outsideSchedulerFn = createSchedulerOutsideFSM(instance);
	}

	//Create fsm scheduler
	createStates(fsm->getStates(), function);
	createTransitions(fsm->getTransitions(), incBB, returnBB, function);
	createSwitchTransition(loadStateVar, BB, returnBB);

	return BB;
}

Function* ActionSchedulerAdder::createSchedulerOutsideFSM(Instance* instance){
	Module* module = decoder->getModule();
	string name = instance->getId();
	name.append("_outside_FSM_scheduler");
	ActionScheduler* actionScheduler = instance->getActionScheduler();
	std::list<Action*>* actions = actionScheduler->getActions();
	
	Function* outsideScheduler = cast<Function>(module->getOrInsertFunction(name, Type::getVoidTy(Context),
										  (Type *)0));
	
	//Create values
	Value *Zero = ConstantInt::get(Type::getInt32Ty(Context), 0);
	Value *One = ConstantInt::get(Type::getInt32Ty(Context), 1);

	// Add a basic block entry and BB to the outside scheduler.
	BasicBlock* BBEntry = BasicBlock::Create(Context, "entry", outsideScheduler);
	BasicBlock* BB1  = BasicBlock::Create(Context, "bb", outsideScheduler);
	BranchInst::Create(BB1, BBEntry);
		
	//Iterate tough actions
	list<Action*>::iterator it;
	BasicBlock* BB = BB1;	
	for ( it=actions->begin() ; it != actions->end(); it++ ){
		BB = createActionTest(*it, BB, BB1, outsideScheduler);
	}

	//Return if no action can be fired
	ReturnInst::Create(Context, BB);

	return outsideScheduler;
}

BasicBlock* ActionSchedulerAdder::createSchedulerNoFSM(Instance* instance, BasicBlock* BB, BasicBlock* incBB, BasicBlock* returnBB, Function* function){
	list<Action*>::iterator it;
	ActionScheduler* actionScheduler = instance->getActionScheduler();
	list<Action*>* actions = actionScheduler->getActions();

	for ( it=actions->begin() ; it != actions->end(); it++ ){
		BB = createActionTest(*it, BB, incBB, function);
	}

	//Create branch from skip to return
	BranchInst::Create(returnBB, BB);

	return BB;
}

BasicBlock* ActionSchedulerAdder::createActionTest(Action* action, BasicBlock* BB, BasicBlock* incBB, Function* function){
	string name = action->getName();
	string skipBrName = "skip_";
	string hasRoomBrName = "hasroom_";
	string fireBrName = "fire_";
	skipBrName.append(name);
	hasRoomBrName.append(name);
	fireBrName.append(name);
	Procedure* body = action->getBody();

	// Add a basic block to bb for firing instructions
	BasicBlock* fireBB = BasicBlock::Create(Context, fireBrName, function);

	// Add a basic block to bb for ski instructions
	BasicBlock* skipBB = BasicBlock::Create(Context, skipBrName, function);

	//Test firing condition of an action
	Procedure* scheduler = action->getScheduler();
	CallInst* callInst = CallInst::Create(scheduler->getFunction(), "",  BB);
	BranchInst* branchInst	= BranchInst::Create(fireBB, skipBB, callInst, BB);

	//Test if rooms are available on ouput
	map<Port*, ConstantInt*>::iterator it;
	map<Port*, ConstantInt*>* outputPattern = action->getOutputPattern();
	
	if (!outputPattern->empty()){
		std::list<Value*>::iterator itValue;
		std::list<Value*> values;

		for ( it=outputPattern->begin() ; it != outputPattern->end(); it++ ){
			Value* hasRoomValue = createOutputTest(it->first, it->second, fireBB);
			TruncInst* truncRoomInst = new TruncInst(hasRoomValue, Type::getInt1Ty(Context),"", fireBB);
			values.push_back(truncRoomInst);
		}

		itValue=values.begin();
		Value* value1 = *itValue;
		for ( itValue=++itValue ; itValue != values.end(); itValue++ ){
			Value* value2 = *itValue;
			value1 = BinaryOperator::Create(Instruction::And,value1, value2, "", fireBB);
		}

		// Add a basic block hasRoom that fires the action
		string hasRoomBrName = "hasRoom_";
		hasRoomBrName.append(name);
		BasicBlock* roomBB = BasicBlock::Create(Context, hasRoomBrName, function);
		CallInst* callInst = CallInst::Create(body->getFunction(), "",  roomBB);

		//Branch hasRoom block to inc i block
		BranchInst::Create(incBB, roomBB);


		//Finally branch fire to hasRoom block if all outputs have free room
		BranchInst* brInst = BranchInst::Create(roomBB, skipBB, value1, fireBB);
		
	}else{
		//Launch action body
		CallInst* callInst = CallInst::Create(body->getFunction(), "",  fireBB);

		//Branch fire basic block to BB basic block
		BranchInst::Create(incBB, fireBB);
	}


	return skipBB;
}

void ActionSchedulerAdder::createStates(map<string, FSM::State*>* states, Function* function){
	map<string, FSM::State*>::iterator it;

	for (it = states->begin(); it != states->end(); it++){
		// Add a basic block for the current state
		BasicBlock* stateBB = BasicBlock::Create(Context, it->first, function);

		//Store the basic block
		BBTransitions.insert(pair<FSM::State*, BasicBlock*>(it->second, stateBB));
	}
}

CallInst* ActionSchedulerAdder::createOutputTest(Port* port, ConstantInt* numTokens, BasicBlock* BB){
	//Load selected port
	LoadInst* loadPort = new LoadInst(port->getGlobalVariable(), "", BB);
	
	//Call hasRoom function
	AbstractFifo* fifo = decoder->getFifo();
	Function* hasRoomFn = fifo->getHasRoomFunction(port->getType());
	Value* hasRoomArgs[] = { loadPort, numTokens};
	CallInst* callInst = CallInst::Create(hasRoomFn, hasRoomArgs, hasRoomArgs+2,"",  BB);

	return callInst;
}

void ActionSchedulerAdder::createSwitchTransition(Value* stateVar, BasicBlock* BB, BasicBlock* returnBB){
	map<FSM::State*, BasicBlock*>::iterator it;

	 SwitchInst* stateSwitch =
      SwitchInst::Create(stateVar, returnBB, BBTransitions.size(),
                         BB);

	 for (it = BBTransitions.begin(); it != BBTransitions.end(); it++){
		 FSM::State* state = it->first;
		 BasicBlock* stateBB = it->second;
		 ConstantInt* stateIndex = ConstantInt::get(Type::getInt32Ty(Context),state->getIndex());

		 stateSwitch->addCase(stateIndex, stateBB);
	 }
}

void ActionSchedulerAdder::createTransitions(map<string, FSM::Transition*>* transitions, BasicBlock* incBB, BasicBlock* returnBB, Function* function){
	map<string, FSM::Transition*>::iterator it;

	for (it = transitions->begin(); it != transitions->end(); it++){
		createTransition(it->second, incBB, returnBB, function);
	}
}

void ActionSchedulerAdder::createTransition(FSM::Transition* transition, BasicBlock* incBB, BasicBlock* returnBB, Function* function){
	createSchedulingTestState(transition->getNextStateInfo(), transition->getSourceState(), incBB, returnBB, function);
}

BasicBlock* ActionSchedulerAdder::createSchedulingTestState(list<FSM::NextStateInfo*>* nextStates, FSM::State* sourceState, BasicBlock* incBB, BasicBlock* returnBB, Function* function){
	//Get source state basic block
	std::map<FSM::State*, llvm::BasicBlock*>::iterator itState;
	itState = BBTransitions.find(sourceState);
	BasicBlock* stateBB = itState->second;

	if (outsideSchedulerFn != NULL){
		CallInst::Create(outsideSchedulerFn, "", stateBB);
	}

	//Iterate though next states of the transition
	list<FSM::NextStateInfo*>::iterator it;
	for (it = nextStates->begin(); it != nextStates->end(); it++){
		stateBB = createActionTestState(*it, sourceState, stateBB, incBB, returnBB, function);
	}

	//Store current state in skip basic block and brancg to return basic block
	ConstantInt* index = ConstantInt::get(Type::getInt32Ty(Context), sourceState->getIndex());
	StoreInst* storeInst = new StoreInst(index, stateVar, stateBB);
	BranchInst::Create(returnBB, stateBB);

	return NULL;
}

BasicBlock* ActionSchedulerAdder::createActionTestState(FSM::NextStateInfo* nextStateInfo, FSM::State* sourceState, BasicBlock* stateBB, BasicBlock* incBB, BasicBlock* returnBB, Function* function){


	//Get information about next state
	Action* action = nextStateInfo->getAction();
	FSM::State* targetState = nextStateInfo->getTargetState();

	//Create a branch for firing next state
	string fireStateBrName = "fire_";
	fireStateBrName.append(action->getName());
	BasicBlock* fireStateBB = BasicBlock::Create(Context, fireStateBrName, function);

	//Create a branch for skip next state
	string skipStateBrName = "skip_";
	skipStateBrName.append(action->getName());
	BasicBlock* skipStateBB = BasicBlock::Create(Context, skipStateBrName, function);

	//Test firing condition of an action
	Procedure* scheduler = action->getScheduler();
	CallInst* callInst = CallInst::Create(scheduler->getFunction(), "",  stateBB);
	BranchInst* branchInst	= BranchInst::Create(fireStateBB, skipStateBB, callInst, stateBB);

	map<Port*, ConstantInt*>::iterator it;
	map<Port*, ConstantInt*>* outputPattern = action->getOutputPattern();
	
	if (!outputPattern->empty()){
		std::list<Value*>::iterator itValue;
		std::list<Value*> values;

		for ( it=outputPattern->begin() ; it != outputPattern->end(); it++ ){
			Value* hasRoomValue = createOutputTest(it->first, it->second, fireStateBB);
			TruncInst* truncRoomInst = new TruncInst(hasRoomValue, Type::getInt1Ty(Context),"", fireStateBB);
			values.push_back(truncRoomInst);
		}

		itValue=values.begin();
		Value* value1 = *itValue;
		for ( itValue=++itValue ; itValue != values.end(); itValue++ ){
			Value* value2 = *itValue;
			value1 = BinaryOperator::Create(Instruction::And,value1, value2, "", fireStateBB);
		}

		// Add a basic block hasRoom that fires the action
		string hasRoomBrName = "hasRoom_";
		hasRoomBrName.append(action->getName());
		BasicBlock* roomBB = BasicBlock::Create(Context, hasRoomBrName, function);

		//Create a basic block skip_hasRoom that store state and return from function
		string skipHasRoomBrName = "skipHasRoom_";
		skipHasRoomBrName.append(action->getName());
		BasicBlock* skipRoomBB = BasicBlock::Create(Context, skipHasRoomBrName, function);
		ConstantInt* index = ConstantInt::get(Type::getInt32Ty(Context), sourceState->getIndex());
		StoreInst* storeInst = new StoreInst(index, stateVar, skipRoomBB);
		BranchInst::Create(returnBB, skipRoomBB);

		//Finally branch fire to hasRoom block if all outputs have free room
		BranchInst* brInst = BranchInst::Create(roomBB, skipRoomBB, value1, fireStateBB);

		fireStateBB = roomBB;
	}

	createActionCallState(nextStateInfo, fireStateBB);

	return skipStateBB;
}

void ActionSchedulerAdder::createActionCallState(FSM::NextStateInfo* nextStateInfo, llvm::BasicBlock* BB){
	std::map<FSM::State*, llvm::BasicBlock*>::iterator it;
	
	//Get next state information
	Action* action = nextStateInfo->getAction();
	FSM::State* nextState = nextStateInfo->getTargetState();

	//Call body of the action
	Procedure* body = action->getBody();
	CallInst* callInst = CallInst::Create(body->getFunction(), "",  BB);

	//Create a branch to the next state
	it = BBTransitions.find(nextState);
	BranchInst* brInst = BranchInst::Create(it->second, BB);

}