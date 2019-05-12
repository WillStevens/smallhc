#include <iostream>

using std::cerr;
using std::endl;

#include "compilestate.h"
#include "hcast.h"
#include "widenum.h"


void typecheck_error(char *s)
{
	state.typecheckgood = false;

	cerr << s << endl;
}

unsigned requiredwidth(unsigned v)
{
	unsigned w = 0;

	while(v)
	{
		v = v>>1;
		w++;
	}

	return w;
}

type *make_cat_type(type *l, type *r)
{
	if (l->get_type() != tp_basic_type || r->get_type() != tp_basic_type)
	{
		typecheck_error("Uncattable types");
		return new basic_type(false,0);
	}
	else
	{
		if (l->get_sign() == r->get_sign())
		{
			return new basic_type(*(basic_type *)l,l->get_width() + r->get_width());
		}
		else
		{
			typecheck_error("signed/unsigned mismatch");
			return new basic_type(false,0);
		}
	}
}

type *make_cat_other_type(type *req, type *inf)
{
	if (req->get_type() != tp_basic_type || inf->get_type() != tp_basic_type)
	{
		typecheck_error("Uncattable types");
		return new basic_type(false,0);
	}
	else
	{
		if (req->get_sign() == inf->get_sign())
		{
			return new basic_type(*(basic_type *)req,req->get_width() - inf->get_width());
		}
		else
		{
			typecheck_error("signed/unsigned mismatch");
			return new basic_type(false,0);
		}
	}
}

bool can_be_represented(widenum wn, type *t)
{
	if (!widenum_positive(wn))
	{
		if (!t->get_sign())
		{
			return false;
		}
		else
		{
			return t->get_width() >= widenum_requiredwidth(wn);
		}
	}
	else
	{
		return t->get_width() - (t->get_sign()?1:0) >= widenum_requiredwidth(wn);
	}
	return true;
}

void program::typecheck(void)
{
	if (decls)
		decls->typecheck();

	body->typecheck();
}

void declarations::typecheck(void)
{
	for(vector<declaration *>::iterator i = decls.begin(); i!=decls.end(); i++)
	{
		(*i)->typecheck();
	}
}

void plain_declaration::typecheck(void)
{
	basic_type *bt = new basic_type(false,atoi(width->c_str()));
	archtype at = at_register;

	decls->typecheck(at,bt);
}

void signal_declaration::typecheck(void)
{
	basic_type *bt = new basic_type(false,atoi(width->c_str()));
	archtype at = at_signal;

	decls->typecheck(at,bt);
}

void extram_declaration::typecheck(void)
{
	basic_type *bt = new basic_type(false,atoi(width->c_str()));
	archtype at = at_extram;

	decls->typecheck(at,bt);
}

void ram_declaration::typecheck(void)
{
	basic_type *bt = new basic_type(false,atoi(width->c_str()));
	archtype at = at_ram;

	decls->typecheck(at,bt);
}

void port_declaration::typecheck(void)
{
	signals->typecheck();

	if (*porttype == "input")
	{
		if (signals->size() == 1)
		{
			identifier_expression *ie = dynamic_cast<identifier_expression *>((*signals)[0]);
						
			if (!ie || !state.symtab->exists(*ie->get_identifier()) || state.symtab->find(*ie->get_identifier()).get_archtype() != at_signal 
				|| !dynamic_cast<basic_type *>(state.symtab->find(*ie->get_identifier()).get_type()) )				
			{
				typecheck_error("input port expression must be signal");
			}
			else if (pins->size() != (*signals)[0]->get_type()->get_width())
			{
				typecheck_error("Wrong number of pins on input port");
			}
		}
		else
		{
			typecheck_error("input port requires 1 signal");
		}
	}
	else if (*porttype == "output")
	{
		if (signals->size() == 1)
		{
			if (pins->size() != (*signals)[0]->get_type()->get_width())
			{
				typecheck_error("Wrong number of pins on output port");
			}
		}
		else
		{
			typecheck_error("output port requires 1 signal");
		}
	}
	else if (*porttype == "bidir")
	{
		if (signals->size() == 3)
		{
			if (pins->size() != (*signals)[0]->get_type()->get_width())
			{
				typecheck_error("Wrong number of pins on bidir port");
			}
			else if ((*signals)[0]->get_type()->get_width() != (*signals)[1]->get_type()->get_width())
			{
				typecheck_error("width input and output signals is different in bidir port");
			}
			else if ((*signals)[2]->get_type()->get_width() != 1)
			{
				typecheck_error("width of output enable signal for bidir port must be 1");
			}
		}
		else
		{
			typecheck_error("bidir port requires 3 signal");
		}
	}
	else
	{
		typecheck_error("Invalid port type");
	}
}

