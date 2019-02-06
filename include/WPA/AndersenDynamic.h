#ifndef ANDERSENDYNAMIC_H
#define ANDERSENDYNAMIC_H

#include "WPA/Andersen.h"

#include <llvm/IR/Function.h>


class AndersenDynamic : public Andersen {
public:
    AndersenDynamic(bool useBackup = false) :
        Andersen(Andersen_WPA),
        refCount(0),
        useBackup(useBackup),
        entry(NULL) {

    }

    AndersenDynamic(const AndersenDynamic &other) :
        Andersen(other),
        refCount(0),
        useBackup(other.useBackup),
        entry(NULL)
    {

    }

    void initialize(llvm::Module& module);

    bool updateCallGraph(const CallSiteToFunPtrMap& callsites);

    void analyze(llvm::Module& module);

    void analyzeFunction(llvm::Module& module, llvm::Function *f);

    bool weakUpdate(NodeID src, NodeID dst);

    bool strongUpdate(NodeID src, NodeID dst);

    void clearPointsTo(NodeID src);

    void clearPointsTo();

    void join(AndersenDynamic *other);

    void filter();

    void postAnalysisCleanup();

    void dump();

    unsigned int refCount;

    bool useBackup;

private:

    bool mayClearPts(PointsTo &pts);

    llvm::Function *entry;
};

#endif
