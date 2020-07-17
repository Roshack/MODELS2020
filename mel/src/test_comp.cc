#include <iostream>
#include <fstream>
#include <vector>
#include "mel_classes.h"
#include "../lib/rapidxml/rapidxml.hpp"
using namespace rapidxml;
using namespace std;

extern int yyparse();

std::istream *lexer_ins_ = nullptr;

std::string result;

vector<unique_ptr<melStmt>> *stmtsPtr;

int main(int argc, char **argv)
{
    if (argc < 3) {
        cout << "Usage: " << string{argv[0]} << " grammar_file xml_file [xml_files] [-o output]" << endl;
        return -1;
    }
    vector<string> fileNames;
    std::string outFileName = "";
    bool outFileFlag = false;
    for (int i = 2; i < argc; ++i) {
        std::string s{argv[i]};
        if (s == "-o" || s == "-O") {
            outFileFlag = true;
            continue;
        }
        if (!outFileFlag) {
            fileNames.emplace_back(s);
        } else {
            outFileName = s;
            outFileFlag = false;
        }
    }
    lexer_ins_ = new fstream{string{argv[1]}};
    yyparse();
    ///cerr << "Stmts pointer now" << stmtsPtr
    vector<unique_ptr<melStmt>> &stmts = *stmtsPtr;
    vector<xml_document<>*> docs;
    /* This was for doing one document...
    xml_document<> doc;
    ifstream inFile{argv[2]};
    vector<char> buffer{istreambuf_iterator<char>{inFile}, istreambuf_iterator<char>{}};
    buffer.emplace_back('\0');
    doc.parse<0>(&buffer[0]);
    */
    ofstream file;
    if (outFileName != "") {
        file.open(outFileName);
    }
    ostream &tafile = outFileName != "" ? file : cout;
    /*
    cout << "Outfile name: " <<outFileName << endl;
    cout << "XML Files: " << endl;
    for (auto &s : fileNames) {
        cout << s << endl;
    }
    */
    vector<vector<char>> buffers;
    for (auto &s : fileNames) {
        docs.emplace_back(new xml_document<>{});
        xml_document<> &doc = *docs.back();
        ifstream inFile{s};
        buffers.emplace_back(vector<char>{istreambuf_iterator<char>{inFile}, istreambuf_iterator<char>{}});
        vector<char> &buffer = buffers.back();
        buffer.emplace_back('\0');
        doc.parse<0>(&buffer[0]);
    }
    /*
    for (int i = 0; i < stmts.size(); ++i) {
        string name = stmts[i].getName();
        int p = stmts[i].numParams();
        for (int j = i+1; j < stmts.size();) {
            if (stmts[j].getName() == name && stmts[j].numParams() == p) {
                stmts[i].addAlternative(std::move(stmts[j]));
                stmts.erase(stmts.begin() + j);
            } else{
                ++j;
            }
        }
    }*/

    for (auto &s : stmts) {
        s->generateResults(stmts, docs);
        //s->outputResults(tafile);
        //for (auto &r : s->results) {
            //cout << r << endl;
        //}
    }

    csvOutputEngine outputEngine{tafile};
    outputEngine.print(stmts);
    
    for (auto & p : docs) {
        delete p;
    }
    
    delete stmtsPtr;
    return 0;
}
