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
@brief Description of the IRLinker class interface
@author Jerome Gorin
@file IRLinker.h
@version 1.0
@date 15/11/2010
*/

//------------------------------
#ifndef IRLINKER_H
#define IRLINKER_H

#include <map>

#include "lib/IRCore/Actor.h"
#include "lib/IRCore/Network/Instance.h"
//------------------------------


/**
 * @class IRLinker
 *
 * @brief This class defines a linker that links instance not instanced with an
 *   an instanced instance if two instances have the same property
 *
 * @author Jerome Gorin
 *
 */
class IRLinker{
public:

    /**
     * @brief Creates an instance writer on the given actor.
     *
     * @param instance : Instance to write
     */
    IRLinker(Decoder* decoder);

    /**
     * @brief Links instances.
     *
     * Links instances from a previous configuration in a new configuration
     *
     * @param instances: a list of Instance to link
     *
     * @return true if the actor is unwritten, otherwise false
     */
    int link(std::list<std::pair<Instance*, Instance*> >* instances);

    ~IRLinker();

private:
    /**
     * @brief Link two instances
     *
     * @param refInstance : the reference Instance to link.
     *
     * @param instance : the destination Instance.
     */
    void linkInstance(Instance* refinstance, Instance* instance);

    /**
     * @brief Link two list of ports
     *
     * Link port's variable from a list of ports to destination ports
     *
     * @param refPorts : reference ports
     *
     * @param ports : the destination ports
     */
    void linkPorts(std::map<std::string, Port*>* refPorts, std::map<std::string, Port*>* ports);


    /** Decoder where instance are linked */
    Decoder* decoder;
};

#endif
