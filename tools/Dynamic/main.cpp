#include <MemoryModel/MemModel.h>
#include <MemoryModel/PointerAnalysis.h>
#include <WPA/Andersen.h>
#include <WPA/AndersenDynamic.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/SourceMgr.h>

using namespace llvm;


static cl::opt<std::string> InputPath(cl::Positional,
                                      cl::desc("<input bitcode>"),
                                      cl::init("-"));

static cl::opt<std::string> TargetFunction(cl::Positional,
                                           cl::desc("<target function>"),
                                           cl::init(""));

void dumpStruct(Type *type) {
    PointerType *pointerType = dyn_cast<PointerType>(type);
    if (!pointerType) {
        return;
    }

    type = pointerType->getElementType();
    type->dump();
    StInfo *stInfo = SymbolTableInfo::SymbolInfo()->getStructInfo(type);
    for (uint32_t &off : stInfo->getFieldOffsetVec()) {
        errs() << "field offset: " << off << "\n";
    }
    for (FieldInfo &fi : stInfo->getFlattenFieldInfoVec()) {
        errs() << "flatten field offset: " << fi.getFlattenOffset() << "\n";
        errs() << "flatten field type: "; fi.getFlattenElemTy()->dump();
    }
}

void visitStore(PointerAnalysis *pta, Function *f, StoreInst *inst) {
    NodeID id = pta->getPAG()->getValueNode(inst->getOperand(1));
    PointsTo &pts = pta->getPts(id);

    errs() << f->getName() << ": "  << *inst << ":\n";
    for (PointsTo::iterator i = pts.begin(); i != pts.end(); ++i) {
        NodeID nodeId = *i;
        PAGNode *pagNode = pta->getPAG()->getPAGNode(nodeId);
        ObjPN *obj = dyn_cast<ObjPN>(pagNode);
        if (!obj) {
            return;
        }

        const Value *value = obj->getMemObj()->getRefVal();
        errs() << "-- node: " << nodeId << "\n";
        errs() << "-- AS: " << *value << "\n";
        errs() << "   -- kind: " << obj->getNodeKind() << "\n";
        GepObjPN *gepObj = dyn_cast<GepObjPN>(obj);
        if (gepObj) {
            errs() << "   -- ls: " << gepObj->getLocationSet().getOffset() << "\n";
        }
    }
}

void visitLoad(PointerAnalysis *pta, Function *f, LoadInst *inst) {
    NodeID id = pta->getPAG()->getValueNode(inst->getOperand(0));
    PointsTo &pts = pta->getPts(id);

    errs() << f->getName() << ": "  << *inst << ":\n";
    for (PointsTo::iterator i = pts.begin(); i != pts.end(); ++i) {
        NodeID nodeId = *i;
        PAGNode *pagNode = pta->getPAG()->getPAGNode(nodeId);
        ObjPN *obj = dyn_cast<ObjPN>(pagNode);
        if (!obj) {
            return;
        }

        errs() << "-- node: " << nodeId << "\n";
        if (isa<DummyObjPN>(obj)) {
            errs() << "-- AS: dummy\n";
            continue;
        }

        const Value *value = obj->getMemObj()->getRefVal();
        errs() << "-- AS: " << *value << "\n";
        errs() << "   -- kind: " << obj->getNodeKind() << "\n";
        GepObjPN *gepObj = dyn_cast<GepObjPN>(obj);
        if (gepObj) {
            errs() << "   -- ls: " << gepObj->getLocationSet().getOffset() << "\n";
        }
    }
}

void dumpPtsResults(PointerAnalysis *pta, Function *target = NULL) {
    Module &module = *pta->getModule();
    for (Function &f : module) {
        if (target && target != &f) {
            continue;
        }
        for (inst_iterator j = inst_begin(f); j != inst_end(f); j++) {
            Instruction *inst = &*j;
            if (inst->getOpcode() == Instruction::Store) {
                visitStore(pta, &f, dyn_cast<StoreInst>(inst));
            }
            if (inst->getOpcode() == Instruction::Load) {
                visitLoad(pta, &f, dyn_cast<LoadInst>(inst));
            }
        }
    }
}

int main(int argc, char *argv[]) {
    llvm::legacy::PassManager pm;
    SMDiagnostic err;

    LLVMContext &context = getGlobalContext();
    cl::ParseCommandLineOptions(argc, argv, "Test\n");

    std::unique_ptr<Module> module = parseIRFile(InputPath, err, context);
    if (!module) {
        err.print(argv[0], errs());
        return 1;
    }

    Function *f = NULL;
    if (TargetFunction != "") {
        f = module->getFunction(TargetFunction);
    }

    //AndersenDynamic *pta = new AndersenDynamic();
    //pta->initialize(*module);
    //pta->analyzeFunction(*module, module->getFunction("target"));

    PointerAnalysis *pta = new Andersen();
    pta->analyze(*module);

    if (f) {
        dumpPtsResults(pta, f);
    } else {
        dumpPtsResults(pta); 
    }

    delete pta;

    return 0;
}
