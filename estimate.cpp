#include "circuit.h"

void circuit::estimate(gatecount &g)
{
	{for(list<component *>::iterator i = comps.begin(); 
		i != comps.end(); i++)
	{
		(*i)->estimate(g);
	}}
}

void ct_register::estimate(gatecount &g)
{
    g.flipflops += width;
}

void ct_multiplexer::estimate(gatecount &g)
{
    g.nand_gates += 1 + width*(inputs.size()/2);
}

void ct_and_gate::estimate(gatecount &g)
{
   g.nand_gates += inputs.size()-1;
}

void ct_or_gate::estimate(gatecount &g)
{
   g.nand_gates += inputs.size()-1;
}

void ct_xor_gate::estimate(gatecount &g)
{
   g.nand_gates += inputs.size()-1;
}

void ct_inverter::estimate(gatecount &g)
{
   g.nand_gates += inputs.size()-1;
}

void ct_adder::estimate(gatecount &g)
{
   g.nand_gates += width*7;
}

void ct_subtractor::estimate(gatecount &g)
{
   g.nand_gates += width*7;
}

void ct_cat::estimate(gatecount &g)
{
}

void ct_select::estimate(gatecount &g)
{
}

void ct_equal::estimate(gatecount &g)
{
    g.nand_gates += width + width - 1;
}

void ct_unequal::estimate(gatecount &g)
{
    g.nand_gates += width + width - 1;
}

void ct_port_in::estimate(gatecount &g)
{
}

void ct_port_out::estimate(gatecount &g)
{
}

void ct_port_inout::estimate(gatecount &g)
{
}

void ct_ram::estimate(gatecount &g)
{
    g.membits += (1<<size) * width;
}

void ct_par_join::estimate(gatecount &g)
{
    g.flipflops += inputs.size();

    g.nand_gates += inputs.size() + inputs.size()-1;
}
