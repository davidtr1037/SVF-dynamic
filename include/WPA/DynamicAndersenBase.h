#ifndef ANDERSENDYNAMIC_H
#define ANDERSENDYNAMIC_H

#include "WPA/Andersen.h"


class DynamicAndersenBase : public Andersen {
public:
    DynamicAndersenBase() : Andersen(Andersen_WPA) {

    }

    void analyze(llvm::Module& module);

    virtual void setup() = 0;
};

#endif
