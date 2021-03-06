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
@version 1.0
@date 15/11/2010
*/

//------------------------------
#include <iostream>
#include <sstream>

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "ActionSchedulerAdder.h"

#include "lib/RVCEngine/Decoder.h"
#include "lib/ConfigurationEngine/Configuration.h"
#include "lib/IRCore/Actor/Action.h"
#include "lib/IRCore/Actor/ActionScheduler.h"
#include "lib/IRCore/Actor/ActionTag.h"
#include "lib/IRCore/Actor.h"
#include "lib/IRCore/Port.h"
#include "lib/IRCore/Actor/Procedure.h"
#include "lib/IRCore/Network/Instance.h"
#include "lib/RoundRobinScheduler/Fifo.h"
#include "lib/IRUtil/FunctionMng.h"
#include "llvm/Support/CommandLine.h"
//------------------------------

using namespace llvm;
using namespace std;

// Will be used to configure fifo for unconnected outputs ports
extern cl::opt<int> FifoSize;

ActionSchedulerAdder::ActionSchedulerAdder(llvm::LLVMContext& C, Decoder* decoder) : Context(C) {
    this->module = decoder->getModule();
    this->decoder = decoder;
    this->entryBB = NULL;
    this->bb1 = NULL;
    this->incBB = NULL;
    this->returnBB = NULL;
}

void ActionSchedulerAdder::transform(Instance* instance) {
    this->instance = instance;

    //Create action scheduler
    createActionScheduler(instance);

    //Create an action scheduler initializer
    if (instance->hasInitializes()){
        createInitialize(instance);
    }
}

void ActionSchedulerAdder::createActionScheduler(Instance* instance){
    //Get properties of the instance
    Module* module = decoder->getModule();
    actionScheduler = instance->getActionScheduler();
    moc = instance->getMoC();

    string name = instance->getId();
    name.append("_scheduler");

    Function* scheduler = cast<Function>(module->getOrInsertFunction(name, Type::getInt32Ty(Context),
                                                                     (Type *)0));
    actionScheduler->setSchedulerFunction(scheduler);

    //Create values
    ConstantInt *Zero = ConstantInt::get(Context, APInt(32, 0));
    ConstantInt *One = ConstantInt::get(Context, APInt(32, 1));


    // Add a basic block entry to the scheduler.
    entryBB = BasicBlock::Create(Context, "entry", scheduler);

    //Create alloca on i and store 0
    AllocaInst* iVar = new AllocaInst(Type::getInt32Ty(Context), "i", entryBB);
    new StoreInst(Zero, iVar, entryBB);

    // Add a basic block to bb and branch entry to bb.
    bb1 = BasicBlock::Create(Context, "bb", scheduler);
    BranchInst::Create(bb1, entryBB);

    // Add a basic block return that return %i
    returnBB = BasicBlock::Create(Context, "return", scheduler);
    LoadInst* loadIRet = new LoadInst(iVar, "i_ret", returnBB);
    ReturnInst::Create(Context, loadIRet, returnBB);


    // Add a basic block inc that return %i and branch to bb
    incBB = BasicBlock::Create(Context, "inc_i", scheduler);
    LoadInst* loadIInc = new LoadInst(iVar, "i_load", incBB);
    BinaryOperator* iAdd = BinaryOperator::CreateNSWAdd(loadIInc, One, "i_add", incBB);
    new StoreInst(iAdd, iVar, incBB);
    BranchInst::Create(bb1, incBB);

    initializeFIFO (instance);
    createScheduler(instance, bb1, incBB, returnBB , scheduler);
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
        //Launch action body
        Procedure* body = (*it)->getBody();
        CallInst::Create(body->getFunction(), "",  BB);
    }

    //Create branch from skip to return
    BranchInst::Create(returnBB, BB);
}

void ActionSchedulerAdder::initializeFIFO (Instance* instance){
    map<string,Port*>::iterator it;

    //Initialize inputs
    map<string,Port*>* inputs = instance->getInputs();
    for (it = inputs->begin(); it != inputs->end(); it++){
        Port* input = it->second;

        if (!input->isConnected()) {
            cout << "Info: Input port " << it->first << " of instance " << instance->getId() << " is not connected in the network." << endl;
            initializeFakeFIFO(input);
        }

        Function* init = Fifo::initializeIn(module, input);
        CallInst::Create(init, "", entryBB->getTerminator());
    }

    //Initialize outputs
    map<string,Port*>* outputs = instance->getOutputs();
    for (it = outputs->begin(); it != outputs->end(); it++){
        Port* output = it->second;

        if (!output->isConnected()) {
            cout << "Info: Output port " << it->first << " of instance " << instance->getId() << " is not connected in the network." << endl;
            initializeFakeFIFO(output);
        }

        Function* init = Fifo::initializeOut(module, output);
        CallInst::Create(init, "", entryBB->getTerminator());
    }

    // Add read/write/peek access
    std::list<Action*>::iterator itAct;
    std::list<Action*>* actions = instance->getActions();
    for (itAct = actions->begin(); itAct != actions->end(); itAct++){
        // Create fifo accesses
        Fifo::createReadWritePeek(*itAct, instance->isTraceActivate());
    }

    //Close inputs
    for (it = inputs->begin(); it != inputs->end(); it++){
        Port* input = it->second;

        if (input->isConnected()){
            Function* close = Fifo::closeIn(module, it->second);
            CallInst::Create(close, "", returnBB->getTerminator());
        }
    }

    //Close outputs
    for (it = outputs->begin(); it != outputs->end(); it++){
        Port* output = it->second;

        if (output->isConnected()){
            Function* close = Fifo::closeOut(module, it->second);
            CallInst::Create(close, "", returnBB->getTerminator());
        }
    }
}

