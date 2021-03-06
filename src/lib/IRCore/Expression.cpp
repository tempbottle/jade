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
@brief Implementation of deriving class from Expression
@author Jerome Gorin
@file Expression.cpp
@version 1.0
@date 15/11/2010
*/

//------------------------------
#include <iostream>

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"

#include "lib/IRCore/Expr/BoolExpr.h"
#include "lib/IRCore/Expr/BinaryExpr.h"
#include "lib/IRCore/Expr/IntExpr.h"
#include "lib/IRCore/Expr/ListExpr.h"
#include "lib/IRCore/Expr/StringExpr.h"
//------------------------------

using namespace std;
using namespace llvm;

int Expr::evaluateAsInteger(){
    if (isIntExpr()){
        return ((IntExpr*)this)->getValue();
    }

    if (isBinaryExpr()){
        BinaryExpr* binExpr = (BinaryExpr*)this;
        int e1 = binExpr->getE1()->evaluateAsInteger();
        int e2 = binExpr->getE1()->evaluateAsInteger();
        BinaryOp* op = binExpr->getOp();

        switch(op->getType()){
        case BinaryOp::TIMES:
            return e1*e2;
            break;

        case BinaryOp::MINUS:
            return e1-e2;
            break;

        case BinaryOp::PLUS:
            return e1+e2;
            break;

        default :
            cerr << "Unsupported binary expression for evaluation" << endl;
            exit(1);
        }

    }

    cerr << "Can't evaluate this expression";
    exit(1);
}

Constant* BoolExpr::getConstant(){
    return ConstantInt::get(Type::getInt1Ty(Context), value);
}

Constant* IntExpr::getConstant(){
    if (constantVal == NULL){
        constantVal = ConstantInt::get(Type::getInt32Ty(Context), value);
    }
    return constantVal;
}

Constant* ListExpr::getConstant(){
    return constantArray;
}

Constant* BinaryExpr::getConstant(){

    return NULL;
}

Constant* StringExpr::getConstant(){

    return NULL;
}
