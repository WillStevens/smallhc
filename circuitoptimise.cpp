#include <iostream>

using std::cout;

#include "circuit.h"

using std::next;


bool circuit::optimise_dup_component(void)
{
	bool rval = false;

	for(list<component *>::iterator i = comps.begin();
		i != comps.end(); i++)
	{
		for(list<component *>::iterator j = i;
			j != comps.end();)
		{
			if (i != j && (*i)->duplicate_of(*j))
			{
				list<component *>::iterator oldj = j;
				(*i)->replace(*j);
				j++;
				(*oldj)->removefromcircuit(this);
				rval = true;
				cout << "Removed dup" << endl;
			}
			else
			{
				j++;
			}
		}
	}

	return rval;
}

bool circuit::optimise_input(void)
{
	bool rval = false;

	for(list<component *>::iterator i = comps.begin();
		i != comps.end();)
	{
		list<component *>::iterator j = i;
		j++;
		
		if ((*i)->optimise_input(this))
		{
			rval = true;
			cout << "Optimised input" << endl;
		}
		
		i = j;

	}

	return rval;
}

bool circuit::optimise_redundant(void)
{
	bool rval = false;

	for(list<component *>::iterator i = comps.begin();
		i != comps.end();)
	{
		list<component *>::iterator j = i;
		j++;
		
		if ((*i)->optimise_redundant(this))
		{
			rval = true;
			cout << "Removed redundant component" << endl;
		}
		
		i = j;

	}

	return rval;
}


// Replace s with this - i.e. the sinks of s are now sunk by this
void signalw::replace(signalw *s)
{
	for(set<signalconnection>::iterator i = s->sinks.begin();
		i != s->sinks.end(); i++)
	{
		(*i).first->replace_input((*i).second,this);
		sinks.insert(*i);
	}
	
	s->sinks.clear();
}

// Replace c with this
void component::replace(component *c)
{
	vector<signalw *>::iterator j = outputs.begin();
	vector<signalw *>::iterator i = c->outputs.begin();

	for(;i != c->outputs.end(); i++,j++)
	{
		(*j)->replace(*i);
	}
}

// -------------------------------- //
// Duplicate component optimisation //
// -------------------------------- //

