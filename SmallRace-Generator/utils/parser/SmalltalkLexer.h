
// Generated from Smalltalk.g4 by ANTLR 4.9

#pragma once


#include "antlr4-runtime.h"




class  SmalltalkLexer : public antlr4::Lexer {
public:
  enum {
    SEPARATOR = 1, STRING = 2, EMBEDC = 3, COMMENT = 4, BLOCK_START = 5, 
    BLOCK_END = 6, CLOSE_PAREN = 7, OPEN_PAREN = 8, PIPE2 = 9, PIPE = 10, 
    PERIOD = 11, SEMI_COLON = 12, LLT = 13, GGT = 14, LTE = 15, GTE = 16, 
    LT = 17, GT = 18, AMP = 19, MINUS = 20, BINARY_SELECTOR = 21, RESERVED_WORD = 22, 
    IDENTIFIER = 23, IDENTIFIER2 = 24, CARROT = 25, UNDERSCORE = 26, UNDERSCORE2 = 27, 
    ASSIGNMENT = 28, COLON = 29, HASH = 30, DOLLAR = 31, EXP = 32, HEXNUM = 33, 
    INTNUM = 34, HEX = 35, LITARR_START = 36, DYNDICT_START = 37, DYNARR_END = 38, 
    DYNARR_START = 39, HEXDIGIT = 40, KEYWORD = 41, BLOCK_PARAM = 42, CHARACTER_CONSTANT = 43
  };

  explicit SmalltalkLexer(antlr4::CharStream *input);
  ~SmalltalkLexer();

  virtual std::string getGrammarFileName() const override;
  virtual const std::vector<std::string>& getRuleNames() const override;

  virtual const std::vector<std::string>& getChannelNames() const override;
  virtual const std::vector<std::string>& getModeNames() const override;
  virtual const std::vector<std::string>& getTokenNames() const override; // deprecated, use vocabulary instead
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;

  virtual const std::vector<uint16_t> getSerializedATN() const override;
  virtual const antlr4::atn::ATN& getATN() const override;

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;
  static std::vector<std::string> _channelNames;
  static std::vector<std::string> _modeNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

