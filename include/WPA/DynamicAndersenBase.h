#ifndef ANDERSENDYNAMIC_H
#define ANDERSENDYNAMIC_H

#include "WPA/Andersen.h"

#include <llvm/IR/Function.h>


class DynamicAndersenBase : public Andersen {
public:
    DynamicAndersenBase() : Andersen(Andersen_WPA), entry(NULL) {

    }

    void initialize(llvm::Module& module);

    void analyze(llvm::Module& module);

    void analyzeFunction(llvm::Module& module, llvm::Function *f);

    virtual void setup() = 0;

private:

    llvm::Function *entry;
};

#endif
