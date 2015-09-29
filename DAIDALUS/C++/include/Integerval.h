/*
 * Copyright (c) 2015 United States Government as represented by
 * the National Aeronautics and Space Administration.  No copyright
 * is claimed in the United States under Title 17, U.S.Code. All Other
 * Rights Reserved.
 */

#ifndef INTEGERVAL
#define INTEGERVAL

#include <string>
#include <vector>

namespace larcfm {

class Integerval {
public:
  int lb;
  int ub;

  Integerval(int lb, int ub);
  std::string toString() const;

  static std::string FmVector(const std::vector<Integerval>& l);

};

}

#endif
