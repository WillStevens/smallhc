#include <iostream>
#include <algorithm>

using std::cout;
using std::endl;
using std::find;

#include "circuit.h"


// Is a circuit self-consistent?
bool circuit::check_circuit(void)
{
	bool rval = true;
	
	// Is every input or output of a component a signal in the circuit?
	for(list<component *>::iterator i = comps.begin();
		i != comps.end(); i++)
	{
		for(unsigned j = 0; j<(*i)->num_inputs(); j++)
		{
			if (find(sigs.begin(),sigs.end(),(*i)->get_input(j)) == sigs.end())
			{
				cout << "Component " << (*i)->get_name() << " input " << (*i)->get_input(j)->get_name() << " not in circuit" << endl;
				rval = false;
			}
		}
		for(unsigned j = 0; j<(*i)->num_outputs(); j++)
		{
			if (find(sigs.begin(),sigs.end(),(*i)->get_output(j)) == sigs.end())
			{
				cout << "Component " << (*i)->get_name() << " output " << (*i)->get_output(j)->get_name() << " not in circuit" << endl;
				rval = false;
			}
		}
	}
	
	// Is every sink or source of a signal a component in the circuit?
	for(list<signalw *>::iterator i = sigs.begin();
		i != sigs.end(); i++)
	{
		for(set<signalconnection>::iterator j = (*i)->get_sinks().begin(); j != (*i)->get_sinks().end(); j++)
		{
			if (find(comps.begin(),comps.end(),j->first) == comps.end())
			{
				cout << "Signal " << (*i)->get_name() << " input " << j->first->get_name() << " not in circuit" << endl;
				rval = false;
			}
		}
		for(set<signalconnection>::iterator j = (*i)->get_sources().begin(); j != (*i)->get_sources().end(); j++)
		{
			if (find(comps.begin(),comps.end(),j->first) == comps.end())
			{
				cout << "Signal " << (*i)->get_name() << " output " << j->first->get_name() << " not in circuit" << endl;
				rval = false;
			}
		}
	}

	// Check consistency between connections and components
	for(list<signalw *>::iterator i = sigs.begin();
		i != sigs.end(); i++)
	{
		for(set<signalconnection>::iterator j = (*i)->get_sinks().begin(); j != (*i)->get_sinks().end(); j++)
		{
			if (j->first->get_input(j->second) != *i)
			{
				cout << "Signal " << (*i)->get_name() << " is not input " << j->second << " of component " << j->first->get_name() << endl;
				rval = false;
			}
		}
		for(set<signalconnection>::iterator j = (*i)->get_sources().begin(); j != (*i)->get_sources().end(); j++)
		{
			if (j->first->get_output(j->second) != *i)
			{
				cout << "Signal " << (*i)->get_name() << " is not output " << j->second << " of component " << j->first->get_name() << endl;
				rval = false;
			}
		}
	}	

	return rval;
}