#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <math.h>
#include <regex>
#include <string>

#ifndef __GLC
#define __GLC

#define LAMBDA "."
/**
 * string -> Vector<string>
 * V -> {regras}
 */
typedef std::map<std::string, std::vector<std::string>> _GLC; // GLC data, rules and variables

class GLC {
private:
  _GLC dataSet;
  std::string initialSymbol;

  bool isVariable(std::string str);
  bool isValidRule(std::string str);
  bool isTerminal(std::string str);
  void readDataFromFile(char* fileName);
  
  std::vector<std::string> findNullables();
  std::regex createRegex(std::vector<std::string> rules, bool findTerminal = false);
  std::vector<std::string> findChain(std::string var);
  std::vector<std::string> findTerminals();
  std::vector<std::string> findReachables(std::string start);

  void removeInitialSymbolRecursion();
  void removeLambdaRules();
  std::vector<std::string> removeChainRules(std::string var);
  void removeUselessSymbols();
  void term();
  void reach();
  void chomsky();
public:
  GLC(char* fileName); // create GLC from file
  void convertToFNC(bool printSteps = false);
  friend std::ostream& operator<<(std::ostream& out, GLC &g);
};
#endif