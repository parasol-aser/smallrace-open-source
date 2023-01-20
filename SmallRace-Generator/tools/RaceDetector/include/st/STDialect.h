#ifndef MLIR_ST_DIALECT_H_
#define MLIR_ST_DIALECT_H_

#include "mlir/IR/Dialect.h"
#include "mlir/IR/Function.h"
#include "mlir/IR/StandardTypes.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

namespace mlir {
namespace st {
namespace detail {
struct StructTypeStorage;
struct ObjectTypeStorage;
} // end namespace detail

/// This is the definition of the ST dialect. A dialect inherits from
/// mlir::Dialect and registers custom attributes, operations, and types (in its
/// constructor). It can also override some general behavior exposed via virtual
/// methods.
class STDialect : public mlir::Dialect {
public:
  explicit STDialect(mlir::MLIRContext *ctx);

  /// A hook used to materialize constant values with the given type.
  Operation *materializeConstant(OpBuilder &builder, Attribute value, Type type,
                                 Location loc) override;

  /// Parse an instance of a type registered to the ST dialect.
  mlir::Type parseType(mlir::DialectAsmParser &parser) const override;

  /// Print an instance of a type registered to the ST dialect.
  void printType(mlir::Type type,
                 mlir::DialectAsmPrinter &printer) const override;

  /// Provide a utility accessor to the dialect namespace. This is used by
  /// several utilities for casting between dialects.
  static llvm::StringRef getDialectNamespace() { return "st"; }
};

//===----------------------------------------------------------------------===//
// ST Operations
//===----------------------------------------------------------------------===//

/// Include the auto-generated header file containing the declarations of the
/// toy operations.
#define GET_OP_CLASSES
#include "st/Ops.h.inc"

//===----------------------------------------------------------------------===//
// ST Types
//===----------------------------------------------------------------------===//

/// Create a local enumeration with all of the types that are defined by ST.
namespace STTypes {
enum Types {
  Struct = mlir::Type::FIRST_TOY_TYPE,
  Object = mlir::Type::Opaque,
};
} // end namespace STTypes

/// This class defines the ST struct type. It represents a collection of
/// element types. All derived types in MLIR must inherit from the CRTP class
/// 'Type::TypeBase'. It takes as template parameters the concrete type
/// (StructType), the base class to use (Type), and the storage class
/// (StructTypeStorage).
class StructType : public mlir::Type::TypeBase<StructType, mlir::Type,
                                               detail::StructTypeStorage> {
public:
  /// Inherit some necessary constructors from 'TypeBase'.
  using Base::Base;

  /// This static method is used to support type inquiry through isa, cast,
  /// and dyn_cast.
  static bool kindof(unsigned kind) { return kind == STTypes::Struct; }

  /// Create an instance of a `StructType` with the given element types. There
  /// *must* be atleast one element type.
  static StructType get(llvm::ArrayRef<mlir::Type> elementTypes);

  /// Returns the element types of this struct type.
  llvm::ArrayRef<mlir::Type> getElementTypes();

  /// Returns the number of element type held by this struct.
  size_t getNumElementTypes() { return getElementTypes().size(); }
};

class ObjectType : public mlir::Type::TypeBase<ObjectType, mlir::Type,
                                               detail::ObjectTypeStorage> {
public:
  /// Inherit some necessary constructors from 'TypeBase'.
  using Base::Base;

  /// This static method is used to support type inquiry through isa, cast,
  /// and dyn_cast.
  static bool kindof(unsigned kind) { return kind == STTypes::Object; }

  /// Create an instance of a `StructType` with the given element types. There
  /// *must* be atleast one element type.
  static ObjectType get(mlir::MLIRContext *ctx);

  /// Returns the element types of this struct type.
  llvm::ArrayRef<mlir::Type> getElementTypes();

  /// Returns the number of element type held by this struct.
  size_t getNumElementTypes() { return getElementTypes().size(); }
};

} // end namespace st
} // end namespace mlir

#endif // MLIR_ST_DIALECT_H_
