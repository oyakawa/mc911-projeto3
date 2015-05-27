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
        return true;
    }
    
    // for um terminador;
    if (isa<TerminatorInst>(I)) {
        return true;
    }
    
    // for instr. de debugging;
    if (isa<DbgInfoIntrinsic>(I)) {
        return true;
    }
    
    // for instr. de excecao;
    if (isa<LandingPadInst>(I)) {
        return true;
    }
    
    // TODO: for usada por outra instr. que está viva
    
    return false;
}

void Liveness::computeBBDefUse(Function &F) {
    // iterando em BBs
    for (Function::iterator blk = F.begin(), e = F.end(); blk != e; ++blk) {
        LivenessInfo li;
        // iterando em Insts
        for (BasicBlock::iterator i = blk->begin(), f = blk->end(); i != f; ++i) {
            // adiciona registrador ao conjunto def
            if (!isa<CmpInst>(i) && !isa<ReturnInst>(i) && !isa<BranchInst>(i) && !isa<StoreInst>(i)
                && !i->getType()->isVoidTy()) {
                // TODO check if validation covers all cases
                // errs() << "Block " << blk->getName() << " defines " << i->getName() << "\n";
                li.def.insert(i);
            }
        }
        for (BasicBlock::iterator i = blk->begin(), f = blk->end(); i != f; ++i) {
            // iterando em operandos
            for (User::op_iterator op = i->op_begin(), x = i->op_end(); op != x; ++op){
                // adiciona operandos ao conjunto use (caso nao tenham sido definidos no bloco)
                if (!isa<Constant>(op->get()) && !op->get()->getType()->isVoidTy() && !op->get()->getType()->isFunctionTy()
                    && !op->get()->getType()->isLabelTy()) {
                    // TODO check if validation covers all cases
                    if (!li.def.count(op->get())) {
                        //errs() << "Block " << blk->getName() << " uses " << op->get()->getName() << "\n";
                        li.use.insert(op->get());
                    }
                }
            }
        }
        bbLivenessMap[blk] = li;
    }
}

void Liveness::computeBBInOut(Function &F) {
    std::set<const Value *> diff, uni, succ_uni, succ_aux;
    bool hasChanges;
    DenseMap<const BasicBlock*, std::set<const Value *> > oldInSets;
    DenseMap<const BasicBlock*, std::set<const Value *> > oldOutSets;
        
    do {
        hasChanges = false;
        // iterando em BBs
        for (Function::iterator blk = F.begin(), e = F.end(); blk != e; ++blk) {
            diff.clear();
            uni.clear();
            succ_uni.clear();            
            succ_aux.clear();
            LivenessInfo li = bbLivenessMap[blk];
            oldInSets[blk] = li.in;
            oldOutSets[blk] = li.out;
            
            // li.in = li.use U (li.out - li.def)
            std::set_difference(li.out.begin(), li.out.end(), 
                                li.def.begin(), li.def.end(), std::inserter(diff, diff.begin()));
            std::set_union(li.use.begin(), li.use.end(), diff.begin(), diff.end(), std::inserter(uni, uni.begin()));
            bbLivenessMap[blk].in = uni;
                           
            // li.out = UNION(in[s], s succeeds blk)
            for (succ_iterator SI = succ_begin(blk), E = succ_end(blk); SI != E; ++SI) {
                BasicBlock *Succ = *SI;
                LivenessInfo sli = bbLivenessMap[Succ];
                //errs() << "Block " << Succ->getName() << " (in[" << sli.in.size() << "]) succeeds " << blk->getName() << "\n";
                std::set_union(succ_uni.begin(), succ_uni.end(), sli.in.begin(), sli.in.end(), std::inserter(succ_aux, succ_aux.begin()));
                succ_uni = succ_aux;
            }
            bbLivenessMap[blk].out = succ_uni;
            errs() << blk->getName() << ": def[" << li.def.size() << "], use[" << li.use.size() << "], in[" << li.in.size() <<"], out[" << li.out.size() << "]\n"; 
        }
        // verifica se houve alteracoes nos conjuntos in; havendo alguma, reiterar
        for (Function::iterator blk = F.begin(), e = F.end(); blk != e; ++blk) {
            if (bbLivenessMap[blk].in != oldInSets[blk]) {
                hasChanges = true;
                break;
            }
        }
    } while(hasChanges);
}

void Liveness::computeIInOut(Function &F) {
    
    LivenessInfo instInfo, blockInfo;
    std::set<const Value *> prevDef;
    std::set<const Value *> prevUse;
    // iterando em Insts
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        Instruction *inst = &*I;
        blockInfo = bbLivenessMap[inst->getParent()];
        // TODO complete the sets
        instInfo.in = blockInfo.in;
        instInfo.out = blockInfo.out;
        //iLivenessMap[inst] = instInfo;
    }
    /*for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        // TODO retrieve instruction info (Appel, p.362)
        Instruction *inst = *I;
        if (isLiveOut(inst, NULL)) {
        }
    }*/
}

bool Liveness::runOnFunction(Function &F) {
    computeBBDefUse(F);
    computeBBInOut(F);
    computeIInOut(F);
    return false;
}

char Liveness::ID = 0;

RegisterPass<Liveness> X("liveness", "Live vars analysis", false, false);