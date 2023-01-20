//===----------------------------------------------------------------------===//
//
// Defines the operations of the ST dialect.
//
//===----------------------------------------------------------------------===//

#ifndef ST_OPS
#define ST_OPS

include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

// Provide a definition of the 'ST' dialect in the ODS framework so that we
// can define our operations.
def ST_Dialect : Dialect {
  let name = "st";
  let cppNamespace = "st";
}

// Base class for ST dialect operations. This operation inherits from the base
// `Op` class in OpBase.td, and provides:
//   * The parent dialect of the operation.
//   * The mnemonic for the operation, or the name without the dialect prefix.
//   * A list of traits for the operation.
class ST_Op<string mnemonic, list<OpTrait> traits = []> :
    Op<ST_Dialect, mnemonic, traits>;

// Provide a definition for the ST StructType for use in ODS. This allows for
// using StructType in a similar way to Tensor or MemRef.
def ST_StructType :
    Type<CPred<"$_self.isa<StructType>()">, "ST struct type">;

def ST_ObjectType :
    Type<CPred<"$_self.isa<ObjectType>()">, "ST object type">;

// Provide a definition of the types that are used within the ST dialect.
def ST_Type : AnyTypeOf<[F64Tensor, AnyType, ST_ObjectType, ST_StructType]>;


//===----------------------------------------------------------------------===//
// ST Operations
//===----------------------------------------------------------------------===//

// We define a ST operation by inheriting from our base 'ST_Op' class above.
// Here we provide the mnemonic and a list of traits for the operation. The
// constant operation is marked as 'NoSideEffect' as it is a pure operation
// and may be removed if dead.
// def ConstantOp : ST_Op<"constant",
//     [ConstantLike, NoSideEffect]> {
//   // Provide a summary and description for this operation. This can be used to
//   // auto-generate documentation of the operations within our dialect.
//   let summary = "constant";
//   let description = [{
//     Constant operation turns a literal into an SSA value. The data is attached
//     to the operation as an attribute. For example:

//     ```mlir
//       %0 = st.constant dense<[[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]>
//                         : tensor<2x3xf64>
//     ```
//   }];

//   // The constant operation takes an attribute as the only input.
//   //F64ElementsAttr ST_ObjectType
//   let arguments = (ins F64ElementsAttr:$value);

//   // The constant operation returns a single value of TensorType.
//   let results = (outs ST_Type);

//   // Specify a parser and printer method.
//   let parser = [{ return ::parseConstantOp(parser, result); }];
//   let printer = [{ return ::print(p, *this); }];

//   // Add custom build methods for the constant operation. These method populates
//   // the `state` that MLIR uses to create operations, i.e. these are used when
//   // using `builder.create<ConstantOp>(...)`.
//   let builders = [
//     // Build a constant with a given constant tensor value.
//     //OpBuilder<"OpBuilder &builder, OperationState &state, "
//     //          "DenseElementsAttr value", [{
//     //  build(builder, state, value.getType(), value);
//     //}]>,
    
//     // Build a constant OpaqueType object.
//     OpBuilder<"OpBuilder &builder, OperationState &state, "
//               "DenseElementsAttr value", [{
//       build(builder, state, value.getType(),value);
//     }]>,

//     // Build a constant with a given constant floating-point value.
//     OpBuilder<"OpBuilder &builder, OperationState &state, double value">
//   ];

//   // Invoke a static verify method to verify this constant operation.
//   let verifier = [{ return ::verify(*this); }];

//   // Set the folder bit so that we can implement constant folders.
//   let hasFolder = 1;
// }


// def ObjectOp : ST_Op<"object",
//     [NoSideEffect]> {
//   // Provide a summary and description for this operation. This can be used to
//   // auto-generate documentation of the operations within our dialect.
//   let summary = "object";
//   let description = [{
//     Object operation turns a literal into an SSA value. The data is attached
//     to the operation as an attribute. For example:

//     ```mlir
//       %0 = st.constant dense<[[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]>
//                         : tensor<2x3xf64>
//     ```
//   }];

//   // The constant operation takes an attribute as the only input.
//   let arguments = (ins FlatSymbolRefAttr:$value);

//   // The constant operation returns a single value of TensorType.
//   let results = (outs ST_Type);

//   // Specify a parser and printer method.
//   //let parser = [{ return ::parseObjectOp(parser, result); }];
//   //let printer = [{ return ::print(p, *this); }];

//   // Add custom build methods for the object operation. These method populates
//   // the `state` that MLIR uses to create operations, i.e. these are used when
//   // using `builder.create<ObjectOp>(...)`.
//   let builders = [
//     // Build a constant OpaqueType object.
//     OpBuilder<"OpBuilder &builder, OperationState &state, "
//               "FlatSymbolRefAttr value", [{
//       build(builder, state, value.getType(),value);
//     }]>
//   ];

