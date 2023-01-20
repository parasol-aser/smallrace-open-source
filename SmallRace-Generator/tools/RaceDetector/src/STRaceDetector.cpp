#include <conflib/conflib.h>
#include <dirent.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <llvm/Analysis/AssumptionCache.h>
#include <llvm/Analysis/TypeBasedAliasAnalysis.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LLVMContext.h>  // for llvm LLVMContext
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Mangler.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>  // IR reader for bit file
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/Signals.h>    // signal for command line
#include <llvm/Support/SourceMgr.h>  // for SMDiagnostic
#include <llvm/Transforms/IPO/AlwaysInliner.h>
#include <o2/Util/Log.h>

#include "SmalltalkLexer.h"
#include "SmalltalkParser.h"
#include "antlr4-runtime.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/ExecutionEngine/ExecutionEngine.h"
#include "mlir/ExecutionEngine/OptUtils.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "mlir/InitAllDialects.h"
#include "mlir/Parser.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Target/LLVMIR.h"
#include "mlir/Transforms/Passes.h"

using namespace antlr4;
using namespace antlrcpp;
using namespace llvm;
using namespace o2;
#include "st/MLIRGen.h"
#include "st/ParserWrapper.h"
#include "st/Passes.h"
// #include "st/STDialect.h"
#include "st/STParserListener.h"
#include "st/STParserVisitor.h"
using namespace st;

cl::opt<std::string> TargetModulePath(cl::Positional,
                                      cl::desc("path to input bitcode file"));
cl::opt<bool> ConfigDumpIR("dump-ir", cl::desc("Dump the generated IR file"));

logger::LoggingConfig initLoggingConf() {
  logger::LoggingConfig config;

  config.enableProgress = conflib::Get<bool>("XenableProgress", true);
  auto conflevel = conflib::Get<std::string>("logger.level", "debug");
  if (conflevel == "trace") {
    config.level = spdlog::level::trace;
  } else if (conflevel == "debug") {
    config.level = spdlog::level::debug;
  } else if (conflevel == "info") {
    config.level = spdlog::level::info;
  } else if (conflevel == "warn") {
    config.level = spdlog::level::warn;
  } else if (conflevel == "error") {
    config.level = spdlog::level::err;
  } else {
    // TODO: Print warning. How to log error about setting up logger?
    config.level = spdlog::level::trace;
  }

  config.enableTerminal = conflib::Get<bool>("logger.toStderr", false);
  config.terminalLevel = config.level;

  config.enableFile = true;
  config.logFolder = conflib::Get<std::string>("logger.logFolder", "./");
  config.logFile = "log.current";
  config.fileLevel = config.level;

  return config;
}

static std::unique_ptr<Module> loadFile(const std::string &FN,
                                        LLVMContext &Context,
                                        bool abortOnFail) {
  SMDiagnostic Err;
  std::unique_ptr<Module> Result;
  Result = parseIRFile(FN, Err, Context);

  if (!Result) {
    if (abortOnFail) {
      Err.print("racedetect", llvm::errs());
      abort();
    }

    LOG_ERROR("error loading file: {}", FN);
    return nullptr;
  }
  return Result;
}

std::set<const Function *> unExploredFunctions;
const std::set<llvm::StringRef> SKIPPED_APIS{
    "llvm.", "pthread_", "__kmpc_",  ".omp.", "omp_",   "_ZNSt",
    "_ZSt",  "_ZNKSt",   "_ZN14o2_", "__o2_", "printf", "je_"};
void addExploredFunction(const Function *f) { unExploredFunctions.erase(f); }
void initUnexploredFunctions(Module *module) {
  auto &functionList = module->getFunctionList();
  for (auto &function : functionList) {
    auto func = &function;
    if (!func->isDeclaration()) {
      bool insert = true;
      // do not count llvm.* pthread_* __kmpc_* __smalltalk_ .omp. omp_ std::
      for (auto str : SKIPPED_APIS) {
        if (func->getName().startswith(str)) {
          insert = false;
          break;
        }
      }
      if (insert) unExploredFunctions.insert(func);
    }
  }
}
void computeUnexploredFunctions() {
  if (unExploredFunctions.size() > 0) {
    llvm::outs() << "\n============= unexplored function ==========\n";
    for (auto func : unExploredFunctions)
      llvm::outs() << (func->getName()) << "\n";
    // TODO: print potential entry points
    // no caller
  }
}


st::ModuleAST moduleAST;

stx::FunctionAST *process(std::string filename, std::string methodname,
                          size_t line, char *s) {
  
  auto *input = new ANTLRInputStream(s);

  auto *lexer = new SmalltalkLexer(input);
  auto *tokens = new CommonTokenStream(lexer);

  tokens->fill();

  auto *parser = new SmalltalkParser(tokens);
  STParserVisitor visitor(filename, methodname, line);
  stx::FunctionAST *res = visitor.visitModule(parser->module());
  return res;
}

