#include "../include/Glc.hpp"
using namespace std;

GLC::GLC(char* fileName) {
  initialSymbol = "S";
  readDataFromFile(fileName);
}

void GLC::readDataFromFile(char* fileName) {
  ifstream file(fileName);
  
  if (!file.is_open()) {
    throw runtime_error("FILE NOT FOUND\n");
  }

  string data;
  while(getline(file, data)) {
    stringstream line(data);
    string var;
    bool before_arrow = true;
    while (getline(line, data, ' ')) {
      before_arrow = before_arrow? data != "->" : before_arrow; 

      if (before_arrow) {
        var = data;
      } else if (isValidRule(data)) {
        dataSet[var].push_back(data);
      }
    }
  }
  file.close();
}

bool GLC::isVariable(string str) {
  return regex_match(str, regex("[A-Z][1-9]*"));
}

bool GLC::isValidRule(string str) {
  return regex_match(str, regex("(([A-Z][1-9]*)|[a-z])*|\\."));
}

bool GLC::isTerminal(string str) {
  return regex_match(str, regex("[a-z]|\\."));
}

void GLC::removeInitialSymbolRecursion(){
  bool recursionFound = false;
  for (_GLC::const_reference v : dataSet) {
    for (string u : v.second) { 
      if (regex_match(u, regex(".*"+initialSymbol+".*"))) {
        string newInitial = "S'";
        dataSet[newInitial].push_back(initialSymbol);
        initialSymbol = newInitial;
        break;
      }
    }
  }
}

regex GLC::createRegex(vector<string> rules, bool findTerminal) {
  if (rules.empty() && !findTerminal) return regex("()*");
  string rgex = "(";
  if(findTerminal) rgex += "[a-z]|";
  for (string r : rules) {
    rgex += r + "|";
  }
  rgex[rgex.length()-1] = ')';
  rgex += "*|\\.";
  return regex(rgex);
}

vector<string> GLC::findNullables() {
  vector<string> nullables;
  for (_GLC::const_reference d : dataSet) {
    vector<string> rules = d.second;
    if (find(rules.begin(), rules.end(), LAMBDA) != rules.end()) {
      nullables.push_back(d.first);
    }
  }

  int nullablesFound = 0;
  do {
    nullablesFound = 0;
    for (_GLC::const_reference d : dataSet) {
      auto rules = d.second;
      regex regexRule = createRegex(nullables);
      for (string r : rules) {
        if (regex_match(r, regexRule) && (find(nullables.begin(), nullables.end(), d.first) == nullables.end())) {
          nullables.push_back(d.first);
          nullablesFound++;
        }
      }
    }
  } while(nullablesFound != 0);
  return nullables;
}

void GLC::convertToFNC(bool printSteps) {
  if (printSteps) cout << "Original: " << "\n" << *this << "\n";
  removeInitialSymbolRecursion(); // OK
  if (printSteps) cout << "Remove Recursion: " << "\n" << *this << "\n";
  removeLambdaRules(); // OK
  if (printSteps) cout << "Remove Lambda Rules: " << "\n" << *this << "\n";
  removeChainRules(initialSymbol); // ok
  if (printSteps) cout << "Remove Chain Rules: " << "\n" << *this << "\n";
  removeUselessSymbols(); // OK
  if (printSteps) cout << "Remove Useless Symbols: " << "\n" << *this << "\n";
  chomsky(); // OK
  if (printSteps) cout << "FNC: " << "\n" << *this << "\n";
}

int countNullableVars(string rule, vector<string> nullable) {
  int n = 0;
  for (int i=0; i < rule.length(); i++) {
    string auxStr = "";
    auxStr += rule[i];
    if (find(nullable.begin(), nullable.end(),  auxStr) != nullable.end()) {
      n++;
    }
  }
  return n;
}

void GLC::removeLambdaRules(){
  vector<string> nullables = findNullables();

  for (string i : nullables) {
    vector<string> rules = dataSet[i];
    for (string r : rules) {
      string newRule = r;
      for (int j=0; j < pow(2,countNullableVars(newRule, nullables)); j++) {
        newRule = r;
        uint8_t pos = 1;
        uint8_t qtd_removed = 0;
        for (uint k=0; k < r.length(); k++) {
          string auxStr = "";
          auxStr += r[k];
          if (find(nullables.begin(), nullables.end(),  auxStr) != nullables.end()) {
            pos <<= 1;
            if (pos & j<<1) {
              newRule.erase(k-qtd_removed++, 1);
            }
          }
        }
        newRule = newRule == "" ? "." : newRule; 
        if (find(dataSet[i].begin(), dataSet[i].end(),  newRule) == dataSet[i].end())
          dataSet[i].push_back(newRule);
      }
    }
    if (i != initialSymbol)
      dataSet[i].erase(remove(dataSet[i].begin(), dataSet[i].end(), "."), dataSet[i].end());
  }
}

vector<string> GLC::findChain(string var) {
  vector<string> chain, prev, _new;
  chain.push_back(var);
  _new.push_back(var);

  while (chain != prev) {
    prev = chain;
    for(string v : _new) {
      if (v == "") continue;
      for (string u : dataSet[v]) { 
        if (isVariable(u)) {
          _new.push_back(u);
          chain.push_back(u);
        }
      }
      _new.erase(remove(_new.begin(), _new.end(), v), _new.end());
    }
  }
  chain.erase(remove(chain.begin(), chain.end(), var), chain.end());
  return chain;
}

