#include "mel_classes.h"
#include "../lib/rapidxml/rapidxml.hpp"
#include <iostream>
#include <memory>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <set>
#include <sstream>
#include <list>
#include <algorithm>
#include <map>
using namespace std;
using namespace rapidxml;

std::string RED = "\033[31m";
std::string WHITE = "\033[0m";
std::string MAGENTA = "\033[35m";

string reEscape(string s) {
    stringstream ss;
    bool escaped = false;
    for (auto &c: s) {
        switch (c) {
            case '\\':
                ss << "\\\\";
                break;
            case '\n':
                ss << "\\n";
                break;
            case '\r':
                ss << "\\r";
                break;
            case '\t':
                ss << "\\t";
                break;
            default:
                ss << c;
                break;
        }
    }
    return ss.str();
}

void melCond::mergeResults(melCond &other) {
    vector<melResult> &lr = this->getResults();
    vector<melResult> &rr = other.getResults();
    vector<melResult> ret;
    map<string,int> fieldsMatched;
    int totalMatchedFields = 0;
    for (auto &r : lr) {
        bool foundAMatch = false;
        for (auto &p : rr) {
            melResult newR{r};
            int matchedFields = 0;
            int matchedValues = 0;
            for (auto &pair : r.fields) {
                auto found = p.fields.find(pair.first);
                if (found != p.fields.end()) {
                    fieldsMatched[pair.first];
                    ++matchedFields;
                    if (found->second == pair.second){
                        fieldsMatched[pair.first]++;
                        ++matchedValues;
                    }
                }
            }
            totalMatchedFields += matchedFields;
            if (matchedFields > 0 && matchedFields == matchedValues) {
                // Found a match.
                foundAMatch = true;
                for (auto & field : p.fields) {
                    newR.fields[field.first] = field.second;
                }
                ret.emplace_back(newR);
            }
        }
        if (!foundAMatch && (isOptional() || other.isOptional())) {
            ret.emplace_back(r);
        }
    }
    // Consider moving this out to statements...
    if (ret.size() > 0) {
        this->setResults(ret);
    } else {
        if (totalMatchedFields == 0) {
            string s = "No matched fields between \033[35m" + getName() + "\033[0m and \033[35m" + other.getName() + "\033[0m";
            throw melError{s};
        }
        string s = "Couldn't match fields: ";
        int count = 0;
        for (auto &p : fieldsMatched) {
            if (p.second==0) {
                if (!count) {
                    s += p.first;
                    ++count;
                }
                else s+= ", " + p.first;
            }
        }
        throw melError{s};
    }
}

std::ostream& operator<<(std::ostream& out, const melResult& r) {
    out << r.tag << "{";
    for (auto &p : r.fields) {
        out << p.first << ":" << p.second <<", ";
    }
    
    return out << "}";
}

char melParam::c = '%';

melParam::melParam(std::string id, std::string modifier) : id{id}, hasMod{modifier != ""},
      prefix{modifier.substr(0, modifier.find(c))},
      suffix{modifier.substr(modifier.find(c) + 1, modifier.length() - 1 - modifier.find(c))} {}

string melParam::modify(string text) {
    if (!hasMod) return text;
    // Check to see if pref and suff are actually found, could be removed later.
    size_t sufLen = suffix.length();
    size_t prefLen = prefix.length();
    size_t prefFind = text.find(prefix);
    size_t suffFind = text.find(suffix);
    if (sufLen && suffFind == string::npos) {
        cerr << RED << "Warning: " << WHITE << "Didn't find suffix: \"" << reEscape(suffix) << "\" in " << MAGENTA << text << WHITE;
        cerr << " - Using whole string." << endl;
        suffFind = text.length();
    }
    if (prefLen && prefFind == string::npos) {
        cerr << RED << "Warning: " << WHITE << "Didn't find prefix: \"" << reEscape(prefix) << "\" in " << MAGENTA << text << WHITE;
        cerr << " - Using whole string." << endl;
        prefFind = 0;
        prefLen = 0;
    }
    // The string from the first character after the prefix to (but not including) the first character of the suffix.
    if (sufLen == 0) {
        suffFind = text.length();
    }
     return text.substr(prefFind + prefLen, suffFind - (prefFind + prefLen));
    
}

std::string melParam::toString() const {
    return id + "[" + prefix + "%" + suffix + "]";
}

bool melRequirement::applyOp(xml_attribute<> *attr) {
    // Could be solved better with OOP maybe... but honestly this is probably faster and do we really need to
    // write a new function each time?
    if (op == "=") {
        return attr->name() != id.ID() || val == attr->value();
    }
    if (op == "!=") {
        bool b =  attr->name() != id.ID() || val != attr->value();
        return b;
    }
    if (op == "nexists") {
        return !(attr->name() == id.ID());
    }
    if (op == "exists") {
        return (attr->name() == id.ID());
    }
}

