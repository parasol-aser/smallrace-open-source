//===----------------------------------------------------------------------===//
//
// This file implements full lowering of Toy operations to LLVM MLIR dialect.
// 'toy.print' is lowered to a loop nest that calls `printf` on each element of
// the input array. The file also sets up the ToyToLLVMLoweringPass. This pass
// lowers the combination of Affine + SCF + Standard dialects to the LLVM one:
//
//                         Affine --
//                                  |
//                                  v
//                                  Standard --> LLVM (Dialect)
//                                  ^
//                                  |
//     'toy.print' --> Loop (SCF) --
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Sequence.h"
#include "mlir/Conversion/AffineToStandard/AffineToStandard.h"
#include "mlir/Conversion/SCFToStandard/SCFToStandard.h"
#include "mlir/Conversion/StandardToLLVM/ConvertStandardToLLVM.h"
#include "mlir/Conversion/StandardToLLVM/ConvertStandardToLLVMPass.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/SCF/SCF.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "st/Passes.h"
//#include "st/STDialect.h"

using namespace mlir;

//===----------------------------------------------------------------------===//
// ToyToLLVM RewritePatterns
//===----------------------------------------------------------------------===//

namespace {
/// Lowers `toy.print` to a loop nest calling `printf` on each of the individual
/// elements of the array.
// class PrintOpLowering : public ConversionPattern {
//  public:
//   explicit PrintOpLowering(MLIRContext *context)
//       : ConversionPattern(st::PrintOp::getOperationName(), 1, context) {}

//   LogicalResult matchAndRewrite(
//       Operation *op, ArrayRef<Value> operands,
//       ConversionPatternRewriter &rewriter) const override {
//     llvm::outs() << "in PrintOpLowering matchAndRewrite\n";
//     if (true) {
//       auto loc = op->getLoc();
//       auto *llvmDialect =
//           op->getContext()->getRegisteredDialect<LLVM::LLVMDialect>();
//       assert(llvmDialect && "expected llvm dialect to be registered");

//       ModuleOp parentModule = op->getParentOfType<ModuleOp>();

//       // Get a symbol reference to the printf function, inserting it if
//       // necessary.
//       auto printfRef = getOrInsertPrintf(rewriter, parentModule, llvmDialect);
//       Value newLineCst = getOrCreateGlobalString(
//           loc, rewriter, "nl", StringRef("\n\0", 2), parentModule, llvmDialect);

//       // Create a loop for each of the dimensions within the shape.
//       // SmallVector<Value, 4> loopIvs;
//       // for (unsigned i = 0, e = memRefShape.size(); i != e; ++i) {
//       //   auto lowerBound = rewriter.create<ConstantIndexOp>(loc, 0);
//       //   auto upperBound = rewriter.create<ConstantIndexOp>(loc,
//       //   memRefShape[i]); auto step = rewriter.create<ConstantIndexOp>(loc,
//       //   1); auto loop =
//       //       rewriter.create<scf::ForOp>(loc, lowerBound, upperBound, step);
//       //   for (Operation &nested : *loop.getBody())
//       //     rewriter.eraseOp(&nested);
//       //   loopIvs.push_back(loop.getInductionVar());

//       //   // Terminate the loop body.
//       //   rewriter.setInsertionPointToEnd(loop.getBody());

//       //   // Insert a newline after each of the inner dimensions of the shape.
//       //   if (i != e - 1)
//       //     rewriter.create<CallOp>(loc, printfRef,
//       //     rewriter.getIntegerType(32),
//       //                             newLineCst);
//       //   rewriter.create<scf::YieldOp>(loc);
//       //   rewriter.setInsertionPointToStart(loop.getBody());
//       // }

//       // Generate a call to printf for the current element of the loop.
//       auto printOp = cast<st::PrintOp>(op);
//       // auto elementLoad = rewriter.create<LoadOp>(loc, printOp.input());
//       rewriter.create<CallOp>(
//           loc, printfRef, rewriter.getIntegerType(32),
//           printOp.input());  // elementLoad printOp.input() newLineCst

//       llvm::outs() << "end PrintOpLowering matchAndRewrite\n";
//     }
//     // Notify the rewriter that this operation has been removed.
//     rewriter.eraseOp(op);
//     return success();
//   }

//  private:
//   /// Return a symbol reference to the printf function, inserting it into the
//   /// module if necessary.
//   static FlatSymbolRefAttr getOrInsertPrintf(PatternRewriter &rewriter,
//                                              ModuleOp module,
//                                              LLVM::LLVMDialect *llvmDialect) {
//     auto *context = module.getContext();
//     if (module.lookupSymbol<LLVM::LLVMFuncOp>("printf"))
//       return SymbolRefAttr::get("printf", context);

//     // Create a function declaration for printf, the signature is:
//     //   * `i32 (i8*, ...)`
//     auto llvmI32Ty = LLVM::LLVMType::getInt32Ty(llvmDialect);
//     auto llvmI8PtrTy = LLVM::LLVMType::getInt8PtrTy(llvmDialect);
//     auto llvmFnType = LLVM::LLVMType::getFunctionTy(llvmI32Ty, llvmI8PtrTy,
//                                                     /*isVarArg=*/true);

//     // Insert the printf function into the body of the parent module.
//     PatternRewriter::InsertionGuard insertGuard(rewriter);
//     rewriter.setInsertionPointToStart(module.getBody());
//     rewriter.create<LLVM::LLVMFuncOp>(module.getLoc(), "printf", llvmFnType);
//     return SymbolRefAttr::get("printf", context);
//   }

//   /// Return a value representing an access into a global string with the given
//   /// name, creating the string if necessary.
//   static Value getOrCreateGlobalString(Location loc, OpBuilder &builder,
//                                        StringRef name, StringRef value,
//                                        ModuleOp module,
//                                        LLVM::LLVMDialect *llvmDialect) {
//     // Create the global at the entry of the module.
//     LLVM::GlobalOp global;
//     if (!(global = module.lookupSymbol<LLVM::GlobalOp>(name))) {
//       OpBuilder::InsertionGuard insertGuard(builder);
//       builder.setInsertionPointToStart(module.getBody());
//       auto type = LLVM::LLVMType::getArrayTy(
//           LLVM::LLVMType::getInt8Ty(llvmDialect), value.size());
//       global = builder.create<LLVM::GlobalOp>(loc, type, /*isConstant=*/true,
//                                               LLVM::Linkage::Internal, name,
//                                               builder.getStringAttr(value));
//     }

//     // Get the pointer to the first character in the global string.
//     Value globalPtr = builder.create<LLVM::AddressOfOp>(loc, global);
//     Value cst0 = builder.create<LLVM::ConstantOp>(
//         loc, LLVM::LLVMType::getInt64Ty(llvmDialect),
//         builder.getIntegerAttr(builder.getIndexType(), 0));
//     return builder.create<LLVM::GEPOp>(
//         loc, LLVM::LLVMType::getInt8PtrTy(llvmDialect), globalPtr,
//         ArrayRef<Value>({cst0, cst0}));
//   }
// };

// class GenericCallOpLowering : public ConversionPattern {
//  public:
//   explicit GenericCallOpLowering(MLIRContext *context)
//       : ConversionPattern(st::GenericCallOp::getOperationName(), 1, context) {}

//   LogicalResult matchAndRewrite(
//       Operation *op, ArrayRef<Value> operands,
//       ConversionPatternRewriter &rewriter) const override {
//     // llvm::outs() << "in GenericCallOpLowering matchAndRewrite\n";

//     auto loc = op->getLoc();

//     auto *llvmDialect =
//         op->getContext()->getRegisteredDialect<LLVM::LLVMDialect>();
//     assert(llvmDialect && "expected llvm dialect to be registered");

//     ModuleOp parentModule = op->getParentOfType<ModuleOp>();

//     // Get a symbol reference to the printf function, inserting it if necessary.
//     // auto printfRef = getOrInsertFunction(op,rewriter, parentModule,
//     // llvmDialect); Value newLineCst = getOrCreateGlobalString(
//     //     loc, rewriter, "nl", StringRef("\n\0", 2), parentModule,
//     //     llvmDialect); rewriter.create<CallOp>(loc, printfRef,
//     //     rewriter.getIntegerType(32),
//     //                             newLineCst);

//     auto funcRef = getOrInsertFunction(op, rewriter, parentModule, llvmDialect);
//     auto callOp = rewriter.create<CallOp>(
//         loc, funcRef, rewriter.getIntegerType(32), operands);
//     auto value = callOp.getResult(0);

//     // Notify the rewriter that this operation has been removed.
//     // rewriter.eraseOp(op);
//     rewriter.replaceOp(op, value);

//     return success();
//   }

//  private:
//   /// Return a symbol reference to the printf function, inserting it into the
//   /// module if necessary.
//   static FlatSymbolRefAttr getOrInsertFunction(Operation *op,
//                                                PatternRewriter &rewriter,
//                                                ModuleOp module,
//                                                LLVM::LLVMDialect *llvmDialect) {
//     auto op2 = cast<st::GenericCallOp>(op);
//     auto name = op2.callee();

//     auto *context = module.getContext();
//     if (module.lookupSymbol<LLVM::LLVMFuncOp>(name))
//       return SymbolRefAttr::get(name, context);

//     // Create a function declaration for printf, the signature is:
//     //   * `i32 (i8*, ...)`
//     auto llvmVoidTy = LLVM::LLVMType::getVoidTy(llvmDialect);
//     auto llvmI8PtrTy = LLVM::LLVMType::getInt8PtrTy(llvmDialect);
//     auto llvmFnType = LLVM::LLVMType::getFunctionTy(llvmVoidTy, llvmI8PtrTy,
//                                                     /*isVarArg=*/false);
//     // auto func_type = builder.getFunctionType(operands[0].getType(),ret_type);
//     // inputs()
//     // Insert the printf function into the body of the parent module.
//     PatternRewriter::InsertionGuard insertGuard(rewriter);
//     rewriter.setInsertionPointToStart(module.getBody());
//     rewriter.create<LLVM::LLVMFuncOp>(module.getLoc(), name, llvmFnType);
//     return SymbolRefAttr::get(name, context);
//   }

//   /// Return a value representing an access into a global string with the given
//   /// name, creating the string if necessary.
//   static Value getOrCreateGlobalString(Location loc, OpBuilder &builder,
//                                        StringRef name, StringRef value,
//                                        ModuleOp module,
//                                        LLVM::LLVMDialect *llvmDialect) {
//     // Create the global at the entry of the module.
//     LLVM::GlobalOp global;
//     if (!(global = module.lookupSymbol<LLVM::GlobalOp>(name))) {
//       OpBuilder::InsertionGuard insertGuard(builder);
//       builder.setInsertionPointToStart(module.getBody());
//       auto type = LLVM::LLVMType::getArrayTy(
//           LLVM::LLVMType::getInt8Ty(llvmDialect), value.size());
//       global = builder.create<LLVM::GlobalOp>(loc, type, /*isConstant=*/true,
//                                               LLVM::Linkage::Internal, name,
//                                               builder.getStringAttr(value));
//     }

//     // Get the pointer to the first character in the global string.
//     Value globalPtr = builder.create<LLVM::AddressOfOp>(loc, global);
//     Value cst0 = builder.create<LLVM::ConstantOp>(
//         loc, LLVM::LLVMType::getInt64Ty(llvmDialect),
//         builder.getIntegerAttr(builder.getIndexType(), 0));
//     return builder.create<LLVM::GEPOp>(
//         loc, LLVM::LLVMType::getInt8PtrTy(llvmDialect), globalPtr,
//         ArrayRef<Value>({cst0, cst0}));
//   }
// };

// struct ObjectOpLowering : public OpRewritePattern<st::ObjectOp> {
//   using OpRewritePattern<st::ObjectOp>::OpRewritePattern;

//   LogicalResult matchAndRewrite(st::ObjectOp op,
//                                 PatternRewriter &rewriter) const final {
//     // We lower "st.object" directly to "std.return".
//     // rewriter.replaceOpWithNewOp<ReturnOp>(op);
//     llvm::outs() << "in ObjectOpLowering matchAndRewrite\n";
//     // auto module = op.getParentOfType<ModuleOp>();
//     // Type *allocPtrType =
//     // llvm::PointerType::get(llvm::IntegerType::get(module.getContext(), 8),
//     // 0); auto allocFun = module.getOrInsertFunction(".st.allocation.api";,
//     // allocPtrType);

//     auto loc = op.getLoc();

//     auto *llvmDialect =
//         op.getContext()->getRegisteredDialect<LLVM::LLVMDialect>();
//     assert(llvmDialect && "expected llvm dialect to be registered");

//     ModuleOp parentModule = op.getParentOfType<ModuleOp>();

//     // Get a symbol reference to the printf function, inserting it if necessary.
//     auto opaqueRef =
//         getOrInsertOpaqueObject(rewriter, parentModule, llvmDialect);
//     auto op2 =
//         rewriter.create<CallOp>(loc, opaqueRef, rewriter.getIntegerType(32));

//     // rewriter.eraseOp(op);
//     return success();
//   }

//   static FlatSymbolRefAttr getOrInsertOpaqueObject(
//       PatternRewriter &rewriter, ModuleOp module,
//       LLVM::LLVMDialect *llvmDialect) {
//     auto *context = module.getContext();
//     if (module.lookupSymbol<LLVM::LLVMFuncOp>("opaqueObject"))
//       return SymbolRefAttr::get("opaqueObject", context);

//     // Create a function declaration for opaqueObject, the signature is:
//     //   * `i32 (i8*, ...)`
//     auto llvmI32Ty = LLVM::LLVMType::getInt32Ty(llvmDialect);
//     auto llvmVoidTy = LLVM::LLVMType::getVoidTy(llvmDialect);
//     auto llvmI8PtrTy = LLVM::LLVMType::getInt8PtrTy(llvmDialect);
//     auto llvmFnType = LLVM::LLVMType::getFunctionTy(llvmI8PtrTy, llvmVoidTy,
//                                                     /*isVarArg=*/false);

//     // Insert the opaqueObject function into the body of the parent module.
//     PatternRewriter::InsertionGuard insertGuard(rewriter);
//     rewriter.setInsertionPointToStart(module.getBody());
//     rewriter.create<LLVM::LLVMFuncOp>(module.getLoc(), "opaqueObject",
//                                       llvmFnType);
//     return SymbolRefAttr::get("opaqueObject", context);
//   }
// };

}  // end anonymous namespace

