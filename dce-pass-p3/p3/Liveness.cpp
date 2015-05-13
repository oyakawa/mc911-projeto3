#include "llvm/IR/Instructions.h"
#include "Liveness.h"

#include <unistd.h>
#include <stdio.h>

using namespace std;

void print_elem(const Value* i) {
    errs() << i->getName() << " ";
}

bool Liveness::isLiveOut(Instruction *I, Value *V){
    return false;
}

void Liveness::computeBBDefUse(Function &F){
    // iterando em BBs
    // for (Function::iterator i = F->begin(), e = F->end(); i != e; ++i) {}
}

void Liveness::computeBBInOut(Function &F){
}

void Liveness::computeIInOut(Function &F) {
    // iterando em Is
    // for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {}
}

bool Liveness::runOnFunction(Function &F) {
    computeBBDefUse(F);
    computeBBInOut(F);
    computeIInOut(F);
    return false;
}

char Liveness::ID = 0;

RegisterPass<Liveness> X("liveness", "Live vars analysis", false, false);





