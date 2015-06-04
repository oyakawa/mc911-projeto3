#include <set>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/User.h"




using namespace llvm;

bool hasSideEffectsExceptAssignment (Instruction *I);

namespace {
    struct ssadce : public FunctionPass {
        static char ID;
        ssadce() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function &F) {
                        
            // W recebe uma lista de todas as instrucoes que atribuem algum valor a algum registrador durante o programa
            std::set<Instruction*> W;
            for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
                if (!isa<CmpInst>(*I) && !isa<ReturnInst>(*I) && !isa<BranchInst>(*I) 
                    && !isa<StoreInst>(*I) && !(*I).getType()->isVoidTy()) {
                    W.insert(cast<Instruction>(&*I));
                }
            }
            
            // enquanto W nao for vazia
            while (!W.empty()) {
                // remove a primeira instrucao de W (poderia ser qualquer outra)
                Instruction *v = *(W.begin());
                W.erase(v);
                // verifica se a lista de usos de v for vazia
                if (v->use_empty()) {
                    // se v não tem efeitos colaterais exceto pela atribuicao ao registrador
                    if (!hasSideEffectsExceptAssignment(v)) {
                        // para cada variavel (instrucao que atribuiu valor a esta variavel)
                        User::op_iterator op = v->op_begin();
                        User::op_iterator x = v->op_end();
                        do {
                            Value *a = op->get();
                            op++;
                            // adiciona instrucoes ao conjunto W
                            if (Instruction *b = dyn_cast<Instruction>(a)) {
                                W.insert(b);
                            }
                        } while (op != x);
                        
                        // remove todas as referencias a v do codigo fonte
                        v->dropAllReferences();
                        v->eraseFromParent();
                    }
            
                }
            }
            
            return false;
        }
    };
}

/**
 * Verifica se a instrucao tem efeitos colaterais exceto pela atribuicao.
 */
bool hasSideEffectsExceptAssignment (Instruction *I) {
    if (I->mayHaveSideEffects()) {
        return true;
    }
    
    if (isa<TerminatorInst>(I)) {
        return true;
    }
    
    if (isa<DbgInfoIntrinsic>(I)) {
        return true;
    }
    
    if (isa<LandingPadInst>(I)) {
        return true;
    }
    
    return false;
}


char ssadce::ID = 0;
static RegisterPass<ssadce> X("dce-ssa", "SSA-DCE PASS", false, false);
