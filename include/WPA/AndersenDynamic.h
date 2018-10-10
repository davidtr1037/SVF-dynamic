#ifndef ANDERSENDYNAMIC_H
#define ANDERSENDYNAMIC_H

#include "WPA/Andersen.h"

#include <llvm/IR/Function.h>


class AndersenDynamic : public Andersen {
public:
    AndersenDynamic() : Andersen(Andersen_WPA), refCount(0), entry(NULL) {

    }

    AndersenDynamic(const AndersenDynamic &other) :
        Andersen(other),
        refCount(0),
        entry(NULL)
    {

    }

    void initialize(llvm::Module& module);

    bool updateCallGraph(const CallSiteToFunPtrMap& callsites);

    void analyze(llvm::Module& module);

    void analyzeFunction(llvm::Module& module, llvm::Function *f);

    bool weakUpdate(NodeID src, NodeID dst);

    bool strongUpdate(NodeID src, NodeID dst);

    void postAnalysisCleanup();

    unsigned int refCount;

private:

    llvm::Function *entry;
};

#endif