void ActionSchedulerAdder::initializeFakeFIFO(Port *port)
{
    // The special Fifo, not connected to anything
    Fifo* fifo = new Fifo(Context, decoder->getModule(), port->getType(), FifoSize);

    // Global variable to access the Fifo
    Type* fifoType = fifo->getGV()->getType();
    GlobalVariable *var =  new GlobalVariable(*module, fifoType, true, GlobalValue::InternalLinkage, 0, port->getName());

    // Register this variable in port
    port->setFifoVar(var);

    var->setInitializer(fifo->getGV());
}

BasicBlock* ActionSchedulerAdder::checkInputPattern(Pattern* pattern, Function* function, BasicBlock* skipBB, BasicBlock* BB){  
    //Pattern is empty, return current basic block
    if (pattern->isEmpty()){
        return BB;
    }

    //Check inputs
    list<Value*>::iterator itValue;
    list<Value*> values;
    map<Port*, ConstantInt*>::iterator it;
    map<Port*, ConstantInt*>* numTokens = pattern->getNumTokensMap();

    for ( it=numTokens->begin() ; it != numTokens->end(); it++ ){
        Port* port = it->first;

        if (port->isInternal() || port->getFifoVar() == NULL || !port->isConnected()){
            // Don't test internal ports
            continue;
        }

        Value* hasTokenValue = Fifo::createInputTest(port, it->second, BB);
        values.push_back(hasTokenValue);
    }

    // No test to do, return basic block
    if (values.empty()){
        return BB;
    }

    // Create resulting hastoken test
    itValue=values.begin();
    Value* value1 = *itValue;
    for ( itValue=++itValue ; itValue != values.end(); itValue++ ){
        Value* value2 = *itValue;
        value1 = BinaryOperator::Create(Instruction::And,value1, value2, "", BB);
    }

    // Add a basic block hasToken that test the isSchedulable of a function
    string hasTokenBrName = "hasToken";
    BasicBlock* tokenBB = BasicBlock::Create(Context, hasTokenBrName, function);

    //Finally branch fire to hasToken block if all inputs have tokens
    BranchInst::Create(tokenBB, skipBB, value1, BB);
    return tokenBB;
}

BasicBlock* ActionSchedulerAdder::checkOutputPattern(Pattern* pattern, llvm::Function* function, llvm::BasicBlock* skipBB, llvm::BasicBlock* BB){
    //No output pattern return basic block
    if (pattern->isEmpty()){
        return BB;
    }

    //Test if rooms are available on output
    list<Value*>::iterator itValue;
    list<Value*> values;
    map<Port*, ConstantInt*>::iterator it;
    map<Port*, ConstantInt*>* numTokens = pattern->getNumTokensMap();


    for ( it=numTokens->begin() ; it != numTokens->end(); it++ ){
        Port* port = it->first;

        if (port->isInternal() || port->getFifoVar() == NULL || !port->isConnected()){
            // Don't test internal ports
            // Don't test unconnected ports
            continue;
        }
        Value* hasRoomValue = Fifo::createOutputTest(port, it->second, BB);
        values.push_back(hasRoomValue);
    }

    // No test to do, return basic block
    if (values.empty()){
        return BB;
    }

    // Create resulting hasroom test
    itValue=values.begin();
    Value* value1 = *itValue;
    for ( itValue=++itValue ; itValue != values.end(); itValue++ ){
        Value* value2 = *itValue;
        value1 = BinaryOperator::Create(Instruction::And,value1, value2, "", BB);
    }

    // Add a basic block hasRoom that fires the action
    string hasRoomBrName = "hasRoom";
    BasicBlock* roomBB = BasicBlock::Create(Context, hasRoomBrName, function);

    //Finally branch fire to hasRoom block if all outputs have free room
    BranchInst::Create(roomBB, skipBB, value1, BB);

    return roomBB;
}
