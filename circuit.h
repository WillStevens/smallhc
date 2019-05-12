#ifndef circuit_h
#define circuit_h

#include <set>
#include <list>
#include <vector>
#include <iostream>
#include <string>
#include <map>

using std::set;
using std::pair;
using std::vector;
using std::list;
using std::ostream;
using std::endl;
using std::map;
using std::string;

#include "macros.h"
#include "widenum.h"
#include "globaltypes.h"

class component;
class circuit;

typedef pair<component *,unsigned> signalconnection;

string generate_name(void);

class signalw
{
public:
	signalw(unsigned w) : valueset(false), width(w)
		{ name = generate_name(); }
	signalw(unsigned w, const widenum &val) 
		: valueset(true), value(val), width(w) 
		{ name = generate_name(); }

	void addsource(component *c, unsigned t)
	{sources.insert(pair<component *,unsigned>(c,t));}
	
	void addsink(component *c, unsigned t)
	{sinks.insert(pair<component *,unsigned>(c,t));}
	
	set<signalconnection> &get_sources(void) {return sources;}
	set<signalconnection> &get_sinks(void) {return sinks;}
	
	void graph_output(ostream &os);
	
	void vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins);
	void vhdl_output_name(ostream &os);
    void sim_output
        (ostream &propfuncs,
         ostream &updatefuncs,
         ostream &sigs,
         ostream &sigdeps_decl,
         ostream &sigdeps_def,
         ostream &update,
         ostream &init);
    void sim_output_name(ostream &os);

	string get_name(void) {return name;}
	
	unsigned get_width(void) {return width;}
	bool is_valueset(void) {return valueset;}
	widenum &get_value(void) {return value;}

	void replace(signalw *s);

	void setpos(list<signalw *>::iterator &p) {pos = p;}

	void removefromcircuit(circuit *c);
	void removesink(component *c, unsigned n)
		{
			sinks.erase(pair<component *,unsigned>(c,n));
		}
	bool removesource(component *c, unsigned n)
		{
			sources.erase(pair<component *,unsigned>(c,n));
			return sources.size() == 0;
		}
private:
	list<signalw *>::iterator pos;

	bool valueset;
	widenum value;
	unsigned width;

	set<signalconnection> sources;
	set<signalconnection> sinks;

	string name;
};

class gatecount
{
public:
    gatecount(void) : nand_gates(0), flipflops(0), membits(0) {}
    void reset(void) {nand_gates = 0; flipflops = 0; membits = 0;}

    void show(ostream &os)
    {
        os << "Gates:" << nand_gates << ", FFs:" << flipflops << ",Membits:" << membits << endl;
    }

    unsigned nand_gates;
    unsigned flipflops;
    unsigned membits;
};

class component
{
public:
	component(unsigned w) 
		: width(w) { name = generate_name(); }

	void add_input(signalw *s)
	{inputs.push_back(s);s->addsink(this,inputs.size()-1);}

	void add_output(signalw *s)
	{outputs.push_back(s);s->addsource(this,outputs.size()-1);}

	unsigned num_inputs(void) {return inputs.size();}
	unsigned num_outputs(void) {return outputs.size();}
	
	signalw *get_input(unsigned n) {return inputs[n];}
	signalw *get_output(unsigned n) {return outputs[n];}

	void replace_input(unsigned n, signalw *s) {inputs[n] = s;}
	
	virtual bool expand(circuit &c) = 0;
	
	virtual void graph_output(ostream &os) = 0;

	virtual void vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins) = 0;
	void vhdl_output_name(ostream &os);

    virtual void sim_output
        (ostream &propfuncs,
         ostream &updatefuncs,
         ostream &sigs,
         ostream &sigdeps_decl,
         ostream &sigdeps_def,
         ostream &update,
         ostream &init) = 0;
    void sim_output_name(ostream &os);

	virtual string get_name(void) = 0;

    virtual void estimate(gatecount &g) = 0;

	virtual bool duplicate_of(component *c) = 0;
	virtual bool optimise_redundant(circuit *c) = 0;
	virtual bool optimise_input(circuit *c) = 0;
	void replace(component *c);

	void setpos(list<component *>::iterator &p) {pos = p;}
	void removefromcircuit(circuit *c);
	
protected:
	list<component *>::iterator pos;

	unsigned width;

	vector<signalw *> inputs;
	vector<signalw *> outputs;

	string name;
};

// Multiplexer width w
// Output:
// Selected (width w)
// Input:
// Select (width 1)
// Signal (width w)

// OR gate width w
// Output:
// Result (width w)
// Input:
// Signal (width w)

// Register width w
// Output:
// Out (width w)
// Input:
// In (width w)
// Enable (width 1) - (if ommitted, set to 1)

// Ram width w, size s
// Output:
// Data out (width w)
// Input:
// Address in (width s)
// Data in (width w)
// WE (width 1)

