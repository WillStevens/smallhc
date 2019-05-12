#ifndef symtab_h
#define symtab_h

#include <string>
#include <list>
#include <map>
#include <vector>

using std::string;
using std::list;
using std::map;
using std::vector;

#include <assert.h>

#include "types.h"
#include "circuit.h"

typedef enum {at_register, at_signal, at_extram, at_ram} archtype;

class symtab_entry
{
public:
	symtab_entry(void) {}

	symtab_entry(type *t, archtype a) : at(a), tp(t) {}

	type *get_type(void) {return tp;}
	archtype get_archtype(void) {return at;}
	
	component *get_component(unsigned n) {return cp[n];}
	void push_component(component *c) {cp.push_back(c);}

	signalw *get_signal(unsigned n) {return sg[n];}
	void push_signal(signalw *s) {sg.push_back(s);}
private:
	archtype at;
	type *tp;
	vector<component *> cp;
	vector<signalw *> sg;
};

class scope
{
public:
	scope(void);
	scope(scope *p);

	scope *exitscope(void);

	scope *enterscope(void);

	void reset(void);

	// TODO - symbol table should report error when duplicate is added.
	void add(string &id, const symtab_entry &se)
		{symbols[id] = se;}
	
	// TODO - not good to have to separate operations for
	// exists and find.
	bool exists(string &id);
	
	symtab_entry &find(string &id);
private:
	map<string,symtab_entry> symbols;
	scope *parent;
	list<scope *> subscopes;
	list<scope *>::iterator currentsubscope;
};

#endif