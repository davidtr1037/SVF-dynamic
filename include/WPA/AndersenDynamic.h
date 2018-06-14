#ifndef ANDERSENDYNAMIC_H
#define ANDERSENDYNAMIC_H

#include "WPA/Andersen.h"

#include <llvm/IR/Function.h>


class AndersenDynamic : public Andersen {
public:
    AndersenDynamic() : Andersen(Andersen_WPA), entry(NULL) {

    }

    void initialize(llvm::Module& module);

    bool updateCallGraph(const CallSiteToFunPtrMap& callsites);

    void analyze(llvm::Module& module);

    void analyzeFunction(llvm::Module& module, llvm::Function *f);

private:

    llvm::Function *entry;
};

#endif
