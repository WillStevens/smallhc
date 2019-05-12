#ifndef compilestate_h
#define compilestate_h

#include "hcast.h"
#include "circuit.h"

class compile_state
{
public:
	compile_state(void);

	// eventually, everything will get deleted.
	~compile_state(void) {}

	bool typecheckgood;
	bool compilegood;
	program *hcast;
	scope *symtab;

	circuit c;
	signalw *startsig;
};

extern compile_state state;

#endif