bool ct_register::duplicate_of(component *c)
{
	ct_register *regc = dynamic_cast<ct_register *>(c);

	if (regc && inputs.size() == regc->inputs.size())
	{
		vector<signalw *>::iterator i,j;

		i = inputs.begin();
		j = regc->inputs.begin();

		while(i != inputs.end() && j != regc->inputs.end())
		{
			if ((*i) != (*j))
				return false;

			i++;
			j++;
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool ct_multiplexer::duplicate_of(component *c)
{
	return false;
}

bool ct_and_gate::duplicate_of(component *c)
{
	return false;
}

bool ct_or_gate::duplicate_of(component *c)
{
	return false;
}

bool ct_xor_gate::duplicate_of(component *c)
{
	return false;
}

bool ct_inverter::duplicate_of(component *c)
{
	return false;
}

bool ct_adder::duplicate_of(component *c)
{
	ct_adder *addc = dynamic_cast<ct_adder *>(c);

	if (addc && inputs.size() == addc->inputs.size())
	{
		if ((inputs[0] == addc->inputs[0] &&
			inputs[1] == addc->inputs[1]) ||
		   (inputs[0] == addc->inputs[1] &&
		    inputs[1] == addc->inputs[0]))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool ct_subtractor::duplicate_of(component  *c)
{
	return false;
}

bool ct_cat::duplicate_of(component *c)
{
	return false;
}

bool ct_equal::duplicate_of(component *c)
{
	return false;
}

bool ct_unequal::duplicate_of(component *c)
{
	return false;
}

bool ct_par_join::duplicate_of(component *c)
{
	return false;
}

bool ct_select::duplicate_of(component *c)
{
	return false;
}

bool ct_port_in::duplicate_of(component *c)
{
	return false;
}

bool ct_port_out::duplicate_of(component *c)
{
	return false;
}

bool ct_port_inout::duplicate_of(component *c)
{
	return false;
}

bool ct_ram::duplicate_of(component *c)
{
	return false;
}

// -------------------------------- //
// Redundant component optimisation //
// -------------------------------- //

bool ct_register::optimise_redundant(circuit *c)
{
	return false;
}

bool ct_multiplexer::optimise_redundant(circuit *c)
{
	return false;
}

bool ct_and_gate::optimise_redundant(circuit *c)
{
	if (inputs.size()==1)
	{
		// Replace output signal with input signal
		inputs[0]->replace(outputs[0]);
		
		// Remove this component from circuit
		removefromcircuit(c);
		
		return true;
	}
	else
	{
		return false;
	}
}

bool ct_or_gate::optimise_redundant(circuit *c)
{
	if (inputs.size()==1)
	{
		// Replace output signal with input signal
		inputs[0]->replace(outputs[0]);
		
		// Remove this component from circuit
		removefromcircuit(c);
		
		return true;
	}
	else
	{
		return false;
	}
}

bool ct_xor_gate::optimise_redundant(circuit *c)
{
	return false;
}

bool ct_inverter::optimise_redundant(circuit *c)
{
	if (inputs.size()==1 && inputs[0]->is_valueset())
	{		
		signalw *cons;
		
		cons = c->get_constant_signal(inputs[0]->get_width(),widenum_invert(inputs[0]->get_value(),inputs[0]->get_width()));
		
		// Replace output signal with constant signal
		cons->replace(outputs[0]);
		
		removefromcircuit(c);
		
		return true;
	}
	else
	{
		return false;
	}
}

bool ct_adder::optimise_redundant(circuit *c)
{
	return false;
}

bool ct_subtractor::optimise_redundant(circuit  *c)
{
	return false;
}

bool ct_cat::optimise_redundant(circuit *c)
{
	return false;
}

bool ct_equal::optimise_redundant(circuit *c)
{
	return false;
}

bool ct_unequal::optimise_redundant(circuit *c)
{
	return false;
}

bool ct_par_join::optimise_redundant(circuit *c)
{
	return false;
}

bool ct_select::optimise_redundant(circuit *c)
{
	return false;
}

bool ct_port_in::optimise_redundant(circuit *c)
{
	return false;
}

bool ct_port_out::optimise_redundant(circuit *c)
{
	return false;
}

bool ct_port_inout::optimise_redundant(circuit *c)
{
	return false;
}

bool ct_ram::optimise_redundant(circuit *c)
{
	return false;
}

// -------------------//
// Input optimisation //
// -------------------//

bool ct_register::optimise_input(circuit *c)
{
	return false;
}

bool ct_multiplexer::optimise_input(circuit *c)
{
	return false;
}

bool ct_and_gate::optimise_input(circuit *c)
{
	bool rval = false;
		
	for(unsigned j = 0; j<inputs.size();)
	{
		if (inputs.size()<2)
			return rval;

		if (inputs[j]->is_valueset())
		{
			if (widenum_ones(inputs[j]->get_value(),inputs[j]->get_width()))
			{
				unsigned i = inputs.size()-1;
				
				inputs[i]->removesink(this,i);
				
				if (j != i)
				{
					inputs[j]->removesink(this,j);
					inputs[j] = inputs[i];
					inputs[j]->addsink(this,j);
				}
				
				inputs.resize(i);
				
				rval = true;
				cout << "and gate 1" << endl;
			}
			else if (widenum_zero(inputs[j]->get_value()))
			{
		      // Replace output signal with input signal
		      inputs[j]->replace(outputs[0]);
		
		      // Remove this component from circuit
		      removefromcircuit(c);
				cout << "and gate 0" << endl;
		
		      return true;
			}
			else
			{
				j++;
			}
		}
		else
		{
			j++;
		}
	}	
	
	return rval;
}

bool ct_or_gate::optimise_input(circuit *c)
{
	bool rval = false;
	
	if (inputs.size()<2)
		return false;
	
	for(unsigned j = 0; j<inputs.size();)
	{
		if (inputs[j]->is_valueset())
		{
			if (widenum_zero(inputs[j]->get_value()))
			{
				unsigned i = inputs.size()-1;
				
				inputs[j]->removesink(this,j);
				inputs[i]->removesink(this,i);
				inputs[j] = inputs[i];
				inputs[j]->addsink(this,j);
				inputs.resize(i);
				
				rval = true;
				
				cout << "Or gate 0" << endl;
			}
			else if (widenum_ones(inputs[j]->get_value(),inputs[j]->get_width()))
			{
		      // Replace output signal with input signal
		      inputs[j]->replace(outputs[0]);
		
		      // Remove this component from circuit
		      removefromcircuit(c);
				cout << "Or gate 1" << endl;
		
		      return true;
			}
			else
			{
				j++;
			}
			
		}
		else
		{
			j++;
		}
	}	
	
	return rval;
}

bool ct_xor_gate::optimise_input(circuit *c)
{
	return false;
}

bool ct_inverter::optimise_input(circuit *c)
{
	return false;
}

bool ct_adder::optimise_input(circuit *c)
{
	return false;
}

bool ct_subtractor::optimise_input(circuit  *c)
{
	return false;
}

bool ct_cat::optimise_input(circuit *c)
{
	return false;
}

bool ct_equal::optimise_input(circuit *c)
{
	return false;
}

bool ct_unequal::optimise_input(circuit *c)
{
	return false;
}

bool ct_par_join::optimise_input(circuit *c)
{
	return false;
}

bool ct_select::optimise_input(circuit *c)
{
	return false;
}

bool ct_port_in::optimise_input(circuit *c)
{
	return false;
}

bool ct_port_out::optimise_input(circuit *c)
{
	return false;
}

bool ct_port_inout::optimise_input(circuit *c)
{
	return false;
}

bool ct_ram::optimise_input(circuit *c)
{
	return false;
}