//   // Invoke a static verify method to verify this constant operation.
//   let verifier = [{ return ::verify(*this); }];
// }


// def AddOp : ST_Op<"add",
//     [NoSideEffect]> {
//   let summary = "element-wise addition operation";
//   let description = [{
//     The "add" operation performs element-wise addition between two tensors.
//     The shapes of the tensor operands are expected to match.
//   }];

//   let arguments = (ins F64Tensor:$lhs, F64Tensor:$rhs);
//   let results = (outs F64Tensor);

//   // Specify a parser and printer method.
//   let parser = [{ return ::parseBinaryOp(parser, result); }];
//   let printer = [{ return ::printBinaryOp(p, *this); }];

//   // Allow building an AddOp with from the two input operands.
//   let builders = [
//     OpBuilder<"OpBuilder &b, OperationState &state, Value lhs, Value rhs">
//   ];
// }

// def CastOp : ST_Op<"cast",
//     [NoSideEffect,
//      SameOperandsAndResultShape]> {
//   let summary = "shape cast operation";
//   let description = [{
//     The "cast" operation converts a tensor from one type to an equivalent type
//     without changing any data elements. The source and destination types
//     must both be tensor types with the same element type. If both are ranked
//     then the rank should be the same and static dimensions should match. The
//     operation is invalid if converting to a mismatching constant dimension.
//   }];

//   let arguments = (ins F64Tensor:$input);
//   let results = (outs F64Tensor:$output);

//   let assemblyFormat = "$input attr-dict `:` type($input) `to` type($output)";

//   // Set the folder bit so that we can fold redundant cast operations.
//   let hasFolder = 1;
// }

// def GenericCallOp : ST_Op<"generic_call",
//     [DeclareOpInterfaceMethods<CallOpInterface>]> {
//   let summary = "generic call operation";
//   let description = [{
//     Generic calls represent calls to a user defined function that needs to
//     be specialized for the shape of its arguments. The callee name is attached
//     as a symbol reference via an attribute. The arguments list must match the
//     arguments expected by the callee. For example:

//     ```mlir
//      %4 = st.generic_call @my_func(%1, %3)
//            : (tensor<2x3xf64>, tensor<2x3xf64>) -> tensor<*xf64>
//     ```

//     This is only valid if a function named "my_func" exists and takes two
//     arguments.
//   }];

//   // The generic call operation takes a symbol reference attribute as the
//   // callee, and inputs for the call.
//   let arguments = (ins FlatSymbolRefAttr:$callee, Variadic<ST_Type>:$inputs);

//   // The generic call operation returns a single value of TensorType or
//   // StructType.
//   let results = (outs ST_Type);

//   // Specialize assembly printing and parsing using a declarative format.
//   let assemblyFormat = [{
//     $callee `(` $inputs `)` attr-dict `:` functional-type($inputs, results)
//   }];

//   // Add custom build methods for the generic call operation.
//   let builders = [
//     OpBuilder<"OpBuilder &builder, OperationState &state, "
//               "StringRef callee, ArrayRef<Value> arguments">
//   ];
// }



// def PrintOp : ST_Op<"print"> {
//   let summary = "print operation";
//   let description = [{
//     The "print" builtin operation prints a given input tensor, and produces
//     no results.
//   }];

//   // The print operation takes an input tensor to print.
//   // We also allow a F64MemRef to enable interop during partial lowering.
//   let arguments = (ins AnyTypeOf<[F64Tensor, AnyType, F64MemRef]>:$input);

//   let assemblyFormat = "$input attr-dict `:` type($input)";
// }

// def ReturnOp : ST_Op<"return", [NoSideEffect, HasParent<"FuncOp">,
//                                  Terminator]> {
//   let summary = "return operation";
//   let description = [{
//     The "return" operation represents a return operation within a function.
//     The operation takes an optional operand and produces no results.
//     The operand type must match the signature of the function that contains
//     the operation. For example:

//     ```mlir
//       func @foo() -> tensor<2xf64> {
//         ...
//         st.return %0 : tensor<2xf64>
//       }
//     ```
//   }];

//   // The return operation takes an optional input operand to return. This
//   // value must match the return type of the enclosing function.
//   let arguments = (ins Variadic<ST_Type>:$input);

//   // The return operation only emits the input in the format if it is present.
//   let assemblyFormat = "($input^ `:` type($input))? attr-dict ";

//   // Allow building a ReturnOp with no return operand.
//   let builders = [OpBuilder<
//     "OpBuilder &b, OperationState &state", [{ build(b, state, llvm::None); }]
//   >];

//   // Provide extra utility definitions on the c++ operation class definition.
//   let extraClassDeclaration = [{
//     bool hasOperand() { return getNumOperands() != 0; }
//   }];

//   // Invoke a static verify method to verify this return operation.
//   let verifier = [{ return ::verify(*this); }];
// }


#endif // ST_OPS
