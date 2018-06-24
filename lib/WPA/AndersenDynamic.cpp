#include "MemoryModel/PAG.h"
#include "MemoryModel/ConsG.h"
#include "WPA/Andersen.h"
#include "WPA/AndersenDynamic.h"
#include "Util/AnalysisUtil.h"

using namespace llvm;
using namespace analysisUtil;


void AndersenDynamic::initialize(Module& module) {
    resetData();
    PointerAnalysis::initialize(module);

    /* create the object only (without actually building the graph) */
    consCG = new ConstraintGraph(pag, false);

    stat = new AndersenStat(this);
}

void AndersenDynamic::analyzeFunction(Module& module, Function *f) {
    entry = f;
    analyze(module);
}

void AndersenDynamic::analyze(Module& module) {
    /* build the reduced constraint graph */
    consCG->buildReducedCG(entry);
    setGraph(consCG);

    /* process address edges */
    processAllAddr();

    do {
        numOfIteration++;
        if (numOfIteration == 0 % OnTheFlyIterBudgetForStat) {
            dumpStat();
        }

        reanalyze = false;

        /* start solving constraints */
        solve();

        double cgUpdateStart = stat->getClk();
        if (updateCallGraph(consCG->getIndirectCallsites())) {
            reanalyze = true;
        }

        double cgUpdateEnd = stat->getClk();
        timeOfUpdateCallGraph += (cgUpdateEnd - cgUpdateStart) / TIMEINTERVAL;
    } while (reanalyze);

    finalize();
}

bool AndersenDynamic::updateCallGraph(const CallSiteToFunPtrMap& callsites) {
    CallEdgeMap newEdges;
    onTheFlyCallGraphSolve(callsites,newEdges);
    FunctionSet newFunctions;

    NodePairSet cpySrcNodes;
    for (auto &it : newEdges) {
        llvm::CallSite cs = it.first;
        for (const Function *f : it.second) {
            consCG->connectCaller2CalleeParams(cs, f, cpySrcNodes);
            newFunctions.insert(f);
        }
    }

    for (const Function *f : newFunctions) {
        consCG->buildReducedCG(f);
    }

    for (auto &it : cpySrcNodes) {
        pushIntoWorklist(it.first);
    }

    return !newEdges.empty();
}

void AndersenDynamic::weakUpdate(NodeID src, NodeID dst) {
    if (dst != getPAG()->getNullPtr()) {
        addPts(src, dst);
    }
}

void AndersenDynamic::strongUpdate(NodeID src, NodeID dst) {
    PointsTo &pts = getPts(src);
    pts.clear();
    if (dst != getPAG()->getNullPtr()) {
        pts.set(dst);
    }
}
