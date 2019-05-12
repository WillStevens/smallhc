#ifndef GLOBALTYPES_H
#define GLOBALTYPES_H

#include <string>
#include <vector>

using std::string;
using std::vector;

/* This file defines classes which might be used at all
 * stages of compilation */

class pin_list
{
public:
	pin_list(string *n) { ps.push_back(new widenum(*n)); }

	void push_back(string *n) { ps.push_back(new widenum(*n)); }

	unsigned size(void) {return ps.size();}

	widenum *operator[](unsigned n) {return ps[n];}
private:
	vector<widenum *> ps;
};

#endif