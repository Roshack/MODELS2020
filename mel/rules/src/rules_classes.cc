#include "rules_classes.h"
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

std::ostream& operator<<(std::ostream & out, const Rule &r) {
  r.printOut(out);
}

std::string CMPTokenToString(int cmp) {
  switch (cmp) {
    case 1:
      return ">";
      break;
    case 2:
      return ">=";
      break;
    case 3:
      return "<";
      break;
    case 4:
      return "<=";
      break;
    case 5:
      return "=";
      break;
    case 6:
      return "!=";
      break;
    default:
      return "??";
      break;
  }
}