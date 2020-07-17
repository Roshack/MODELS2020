#ifndef LSR_CLASSES_H
#define LSR_CLASSES_H
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <set>
#include <sstream>
#include <list>
#include <map>
#include <utility>
#include <stdexcept>
#include "../lib/rapidxml/rapidxml.hpp"


// absFunction(X) |- ownedFunctions {id:X, label:name};
extern std::string RED;
extern std::string WHITE;
extern std::string MAGENTA;

class melStmt;

class melError : public std::runtime_error {
  public:
    melError(std::string s) : std::runtime_error{s} {}

};


class melParam {
    std::string id;
    bool hasMod;
    std::string prefix, suffix;
    static char c;
    
  public:
    // Assumes modifier contains "%"!
    melParam(std::string id, std::string modifier);
    

    // Returns the substring between pref and suff if applicable.
    std::string modify(std::string text);

    // Returns ID.
    const std::string &ID() { return id; }
    
    std::string toString() const;
};

struct melResult {
    std::string tag;
    std::unordered_map<std::string,std::string> fields;

    std::unordered_map<std::string, std::string> &displayableFields() {
        return fields;
    }
};
std::ostream& operator<<(std::ostream&, const melResult&);

class melRequirement {
    std::string op;
    std::string val;
    melParam id;

    bool applyOp(rapidxml::xml_attribute<> *attr);
    bool combine(bool, bool);
  public:
    melRequirement(std::string op, melParam id, std::string val) : op{op}, val{val}, id{id} {}
    bool meetsReq(rapidxml::xml_node<> *node);
    
    std::string toString() const {
      std::string s = id.toString() + " " + op + " " + val;
    }
};


class melCond {
    bool optional = false;
  public:
    virtual ~melCond() {}
    virtual void extract(std::vector<std::unique_ptr<melStmt>> &, std::vector<rapidxml::xml_document<>*> &docs) = 0;
    virtual std::string toString() const = 0;
    virtual std::vector<melResult> &getResults() = 0;
    virtual void mergeResults(melCond &other);
    virtual melCond* clone() = 0;
    virtual std::vector<std::pair<melParam,melParam>> getParams() = 0;

    virtual std::string getName() = 0;

    virtual bool meetsReq(rapidxml::xml_node<> *node) = 0;

    void setOptional(bool b) { optional = b; }
    bool isOptional() { return optional; }

  protected:
  friend class melStmt;
    virtual void setResults(std::vector<melResult>) = 0;
};

class melIDCond : public melCond {
    std::string name;
    std::vector<melResult> results;
  public:
    melIDCond(std::string name) : name{name} {}
    void extract(std::vector<std::unique_ptr<melStmt>> &, std::vector<rapidxml::xml_document<>*>&) override;
    std::string toString() const override;
    std::vector<melResult> &getResults() override {return results;}
    
    melCond* clone() {
        return new melIDCond{*this};
    }

    std::vector<std::pair<melParam,melParam>> getParams() override {
          return std::vector<std::pair<melParam,melParam>>{};
    }

    bool meetsReq(rapidxml::xml_node<> *node) { 
      return true; // TODO: Does this need to have an implementation?
    }

    std::string getName() override { return name; }

  protected:
    friend class melStmt;
      virtual void setResults(std::vector<melResult> r) override {
        results = r;
      }
};

class melFunction : public melCond {
  protected:
    std::string name;
    // Function params can be XMLNodes or jsut IDs, at least for now.
    std::vector<std::unique_ptr<melCond>> params;
    // Functions can have their own "melParams", binding the fields of
    // that function to the target.
    std::vector<std::pair<melParam,melParam>> bindings;
    std::vector<melResult> results;

  public:
    melFunction(std::string n, 
                std::vector<std::unique_ptr<melCond>> &&p, 
                std::vector<std::pair<melParam,melParam>> b) : name{n}, 
                params{std::move(p)}, bindings{b} {} 
    ~melFunction() {}
    void extract(std::vector<std::unique_ptr<melStmt>> &, std::vector<rapidxml::xml_document<>*> &docs) override;
    std::string toString() const override;
    
    std::vector<melResult> &getResults() override {return results;}
    
    //void mergeResults(melCond &other);
    
    melCond* clone() {
        return nullptr;
    }
    std::vector<std::pair<melParam,melParam>> getParams() override { return bindings; }
    
    bool meetsReq(rapidxml::xml_node<> *node) {
      return true; // TODO - this progably needs an implemtnation
      // But that requires given melFunctions requirements in the parser as well.
    }

    std::string getName() override { return name; }

  protected:
    friend class melStmt;
      virtual void setResults(std::vector<melResult> r) override {
        results = r;
      }
};

class melParent : public melFunction {
    std::vector<melResult> pResults;
    