// c++17 only
void initParserForFile(stx::ModuleAST *module, std::string &fullPathName,
                       StringRef &dname) {
  StringRef name(fullPathName);
  if (dname.endswith(".ws")) {
    std::ifstream ws(name.str());
    std::string contents((std::istreambuf_iterator<char>(ws)),
                         std::istreambuf_iterator<char>());
    llvm::outs() << "=== Test WS Start ===\n";
    llvm::outs() << contents << "\n";
    llvm::outs() << "=== Test WS End ===\n";
    module->entry_point =
        process(fullPathName, "main", 0, (char *)contents.c_str());
  } else if (dname.endswith(".st")) {
    xmlDocPtr doc;
    xmlNodePtr cur;
    doc = xmlReadFile(name.data(), NULL, XML_PARSE_BIG_LINES);
    if (doc == NULL) {
      fprintf(stderr, "Document not parsed successfully. \n");
    }
    cur = xmlDocGetRootElement(doc);

    if (cur == NULL) {
      fprintf(stderr, "empty document\n");
      xmlFreeDoc(doc);
    }

    if (xmlStrcmp(cur->name, (const xmlChar *)"st-source")) {
      fprintf(stderr, "Document of the wrong type.");
      xmlFreeDoc(doc);
    } else {
      xmlNodePtr nptr = cur->xmlChildrenNode;
      while (nptr->next) {
        
        if (0 == xmlStrcmp(nptr->name, (const xmlChar *)"class")) {
          
          auto classAST = new ClassAST();

          cout << "***  Processing Class Data..." << endl;
          xmlNodePtr methodpt = nptr->xmlChildrenNode;
          while (methodpt != NULL) {
            if (!methodpt->xmlChildrenNode) {
              methodpt = methodpt->next;
              continue;
            }
            if (!xmlStrcmp(methodpt->name, BAD_CAST("name"))) {
              classAST->class_name =
                  ((char *)XML_GET_CONTENT(methodpt->xmlChildrenNode));
              
              classAST->line = xmlGetLineNo(methodpt);
              classAST->fileName = fullPathName;
            }
            if (!xmlStrcmp(methodpt->name, BAD_CAST("environment"))) {
              classAST->environment =
                  ((char *)XML_GET_CONTENT(methodpt->xmlChildrenNode));
              
            }
            if (!xmlStrcmp(methodpt->name, BAD_CAST("super"))) {
              classAST->super_class =
                  ((char *)XML_GET_CONTENT(methodpt->xmlChildrenNode));
            }
            if (!xmlStrcmp(methodpt->name, BAD_CAST("private"))) {
              classAST->privateinfo =
                  ((char *)XML_GET_CONTENT(methodpt->xmlChildrenNode));
            }
            if (!xmlStrcmp(methodpt->name, BAD_CAST("indexed-type"))) {
              classAST->indexed_type =
                  ((char *)XML_GET_CONTENT(methodpt->xmlChildrenNode));
            }
            if (!xmlStrcmp(methodpt->name, BAD_CAST("inst-vars"))) {
              string tmp = ((char *)XML_GET_CONTENT(methodpt->xmlChildrenNode));
              std::stringstream ss(tmp);
              std::string buf;
              while (ss >> buf) {
                classAST->inst_vars.push_back(buf);
                llvm::outs() << "inst_vars: " << buf << "\n";
              }

            }
            if (!xmlStrcmp(methodpt->name, BAD_CAST("class-inst-vars"))) {
              string tmp = ((char *)XML_GET_CONTENT(methodpt->xmlChildrenNode));
              std::stringstream ss(tmp);
              std::string buf;
              while (ss >> buf) {
                classAST->class_inst_vars.push_back(buf);
                llvm::outs() << "class_inst_vars: " << buf << "\n";
              }
            }
            if (!xmlStrcmp(methodpt->name, BAD_CAST("imports"))) {
              classAST->imports =
                  ((char *)XML_GET_CONTENT(methodpt->xmlChildrenNode));
            }
            if (!xmlStrcmp(methodpt->name, BAD_CAST("category"))) {
              classAST->category =
                  ((char *)XML_GET_CONTENT(methodpt->xmlChildrenNode));
            }
            methodpt = methodpt->next;
          }
          module->addClassAST(classAST);
        } else if (0 == xmlStrcmp(nptr->name, (const xmlChar *)"methods")) {
          xmlChar *className = nullptr;
          {
            // find class id
            xmlNodePtr methodpt = nptr->xmlChildrenNode;
            while (methodpt->next) {
              if (0 == xmlStrcmp(methodpt->name, (const xmlChar *)"class-id")) {
                className = XML_GET_CONTENT(methodpt->xmlChildrenNode);
                // llvm::outs()
                //     << "*** find class-id: " << (char *)className << "\n";
              }
              methodpt = methodpt->next;
            }
          }

          xmlNodePtr methodpt = nptr->xmlChildrenNode;
          while (methodpt != NULL) {
            if (!xmlStrcmp(methodpt->name, BAD_CAST("body"))) {
              auto package = className;
              if (!package)
                package = xmlGetProp(methodpt, (const xmlChar *)("package"));

              auto selector =
                  xmlGetProp(methodpt, (const xmlChar *)("selector"));

              std::string methodName =
                  std::string("st.") + std::string((char *)selector) +
                  std::string("$") + std::string((char *)package);
              llvm::outs() << "filename: " << fullPathName
                           << " method: " << methodName
                           << " line: " << methodpt->line << "\n";
              if (methodpt->xmlChildrenNode) {
                auto funcAst =
                    process(fullPathName, methodName, xmlGetLineNo(methodpt) - 1,
                            (char *)XML_GET_CONTENT(methodpt->xmlChildrenNode));
                if (funcAst) module->addFunctionAST(funcAst);
              }
            }
            methodpt = methodpt->next;
          }
        }
        nptr = nptr->next;
      }
    }
    /*
     * Cleanup function for the XML library.
     */
    xmlCleanupParser();
    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();
  }
}
bool initParser(stx::ModuleAST *module) {
  // std::ios::sync_with_stdio(true);
  // if(argc!=3){
  //     llvm::outs() << "Usage: <file in.st> <test file.ws>" << "\n";
  //     return 0;
  // }

  StringRef path(TargetModulePath);
  struct dirent *entry;
  DIR *dir = opendir(path.data());
  if (dir == NULL) {
    // is it a file?
    FILE *fptr = fopen(path.data(), "r");
    if (fptr == NULL) {
      llvm::errs() << "input file does not exist: " << path << "\n";
      return false;
    }
    // File exists hence close file
    fclose(fptr);
    std::string fullPathName = realpath(path.data(), NULL);

    initParserForFile(module, fullPathName, path);
    return true;
  }
  llvm::outs() << "path: " << path << "\n";

  while ((entry = readdir(dir)) != NULL) {
    StringRef dname(entry->d_name);
    if (!dname.endswith(".ws") && !dname.endswith(".st")) continue;
    // only parse .ws and .st files
    std::string fullPathName = path.str() + "/" + dname.str();
    llvm::outs() << "file: " << fullPathName << "\n";
    initParserForFile(module, fullPathName, dname);
  }
  closedir(dir);
  return true;
}
int dumpLLVMIR(mlir::ModuleOp module) {
  llvm::LLVMContext llvmContext;
  auto llvmModule = mlir::translateModuleToLLVMIR(module, llvmContext);
  if (!llvmModule) {
    llvm::errs() << "Failed to emit LLVM IR\n";
    return -1;
  }

  // Initialize LLVM targets.
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  mlir::ExecutionEngine::setupTargetTriple(llvmModule.get());

  if (ConfigDumpIR) {
    std::error_code err;
    llvm::raw_fd_ostream outfile("t.ll", err, llvm::sys::fs::F_None);
    if (err) {
      llvm::errs() << "Error dumping IR!\n";
    }

    llvmModule->print(outfile, nullptr);
    outfile.close();
  }

  return 0;
}

