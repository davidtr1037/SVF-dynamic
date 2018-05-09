#include "MemoryModel/PAG.h"
#include "WPA/Andersen.h"
#include "WPA/DynamicAndersenBase.h"
#include "Util/AnalysisUtil.h"

#include <llvm/Support/CommandLine.h> // for tool output file

using namespace llvm;
using namespace analysisUtil;


void DynamicAndersenBase::analyze(llvm::Module& module) {
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