#define COMPONENT_CLASS(comptype) \
class comptype : public component \
{ \
public: \
	comptype(unsigned w) : component(w) {} \
	bool expand(circuit &c); \
	void graph_output(ostream &os); \
	void vhdl_output(ostream &defs, ostream &ports, \
			ostream &sigs, ostream &body, ostream &pins); \
    void sim_output \
        (ostream &propfuncs, \
         ostream &updatefuncs, \
         ostream &sigs, \
         ostream &sigdeps_decl, \
         ostream &sigdeps_def, \
         ostream &update, \
         ostream &init); \
  bool duplicate_of(component *c); \
  bool optimise_redundant(circuit *c); \
  bool optimise_input(circuit *c); \
  void estimate(gatecount &g); \
  string get_name(void) {return string(STRINGIFY(comptype)) + string("_") + name;} \
};

#define COMPONENT_CLASS_DATA(comptype,CPARAMS,INIT,DATA) \
class comptype : public component \
{ \
public: \
	comptype(unsigned w,CPARAMS) : component(w), INIT {} \
	bool expand(circuit &c); \
	void graph_output(ostream &os); \
	void vhdl_output(ostream &defs, ostream &ports, \
			ostream &sigs, ostream &body, ostream &pins); \
    void sim_output \
        (ostream &propfuncs, \
         ostream &updatefuncs, \
         ostream &sigs, \
         ostream &sigdeps_decl, \
         ostream &sigdeps_def, \
         ostream &update, \
         ostream &init); \
	bool duplicate_of(component *c); \
    bool optimise_redundant(circuit *c); \
	bool optimise_input(circuit *c); \
    void estimate(gatecount &g); \
    string get_name(void) {return string(STRINGIFY(comptype)) + string("_") + name;} \
private: \
	DATA \
};

#define COMPONENT_CLASS_DATA2(comptype,CPARAMS1,CPARAMS2,INIT1,INIT2,DATA) \
class comptype : public component \
{ \
public: \
	comptype(unsigned w,CPARAMS1,CPARAMS2) : component(w), INIT1, INIT2 {} \
	bool expand(circuit &c); \
	void graph_output(ostream &os); \
	void vhdl_output(ostream &defs, ostream &ports, \
			ostream &sigs, ostream &body, ostream &pins); \
    void sim_output \
        (ostream &propfuncs, \
         ostream &updatefuncs, \
         ostream &sigs, \
         ostream &sigdeps_decl, \
         ostream &sigdeps_def, \
         ostream &update, \
         ostream &init); \
	bool duplicate_of(component *c); \
    bool optimise_redundant(circuit *c); \
	bool optimise_input(circuit *c); \
    void estimate(gatecount &g); \
    string get_name(void) {return string(STRINGIFY(comptype)) + string("_") + name;} \
private: \
	DATA \
};

COMPONENT_CLASS(ct_register)
COMPONENT_CLASS(ct_multiplexer)
COMPONENT_CLASS(ct_and_gate)
COMPONENT_CLASS(ct_or_gate)
COMPONENT_CLASS(ct_xor_gate)
COMPONENT_CLASS(ct_inverter)
COMPONENT_CLASS(ct_adder)
COMPONENT_CLASS(ct_subtractor)
COMPONENT_CLASS(ct_cat)
COMPONENT_CLASS(ct_equal)
COMPONENT_CLASS(ct_unequal)
COMPONENT_CLASS(ct_par_join)
COMPONENT_CLASS_DATA2(ct_select,unsigned tp,unsigned bt,top(tp),bottom(bt),unsigned top;unsigned bottom;)
COMPONENT_CLASS_DATA(ct_port_in,pin_list *ps,plist(ps),pin_list *plist;)
COMPONENT_CLASS_DATA(ct_port_out,pin_list *ps,plist(ps),pin_list *plist;)
COMPONENT_CLASS_DATA(ct_port_inout,pin_list *ps,plist(ps),pin_list *plist;)
COMPONENT_CLASS_DATA(ct_ram,unsigned s,size(s),unsigned size;)

class circuit
{
public:
	circuit(void) {}

	void sort(void);

	void add_component(component *c);

	void add_signal(signalw *s);

	signalw *get_constant_signal(unsigned w, const widenum &val);

	// Perform a single pass of the duplicate component 
	// optimisation
	// (returns true if at least one optimisation was made)
	bool optimise_dup_component(void);

	// Perform a single pass of the input optimisation
	// (returns true if at least one optimisation was made)
	bool optimise_input(void);
	
	// Perform a single pass of the redundant component optimisation
	// (returns true if at least one optimisation was made)
	bool optimise_redundant(void);

	void expand(void);

	void graph_output(ostream &os);	
	void vhdl_output(ostream &defs, ostream &ports,
			ostream &signals, ostream &body, ostream &pins);
    void sim_output
        (ostream &propfuncs,
         ostream &updatefuncs,
         ostream &signals,
         ostream &sigdeps_decl,
         ostream &sigdeps_def,
         ostream &update,
         ostream &init);

    void estimate(gatecount &g);

	void erasecomponent(list<component *>::iterator &i)
	{ comps.erase(i);}

	void erasesignal(list<signalw *>::iterator &i)
	{ sigs.erase(i);}
		
	bool check_circuit(void);
	
private:
	list<component *> comps;
	list<signalw *> sigs;
};

#endif