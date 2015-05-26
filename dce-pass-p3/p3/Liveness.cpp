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
            // adiciona registrador ao conjunto def
            if (!isa<CmpInst>(i) && !isa<ReturnInst>(i) && !isa<BranchInst>(i)) {
                // TODO check if validation covers all cases
                li.def.insert(i);
            }
            // adiciona operandos ao conjunto use
            for (User::op_iterator op = i->op_begin(), x = i->op_end(); op != x; ++op){
                if (!isa<Constant>(op->get()) && !op->get()->getType()->isVoidTy()) {
                    // TODO check if validation covers all cases
                    li.use.insert(*op);
                }
            }
        }
        bbLivenessMap.insert(std::pair<const BasicBlock*, LivenessInfo>(blk,li));
    }
}

void Liveness::computeBBInOut(Function &F) {
    bool hasChanges = true;
    std::set<const Value *> diff, uni, succ_uni;
    while (hasChanges) {
        // iterando em BBs
        for (Function::iterator blk = F.begin(), e = F.end(); blk != e; ++blk) {
            diff.clear();
            uni.clear();
            succ_uni.clear();
            LivenessInfo li = bbLivenessMap[blk];
            std::set<const Value *> in1 = li.in;
            std::set<const Value *> out1 = li.out;
            // li.in = li.use U (li.out - li.def)
            std::set_difference(li.out.begin(), li.out.end(), 
                                li.def.begin(), li.def.end(), std::inserter(diff, diff.begin()));
            std::set_union(li.use.begin(), li.use.end(), diff.begin(), diff.end(), std::inserter(uni, uni.begin()));
            li.in = uni;
                           
            // li.out = UNION(in[s], s succeeds blk) TODO
            for (succ_iterator SI = succ_begin(blk), E = succ_end(blk); SI != E; ++SI) {
                errs() << "Block " << SI->getName() << " succeeds " << blk->getName() << "\n";
                BasicBlock *Succ = *SI;
                LivenessInfo sli = bbLivenessMap[Succ];
                std::set_union(succ_uni.begin(), succ_uni.end(), sli.in.begin(), sli.in.end(), std::inserter(succ_uni, succ_uni.begin()));
            }
            li.out = succ_uni;
            errs() << blk->getName() << ": def[" << li.def.size() << "], use[" << li.use.size() << "], in[" << li.in.size() <<"], out[" << li.out.size() << "]\n"; 
            hasChanges = false;
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
    computeBBInOut(F);
    //computeIInOut(F);
    return false;
}

char Liveness::ID = 0;

RegisterPass<Liveness> X("liveness", "Live vars analysis", false, false);