vector<string> GLC::removeChainRules(string var){
  auto chain = findChain(var);
  for (string u : chain) { 
    if (find(dataSet[var].begin(), dataSet[var].end(), u) == dataSet[var].end()) continue;
    dataSet[var].erase(remove(dataSet[var].begin(), dataSet[var].end(), u), dataSet[var].end());
    auto newRules = removeChainRules(u);
    for (string i : newRules) {
      if (find(dataSet[var].begin(), dataSet[var].end(),  i) == dataSet[var].end()) {
        dataSet[var].push_back(i);
      }
    } 
  }
  return dataSet[var];
}

void GLC::removeUselessSymbols(){
  term();
  reach();
}

vector<string> GLC::findTerminals() {
  int terminalsFound = 0;
  vector<string> terminals;
  do {
    for (_GLC::reference v : dataSet) {
      for (string u : v.second) { 
        if (regex_match(u, regex(createRegex(terminals, true)))) {
          terminals.push_back(v.first);
          break;
        }
      }
    }
  } while (terminalsFound != 0);
  return terminals;
}

void GLC::term(){
  vector<string> terminal = findTerminals();
  vector<string> rulesToErase;
  regex terminalRegex = createRegex(terminal, true);
  for (_GLC::iterator v = dataSet.begin(); v != dataSet.end(); v++) { 
    if (find(terminal.begin(), terminal.end(), v->first) == terminal.end()) {
      rulesToErase.push_back(v->first);
      continue;
    }
    for (vector<string>::iterator u = v->second.begin(); u < v->second.end(); u++) { 
      if (!regex_match(*u, terminalRegex)) {
        v->second.erase(remove(v->second.begin(), v->second.end(), *u), v->second.end());
        u--;
      }
    }
  }
  for(string rule: rulesToErase) {
    dataSet.erase(rule);
  }
}

vector<string> GLC::findReachables(string start) {
  vector<string> reachables, queue;
  regex r = regex("[A-Z][1-9]*");
  queue.push_back(start);
  while(!queue.empty()) {
    string s = queue.front(); queue.pop_back();
    for (string rule : dataSet[s]) {
      regex_iterator<string::iterator> begin(rule.begin(), rule.end(), r);
      regex_iterator<string::iterator> end;
      while (begin!=end) {
        if (find(reachables.begin(), reachables.end(), begin->str()) == reachables.end()) {
          reachables.push_back(begin->str());
          queue.push_back(begin->str());
        }
        begin++;
      }
    }
  }
  return reachables;
}

void GLC::reach(){
  vector<string> mustRemove, reachables = findReachables(initialSymbol);
  reachables.push_back(initialSymbol);
  
  for (_GLC::reference v : dataSet) {
    if (find(reachables.begin(), reachables.end(), v.first) == reachables.end()){
      mustRemove.push_back(v.first);
    }
  }
  for(string r : mustRemove) {
    dataSet.erase(r);
  }
}

string transform(char v) {
  string aux = "";
  aux += toupper(v);
  aux += "'";
  return aux;
}

void GLC::chomsky() {
  int counter = 1;
  map<string, string> newRules;
  regex r = regex("([A-Z]\\')|([A-Z][1-9]*)");
  
  for (_GLC::iterator v = dataSet.begin(); v != dataSet.end(); v++) { 
    for (vector<string>::iterator u = v->second.begin(); u < v->second.end(); u++) { 
      if (u->size() < 2) continue;
      string nRule = *u;
      for(char c : *u) {
        string aux = "";
        aux += c;
        if (isTerminal(aux)) {
          newRules[aux] = transform(c);
          nRule = regex_replace(nRule, std::regex(aux), newRules[aux]);
        }
      }
      *u = nRule; 
    }
  }

  for (_GLC::iterator v = dataSet.begin(); v != dataSet.end(); v++) { 
    for (vector<string>::iterator u = v->second.begin(); u < v->second.end(); u++) { 
      regex_iterator<string::iterator> begin(u->begin(), u->end(), r);
      regex_iterator<string::iterator> end;
      if (distance(begin, end) < 2) continue;
      if (distance(begin, end) > 2) {
        while (distance(begin, end) > 2) {
          string ultima = (begin++)->str();
          string penultima;
          while (begin!=end) {
            penultima = ultima;
            ultima = begin->str();
            begin++;
          }
          string rule = penultima + ultima;
          if (newRules.find(rule) == newRules.end()) {
            string nRule = "T";
            nRule += to_string(counter++);
            newRules[rule] = nRule;
          } 
          *u = regex_replace(*u, std::regex(rule), newRules[rule]); 
        }
        regex_iterator<string::iterator> begin(u->begin(), u->end(), r);
        regex_iterator<string::iterator> end;
      }
    }
  }

  for (map<string, string>::reference v: newRules) {
    dataSet[v.second].push_back(v.first);
  }
}

  ostream& operator<<(ostream& out, GLC &g) {
    for (_GLC::reference v : g.dataSet) {
      out << v.first << " -> ";
      for (int i = 0; i < v.second.size(); i++) {
        out << v.second[i];
        if (i < v.second.size()-1) out << " | ";
      }
      out << "\n";
    }
    return out;
}