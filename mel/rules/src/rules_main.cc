#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "rules_classes.h"
#include <memory>
#include <fstream>
using namespace std;
extern int yyparse();
std::istream *lexer_ins_ = nullptr;

unordered_map<string,string> decls;

unordered_set<string> entities;

vector<unique_ptr<Rule>> rules;

int outputType = CSVOUT;

int declContains(string key) {
    return decls.find(key) != decls.end();
}

int main(int argc, char **argv)
{
    lexer_ins_ = &cin;
    yyparse();

    if (outputType == CSVOUT) {
        ofstream edges = ofstream{"edges.csv"};
        ofstream nodes = ofstream{"nodes.csv"};
        edges << ":START_ID\t:END_ID\t:TYPE\tcond" << endl;
        nodes << ":ID\t:LABEL\tname" << endl;
        for (auto &it : entities) {
            nodes << (declContains(it) ? decls[it] : it) << "\t" << "ruleNode" << "\t" << (declContains(it) ? decls[it] : it) << endl;
        }
        for (auto &it : rules) {
            it->printOut(edges);
            edges << endl;
        }
    }
    return 0;
}