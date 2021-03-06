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
@brief Description of the ActionSchedulerAdder interface
@author Jerome Gorin
@file ActionSchedulerAdder.h
@version 1.0
@date 15/11/2010
*/

//------------------------------
#ifndef ACTIONSCHEDULERADDER_H
#define ACTIONSCHEDULERADDER_H

#include <list>
#include <map>
#include "lib/IRCore/Actor/FSM.h"

namespace llvm{
class BasicBlock;
class CallInst;
class ConstantInt;
class Function;
class Instruction;
class LLVMContext;
class Module;
class Value;
}

class Action;
class ActionScheduler;
class Actor;
class Instance;
class Decoder;
class Port;
class Variable;
class MoC;
class Pattern;
class StateVar;
//------------------------------

/**
 * @brief  This class defines a robin robin scheduler of a decoder.
 *
 * @author Jerome Gorin
 *
 */
class ActionSchedulerAdder {
public:
    /**
     *  @brief Constructor
     *
     *  Create a new round robin scheduler for the given decoder
     *
     *  @param jit : JIT for including instructions
     *
     *  @param decoder : the Decoder to insert the round robin scheduler into
     */
    ActionSchedulerAdder(llvm::LLVMContext& C, Decoder* decoder);
    ~ActionSchedulerAdder(){}

    /**
     *  @brief Start the transformation on an instance
     *
     *  @param instance : the Instance to transform
     */
    void transform(Instance* instance);

protected:

    /**
     *  @brief Create an action scheduler
     *
     *  Create an action scheduler in the instance
     *
     *  @param instance : the Instance to add the action scheduler
     */
    virtual void createActionScheduler(Instance* instance);

    /**
     *  @brief Create the scheduler of actions
     *
     * @param instance: the Instance to add the scheduler
     *
     * @param BB : llvm::BasicBlock where scheduler is add
     *
     * @param incBB : llvm::BasicBlock where scheduler has to branch in case of success
     *
     * @param returnBB : llvm::BasicBlock where scheduler has to branch in case of return
     *
     * @param function : llvm::Function where the scheduler is added
     */
    virtual void createScheduler(Instance* instance, llvm::BasicBlock* BB, llvm::BasicBlock* incBB, llvm::BasicBlock* returnBB, llvm::Function* scheduler) = 0;

    /**
     *  @brief Create initialize
     *
     *  Create the initialize action scheduler for the instance
     *
     *  @param instance : the Instance to add the initialization action scheduler
     */
    void createInitialize(Instance* instance);

    /**
     * @brief check a output pattern for an Action
     *
     * @param action : the Action to test
     *
     * @param function : function where test is added
     *
     * @param skipBB : llvm::BasicBlock where to branch in case of fail
     *
     * @param BB : llvm::BasicBlock where add instructions
     *
     * @return the next basic block to add instruction
     */
    virtual llvm::BasicBlock* checkOutputPattern(Pattern* pattern, llvm::Function* function, llvm::BasicBlock* skipBB, llvm::BasicBlock* BB);

    /**
     * @brief Check input pattern of an action
     *
     * @param pattern : the Pattern to check
     *
     * @param function : llvm::Function to add the check
     *
     * @param skipBB : llvm::BasicBlock to branch in case of failure
     *
     * @param BB : llvm::BasicBlock that start the checking
     *
     * @return the last llvm::BasicBlock to add instruction
     */
    virtual llvm::BasicBlock* checkInputPattern(Pattern* pattern, llvm::Function* function, llvm::BasicBlock* skipBB, llvm::BasicBlock* BB);

    virtual void initializeFIFO (Instance* instance);

    /**
     * @brief Initialize a Fifo in the special case of unconnected port.
     *
     * This is necessary since Fifo are usually created in Connector, from information parsed in a Connection.
     * In the case of unconnected port, no connection is associated with the port. We create it in this method.
     *
     * @param port The unconnected port
     */
    void initializeFakeFIFO(Port *port);

    /** LLVM Context */
    llvm::LLVMContext &Context;

    /** Decoder to apply the transformation */
    Decoder* decoder;

    /** Action Scheduler of the Instant */
    ActionScheduler* actionScheduler;

    /** MoC of the Instance */
    MoC* moc;

    /** Current instance */
    Instance* instance;

    /** Current module */
    llvm::Module* module;

    /** ActionScheduler BB */
    llvm::BasicBlock* entryBB;
    llvm::BasicBlock* bb1;
    llvm::BasicBlock* incBB;
    llvm::BasicBlock* returnBB;
};

#endif