    void walk(rapidxml::xml_node<> *node);
  public:
    melParent(std::vector<std::unique_ptr<melCond>> &&p, std::vector<std::pair<melParam,melParam>> b) : melFunction{"parent", 
                                                                                                                    std::move(p), b} {}
    void extract(std::vector<std::unique_ptr<melStmt>> &, std::vector<rapidxml::xml_document<>*> &docs) override;
    std::vector<melResult> &getResults() override {return pResults;}
    //void mergeResults(melCond &other) override;
    
    melCond* clone() {
        return nullptr;
    }
    std::vector<std::pair<melParam,melParam>> getParams() override { return melFunction::getParams(); }

    bool meetsReq(rapidxml::xml_node<> *node) {
        return true; // TODO: does this need impl?
    }

    std::string getName() override { 
      return "Parent(" + params[0]->getName() + "," + params[1]->getName() + ")"; 
    }
  protected:
  friend class melStmt;
    virtual void setResults(std::vector<melResult> r) override {
      pResults = r;
    }
};

class melAncestor : public melFunction {
    std::vector<melResult> aResults;
    void walk(rapidxml::xml_node<>*, std::vector<rapidxml::xml_node<>*> &);
  public:
    melAncestor(std::vector<std::unique_ptr<melCond>> &&p, std::vector<std::pair<melParam,melParam>> b) : melFunction{"ancestor", std::move(p), b} {}
    void extract(std::vector<std::unique_ptr<melStmt>> &, std::vector<rapidxml::xml_document<>*> &docs) override;
    std::vector<melResult> &getResults() override {return aResults;}

    melCond* clone() {
        return nullptr;
    }

    bool meetsReq(rapidxml::xml_node<> *node) {
        return true; // TODO: does this need impl?
    }

    std::string getName() override { 
      return "Ancestor(" + params[0]->getName() + "," + params[1]->getName() + ")"; 
    }

  protected:
  friend class melStmt;
    virtual void setResults(std::vector<melResult> r) override {
      aResults = r;
    }
};

class melXMLNode : public melCond {
    std::vector<std::vector<std::string>> found;
    std::string tag;
    std::vector<std::pair<melParam,melParam>> fields;
    std::vector<melRequirement> reqs;
    std::vector<melResult> results;

    std::map<std::string, int> foundParamCounts;
    
    void walk(rapidxml::xml_node<> *node);
    
  public:
    melXMLNode(std::string tag, std::vector<std::pair<melParam,melParam>> fields, std::vector<melRequirement> reqs) : tag{tag}, fields{fields}, reqs{reqs} {}
    ~melXMLNode() {}
    void extract(std::vector<std::unique_ptr<melStmt>> &, std::vector<rapidxml::xml_document<>*> &docs) override;
    std::string toString() const override;
    
    std::vector<melResult> &getResults() override {return results;}
    melCond* clone() {
        return new melXMLNode{*this};
    }

    std::vector<std::pair<melParam,melParam>> getParams() override { return fields; }

    bool meetsReq(rapidxml::xml_node<> *node) {
      for (auto &r : reqs) {
        if (!r.meetsReq(node)) return false;
      }
      return true;
    }

    std::string getName() override { return tag; }

    // Given one XML node produces the corresponding fields of a melResult should there be one.
    std::unordered_map<std::string,std::string> produceResult(rapidxml::xml_node<>* node);

    friend class melParent;
    friend class melAncestor;


    protected:
    friend class melStmt;
      virtual void setResults(std::vector<melResult> r) override {
        results = r;
      }
};

class melStmt {
    std::string name;
    std::vector<melParam> params;
    std::vector<melStmt> alternatives;
    std::vector<std::unique_ptr<melCond>> conds;
    bool dontPrint = false;
    bool noDup = false;
    bool noAssoc = false;
  public:
    melStmt(std::string n, std::vector<melParam> p, std::vector<std::unique_ptr<melCond>> &&c) : name{n}, params{p}, conds{std::move(c)} {}
    void generateResults(std::vector<std::unique_ptr<melStmt>> &, std::vector<rapidxml::xml_document<>*> &docs);
    std::string toString() const;
    std::string getName() const {return name;}
    void outputResults(std::ostream&);
    std::vector<melResult> results;
    friend class melFunction;
    void hideOutput(bool b) { dontPrint = b; }

    int numParams() { return params.size(); }

    bool isParam(std::string s) {
      for (auto &p : params) {
        if (s == p.ID()) {
          return true;
        }
      }
      return false;
    }

    void addAlternative(melStmt &&s) { alternatives.emplace_back(std::move(s)); }

    std::vector<std::string> paramNames() {
      std::vector<std::string> ret;
      for (auto &p : params) {
        ret.emplace_back(p.ID());
      }
      return ret;
    }

    void setNoDup(bool b) { noDup = b; }
    void setNoAssoc(bool b) { noAssoc = b; }
};


class melOutputEngine {
  public:
    virtual void print(std::vector<std::unique_ptr<melStmt>>&) = 0;
};

class csvOutputEngine : public melOutputEngine {
    std::ostream &out;
  public:
    csvOutputEngine(std::ostream &out) : out{out} {};
    void print(std::vector<std::unique_ptr<melStmt>>&) override;
};


#endif

