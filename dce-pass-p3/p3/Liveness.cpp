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
    if (I->mayHaveSideEffects()) {
        //errs() << "Side effects! ";
        return true;
    }
    
    // for um terminador;
    if (isa<TerminatorInst>(I)) {        
        //errs() << "Terminator! ";
        return true;
    }
    
    // for instr. de debugging;
    if (isa<DbgInfoIntrinsic>(I)) {
        //errs() << "Debug instruction! ";
        return true;
    }
    
    // for instr. de excecao;
    if (isa<LandingPadInst>(I)) {
        //errs() << "Exception instruction! ";
        return true;
    }
    
    // TODO: for usada por outra instr. que está viva
    
    return false;
}

void Liveness::computeBBDefUse(Function &F) {
    // iterando em BBs
    for (Function::iterator blk = F.begin(), e = F.end(); blk != e; ++blk) {
        LivenessInfo li;
        // iterando em Is
        for (BasicBlock::iterator i = blk->begin(), f = blk->end(); i != f; ++i) {
            // adiciona própria instrução no conjunto def
            // errs() << "Block " << blk->getName() << " defines " << *i << "\n";
            li.def.insert(i);
            // adiciona operandos no conjunto use
            for (User::op_iterator op = i->op_begin(), x = i->op_end(); op != x; ++op){
                // errs() << *i << " Block " << blk->getName() << " uses " << *op << "\n";
                li.use.insert(*op);
            }
        }
        bbLivenessMap.insert(std::pair<const BasicBlock*, LivenessInfo>(blk,li));
        // TODO check if maps entries are being set correctly
    }
}

void Liveness::computeBBInOut(Function &F) {
    // iterando em BBs
    for (Function::iterator blk = F.begin(), e = F.end(); blk != e; ++blk) {
        // iterando em Is
        for (BasicBlock::iterator i = blk->begin(), f = blk->end(); i != f; ++i) {
            // 
            // 
        }
    }
}

void Liveness::computeIInOut(Function &F) {
    // iterando em Is
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        if (isLiveOut((Instruction*)&*I, NULL)) {
            //errs() << *I << " is live out.\n";
        }
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