void initLLVMIR(stx::ModuleAST *moduleAST) {
  
  // Register any command line options.
  mlir::registerAsmPrinterCLOptions();
  mlir::registerMLIRContextCLOptions();
  mlir::registerPassManagerCLOptions();

  // If we aren't dumping the AST, then we are compiling with/to MLIR.

  mlir::MLIRContext context;
  mlir::OwningModuleRef module;
  
  module = stx::mlirGenFull(context, *moduleAST);

    mlir::PassManager pm(&context);
    // Apply any generic pass manager command line options and run the pipeline.
    applyPassManagerCLOptions(pm);

    // Finish lowering the toy IR to the LLVM dialect.
    pm.addPass(mlir::st::createLowerToLLVMPass());
    if (mlir::failed(pm.run(*module))) {
      // llvm::errs() << "Failed to run createLowerToLLVMPass pass \n";
    }
  dumpLLVMIR(*module);
}
int main(int argc, char **argv) {
  // InitLLVM will setup signal handler to print stack trace when the program
  // crashes.
  InitLLVM x(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);

  // We don't use args in conflib for now
  std::map<std::string, std::string> args;
  conflib::Initialize(args, true);

  auto logConfig = initLoggingConf();
  logger::init(logConfig);

  stx::ModuleAST *module = new stx::ModuleAST();
  bool success = initParser(module);
  if (success) initLLVMIR(module);
  llvm::outs() << "SmallRace Generator -- Completed." << "\n";
  return 0;
}
