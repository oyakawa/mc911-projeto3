# MC911 - Projeto 3
## Links:
* Left operand of an instruction: http://lists.cs.uiuc.edu/pipermail/llvmdev/2010-April/030721.html and http://stackoverflow.com/questions/24434771/llvm-get-operand-and-lvalue-name-of-an-instruction
* Useful Appel sections: 10.1 (p.205), 17.4 (p.360), 19.3 (p.417) 
* 

## Running the pass
* opt -S -load dce-pass-p3/Release/P3.so -liveness <input>.ll -o <output>.ll