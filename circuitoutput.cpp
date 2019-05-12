#include "macros.h"
#include "circuit.h"

void signalw::graph_output(ostream &os)
{
	for(set<signalconnection>::iterator i = sinks.begin(); i != sinks.end(); i++)
	{
		os << get_name() << "->" << i->first->get_name() << endl;
	}
}

#define COMPONENT_OUTPUT(comptype) \
void comptype::graph_output(ostream &os) \
{ \
    for(vector<signalw *>::iterator i = outputs.begin(); i != outputs.end(); i++) \
	{ \
	os << get_name() << "->" << (*i)->get_name() << endl; \
	} \
}

COMPONENT_OUTPUT(ct_register)
COMPONENT_OUTPUT(ct_multiplexer)
COMPONENT_OUTPUT(ct_and_gate)
COMPONENT_OUTPUT(ct_or_gate)
COMPONENT_OUTPUT(ct_xor_gate)
COMPONENT_OUTPUT(ct_inverter)
COMPONENT_OUTPUT(ct_adder)
COMPONENT_OUTPUT(ct_subtractor)
COMPONENT_OUTPUT(ct_cat)
COMPONENT_OUTPUT(ct_select)
COMPONENT_OUTPUT(ct_equal)
COMPONENT_OUTPUT(ct_unequal)
COMPONENT_OUTPUT(ct_port_in)
COMPONENT_OUTPUT(ct_port_out)
COMPONENT_OUTPUT(ct_port_inout)
COMPONENT_OUTPUT(ct_ram)
COMPONENT_OUTPUT(ct_par_join)

void circuit::graph_output(ostream &os)
{
	{for(list<signalw *>::iterator i = sigs.begin(); 
		i != sigs.end(); i++)
	{
		(*i)->graph_output(os);
	}}

	{for(list<component *>::iterator i = comps.begin(); 
		i != comps.end(); i++)
	{
		(*i)->graph_output(os);
	}}
}
