#include <iostream>

using std::cerr;
using std::endl;

#include "hcast.h"
#include "compilestate.h"
#include "macros.h"

void compile_error(char *s)
{
	state.compilegood = false;

	cerr << s << endl;
}

void program::compile(signalw *active)
{
	if (decls)
		decls->compile();

	if (body)
	{
		pathlengthinfo pli;
		body->compile(active,pli);
	}
}

void declarations::compile(void)
{
	for(vector<declaration *>::iterator i = decls.begin();
		i!=decls.end(); i++)
	{
		(*i)->compile();
	}
}


void plain_declaration::compile(void)
{
	decls->compile();
}

void signal_declaration::compile(void)
{
	decls->compile();
}

void extram_declaration::compile(void)
{
	decls->compile();
}

void ram_declaration::compile(void)
{
	decls->compile();
}

void port_declaration::compile(void)
{
	if (*porttype == "input")
	{
		symtab_entry &se = state.symtab->find(
			*dynamic_cast<identifier_expression *>((*signals)[0])->get_identifier()
											);
		unsigned w = se.get_type()->get_width();

		component *mux = se.get_component(0);
		component *port = new ct_port_in(w,pins);
		signalw *insig = new signalw(w);

		port->add_output(insig);

		signalw *dummyhigh = state.c.get_constant_signal(1,widenum(1));

		mux->add_input(dummyhigh);
	
		mux->add_input(insig);

		state.c.add_component(port);
		state.c.add_signal(insig);
	}
	else if (*porttype == "output")
	{
		unsigned w = (*signals)[0]->get_type()->get_width();

		signalw *dummyhigh = state.c.get_constant_signal(1,widenum(1));
		signalw *outsig = (*signals)[0]->compile(dummyhigh);

		component *port = new ct_port_out(w,pins);

		port->add_input(outsig);

		state.c.add_component(port);
	}
	else if (*porttype == "bidir")
	{
		unsigned w = (*signals)[0]->get_type()->get_width();

		signalw *dummyhigh = state.c.get_constant_signal(1,widenum(1));
		signalw *outsig = (*signals)[1]->compile(dummyhigh);
        signalw *oesig = (*signals)[2]->compile(dummyhigh);

		component *port = new ct_port_inout(w,pins);

		port->add_input(outsig);
        port->add_input(oesig);

		symtab_entry &se = state.symtab->find(
			*dynamic_cast<identifier_expression *>((*signals)[0])->get_identifier()
											);
		component *mux = se.get_component(0);
		signalw *insig = new signalw(w);

		port->add_output(insig);

		mux->add_input(dummyhigh);
	
		mux->add_input(insig);

		state.c.add_component(port);
		state.c.add_signal(insig);
	}
}

void declarator::compile(void)
{
	symtab_entry &se = state.symtab->find(*id);
	archtype at;

	switch(at = se.get_archtype())
	{
	case at_register:
	case at_signal:
		if (se.get_type()->get_type() != tp_basic_type)
		{
			compile_error("signals and registers must be of basic types");
		}
		else
		{

			unsigned w = se.get_type()->get_width();

			component *mux = new ct_multiplexer(w);
			signalw *muxout = new signalw(w);
			
			mux->add_output(muxout);

			se.push_component(mux);
			state.c.add_component(mux);
			state.c.add_signal(muxout);

			if (at==at_register)
			{
				component *orgate = new ct_or_gate(1);			
				component *reg = new ct_register(w);
				signalw *regout = new signalw(w);
				signalw *regen = new signalw(1);

				orgate->add_output(regen);

				reg->add_output(regout);

				reg->add_input(muxout);
				
				reg->add_input(regen);

				se.push_component(orgate);
				state.c.add_component(orgate);
				state.c.add_component(reg);
				state.c.add_signal(regout);
				state.c.add_signal(regen);

				se.push_signal(regout);
			}
			else
			{
				se.push_signal(muxout);
			}
		}
		break;
	case at_ram:
	case at_extram:
		if (se.get_type()->get_type() != tp_array_type)
		{
			compile_error("rams and extrams must be of array type");
		}
		else
		{
			if (se.get_type()->get_subtype()->get_type() != tp_basic_type)
			{
				compile_error("rams and extrams must be one-dimensional");
			}
			else
			{
				unsigned w = se.get_type()->get_width();
				unsigned s = widenum_requiredwidth(widenum(se.get_type()->get_size()-1));

				if (at == at_ram)
				{
					component *ram = new ct_ram(w,s);

					signalw *datain = new signalw(w);
					signalw *dataout = new signalw(w);
					signalw *addrin = new signalw(s);
					signalw *writeenable = new signalw(1);

					ram->add_input(addrin);

					ram->add_input(datain);

					ram->add_input(writeenable);

					ram->add_output(dataout);

					component *mux = new ct_multiplexer(w);
					
					mux->add_output(datain);

					component *addrmux = new ct_multiplexer(s);
					
					addrmux->add_output(addrin);

					component *orgate = new ct_or_gate(1);			

					orgate->add_output(writeenable);
					
					se.push_component(mux);
					se.push_component(orgate);
					se.push_component(ram);
					se.push_component(addrmux);
					se.push_signal(dataout);

					state.c.add_component(ram);
					state.c.add_component(mux);
					state.c.add_component(addrmux);
					state.c.add_component(orgate);
					state.c.add_signal(datain);
					state.c.add_signal(dataout);
					state.c.add_signal(addrin);
					state.c.add_signal(writeenable);

				}
				else // at == at_extram
				{
				}
			}
		}
	}
}

