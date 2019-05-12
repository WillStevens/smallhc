#ifndef hcast_h
#define hcast_h

#include <string>
#include <vector>

using namespace std;

#include "circuit.h"
#include "symtab.h"
#include "widenum.h"
#include "globaltypes.h"

extern unsigned LineNumber;

class program;
class declarations;
class declaration;
class plain_declaration;
class signal_declaration;
class extram_declaration;
class ram_declaration;
class declarator;
class plain_declarator;
class array_declarator;
class declarators;
class statements;
class statement;
class compound_statement;
class seq_statement;
class par_statement;
class while_statement;
class if_statemenmt;
class assignment_statement;
class expression;
class expression_list;
class identifier_expression;
class number_expression;
class add_expression;
class subtract_expression;
class cat_expression;
class bitand_expression;
class bitor_expression;
class bitxor_expression;
class equal_expression;
class unequal_expression;
class booland_expression;
class boolor_expression;
class com_expression;
class neg_expression;
class select_expression;

class pathlengthinfo
{
public:
	enum {infinite = 0xffffffff};

	void IncMin(unsigned i)
		{if (min_length != infinite) min_length += i;}
	void IncMax(unsigned i) 
		{if (max_length != infinite) max_length += i;}
	void Increment(unsigned i) {IncMin(i);IncMax(i);}
	void SetMaxInfinite(void) {max_length = infinite;}

	unsigned min_length;
	unsigned max_length;
};


class program
{
public:
	program(declarations *d, compound_statement *b)
		: decls(d), body(b) {}

	void typecheck(void);

	void compile(signalw *active);
private:
	declarations *decls;	// may be NULL
	compound_statement *body;
};

class declarations
{
public:
	declarations(declaration *d) { decls.push_back(d); }

	void push_back(declaration *d) { decls.push_back(d); }

	void typecheck(void);

	void compile(void);
private:
	vector<declaration *> decls;
};


class declaration
{
public:
	virtual void typecheck(void) = 0;
	virtual void compile(void) = 0;

};

class plain_declaration : public declaration
{
public:
	plain_declaration(string *w, declarators *d)
		: width(w), decls(d) {}

	void typecheck(void);
	void compile(void);
private:
	string *width;
	declarators *decls;
};

class signal_declaration : public declaration
{
public:
	signal_declaration(string *w, declarators *d)
		: width(w), decls(d) {}

	void typecheck(void);
	void compile(void);
private:
	string *width;
	declarators *decls;
};

class extram_declaration : public declaration
{
public:
	extram_declaration(string *w, declarators *d)
		: width(w), decls(d) {}

	void typecheck(void);
	void compile(void);
private:
	string *width;
	declarators *decls;
};

class ram_declaration : public declaration
{
public:
	ram_declaration(string *w, declarators *d)
		: width(w), decls(d) {}

	void typecheck(void);
	void compile(void);
private:
	string *width;
	declarators *decls;
};

class port_declaration : public declaration
{
public:
	port_declaration(string *pt, expression_list *ss,
		pin_list *ps) : porttype(pt), signals(ss), pins(ps) {}

	void typecheck(void);
	void compile(void);
private:
	string *porttype;
	expression_list *signals;
	pin_list *pins;
};

class declarator
{
public:
	declarator(string *i) : id(i) {}
	virtual ~declarator() {}

	virtual void typecheck(archtype at, basic_type *bt) = 0;
	virtual void compile(void);
protected:
	string *id;
};

class plain_declarator : public declarator
{
public:
	plain_declarator(string *i) : declarator(i) {}

	void typecheck(archtype at, basic_type *bt);
};

class array_declarator : public declarator
{
public:
	array_declarator(string *i, string *s) : declarator(i), size(s) {}

	void typecheck(archtype at, basic_type *bt);
private:
	string *size;
};

class declarators
{
public:
	declarators(declarator *d) { decls.push_back(d); }

	void push_back(declarator *d) { decls.push_back(d); }

	void typecheck(archtype at, basic_type *bt);
	void compile(void);
private:
	vector<declarator *> decls;
};

class statements
{
public:
	statements(statement *d) { ss.push_back(d); }

	void push_back(statement *d) { ss.push_back(d); }

	void typecheck(void);
	signalw *compile(signalw *active,pathlengthinfo &pathlength);
	signalw *compile_par(signalw *active,pathlengthinfo &pathlength);
private:
	vector<statement *> ss;
};

class statement
{
public:
	virtual void typecheck(void) = 0;
	virtual signalw *compile(signalw *active,pathlengthinfo &pathlength) = 0;

};

class delay_statement : public statement
{
public:
	delay_statement(void) {}

	void typecheck(void) {}
	signalw *compile(signalw *active,pathlengthinfo &pathlength);
};

class compound_statement : public statement
{
public:
	compound_statement(declarations *d, statements *s)
		: decls(d), ss(s) {}

	void typecheck(void);
	signalw *compile(signalw *active,pathlengthinfo &pathlength);
	signalw *compile_par(signalw *active,pathlengthinfo &pathlength);
private:
	declarations *decls;
	statements *ss;
};

class seq_statement : public statement
{
public:
	seq_statement(compound_statement *b)
		: body(b) {}

