#include <cstdio>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct ssadce : public FunctionPass {
        static char ID;
        ssadce() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function &F) {
            
            /**
             * SSA - Dead Code Elimination
             * ===========================
             * 
             * WHILE there is some variable v with no uses
             * AND the statement that defines v has no other side effects
             * DO delete the statement that defines v.
             * 
             * W <-- a list of all variables in the SSA program
             * while W is not empty
             *   remove some variable v from W
             *   if v's list of uses is empty
             *     let S be v's statement of definition
             *     if S has no side effects others than the assignment to v
             *       delete S from the program
             *       foreach variable x used by S
             *         delete S from the list of uses of x
             *         W <-- W union x
             * 
             */
            
            
            
            for (User *U : F.users()) {

                if (Instruction *inst = dyn_cast<Instruction>(U)) {
                    // while W is not empty
                    while (!inst->use_empty()) {
                        errs() << "\nIterating over non empty user: ";
                        errs() << *inst << "\n";
                        // remove some variable v from W
                        // inst++;
                        // inst->eraseFromParent();
                                /*
                        for (Use &var : inst->operands()) {
                            // if v's list of uses is empty
                            if (var->use_empty()) {
                                
                                errs << var->getOperands() << "\n";
                                //errs() << "v's list of uses is empty \n";
                                // let S be v's statement of definition
                                // S = ?
                                // if S has no side effects
                                // other than the assignment to v
                                
                                if (!var->mayHaveSideEffects()) { // TODO: FRACO
                                    for (Use *x : var->operands()) {
                                        errs() << "AOooo\n";
                                    }
                                }
                            }
                        }
                                */
                        
                    }
                    
           
                }
            }
            
            return false;
        }
    };
}

char ssadce::ID = 0;
static RegisterPass<ssadce> X("ssa-dce", "SSA-DCE PASS", false, false);
