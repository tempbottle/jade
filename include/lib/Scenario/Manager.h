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
@brief Description of the Manager class interface
@author Jerome Gorin
@file Manager.h
@version 1.0
@date 26/01/2011
*/

//------------------------------
#ifndef MANAGER_H
#define MANAGER_H
#include <string>

#include "lib/Scenario/Event/LoadEvent.h"
#include "lib/Scenario/Event/StartEvent.h"
#include "lib/Scenario/Event/StopEvent.h"
#include "lib/Scenario/Event/SetEvent.h"
#include "lib/Scenario/Event/WaitEvent.h"
#include "lib/Scenario/Event/PauseEvent.h"
#include "lib/Scenario/Event/PrintEvent.h"
#include "lib/Scenario/Event/VerifyEvent.h"
#include "lib/Scenario/Event/RemoveEvent.h"
#include "lib/Scenario/Event/ListEvent.h"

class RVCEngine;
class Network;
class Scenario;
//------------------------------

/**
 * @brief  This class represents a manager for the DecoderEngine.
 *
 * @author Jerome Gorin
 *
 */
class Manager {
public:
    /*!
     *  @brief Constructor
     *
     * Creates a new manager for the DecoderEngine.
     *
     * @param engine : the DecoderEngine to manage
     *
     * @param verify : decoder has to be verify or not
     *
     * @param verbose : display actions taken
     */
    Manager(RVCEngine* engine, int optLevel = 0, bool verify = false, bool verbose = false);

    /*!
     *  @brief Start the manager
     *
     * Start the manager with the given scenario.
     *
     * @param scFile : the file that contains a scenario.
     *
     * @return true if scenario finished correctly, otherwise false
     */
    bool start(std::string scFile);

    /*!
     *  @brief Start an event
     *
     * @param newEvent : the Event to start.
     *
     * @return true if Event finished correctly, otherwise false
     */
    bool startEvent(Event* newEvent);

    /*!
     *  @brief Destructor
     *
     * Delete the scenario.
     */
    ~Manager(){}

private:

    /*!
     *  @brief run a load event
     *
     * @param loadEvent : the LoadEvent to run.
     *
     * @return true if event finished correctly, otherwise false
     */
    bool runLoadEvent(LoadEvent* loadEvent);

    /*!
     *  @brief run a start event
     *
     * @param startEvent : the StartEvent to run.
     *
     * @return true if event finished correctly, otherwise false
     */
    bool runStartEvent(StartEvent* startEvent);

    /*!
     *  @brief run a wait event
     *
     * @param waitEvent : the WaitEvent to run.
     *
     * @return true if event finished correctly, otherwise false
     */
    bool runWaitEvent(WaitEvent* waitEvent);

    /*!
     *  @brief run a stop event
     *
     * @param stopEvent : the StopEvent to run.
     *
     * @return true if event finished correctly, otherwise false
     */
    bool runStopEvent(StopEvent* stopEvent);

    /*!
     *  @brief run a set event
     *
     * @param setEvent : the SetEvent to run.
     *
     * @return true if event finished correctly, otherwise false
     */
    bool runSetEvent(SetEvent* setEvent);

    /*!
     *  @brief run a pause event
     *
     * @param pauseEvent : the PauseEvent to run.
     *
     * @return true if event finished correctly, otherwise false
     */
    bool runPauseEvent(PauseEvent* pauseEvent);

    /*!
     *  @brief run a print event
     *
     * @param printEvent : the PrintEvent to run.
     *
     * @return true if event finished correctly, otherwise false
     */
    bool runPrintEvent(PrintEvent* printEvent);

    /*!
     *  @brief run a verify event
     *
     * @param verifyEvent : the VerifyEvent to run.
     *
     * @return true if event finished correctly, otherwise false
     */
    bool runVerifyEvent(VerifyEvent* verifyEvent);

    /*!
     *  @brief run a remove event
     *
     * @param removeEvent : the RemoveEvent to run.
     *
     * @return true if event finished correctly, otherwise false
     */
    bool runRemoveEvent(RemoveEvent* removeEvent);

    /*!
     *  @brief run a list event
     *
     * @param listEvent : the ListEvent to run.
     *
     * @return true if event finished correctly, otherwise false
     */
    bool runListEvent(ListEvent* listEvent);


    /** Decoder engine to manage*/
    RVCEngine* engine;

    /** Network loads */
    std::map<int, Network*> networks;

    /** Network pointer */
    std::map<int, Network*>::iterator netPtr;

    /** Verify generated decoder */
    bool verify;

    /** Verbose events */
    bool verbose;

    /** optimization level*/
    int optLevel;
};

#endif
