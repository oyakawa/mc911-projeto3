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
    
    if (instMap[I] == 1) {
        return true;
    }
    if (instMap[I] == 2) {
        return false;
    }
    
    // Instr. trivialmente viva se:
    // seu comportamento gerar efeitos colaterais;
    if (I->mayHaveSideEffects()) {
        instMap[I] = 1;
        return true;
    }
    
    // for um terminador;
    if (isa<TerminatorInst>(I)) {
        instMap[I] = 1;
        return true;
    }
    
    // for instr. de debugging;
    if (isa<DbgInfoIntrinsic>(I)) {
        instMap[I] = 1;
        return true;
    }
    
    // for instr. de excecao;
    if (isa<LandingPadInst>(I)) {
        instMap[I] = 1;
        return true;
    }
    
    // atribuir a um registrador live-out que for usado por outra instr. viva;
    bool isAlive = false;
    if (iLivenessMap[I].out.count(I)) {
        for (Value::user_iterator user = I->user_begin(), e = I->user_end(); user != e; user++) {
            
            if (Instruction *v = dyn_cast<Instruction>(*user)) {
                if (isLiveOut(v, nullptr)) {
                    isAlive = true;
                    break;
                }
            }
        }
        if (isAlive) {
            instMap[I] = 1;
            return true;
        }
        else {
            instMap[I] = 2;
            return false;
        }
    }
    
    instMap[I] = 2;
    return false;
}

void Liveness::computeBBDefUse(Function &F) {
    
    // iterando em BBs
    for (Function::iterator blk = F.begin(), e = F.end(); blk != e; ++blk) {
        
        LivenessInfo li;
        
        // iterando em Insts
        for (BasicBlock::iterator i = blk->begin(), f = blk->end(); i != f; ++i) {
            
            // adiciona registrador ao conjunto def
            if (!isa<CmpInst>(i) && !isa<ReturnInst>(i) && !isa<BranchInst>(i) 
                && !isa<StoreInst>(i) && !i->getType()->isVoidTy()) {
                
                li.def.insert(i);
                iLivenessMap[i].def.insert(i);
            }
        }
        
        // iterando em Insts
        for (BasicBlock::iterator i = blk->begin(), f = blk->end(); i != f; ++i) {
            
            // iterando em operandos
            for (User::op_iterator op = i->op_begin(), x = i->op_end(); op != x; ++op) {
                
                // adiciona operandos ao conjunto use (caso nao tenham sido definidos no bloco)
                if (!isa<Constant>(op->get()) && !op->get()->getType()->isVoidTy() 
                    && !op->get()->getType()->isFunctionTy()
                    && !op->get()->getType()->isLabelTy()) {
                    
                    iLivenessMap[i].use.insert(op->get());
                    if (!li.def.count(op->get())) {
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
        
        // iterando em BBs (ordem reversa)
        Function::iterator blk, e;
        blk = F.end();
        e = F.begin();
        
        do {
            blk--;
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
            std::set_union(li.use.begin(), li.use.end(), diff.begin(), 
                           diff.end(), std::inserter(uni, uni.begin()));
            bbLivenessMap[blk].in = uni;
                           
            // li.out = UNION(in[s], s succeeds blk)
            for (succ_iterator SI = succ_begin(blk), E = succ_end(blk); SI != E; ++SI) {
                BasicBlock *Succ = *SI;
                LivenessInfo sli = bbLivenessMap[Succ];
                std::set_union(succ_uni.begin(), succ_uni.end(), 
                               sli.in.begin(), sli.in.end(), std::inserter(succ_aux, succ_aux.begin()));
                succ_uni = succ_aux;
            }
            bbLivenessMap[blk].out = succ_uni;
        } while (blk != e);
        
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
    std::set<const Value *> currIn, succIn, diff, uni;
    
    // iterando em BBs
    for (Function::iterator blk = F.begin(), e = F.end(); blk != e; ++blk) {
        
        blockInfo = bbLivenessMap[blk];
        currIn = blockInfo.in;
        succIn = blockInfo.out;
        
        // iterando em Insts (ordem reversa)
        BasicBlock::iterator i, f;
        i = blk->end();
        f = blk->begin();
        do {
            i--;
            instInfo = iLivenessMap[i];
            instInfo.out = succIn;
            std::set_difference(instInfo.out.begin(), instInfo.out.end(), 
                                instInfo.def.begin(), instInfo.def.end(), std::inserter(diff, diff.begin()));
            std::set_union(instInfo.use.begin(), instInfo.use.end(), 
                           diff.begin(), diff.end(), std::inserter(uni, uni.begin()));
            instInfo.in = uni;
            iLivenessMap[i] = instInfo;
            succIn = uni;
        } while (i != f);
    }
    
    
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
        // iterando em Insts
        instMap[cast<Instruction*>(I)] = 0; // 0 = indefinido; 1 = vivo; 2 = morto;
    
    
    // iterando em BBs
    for (Function::iterator blk = F.begin(), e = F.end(); blk != e; ++blk) {
        
        // iterando em Insts
        BasicBlock::iterator i, f;
        i = blk->begin();
        f = blk->end();
        
        while (i != f) {
            Instruction *inst = i;
            i++;
            if (!isLiveOut(inst, nullptr)) {
                if (!isa<CmpInst>(inst) && !isa<ReturnInst>(inst) && !isa<BranchInst>(inst)
                    && !isa<StoreInst>(inst) && !inst->getType()->isVoidTy()) {
                    
                    // Se instr. nao live-out e atribui valor a registr.,
                    // esta morta.
                    inst->dropAllReferences();
                    inst->eraseFromParent();                
                }
            }
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

RegisterPass<Liveness> X("dce-liveness", "Live vars analysis", false, false);