void declarators::compile(void)
{
	for(vector<declarator *>::iterator i = decls.begin();
		i!=decls.end(); i++)
	{
		(*i)->compile();
	}
}


signalw *statements::compile(signalw *active,pathlengthinfo &pathlength)
{
	for(vector<statement *>::iterator i = ss.begin();
		i!=ss.end(); i++)
	{
		active = (*i)->compile(active,pathlength);
	}

	return active;
}

signalw *statements::compile_par(signalw *active,pathlengthinfo &pathlength)
{
	component *joinblock = new ct_par_join(1);
	signalw *joinblockout = new signalw(1);

	state.c.add_component(joinblock);
	state.c.add_signal(joinblockout);

	joinblock->add_output(joinblockout);

	vector<signalw *> outsigs;
	vector<pathlengthinfo *> pathlengths;

	unsigned pathmin=0,pathmax=0;
	signalw *maxsig;

	{for(vector<statement *>::iterator i = ss.begin();
		i!=ss.end(); i++)
	{
		pathlengths.push_back(new pathlengthinfo());

		outsigs.push_back((*i)->compile(active,*pathlengths.back() ));

		if (pathlengths.back()->min_length > pathmin)
			pathmin = pathlengths.back()->min_length;

		if (pathlengths.back()->max_length > pathmax)
		{
			maxsig = outsigs.back();
			pathmax = pathlengths.back()->max_length;
		}
	}}

	joinblock->add_input(maxsig);
		
	vector<pathlengthinfo *>::iterator j = pathlengths.begin();
	{for(vector<signalw *>::iterator i = outsigs.begin();
		i!=outsigs.end(); i++,j++)
	{
		if ((*i) != maxsig && (*j)->max_length>pathmin)
		{
			joinblock->add_input(*i);
		}

		delete (*j);
	}}


	pathlength.IncMin(pathmin);
	pathlength.IncMax(pathmax);

	return joinblockout;
}

signalw *delay_statement::compile(signalw *active,pathlengthinfo &pathlength)
{
	component *reg = new ct_register(1);
	signalw *regout = new signalw(1);

	reg->add_input(active);

	reg->add_output(regout);

	state.c.add_component(reg);
	state.c.add_signal(regout);

	pathlength.Increment(1);

	return regout;
}

signalw *compound_statement::compile(signalw *active,pathlengthinfo &pathlength)
{
	if (decls)
		decls->compile();

	return ss->compile(active,pathlength);
}

signalw *compound_statement::compile_par(signalw *active,pathlengthinfo &pathlength)
{
	if (decls)
		decls->compile();

	return ss->compile_par(active,pathlength);
}

signalw *seq_statement::compile(signalw *active,pathlengthinfo &pathlength)
{
	return body->compile(active,pathlength);
}

signalw *par_statement::compile(signalw *active,pathlengthinfo &pathlength)
{
	return body->compile_par(active,pathlength);
}

signalw *while_statement::compile(signalw *active,pathlengthinfo &pathlength)
{
	component *controlor = new ct_or_gate(1);
	component *trueand = new ct_and_gate(1);
	component *falseand = new ct_and_gate(1);
	component *falsenot = new ct_inverter(1);

	signalw *controlorout = new signalw(1);
	signalw *falsenotout = new signalw(1);
	signalw *trueandout = new signalw(1);
	signalw *falseandout = new signalw(1);

	pathlengthinfo bodypathlength;

	signalw *test_sig = test->compile(controlorout);
	signalw *loop_sig = body->compile(trueandout,bodypathlength);

	controlor->add_input(loop_sig);
	controlor->add_input(active);
	controlor->add_output(controlorout);

	trueand->add_input(controlorout);
	trueand->add_input(test_sig);
	trueand->add_output(trueandout);

	falsenot->add_input(test_sig);
	falsenot->add_output(falsenotout);

	falseand->add_input(controlorout);
	falseand->add_input(falsenotout);
	falseand->add_output(falseandout);

	state.c.add_component(trueand);
	state.c.add_component(falseand);
	state.c.add_component(falsenot);
	state.c.add_component(controlor);

	state.c.add_signal(controlorout);
	state.c.add_signal(falsenotout);
	state.c.add_signal(trueandout);
	state.c.add_signal(falseandout);

	pathlength.SetMaxInfinite();

	return falseandout;
}

