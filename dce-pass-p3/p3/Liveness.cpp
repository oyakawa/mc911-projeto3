#include "llvm/IR/Instructions.h"
#include "Liveness.h"
#include "llvm/IR/IntrinsicInst.h"

#include <unistd.h>
#include <stdio.h>

using namespace std;

void print_elem(const Value* i) {
    errs() << i->getName() << " ";
}

bool Liveness::isLiveOut(Instruction *I, Value *V) {
    // Instr. trivialmente viva se:
    // seu comportamento gerar efeitos colaterais;
    if (I->mayHaveSideEffects())
         return true;
    
    // for um terminador;
    if (isa<TerminatorInst>(I))
         return true;
    
    // for instr. de debugging;
    if (isa<DbgInfoIntrinsic>(I))
         return true;
    
    // for instr. de excecao;
    if (isa<LandingPadInst>(I))
         return true;
    
    // TODO: for usada por outra instr. que está viva
    
    return false;
}

void Liveness::computeBBDefUse(Function &F) {
    // iterando em BBs
    for (Function::iterator i = F.begin(), e = F.end(); i != e; ++i) {
        errs() << "Basic block (name=" << i->getName() << ") has " << i->size() << " instructions.\n";
    }
}

void Liveness::computeBBInOut(Function &F) {
    // iterando em BBs
    for (Function::iterator i = F.begin(), e = F.end(); i != e; ++i) {}
}

void Liveness::computeIInOut(Function &F) {
    // iterando em Is
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        if(isLiveOut((Instruction*)&*I, NULL)) 
            errs() << *I << " is live out!\n";
    }
}

bool Liveness::runOnFunction(Function &F) {
    computeBBDefUse(F);
    computeBBInOut(F);
    computeIInOut(F);
    return false;
}

char Liveness::ID = 0;

RegisterPass<Liveness> X("liveness", "Live vars analysis", false, false);





