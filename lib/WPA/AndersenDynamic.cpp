#include "MemoryModel/PAG.h"
#include "MemoryModel/ConsG.h"
#include "WPA/Andersen.h"
#include "WPA/AndersenDynamic.h"
#include "Util/AnalysisUtil.h"

using namespace llvm;
using namespace analysisUtil;


void AndersenDynamic::initialize(Module& module) {
    /* don't create the callgraph at this point... */
    PointerAnalysis::initialize(module, false);
    /* create the object only (without actually building the graph) */
    consCG = new ConstraintGraph(pag, false);
}

void AndersenDynamic::analyzeFunction(Module& module, Function *f) {
    entry = f;
    getPAG()->clearAddedFields();
    if (useBackup) {
        createBackup();
    }
    analyze(module);
}

void AndersenDynamic::analyze(Module& module) {
    initializeCallGraph(module);

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

bool AndersenDynamic::weakUpdate(NodeID src, NodeID dst) {
    bool changed = false;

    if (dst != getPAG()->getNullPtr()) {
        changed = addPts(src, dst);
    }

    return changed;
}

bool AndersenDynamic::strongUpdate(NodeID src, NodeID dst) {
    PointsTo &pts = getPts(src);

    /* update the reverse mapping */
    for (NodeID target : pts) {
        PointsTo &rev = getRevPts(target);
        rev.reset(src);
    }

    /* update the direct mapping */
    pts.clear();
    if (dst != getPAG()->getNullPtr()) {
        pts.set(dst);
    }

    return true;
}

void AndersenDynamic::join(AndersenDynamic *other) {
    auto &otherMap = other->getPTDataTy()->getPtsMap();
    for (auto i : otherMap) {
        NodeID src = i.first;
        PointsTo &otherPts = i.second;
        PointsTo &pts = getPts(src);
        if (pts == otherPts) {
            continue;
        }

        for (NodeID x : otherPts) {
            weakUpdate(src, x);
        }
    }
}

void AndersenDynamic::postAnalysisCleanup() {
    getPAG()->restoreFields();
    if (useBackup) {
        restoreFromBackup();
    }
    Andersen::postAnalysisCleanup();
}
