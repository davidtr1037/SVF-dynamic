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

    bool weakUpdate(NodeID src, NodeID dst);

    bool strongUpdate(NodeID src, NodeID dst);

private:

    llvm::Function *entry;
};

#endif
