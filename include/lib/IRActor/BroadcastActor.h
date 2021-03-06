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
@brief Description of the Broadcast class interface
@author Jerome Gorin
@file BroadcastActor.h
@version 1.0
@date 15/11/2010
*/

//------------------------------
#ifndef BROADCASTACTOR_H
#define BROADCASTACTOR_H

#include "lib/IRCore/Actor.h"

namespace llvm{
class LLVMContext;
class Function;
class IntegerType;
class Module;
class Type;
class BasicBlock;
class LoadInst;
class Value;
}

class Decoder;
class AbstractFifo;
class InstancedActor;
//------------------------------

/**
 * @brief  This class defines a source actor to be connected in the DecoderEngine.
 *
 * @author Jerome Gorin
 *
 */
class BroadcastActor  : public Actor {
public:
    BroadcastActor(llvm::LLVMContext& C, Decoder* decoder, std::string name, int numOutputs, llvm::IntegerType* type);
    ~BroadcastActor();

    /**
     *  @brief Indicate whether this actor is parseable or not
     *
     *  @return boolean designing the actor parsing ability
     *
     */
    bool isParseable(){return false;}


    /**
     *  @brief return type of broadcast
     *
     *  @return llvm::Type of the broadcast
     *
     */
    llvm::IntegerType* getType() {
        return type;
    }

    /**
     *  @brief return input port of the broadcast
     *
     *  @return input port of the broadcast
     *
     */
    Port* getInput() {
        return inputs->begin()->second;
    }

    bool isBroadcast(){return true;}

private:

    /** The current decoder  */
    Decoder* decoder;

    /** Number of output of the broadcast */
    int numOutputs;

    /** Port type of the broadcast */
    llvm::IntegerType* type;

    /** LLVM Context */
    llvm::LLVMContext &Context;

    /**
     *  @brief Create the actor broadcast
     */
    void createActor();

    /**
     *  @brief Create the actions of broadcast actor
     */
    void createAction();

    /**
     *  @brief Create the MoC of broadcast actor
     */
    MoC* createMoC();

    /**
     *  @brief Create the scheduler of the broadcast action
     */
    Procedure* createScheduler();

    /**
     *  @brief Create the body of the broadcast action
     */
    Procedure* createBody();

    /**
     *  @brief Create the pattern of the broadcast action
     *
     *  @param ports : ports of the action
     */
    Pattern* createPattern(std::map<std::string, Port*>* ports);

    /**
     *  @brief Create a read fifo action in broadcast
     *
     *  Add llvm instruction to read the fifo corresponding to the given port
     *
     *  @param current : llvm::BasicBlock to add the instructions
     *
     *  @return llvm::Value read from the fifo
     */
    llvm::Value* createReadFifo(Port* port, llvm::BasicBlock* current);

    /**
     *  @brief Create a write fifo action in broadcast
     *
     *  Add llvm instruction to write the fifo corresponding to the given port
     *
     *  @param port : the Port to read
     *
     *  @param token : the llvm::Value of the token to write
     *
     *  @param current : llvm::BasicBlock to add the instructions
     */
    void createWriteFifo(Port* port, llvm::Value* token ,llvm::BasicBlock* current);

    /**
     *  @brief Create an has token test in broadcast
     *
     *  Add llvm instruction to test the number of token present in the given port
     *
     *  @param port : the Port to test
     *
     *  @param current : llvm::BasicBlock to add the instructions
     */
    llvm::Value* createHasTokenTest(Port* port, llvm::BasicBlock* current);

    /**
     *  @brief Set read to end
     *
     *  Call set readEnd function on the given port
     *
     *  @param port : the Port to set to readEnd
     *
     *  @param current : llvm::BasicBlock to add the instructions
     */
    void createSetReadEnd(Port* port, llvm::BasicBlock* current);
};

#endif