void declarators::typecheck(archtype at, basic_type *bt)
{
	for(vector<declarator *>::iterator i = decls.begin(); i!=decls.end(); i++)
	{
		(*i)->typecheck(at,bt);
	}
}

void plain_declarator::typecheck(archtype at, basic_type *bt)
{
	state.symtab->add(*id,symtab_entry(bt,at));
}

void array_declarator::typecheck(archtype at, basic_type *bt)
{
	state.symtab->add(*id,symtab_entry(new array_type(bt,atoi(size->c_str())),at));
}

void compound_statement::typecheck(void)
{
	if (decls)
	{
		state.symtab = state.symtab->enterscope();

		decls->typecheck();
	}

	if (ss)
	{
		ss->typecheck();
	}

	if (decls)
	{
		state.symtab = state.symtab->exitscope();
	}
}

void statements::typecheck(void)
{
	for(vector<statement *>::iterator i = ss.begin(); i!=ss.end(); i++)
	{
		(*i)->typecheck();
	}
}

void seq_statement::typecheck(void)
{
	body->typecheck();
}

void par_statement::typecheck(void)
{
	body->typecheck();
}

void while_statement::typecheck(void)
{
	basic_type bt(false,1);
		
	test->typecheck(&bt);
	
	body->typecheck();
}

void if_statement::typecheck(void)
{
	basic_type bt(false,1);

	test->typecheck(&bt);
	
	trues->typecheck();

	if (falses)
	{
		falses->typecheck();
	}
}

void plain_assignment_statement::typecheck(void)
{
	if (state.symtab->exists(*id))
	{
		symtab_entry &se = state.symtab->find(*id);

		if (se.get_type()->get_type() == tp_basic_type)
		{			
			expr->typecheck(se.get_type());

		}
		else
		{
			typecheck_error("Invalid type on left of assignment");			
		}
	}
	else
	{
		typecheck_error("Undeclared identifier");
	}
}

void array_assignment_statement::typecheck(void)
{
	if (state.symtab->exists(*id))
	{
		symtab_entry &se = state.symtab->find(*id);

		if (se.get_type()->get_type() == tp_array_type)
		{
			basic_type bt(false,requiredwidth(se.get_type()->get_size()-1));

			index->typecheck(&bt);

			expr->typecheck(se.get_type()->get_subtype());

		}
		else
		{
			typecheck_error("Invalid type on left of array assignment");			
		}
	}
	else
	{
		typecheck_error("Undeclared identifier");
	}
}

void expression_list::typecheck(void)
{
	for(vector<expression *>::iterator i = es.begin(); i!=es.end();
		i++)
	{
		(*i)->typecheck(NULL);
	}
}

void identifier_expression::typecheck(type *req_tp)
{
	if (state.symtab->exists(*id))
	{
		symtab_entry &se = state.symtab->find(*id);

		tp = se.get_type();

		if (!equal_types(tp,req_tp))
		{
			typecheck_error("Type mismatch");
		}
	}
	else
	{
		typecheck_error("Undeclared identifier");
		tp = new basic_type(false,0);
	}
}

