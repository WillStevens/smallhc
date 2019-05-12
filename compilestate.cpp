#include "compilestate.h"

compile_state state;

compile_state::compile_state(void)
{
	symtab = new scope; 
	typecheckgood = true;
	compilegood = true;

	component *startreg = new ct_register(1);
	signalw *startregin = state.c.get_constant_signal(1,widenum(1));
	signalw *startregout = new signalw(1);
	component *startinverter = new ct_inverter(1);

	startsig = new signalw(1);

	startreg->add_input(startregin);
	startreg->add_output(startregout);

	startinverter->add_input(startregout);
	startinverter->add_output(startsig);

	c.add_component(startreg);
	c.add_signal(startregout);
	c.add_component(startinverter);
	c.add_signal(startsig);
}
