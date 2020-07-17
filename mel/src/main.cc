#include "../lib/rapidxml/rapidxml.hpp"
#include <vector>
#include <map>
#include <fstream>
#include <iterator>
#include <iostream>
#include <algorithm>
using namespace rapidxml;
using namespace std;


class XMLWalker {
    string fileName;
    map<string, int> tagCounts;
    string indent = "";
  public:
    XMLWalker(string fName) : fileName{fName} {}
    void process() {
        xml_document<> doc;
        ifstream inFile{fileName};
        vector<char> buffer{istreambuf_iterator<char>{inFile}, istreambuf_iterator<char>{}};
        buffer.emplace_back('\0');
        doc.parse<0>(&buffer[0]);
        
        xml_node<> *theNode = doc.first_node();
        walk(theNode);
        vector<pair<string, int>> sorted_map;
        for (auto &p : tagCounts) {
                sorted_map.emplace_back(p);
        }
        sort(sorted_map.begin(), sorted_map.end(), 
            [](pair<string, int>lhs, pair<string, int> rhs) {
                return lhs.second > rhs.second;
            });
        for (auto &p : sorted_map) {
            cout << p.first << " - " << p.second << endl;
        }
    }
    
    
    void walk(xml_node<> *node) {
        ++tagCounts[node->name()];
        indent += "  ";
        xml_node<> *n = node->first_node();
        while (n != nullptr) {
            walk(n);
            n = n->next_sibling();
        }
        indent = indent.substr(0, indent.size() - 2);
    }
    
};


int main(int argc, char** argv) {
    string s{argv[1]};
    XMLWalker walker{s};
    
    walker.process();
}