	void typecheck(void);
	signalw *compile(signalw *active,pathlengthinfo &pathlength);
private:
	compound_statement *body;
};

class par_statement : public statement
{
public:
	par_statement(compound_statement *b)
		: body(b) {}

	void typecheck(void);
	signalw *compile(signalw *active,pathlengthinfo &pathlength);
private:
	compound_statement *body;
};

class while_statement : public statement
{
public:
	while_statement(expression *e, statement *b)
		: test(e), body(b) {}

	void typecheck(void);
	signalw *compile(signalw *active,pathlengthinfo &pathlength);
private:
	expression *test;
	statement *body;
};

class if_statement : public statement
{
public:
	if_statement(expression *t, statement *tr, statement *f)
		: test(t), trues(tr), falses(f) {}

	void typecheck(void);
	signalw *compile(signalw *active,pathlengthinfo &pathlength);
private:
	expression *test;
	statement *trues;
	statement *falses;
};

class plain_assignment_statement : public statement
{
public:
	plain_assignment_statement(string *i, expression *e)
		: id(i), expr(e) {}
	
	void typecheck(void);
	signalw *compile(signalw *active,pathlengthinfo &pathlength);
private:
	string *id;
	expression *expr;
};

class array_assignment_statement : public statement
{
public:
	array_assignment_statement(string *i, expression *ind, expression *e)
		: id(i), index(ind), expr(e) {}
	
	void typecheck(void);
	signalw *compile(signalw *active,pathlengthinfo &pathlength);
private:
	string *id;
	expression *index;
	expression *expr;
};

class expression
{
public:
	virtual void typecheck(type *req_tp) = 0;
	virtual signalw *compile(signalw *active) = 0;
	
	type *get_type(void) {return tp;}
	virtual bool can_infer_type(void) {return false;}

	virtual bool is_constant(void) {return false;}
	virtual expression *fold_constant(void) {return this;}
	virtual widenum get_value(void) {return widenum(0);}
protected:
	type *tp;
};

class expression_list
{
public:
	expression_list(expression *e) { es.push_back(e); }

	void push_back(expression *e) { es.push_back(e); }

	void typecheck(void);

	unsigned size(void) {return es.size();}

	expression *operator[](unsigned n) {return es[n];}
private:
	vector<expression *> es;
};

class identifier_expression : public expression
{
public:
	identifier_expression(string *i)
		: id(i) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return true;}

	string *get_identifier(void) {return id;}
private:
	string *id;
};

class number_expression : public expression
{
public:
	number_expression(string *n) : num(*n) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);
	
	bool is_constant(void) {return true;}
	widenum get_value(void) {return num;}
private:
	widenum num;
};

class add_expression : public expression
{
public:
	add_expression(expression *l, expression *r)
		: left(l), right(r) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return left->can_infer_type() || right->can_infer_type();}
private:
	expression *left,*right;
};

class subtract_expression : public expression
{
public:
	subtract_expression(expression *l, expression *r)
		: left(l), right(r) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return left->can_infer_type() || right->can_infer_type();}
private:
	expression *left,*right;
};

class cat_expression : public expression
{
public:
	cat_expression(expression *l, expression *r)
		: left(l), right(r) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return left->can_infer_type() && right->can_infer_type();}
private:
	expression *left,*right;
};

class bitand_expression : public expression
{
public:
	bitand_expression(expression *l, expression *r)
		: left(l), right(r) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return left->can_infer_type() || right->can_infer_type();}
private:
	expression *left,*right;
};

class bitor_expression : public expression
{
public:
	bitor_expression(expression *l, expression *r)
		: left(l), right(r) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return left->can_infer_type() || right->can_infer_type();}
private:
	expression *left,*right;
};

class bitxor_expression : public expression
{
public:
	bitxor_expression(expression *l, expression *r)
		: left(l), right(r) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return left->can_infer_type() || right->can_infer_type();}
private:
	expression *left,*right;
};

class equal_expression : public expression
{
public:
	equal_expression(expression *l, expression *r)
		: left(l), right(r) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return true;}
private:
	expression *left,*right;
};

class unequal_expression : public expression
{
public:
	unequal_expression(expression *l, expression *r)
		: left(l), right(r) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return true;}
private:
	expression *left,*right;
};

class booland_expression : public expression
{
public:
	booland_expression(expression *l, expression *r)
		: left(l), right(r) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return true;}
private:
	expression *left,*right;
};

class boolor_expression : public expression
{
public:
	boolor_expression(expression *l, expression *r)
		: left(l), right(r) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return true;}
private:
	expression *left,*right;
};

class com_expression : public expression
{
public:
	com_expression(expression *e) : expr(e) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return expr->can_infer_type();}
private:
	expression *expr;
};

class neg_expression : public expression
{
public:
	neg_expression(expression *e) : expr(e) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return expr->can_infer_type();}
private:
	expression *expr;
};

class select_expression : public expression
{
public:
	select_expression(expression *e, expression *l, expression *r)
		: expr(e), numl(l), numr(r) {}

	void typecheck(type *req_tp);
	signalw *compile(signalw *active);

	bool can_infer_type(void) {return expr->can_infer_type();}
private:
	expression *expr;
	expression *numl,*numr;
};

#endif