void number_expression::typecheck(type *req_tp)
{
	if (!req_tp)
	{
		typecheck_error("Cannot infer type");
		tp = new basic_type(false,0);
	}
	else
	{
		if (req_tp->get_type() == tp_basic_type)
		{
			if (!can_be_represented(num,req_tp))
			{
				typecheck_error("Value cannot be represented");
			}

			tp = new basic_type(*(basic_type *)req_tp);
		}
		else
		{
			typecheck_error("Numeric constant found in unexpected place");
			tp = new basic_type(false,0);
		}
	}
}


void add_expression::typecheck(type *req_tp)
{
	if (req_tp)
	{
		left->typecheck(req_tp);
		right->typecheck(req_tp);

		tp = left->get_type();
	}
	else if (left->can_infer_type())
	{
		left->typecheck(NULL);
		right->typecheck(left->get_type());

		tp = left->get_type();
	}
	else if (right->can_infer_type())
	{
		right->typecheck(NULL);
		left->typecheck(left->get_type());

		tp = left->get_type();
	}
	else
	{
		typecheck_error("Cannot infer type");
		tp = new basic_type(false,0);
	}
}

void subtract_expression::typecheck(type *req_tp)
{
	if (req_tp)
	{
		left->typecheck(req_tp);
		right->typecheck(req_tp);

		tp = left->get_type();
	}
	else if (left->can_infer_type())
	{
		left->typecheck(NULL);
		right->typecheck(left->get_type());

		tp = left->get_type();
	}
	else if (right->can_infer_type())
	{
		right->typecheck(NULL);
		left->typecheck(left->get_type());

		tp = left->get_type();
	}
	else
	{
		typecheck_error("Cannot infer type");
		tp = new basic_type(false,0);
	}
}

void cat_expression::typecheck(type *req_tp)
{
	expression *inf,*other;

	if (left->can_infer_type())
	{
		inf = left;
		other = right;
	}
	else if (right->can_infer_type())
	{
		inf = right;
		other = left;
	}
	else
	{
		typecheck_error("Cannot infer type");
		tp = new basic_type(false,0);
	}

	if (req_tp == NULL)
	{
		if (!other->can_infer_type())
		{
			typecheck_error("Cannot infer type");
			tp = new basic_type(false,0);
		}
		else
		{
			inf->typecheck(NULL);
			other->typecheck(NULL);

			tp = make_cat_type(left->get_type(),right->get_type());
		}
	}
	else
	{
		inf->typecheck(NULL);
		type *itp = make_cat_other_type(req_tp,inf->get_type());
		other->typecheck(itp);
		delete itp;

		tp = make_cat_type(left->get_type(),right->get_type());
	}

	if (!equal_types(tp,req_tp))
	{
		typecheck_error("Type mismatch");
	}
}

void bitand_expression::typecheck(type *req_tp)
{
	if (req_tp)
	{
		left->typecheck(req_tp);
		right->typecheck(req_tp);

		tp = left->get_type();
	}
	else if (left->can_infer_type())
	{
		left->typecheck(NULL);
		right->typecheck(left->get_type());

		tp = left->get_type();
	}
	else if (right->can_infer_type())
	{
		right->typecheck(NULL);
		left->typecheck(left->get_type());

		tp = left->get_type();
	}
	else
	{
		typecheck_error("Cannot infer type");
		tp = new basic_type(false,0);
	}
}

void bitor_expression::typecheck(type *req_tp)
{
	if (req_tp)
	{
		left->typecheck(req_tp);
		right->typecheck(req_tp);

		tp = left->get_type();
	}
	else if (left->can_infer_type())
	{
		left->typecheck(NULL);
		right->typecheck(left->get_type());

		tp = left->get_type();
	}
	else if (right->can_infer_type())
	{
		right->typecheck(NULL);
		left->typecheck(left->get_type());

		tp = left->get_type();
	}
	else
	{
		typecheck_error("Cannot infer type");
		tp = new basic_type(false,0);
	}
}