bool melRequirement::meetsReq(rapidxml::xml_node<> *node) {
    bool meetsReqs = true;
    if (op=="exists") meetsReqs = false;
    // Must contain for =, not necessary for !=
    bool foundAttr = op == "=" ? false : true;
    if (id.ID() == "mel.contents") {
        if (op == "=") {
            return node->value() == val;
        }
        if (op == "!=") {
            return node->value() != val;
        }
    }
    for (xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
        if (attr->name() == id.ID()) foundAttr = true;
        meetsReqs = combine(meetsReqs,applyOp(attr));
    }
    return meetsReqs && foundAttr;
}

bool melRequirement::combine(bool a, bool b) {
    // Again could be one of the virtual functions for OOP but do we want to write a new class/function every single time?
    if (op == "exists") {
        // Currently Exists is only op that is an or
        return a || b;
    } else {
        // nexist, = , and != are all and.
        return a && b;
    }
}

void melIDCond::extract(vector<unique_ptr<melStmt>> &stmts, std::vector<rapidxml::xml_document<>*> &docs) {}

string melIDCond::toString() const {
    return name;
}

void melFunction::extract(vector<unique_ptr<melStmt>> &stmts, std::vector<rapidxml::xml_document<>*> &docs) {
    for (auto &p : stmts) {
        if (p->getName() == name && p->numParams() == params.size()) {
            map<string,string> paramMap;
            for (size_t i = 0; i < p->params.size(); ++i) {
                paramMap[p->params[i].ID()] = params[i]->toString();
            }
            for (size_t i = 0; i < bindings.size(); ++i) {
                paramMap[bindings[i].first.ID()] = bindings[i].second.ID();
            }
            for (auto &r : p->results) {
                melResult myR;
                myR.tag = name;
                for (auto &f : r.fields) {
                    if (paramMap.find(f.first) != paramMap.end()) {
                        myR.fields[paramMap[f.first]] = f.second;
                    } 
                }
                results.emplace_back(myR);
            }
        }
    }
}

string melFunction::toString() const {
    string s = name;
    s += "(";
    for (auto &p : params) {
        s += p->toString() + ",";
    }
    s += ") {";
    for (auto &b : bindings) {
        s += b.first.toString() + ":" + b.second.toString() +  ",";
    }
    s +="}";
    return s;
}


void melParent::walk(rapidxml::xml_node<> *node) {
    if (!node) return;
    melXMLNode* parent = dynamic_cast<melXMLNode*>(params[0].get());
    melXMLNode* child = dynamic_cast<melXMLNode*>(params[1].get());
    if (node->name() == parent->tag && parent->meetsReq(node)) {
        xml_node<> *n = node->first_node();
        while (n != nullptr) {
            if (n->name() == child->tag && child->meetsReq(n)) {
                melResult r;
                r.tag = "Parent";
                for (auto &p : parent->fields) {
                    if (p.first.ID() == "mel.contents") {
                        r.fields[p.second.ID()] = p.first.modify(node->value());
                        continue;
                    }
                    for (xml_attribute<> *attr = node->first_attribute();
                        attr; attr = attr->next_attribute()) {
                        if (attr->name() == p.first.ID()) {
                            r.fields[p.second.ID()] = p.first.modify(attr->value());
                        }
                    }
                }
                for (auto &p : child->fields) {
                    if (p.first.ID() == "mel.contents") {
                        r.fields[p.second.ID()] = p.first.modify(n->value());
                    }
                    for (xml_attribute<> *attr = n->first_attribute();
                        attr; attr = attr->next_attribute()) {
                        if (attr->name() == p.first.ID()) {
                            r.fields[p.second.ID()] = p.first.modify(attr->value());
                        }
                    }
                }
                pResults.emplace_back(r);
            }
            n = n->next_sibling();
        }
    }
    xml_node<> *n = node->first_node();
    while (n != nullptr) {
        walk(n);
        n = n->next_sibling();
    }
}

void melParent::extract(std::vector<std::unique_ptr<melStmt>> &, std::vector<rapidxml::xml_document<>*> &docs) {
    for (auto & doc : docs) {
        xml_node<> *root = doc->first_node();
        walk(root);

    }
}



