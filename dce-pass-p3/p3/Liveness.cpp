#include "Liveness.h"
#include "llvm/IR/IntrinsicInst.h"

#include <unistd.h>
#include <stdio.h>
#include <algorithm>

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
            // adiciona própria instrução ao conjunto def
            li.def.insert(i);
            // adiciona operandos ao conjunto use
            for (User::op_iterator op = i->op_begin(), x = i->op_end(); op != x; ++op){
                if (!isa<ConstantInt>(op)) {
                    // TODO check if validation covers all cases
                    errs() << *i << " Block " << blk->getName() << " uses " << *op << "\n";
                    li.use.insert(*op);
                }
            }
        }
        bbLivenessMap.insert(std::pair<const BasicBlock*, LivenessInfo>(blk,li));
        // TODO check if maps entries are being set correctly
    }
}

void Liveness::computeBBInOut(Function &F) {
    bool hasChanges = true;
    while (hasChanges) {
        // iterando em BBs
        for (Function::iterator blk = F.begin(), e = F.end(); blk != e; ++blk) {
            LivenessInfo li = bbLivenessMap[blk];
            std::set<const Value *> in1 = li.in;
            std::set<const Value *> out1 = li.out;
            // TODO: li.in = li.use U (li.out - li.def)
            // std::set<const Value *> diff =  std::set_difference(??)
            // li.in = std::set_union( ??);
            // TODO: li.out = UNION(in[s], s succeeds blk)
            // ...
        }
    }
}

void Liveness::computeIInOut(Function &F) {
    // iterando em Is
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        // TODO retrieve instruction info (Appel, p.362)
        if (isLiveOut((Instruction*)&*I, NULL)) {
        }
    }
}

bool Liveness::runOnFunction(Function &F) {
    computeBBDefUse(F);
    //computeBBInOut(F);
    //computeIInOut(F);
    return false;
}

char Liveness::ID = 0;

RegisterPass<Liveness> X("liveness", "Live vars analysis", false, false);