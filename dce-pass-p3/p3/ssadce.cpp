#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
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
             */
            for (User *U : F.users()) {
                if (Instruction *Inst = dyn_cast<Instruction>(U)) {
                    errs() << "F is used in instruction:\n ";
                    errs() << *Inst << "\n";
                    int nop = U.getUser()->getNumOperands();
                    for (Use &U : Inst->operands()) {
                        Value *v = U.get();
                        errs() << "      Operands in use: " << *v << "\n";
                        
                    }
                }
            }
            /*
            for (inst_iterator i = inst_begin(F), e = inst_end(F);
                 i != e; ++i) {
                
            }
            errs() << "SSA-DCE: ";
            errs() << F.getName() << '\n';
            */
            return false;
        }
    };
}

char ssadce::ID = 0;
static RegisterPass<ssadce> X("ssa-dce", "SSA-DCE PASS", false, false);