void melAncestor::walk(rapidxml::xml_node<>*node, vector<xml_node<>*> &ancestors) {
    // Vector is a stack of xml_node<>*'s to the ancestors. Back of the vector
    // is the most recent ancestor.
    if (!node) return;
    melXMLNode* parent = dynamic_cast<melXMLNode*>(params[0].get());
    melXMLNode* child = dynamic_cast<melXMLNode*>(params[1].get());
    
    // If we're within scope of an ancestor
    // TODO: Decide should we gen result for each pair of ancestor/child? Like if the
    //       ancestor stack is 5 deep should we gen 5 results for each thing we find?
    //       Might want that to be an optional other function
    if (ancestors.size() > 0) {
        unordered_map<string,string> parentFields = parent->produceResult(ancestors.back());
        if (node->name() == child->tag && child->meetsReq(node)) {
            melResult r;
            r.tag = "Ancestor";
            r.fields = child->produceResult(node);
            for (auto &p : parentFields) {
                r.fields[p.first] = p.second;
            }
            aResults.emplace_back(r);
        }
    }
    bool isAncestor = (node->name() == parent->tag && parent->meetsReq(node));
    if (isAncestor) ancestors.emplace_back(node);
    xml_node<> *n = node->first_node();
    while (n != nullptr) {
        walk(n, ancestors);
        n = n->next_sibling();
    }
    if (isAncestor) ancestors.pop_back();

}

void melAncestor::extract(std::vector<std::unique_ptr<melStmt>> &, std::vector<rapidxml::xml_document<>*> &docs) {
    for (auto & doc: docs) {
        vector<xml_node<>*> ancestors;
        xml_node<> *root = doc->first_node();
        walk(root, ancestors);
    }
}

void melXMLNode::walk(rapidxml::xml_node<> *node) {
    if (!node) return;
    if (node->name() == tag) {
        melResult r;
        r.tag = tag;
        bool meetsReqs = true;
        meetsReqs = meetsReqs && meetsReq(node);
        if (meetsReqs) {
            for (xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
                meetsReqs = meetsReqs && meetsReq(node);
                if (!meetsReqs) break;
                for (auto &p : fields) {
                    if (p.first.ID() == "mel.contents") {
                        r.fields[p.second.ID()] = p.first.modify(node->value());
                    }
                    if (attr->name() == p.first.ID()) {
                        foundParamCounts[p.first.ID()]++;
                        r.fields[p.second.ID()] = p.first.modify(attr->value());
                    }
                }
            }
        }
        if (meetsReqs) results.emplace_back(r);
    }
    xml_node<> *n = node->first_node();
    while (n != nullptr) {
        walk(n);
        n = n->next_sibling();
    }
}


void melXMLNode::extract(vector<unique_ptr<melStmt>> &stmts, std::vector<rapidxml::xml_document<>*> &docs) {
    foundParamCounts = map<string, int>{};
    for (auto &p : fields) {
        if (p.first.ID() != "mel.contents") {
            foundParamCounts[p.first.ID()] = 0;
        }
    }
    for (auto & doc : docs) {
        xml_node<> *root = doc->first_node();
        walk(root);
    }
    for (auto &pair : foundParamCounts) {
        if (pair.second == 0) {
            cerr << RED << "WARNING: " << WHITE << "Found no occurrences of attribute " << MAGENTA << pair.first << WHITE;
            cerr << " in rule " << MAGENTA << getName() << WHITE << endl;
            cerr << "\t This may mean you mispelled the attribute." << endl;
        }
    }
}

std::string melXMLNode::toString() const {
    string s = tag;
    s += "{ ";
    for (auto &p : fields) {
        s += p.first.toString() + ":" + p.second.toString() +  ", ";
    }
    for (auto &p : reqs) {
        s += p.toString() + ", ";
    }
    s += "} ";
    return s;
}

unordered_map<string,string> melXMLNode::produceResult(rapidxml::xml_node<>* node) {
    unordered_map<string,string> res;
    if (!meetsReq(node)) return res;
    for (auto &p : fields) {
        if (p.first.ID() == "mel.contents") {
            res[p.second.ID()] = p.first.modify(node->value());
            continue;
        }
        for (xml_attribute<> *attr = node->first_attribute();
            attr; attr = attr->next_attribute()) {
            if (attr->name() == p.first.ID()) {
                res[p.second.ID()] = p.first.modify(attr->value());
            }
        }
    }
    return res;
}



string melStmt::toString() const {
    string s = name;
    s += "(";
    for (auto &p : params) {
        s += p.toString() + ",";
    }
    s += ") |- ";
    for (auto &c : conds) {
        s += c->toString();
    }
    return s;
}