signalw *if_statement::compile(signalw *active,pathlengthinfo &pathlength)
{
	component *controlor = new ct_or_gate(1);
	component *trueand = new ct_and_gate(1);
	component *falseand = new ct_and_gate(1);
	component *falsenot = new ct_inverter(1);

	signalw *falsenotout = new signalw(1);
	signalw *trueandout = new signalw(1);
	signalw *falseandout = new signalw(1);
	signalw *controlorout = new signalw(1);

	pathlengthinfo falsepathlength,truepathlength;

	signalw *test_sig = test->compile(active);
	signalw *true_sig = trues->compile(trueandout,truepathlength);
	
	if (falses)
	{
	  signalw *false_sig = falses->compile(falseandout,falsepathlength);
  	  controlor->add_input(false_sig);
	}

	controlor->add_input(true_sig);
	controlor->add_output(controlorout);

	trueand->add_input(active);
	trueand->add_input(test_sig);
	trueand->add_output(trueandout);

	falsenot->add_input(test_sig);
	falsenot->add_output(falsenotout);

	falseand->add_input(active);
	falseand->add_input(falsenotout);
	falseand->add_output(falseandout);

	state.c.add_component(trueand);
	state.c.add_component(falseand);
	state.c.add_component(falsenot);
	state.c.add_component(controlor);

	state.c.add_signal(controlorout);
	state.c.add_signal(falsenotout);
	state.c.add_signal(trueandout);
	state.c.add_signal(falseandout);

	pathlength.IncMin(min(falsepathlength.min_length,
						  truepathlength.min_length));

	pathlength.IncMax(max(falsepathlength.max_length,
					  truepathlength.max_length));

	return controlorout;
}

signalw *plain_assignment_statement::compile(signalw *active,pathlengthinfo &pathlength)
{
	symtab_entry &se = state.symtab->find(*id);
	
	archtype at = se.get_archtype();

	component *reg = new ct_register(1);
	signalw *regout = new signalw(1);

	reg->add_input(active);

	reg->add_output(regout);

	component *mux = se.get_component(0);

	signalw *exprsig = expr->compile(active);

	mux->add_input(active);

	mux->add_input(exprsig);

	if (at == at_register)
	{
		component *orgate = se.get_component(1);

		orgate->add_input(active);
	}

	state.c.add_component(reg);
	state.c.add_signal(regout);

	pathlength.Increment(1);

	return regout;
}

signalw *array_assignment_statement::compile(signalw *active,pathlengthinfo &pathlength)
{
    // Generate a 2-cycle ram write, with WEN going low for the first cycle,
    // but data and address remaining stable for the whole lot.
	component *reg = new ct_register(1);
    component *reg2 = new ct_register(1);
	signalw *regout = new signalw(1);
    signalw *regout2 = new signalw(1);

	state.c.add_component(reg);
	state.c.add_signal(regout);
    state.c.add_component(reg2);
    state.c.add_signal(regout2);

	reg->add_input(active);
	reg->add_output(regout);
    reg2->add_input(regout);
    reg2->add_output(regout2);

	symtab_entry &se = state.symtab->find(*id);

	component *addrmux = se.get_component(3);

    signalw *indexexpr = index->compile(active);

	addrmux->add_input(active);
	addrmux->add_input(indexexpr);
	addrmux->add_input(regout);
	addrmux->add_input(indexexpr);

	component *datamux = se.get_component(0);
	component *dataor = se.get_component(1);

	dataor->add_input(active);

    signalw *dataexpr = expr->compile(active);
	datamux->add_input(active);
	datamux->add_input(dataexpr);
	datamux->add_input(regout);
	datamux->add_input(dataexpr);

	pathlength.Increment(2);

	return regout2;

#if 0
	component *reg = new ct_register(1);
	signal *regout = new signal(1);

	state.c.add_component(reg);
	state.c.add_signal(regout);

	reg->add_input(active);
	reg->add_output(regout);

	symtab_entry &se = state.symtab->find(*id);

	component *addrmux = se.get_component(3);

	addrmux->add_input(active);
	addrmux->add_input(index->compile(active));

	component *datamux = se.get_component(0);
	component *dataor = se.get_component(1);

	dataor->add_input(active);

	datamux->add_input(active);
	datamux->add_input(expr->compile(active));

	pathlength.Increment(1);

	return regout;
#endif
}


