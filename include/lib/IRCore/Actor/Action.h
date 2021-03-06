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
@brief Description of the Action interface
@author Jerome Gorin
@file Action.h
@version 1.0
@date 15/11/2010
*/

//------------------------------
#ifndef ACTION_H
#define ACTION_H

#include <map>
#include <string>

namespace llvm{
class ConstantInt;
}

#include "lib/IRCore/Entity.h"
#include "lib/IRCore/Actor/ActionTag.h"
#include "lib/IRCore/Actor/ActionScheduler.h"
#include "lib/IRCore/Actor/Pattern.h"

class Port;
class Procedure;
//------------------------------

/**
 * @brief  This class defines an action scheduler for a Functional Unit.
 *
 * @author Jerome Gorin
 *
 */
class Action {
public:

    /**
     *
     * @brief Constructor
     *
     * Creates a new action.
     *
     * @param location : Location of the action
     *
     * @param tag: action Tag
     *
     * @param scheduler : Procedure that computes scheduling information
     *
     * @param body : Procedure that holds the body of the action
     *
     * @param parent : parent entity of the action
     */
    Action(ActionTag* tag, Pattern* inputPattern, Pattern* outputPattern, Pattern* peekPattern, Procedure* scheduler, Procedure* body, Entity* parent) {
        this->tag = tag;
        this->body = body;
        this->scheduler = scheduler;
        this->inputPattern = inputPattern;
        this->outputPattern = outputPattern;
        this->peekPattern = peekPattern;
        this->parent = parent;
    }

    ~Action();

    /**
     *  @brief Getter of the body of the action
     *
     *  @return the body of the action
     */
    Procedure* getBody(){return body;}

    /**
     *  @brief Setter of the body of the action
     *
     *  @param body : Procedure of the body of the action
     */
    void setBody(Procedure* body){this->body = body;}

    /**
     *  @brief Getter of the scheduler of the action
     *
     *  @return the scheduler of the action
     */
    Procedure* getScheduler(){return scheduler;}

    /**
     *  @brief Getter of the tag of the action
     *
     *  @return the tag of the action
     */
    ActionTag* getTag(){return tag;}

    /**
     *  @brief Setter of the scheduler of the action
     *
     *  @param the scheduler of the action
     */
    void setScheduler(Procedure* scheduler){this->scheduler = scheduler;}

    /**
     *  @brief return input pattern of the action
     *
     *  @param a map of input pattern
     */
    Pattern* getInputPattern(){return inputPattern;}

    /**
     *  @brief return output pattern of the action
     *
     *  @param a map of output pattern
     */
    Pattern* getOutputPattern(){return outputPattern;}

    /**
     *  @brief return peek pattern of the action
     *
     *  @param a map of peek pattern
     */
    Pattern* getPeekPattern(){return peekPattern;}

    /**
     * @brief Returns action name (tag or body name)
     *
     * @return action string name
     */
    std::string getName();

    /**
     *  brief Returns the right CAL action name
     *
     * @return CAL action string name
     */
    std::string getCalName();

    /**
     *  brief Returns the parent of the action
     *
     * @return parent of the action
     */
    Entity* getParent(){return parent;}

private:
    ActionTag* tag;
    Procedure* body;
    Procedure* scheduler;
    Pattern* inputPattern;
    Pattern* outputPattern;
    Pattern* peekPattern;
    Entity* parent;
};

#endif
