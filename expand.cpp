#include "circuit.h"

void circuit::expand(void)
{
	for(list<component *>::iterator i = comps.begin();
		i != comps.end();)
	{
		if ((*i)->expand(*this))
		{
			list<component *>::iterator oldi = i;
			i++;
			(*oldi)->removefromcircuit(this);
		}
		else
		{
			i++;
		}
	}

}

bool ct_register::expand(circuit &c)
{
	return false;
}

bool ct_multiplexer::expand(circuit &c)
{
	return false;
}

bool ct_and_gate::expand(circuit &c)
{
	return false;
}

bool ct_or_gate::expand(circuit &c)
{
	return false;
}

bool ct_xor_gate::expand(circuit &c)
{
	return false;
}

bool ct_inverter::expand(circuit &c)
{
	return false;
}

bool ct_adder::expand(circuit &c)
{
	return false;
}

bool ct_subtractor::expand(circuit &c)
{
	return false;
}

bool ct_cat::expand(circuit &c)
{
	return false;
}

bool ct_equal::expand(circuit &c)
{
	return false;
}

bool ct_unequal::expand(circuit &c)
{
	return false;
}

bool ct_par_join::expand(circuit &c)
{
	if (inputs.size() > 1)
	{
		component *joinand = new ct_and_gate(1);

		joinand->add_output(outputs[0]);

		c.add_component(joinand);

		component *joinor;
		component *joinxor;
		component *joinff;
		signalw *joinxorout;
		signalw *joinffout;
		signalw *joinorout;

		for(vector<signalw *>::iterator i = inputs.begin();
			i != inputs.end(); i++)
		{
			joinor = new ct_or_gate(1);
			joinxor = new ct_xor_gate(1);
			joinff = new ct_register(1);
		
			joinxorout = new signalw(1);
			joinffout = new signalw(1);
			joinorout = new signalw(1);

			joinor->add_output(joinorout);
			joinxor->add_output(joinxorout);
			joinff->add_output(joinffout);

			joinff->add_input(*i);
			joinff->add_input(joinxorout);

			joinxor->add_input(*i);
			joinxor->add_input(outputs[0]);

			joinor->add_input(*i);
			joinor->add_input(joinffout);

			joinand->add_input(joinorout);

			c.add_component(joinor);
			c.add_component(joinxor);
			c.add_component(joinff);
	
			c.add_signal(joinxorout);
			c.add_signal(joinffout);
			c.add_signal(joinorout);
		}

	}
	else
	{
		inputs[0]->replace(outputs[0]);
	}

	return true;
}

bool ct_select::expand(circuit &c)
{
	return false;
}

bool ct_port_in::expand(circuit &c)
{
	return false;
}

bool ct_port_out::expand(circuit &c)
{
	return false;
}

bool ct_port_inout::expand(circuit &c)
{
	return false;
}

bool ct_ram::expand(circuit &c)
{
	return false;
}