signalw *identifier_expression::compile(signalw *active)
{
	symtab_entry &se = state.symtab->find(*id);
	
	return se.get_signal(0);
}

signalw *number_expression::compile(signalw *active)
{
	signalw *rval = state.c.get_constant_signal(tp->get_width(),num);

	return rval;
}

signalw *add_expression::compile(signalw *active)
{
	component *adder = new ct_adder(tp->get_width());
	signalw *adderout = new signalw(tp->get_width());

	adder->add_input(left->compile(active));
	adder->add_input(right->compile(active));

	adder->add_output(adderout);

	state.c.add_component(adder);

	state.c.add_signal(adderout);

	return adderout;
}

signalw *subtract_expression::compile(signalw *active)
{
	return NULL;
}

signalw *cat_expression::compile(signalw *active)
{
	component *cat = new ct_cat(tp->get_width());
	signalw *catout = new signalw(tp->get_width());

	cat->add_input(left->compile(active));
	cat->add_input(right->compile(active));

	cat->add_output(catout);

	state.c.add_component(cat);
	state.c.add_signal(catout);

	return catout;
}

signalw *bitand_expression::compile(signalw *active)
{
	component *andgate = new ct_and_gate(tp->get_width());
	signalw *andgateout = new signalw(tp->get_width());

	andgate->add_input(left->compile(active));
	andgate->add_input(right->compile(active));

	andgate->add_output(andgateout);

	state.c.add_component(andgate);

	state.c.add_signal(andgateout);

	return andgateout;
}

signalw *bitor_expression::compile(signalw *active)
{
	component *orgate = new ct_or_gate(tp->get_width());
	signalw *orgateout = new signalw(tp->get_width());

	orgate->add_input(left->compile(active));
	orgate->add_input(right->compile(active));

	orgate->add_output(orgateout);

	state.c.add_component(orgate);

	state.c.add_signal(orgateout);

	return orgateout;
}

signalw *bitxor_expression::compile(signalw *active)
{
	component *xorgate = new ct_xor_gate(tp->get_width());
	signalw *xorgateout = new signalw(tp->get_width());

	xorgate->add_input(left->compile(active));
	xorgate->add_input(right->compile(active));

	xorgate->add_output(xorgateout);

	state.c.add_component(xorgate);

	state.c.add_signal(xorgateout);

	return xorgateout;
}

signalw *equal_expression::compile(signalw *active)
{
	component *comp = new ct_equal(tp->get_width());
	signalw *equalout = new signalw(1);

	comp->add_input(left->compile(active));
	comp->add_input(right->compile(active));
	comp->add_output(equalout);

	state.c.add_component(comp);
	state.c.add_signal(equalout);

	return equalout;
}

signalw *unequal_expression::compile(signalw *active)
{
	component *comp = new ct_unequal(tp->get_width());
	signalw *unequalout = new signalw(1);

	comp->add_input(left->compile(active));
	comp->add_input(right->compile(active));
	comp->add_output(unequalout);

	state.c.add_component(comp);
	state.c.add_signal(unequalout);

	return unequalout;
}

signalw *booland_expression::compile(signalw *active)
{
    component *andgate = new ct_and_gate(1);
    signalw *andout = new signalw(1);

    andgate->add_output(andout);
    andgate->add_input(left->compile(active));
    andgate->add_input(right->compile(active));

    state.c.add_component(andgate);
    state.c.add_signal(andout);

    return andout;
}

signalw *boolor_expression::compile(signalw *active)
{
	return NULL;
}

signalw *com_expression::compile(signalw *active)
{
	return NULL;
}

signalw *neg_expression::compile(signalw *active)
{
	return NULL;
}

signalw *select_expression::compile(signalw *active)
{
	identifier_expression *ie =
		dynamic_cast<identifier_expression *>(expr);

	if (ie)
	{
		symtab_entry &se = state.symtab->find(*(ie->get_identifier()));

		if (dynamic_cast<array_type *>(se.get_type()))
		{
			if (numl != numr)
			{
				compile_error("Invalid ram selection");
				return NULL;
			}
			else
			{
				component *addrmux = se.get_component(3);

				addrmux->add_input(active);

				addrmux->add_input(numl->compile(active));

				return expr->compile(active);
			}
		}
	}

	unsigned leftval = widenum_unsigned(dynamic_cast<number_expression *>(numl)->get_value());
	unsigned rightval = widenum_unsigned(dynamic_cast<number_expression *>(numr)->get_value());

	component *select = new ct_select(tp->get_width(),leftval,rightval);
	signalw *selectout = new signalw(tp->get_width());

	select->add_input(expr->compile(active));
	select->add_output(selectout);

	state.c.add_component(select);
	state.c.add_signal(selectout);

	return selectout;
}