void melStmt::generateResults(vector<unique_ptr<melStmt>> &stmts, vector<xml_document<>*> &docs) {
    
    vector<vector<melResult>*> allResults;
    for (auto &c : conds) {
        //xml_node<> *theNode = doc.first_node();
        c->extract(stmts, docs);
        vector<melResult> &condResults = c->getResults();
        allResults.emplace_back(&condResults);
    }
    if (allResults.size() == 1) {
        for (auto &p : *allResults[0]) {
            melResult myR{p};
            myR.tag = name;
            results.emplace_back(myR);
        }
        return;
    }
    // Slow bad loop that merges multiple times to make sure we didn't miss anything.
    for (size_t i = 0; i < conds.size()-1; ++i) {
        for (size_t j = 1; j < conds.size(); ++j) {
            try {
                conds[0]->mergeResults(*conds[j]);
            } catch (melError &e) {
                if (i == conds.size()-2) {
                    cerr << "\033[31m" << "WARNING: " << "\033[0m" << "For statement " << "\033[35m" << name << "\033[0m" << " " << e.what() << endl;
                    cerr << "\t Producing no results for that rule - this may cause cascading errors." << endl;
                    conds[0]->setResults(vector<melResult>{});
                    break;
                }
            }
        }
    }
    for (auto &p : *allResults[0]) {
        melResult myR{p};
        myR.tag = name;
        results.emplace_back(myR);
    }
    // Rest is not relevant if not relationship
    if (params.size() < 2) return;
    // Remove self relations.
    if (noDup) {
        for (int i = 0; i < results.size();) {
            bool forDeletion = false;
            // Pointer instead of reference so we can null it out if we remove, so we don't accidentally
            // use a dangling ref.
            melResult *res = &results[i];
            for (auto &p : params) {
                for (auto &r : params) {
                    if (p.ID() == r.ID()) continue;
                    if (res->fields[p.ID()] == res->fields[r.ID()]) {
                        forDeletion = true;
                        break;
                    }
                }
                if (forDeletion) break;
            }
            if (forDeletion) {
                results.erase(results.begin() + i);
                res = nullptr;
            } else {
                ++i;
            }
        }
    }
    // remove commutative results.
    if (noAssoc) {
        for (int i = 0; i < results.size(); ++i) {
            for (int j = i+1; j < results.size();) {
                melParam &lhs = params[0];
                melParam &rhs = params[1];
                if (results[i].fields[lhs.ID()] == results[j].fields[rhs.ID()] &&
                    results[i].fields[rhs.ID()] == results[j].fields[lhs.ID()]) {
                    results.erase(results.begin() + j);
                } else {
                    ++j;
                }
            }
        }
    }
}

void melStmt::outputResults(std::ostream &out) {
    if (dontPrint) return;
    for (auto &r : results) {
        vector<string> arguments;
        for (auto & p : params) {
            for (auto &f : r.fields) {
                if (f.first == p.ID()) arguments.emplace_back(f.second);
            }
        }
        if (arguments.size() == 0) {
            continue;
        }
        if (arguments.size() == 1) {
            out << "$INSTANCE " << arguments[0] << " " << name << endl;
            if (r.fields.size() <= params.size()) continue;
            out << arguments[0] << " {";
            for (auto &p : r.fields) {
                if (p.first == params[0].ID()) continue;
                out << p.first << " = \"" << p.second << "\" ";
            }
            out << "}" << endl;
        } else {
            // TODO: Strip extra fields like I and O for dataflow.
            out << name << " " << arguments[0] << " " << arguments[1] << endl;
            if (r.fields.size() <= params.size()) continue;
            out << "(" << name << " " << arguments[0] << " " << arguments[1] << ") {";
            for (auto &p : r.fields) {
                if (p.first == params[0].ID() || p.first == params[1].ID()) continue;
                out << p.first << " = \"" << p.second << "\" ";
            }
            out << "}" << endl;
        }
    }
}





void csvOutputEngine::print(std::vector<std::unique_ptr<melStmt>> &stmts) {
    set<string> attributes;
    for (auto &s : stmts) {
        for (auto &r: s->results) {
            for (auto &p : r.displayableFields()) {
                if (!s->isParam(p.first)){
                    attributes.emplace(p.first);
                }
            }
        }
    }
    out << "FactType,Param1,Param2";
    for (auto & a: attributes) {
        out << "," << a;
    }
    out << endl;
    for (auto &s : stmts) {
        vector<string> paramNames = s->paramNames();
        for (auto &r : s->results) {
            out << s->getName() << "," << r.fields[paramNames[0]] << ",";
            if (paramNames.size() == 2) out << r.fields[paramNames[1]];
            for (auto &a : attributes) {
                out << ",";
                auto found = r.fields.find(a);
                if (!s->isParam(a) && found != r.fields.end()) {
                    out << reEscape(found->second);
                }
            }
            out << endl;
        }
    }
}