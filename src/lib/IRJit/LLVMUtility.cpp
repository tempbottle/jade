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
@brief Implementation of utility function in class JIT
@author Jerome Gorin
@file Utility.cpp
@version 1.0
@date 15/11/2010
*/

//------------------------------
#include <iostream>
#include <fstream>

#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"


#include "lib/RVCEngine/Decoder.h"
#include "lib/IRJit//LLVMUtility.h"
//------------------------------

using namespace llvm;
using namespace std;

extern cl::opt<std::string> OutputDir;

void LLVMUtility::printModule(Decoder* decoder, string fileName){
    Module* module = decoder->getModule();

    if(fileName == "") {
        string netName = decoder->getConfiguration()->getNetwork()->getName();
        fileName = netName + "_FULL_MODULE.ll";
    }

    std::string ErrorInfo;
    //Preparing output file
    string outputPath = OutputDir + fileName;

    //Preparing output
    std::auto_ptr<raw_fd_ostream> outStream(new raw_fd_ostream(outputPath.c_str(), ErrorInfo, sys::fs::F_None));
    if (!ErrorInfo.empty()) {
        std::cerr << ErrorInfo << endl;
        return;
    }
    *outStream << *module;

}

void LLVMUtility::verify(string file, Decoder* decoder){
    std::string Err;
    Module* module = decoder->getModule();

    raw_string_ostream err_stream(Err);
    if (verifyModule(*module, &err_stream)) {
        ofstream output;

        //Preparing output file
        string OutFile = file;
        OutFile.insert(0,OutputDir);

        //Preparing output
        output.open(OutFile.c_str());
        output << Err;
        cerr << "Error found in the current decoder, output " << OutFile << " error file" << endl;
    } else {
        cout << "Generated decoder is ok." << endl;
    }
}
