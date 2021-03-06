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
@brief Implementation of LLVMOptimization
@author Jerome Gorin
@file DecoderExecution.cpp
@version 1.0
@date 15/11/2010
*/

//------------------------------
#include <iostream>

#include "lib/RVCEngine/Decoder.h"
#include "lib/IRJit/LLVMOptimizer.h"

#include "llvm/LinkAllPasses.h"
#include "llvm/IR/Module.h"
#include "llvm/PassManager.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassNameParser.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
//------------------------------
using namespace llvm;
using namespace std;

//Optimization flag
bool VerifyEach = false;
bool StandardCompileOpts = false;
bool StandardLinkOpts = true;
bool DisableInline = false;
bool UnitAtATime = false;
bool DisableInternalize = false;

cl::opt<std::string>
TargetTriple("mtriple", cl::desc("Override target triple for module"));

void LLVMOptimizer::optimize(int optLevel){

    // Initialize passes
    PassRegistry &Registry = *PassRegistry::getPassRegistry();
    initializeCore(Registry);
    initializeScalarOpts(Registry);
    initializeIPO(Registry);
    initializeAnalysis(Registry);
    initializeIPA(Registry);
    initializeTransformUtils(Registry);
    initializeInstCombine(Registry);
    initializeInstrumentation(Registry);
    initializeTarget(Registry);

    Module* module = decoder->getModule();

    // Allocate a full target machine description only if necessary.
    // FIXME: The choice of target should be controllable on the command line.
    std::auto_ptr<TargetMachine> target;

    // Create a PassManager to hold and optimize the collection of passes we are
    // about to build...
    //
    legacy::PassManager Passes;

    // Add an appropriate TargetLibraryInfo pass for the module's triple.
    TargetLibraryInfo *TLI;
    if (TargetTriple != "") {
        TLI = new TargetLibraryInfo(Triple(TargetTriple));
    } else {
        TLI = new TargetLibraryInfo(Triple(module->getTargetTriple()));
    }

    Passes.add(TLI);

    // Add an appropriate DataLayout instance for this module.
    Passes.add(new DataLayoutPass(module));

    std::unique_ptr<FunctionPassManager> FPasses;
    if (optLevel > 0) {
        FPasses.reset(new FunctionPassManager(module));
        FPasses->add(new DataLayoutPass(module));
    }

    AddOptimizationPasses(Passes, *FPasses, optLevel);

    /* TODO: Enhance

    // Create a new optimization pass for each one specified on the command line
    for (unsigned i = 0; i < PassList.size(); ++i) {
        // Check to see if -std-compile-opts was specified before this option.  If
        // so, handle it.
        if (StandardCompileOpts &&
                StandardCompileOpts.getPosition() < PassList.getPosition(i)) {
            AddStandardCompilePasses(Passes);
            StandardCompileOpts = false;
        }

        if (StandardLinkOpts &&
                StandardLinkOpts.getPosition() < PassList.getPosition(i)) {
            AddStandardLinkPasses(Passes);
            StandardLinkOpts = false;
        }

        if (OptLevelO1 && OptLevelO1.getPosition() < PassList.getPosition(i)) {
            AddOptimizationPasses(Passes, *FPasses, 1);
            OptLevelO1 = false;
        }

        if (OptLevelO2 && OptLevelO2.getPosition() < PassList.getPosition(i)) {
            AddOptimizationPasses(Passes, *FPasses, 2);
            OptLevelO2 = false;
        }

        if (OptLevelO3 && OptLevelO3.getPosition() < PassList.getPosition(i)) {
            AddOptimizationPasses(Passes, *FPasses, 3);
            OptLevelO3 = false;
        }
    }


    // If -std-compile-opts was specified at the end of the pass list, add them.
    if (StandardCompileOpts) {
        AddStandardCompilePasses(Passes);
        StandardCompileOpts = false;
    }

    if (StandardLinkOpts) {
        AddStandardLinkPasses(Passes);
        StandardLinkOpts = false;
    }
    */

    if (optLevel > 0) {
        FPasses->doInitialization();
        for (Module::iterator F = module->begin(), E = module->end(); F != E; ++F)
            FPasses->run(*F);
        FPasses->doFinalization();
    }

    // Now that we have all of the passes ready, run them.
    Passes.run(*module);

}


/// AddOptimizationPasses - This routine adds optimization passes
/// based on selected optimization level, OptLevel. This routine
/// duplicates llvm-gcc behaviour.
///
/// OptLevel - Optimization Level
void LLVMOptimizer::AddOptimizationPasses(PassManagerBase &MPM, FunctionPassManager &FPM, unsigned OptLevel) {
    PassManagerBuilder Builder;
    Builder.OptLevel = OptLevel;

    if (DisableInline) {
        // No inlining pass
    } else if (OptLevel > 1) {
        unsigned Threshold = 225;
        if (OptLevel > 2)
            Threshold = 275;
        Builder.Inliner = createFunctionInliningPass(Threshold);
    } else {
        Builder.Inliner = createAlwaysInlinerPass();
    }
    Builder.DisableUnitAtATime = !UnitAtATime;
    Builder.DisableUnrollLoops = OptLevel == 0;

    Builder.populateFunctionPassManager(FPM);
    Builder.populateModulePassManager(MPM);
}