//===----------------------------------------------------------------------===//
// ToyToLLVMLoweringPass
//===----------------------------------------------------------------------===//

namespace {
struct STToLLVMLoweringPass
    : public PassWrapper<STToLLVMLoweringPass, OperationPass<ModuleOp>> {
  void runOnOperation() final;
};
}  // end anonymous namespace

void STToLLVMLoweringPass::runOnOperation() {
  // llvm::outs() << "in STToLLVMLoweringPass\n";

  // The first thing to define is the conversion target. This will define the
  // final target for this lowering. For this lowering, we are only targeting
  // the LLVM dialect.
  LLVMConversionTarget target(getContext());
  target.addLegalOp<ModuleOp, ModuleTerminatorOp>();

  // During this lowering, we will also be lowering the MemRef types, that are
  // currently being operated on, to a representation in LLVM. To perform this
  // conversion we use a TypeConverter as part of the lowering. This converter
  // details how one type maps to another. This is necessary now that we will be
  // doing more complicated lowerings, involving loop region arguments.
  LLVMTypeConverter typeConverter(&getContext());

  // Now that the conversion target has been defined, we need to provide the
  // patterns used for lowering. At this point of the compilation process, we
  // have a combination of `toy`, `affine`, and `std` operations. Luckily, there
  // are already exists a set of patterns to transform `affine` and `std`
  // dialects. These patterns lowering in multiple stages, relying on transitive
  // lowerings. Transitive lowering, or A->B->C lowering, is when multiple
  // patterns must be applied to fully transform an illegal operation into a
  // set of legal ones.
  OwningRewritePatternList patterns;
  // populateAffineToStdConversionPatterns(patterns, &getContext());
  // populateLoopToStdConversionPatterns(patterns, &getContext());
  populateStdToLLVMConversionPatterns(typeConverter, patterns);

  // The only remaining operation to lower from the `toy` dialect, is the
  // PrintOp.
  // patterns.insert<PrintOpLowering>(&getContext());
  // patterns.insert<GenericCallOpLowering>(&getContext());
  // patterns.insert<ObjectOpLowering>(&getContext());

  // We want to completely lower to LLVM, so we use a `FullConversion`. This
  // ensures that only legal operations will remain after the conversion.
  auto module = getOperation();

  // llvm::outs() << "in applyFullConversion\n";
  FrozenRewritePatternList tmpPatterns = FrozenRewritePatternList(std::move(patterns));

  if (failed(applyFullConversion(module, target, tmpPatterns)))
    signalPassFailure();
}

/// Create a pass for lowering operations the remaining `Toy` operations, as
/// well as `Affine` and `Std`, to the LLVM dialect for codegen.
std::unique_ptr<mlir::Pass> mlir::st::createLowerToLLVMPass() {
  return std::make_unique<STToLLVMLoweringPass>();
}