void bitxor_expression::typecheck(type *req_tp)
{
	if (req_tp)
	{
		left->typecheck(req_tp);
		right->typecheck(req_tp);

		tp = left->get_type();
	}
	else if (left->can_infer_type())
	{
		left->typecheck(NULL);
		right->typecheck(left->get_type());

		tp = left->get_type();
	}
	else if (right->can_infer_type())
	{
		right->typecheck(NULL);
		left->typecheck(left->get_type());

		tp = left->get_type();
	}
	else
	{
		typecheck_error("Cannot infer type");
		tp = new basic_type(false,0);
	}
}

void equal_expression::typecheck(type *req_tp)
{
	tp = new basic_type(false,1);

	if (!equal_types(req_tp,tp))
	{
		typecheck_error("Boolean expression where non-boolean value required");
	}
	else
	{
		if (left->can_infer_type())
		{
			left->typecheck(NULL);
			right->typecheck(left->get_type());
		}
		else if (right->can_infer_type())
		{
			right->typecheck(NULL);
			left->typecheck(right->get_type());
		}
		else
		{
			typecheck_error("Cannot infer type");
		}
	}

}

void unequal_expression::typecheck(type *req_tp)
{
	tp = new basic_type(false,1);

	if (!equal_types(req_tp,tp))
	{
		typecheck_error("Boolean expression where non-boolean value required");
	}
	else
	{
		if (left->can_infer_type())
		{
			left->typecheck(NULL);
			right->typecheck(left->get_type());
		}
		else if (right->can_infer_type())
		{
			right->typecheck(NULL);
			left->typecheck(right->get_type());
		}
		else
		{
			typecheck_error("Cannot infer type");
		}
	}
}

void booland_expression::typecheck(type *req_tp)
{
	tp = new basic_type(false,1);

	if (!equal_types(req_tp,tp))
	{
		typecheck_error("Boolean expression where non-boolean value required");
	}
	else
	{
		left->typecheck(tp);
		right->typecheck(tp);
	}
}

void boolor_expression::typecheck(type *req_tp)
{
	tp = new basic_type(false,1);

	if (!equal_types(req_tp,tp))
	{
		typecheck_error("Boolean expression where non-boolean value required");
	}
	else
	{
		left->typecheck(tp);
		right->typecheck(tp);
	}
}

void com_expression::typecheck(type *req_tp)
{
	expr->typecheck(req_tp);
}

void neg_expression::typecheck(type *req_tp)
{
	expr->typecheck(req_tp);
}

void select_expression::typecheck(type *req_tp)
{
	expr->typecheck(NULL);

	if (expr->get_type()->get_type() == tp_basic_type)
	{
		if (!numl->is_constant() || !numr->is_constant())
		{
			typecheck_error("Selection range must be constant");
			tp = new basic_type(false,0);
		}
		else
		{
			numl = numl->fold_constant();
			numr = numr->fold_constant();

			if (widenum_positive(numl->get_value())
				&& widenum_positive(numr->get_value()))
			{
				if (widenum_gte(numl->get_value(),numr->get_value()))
				{
					tp = new basic_type(*(basic_type *)(expr->get_type()),
						1+widenum_unsigned(widenum_subtract(numl->get_value(),numr->get_value())));	
					
					if (!equal_types(tp,req_tp))
					{
						typecheck_error("Type mismatch");
					}
				}
				else
				{
					typecheck_error("Negative range in selection");
					tp = new basic_type(false,0);
				}
			}
			else
			{
				typecheck_error("Invalid negative values in selection");
				tp = new basic_type(false,0);
			}
		}
	}
	else if (expr->get_type()->get_type() == tp_array_type)
	{
		if (!equal_types(expr->get_type()->get_subtype(),req_tp))
		{
			typecheck_error("Type mismatch");
		}

		if (numl != numr)
		{
			typecheck_error("Illegal select for array");
		}

		type *indextype = 
			new basic_type(false,
				widenum_requiredwidth(widenum(expr->get_type()->get_size()-1))
						);

		numl->typecheck(indextype);

		tp = expr->get_type()->get_subtype();
	}
	else
	{
		typecheck_error("Illegal type on left of selection");
		tp = new basic_type(false,0);
	}
}
