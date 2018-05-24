#include "MemoryModel/PAG.h"
#include "MemoryModel/ConsG.h"
#include "WPA/Andersen.h"
#include "WPA/DynamicAndersenBase.h"
#include "Util/AnalysisUtil.h"

using namespace llvm;
using namespace analysisUtil;


void DynamicAndersenBase::initialize(Module& module) {
    resetData();
    PointerAnalysis::initialize(module);

    /* create the object only (without actually building the graph) */
    consCG = new ConstraintGraph(pag, false);

    stat = new AndersenStat(this);
}

void DynamicAndersenBase::analyzeFunction(Module& module, Function *f) {
    entry = f;
    analyze(module);
}

void DynamicAndersenBase::analyze(Module& module) {
    consCG->buildReducedCG(entry);
    setGraph(consCG);

    /* a custuomized setup of the points-to image */
    setup();

    /* process address edges */
    processAllAddr();

    do {
        reanalyze = false;

        /* start solving constraints */
        solve();

        double cgUpdateStart = stat->getClk();
        if (updateCallGraph(getIndirectCallsites())) {
            reanalyze = true;
        }

        double cgUpdateEnd = stat->getClk();
        timeOfUpdateCallGraph += (cgUpdateEnd - cgUpdateStart) / TIMEINTERVAL;
    } while (reanalyze);

    finalize();
}
