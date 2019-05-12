#include <iostream>

using namespace std;

#include "circuit.h"

// The files produced during simulation output are:
// propfuncs - derive output values from inputs of combinatorial components, and schedule call of downstream propagation functions
// updatefuncs - function for updating stored value of register
// signals - declare variables corresponding to signals
// sigdeps_decl - declare arrays of propagation functions for components sinks of each net 
// sigdeps_def - define arrays declared in sigdeps_decl
// update - set outputs of register: first pass calls updatefuncs, second pass sets value of output net
// init - set initial values of all variables corresponding to signals

void circuit::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &signals,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
	{for(list<signalw *>::iterator i = sigs.begin(); 
		i != sigs.end(); i++)
	{
		(*i)->sim_output
            (propfuncs,
             updatefuncs,
             signals,
             sigdeps_decl,
             sigdeps_def,
             update,
             init);
	}}

	{for(list<component *>::iterator i = comps.begin(); 
		i != comps.end(); i++)
	{
		(*i)->sim_output
            (propfuncs,
             updatefuncs,
             signals,
             sigdeps_decl,
             sigdeps_def,
             update,
             init);
	}}
}

void signalw::sim_output_name(ostream &os)
{
	os << name;
}

void signalw::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
	// Simultation outputs needed for a signal are:
	// - Declaration and definitions of array of propagation functions for components that are sinks of this signal.
	// - Declaration of variable corresponding to signal
	// - Initialisation of variable corresponding to signal
	
    // declare an array of pointers to component functions that
    // are connected to this net.
    sigdeps_decl << "static PROPAGATION_PMEMB ";
    sim_output_name(sigdeps_decl);
    sigdeps_decl << "_OUTPUTS[" << sinks.size()+1 << "];" << endl;

    
    sigdeps_def << "PROPAGATION_PMEMB SIMCLASS::";
    sim_output_name(sigdeps_def);
    sigdeps_def << "_OUTPUTS[" << sinks.size()+1 << "] = {" << endl;

    for(set<signalconnection>::iterator i = sinks.begin();
        i != sinks.end(); i++)
    {
        sigdeps_def << "&SIMCLASS::";
        (*i).first->sim_output_name(sigdeps_def);
        sigdeps_def << "_PROPAGATE," << endl;
    }
    
    sigdeps_def << "0};" << endl;

    sigs << "unsigned ";
	sim_output_name(sigs);
    sigs << ";";
	sigs << endl;

	sim_output_name(init);
	init << " = ";
	if (valueset)
	{
		widenum_output(value,init);
	}
    else
    {
        init << 0;
    }

	init << ";" << endl;
	
	if (valueset)
	{
		init << "SCHEDULE(";
        sim_output_name(init);
        init << "_OUTPUTS);" << endl;
	}
}

void component::sim_output_name(ostream &os)
{
	os << name;
}

void ct_register::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    // declare a variable for the registered value
    sigs << "unsigned ";
	outputs[0]->sim_output_name(sigs);
    sigs << "_REG;";
	sigs << endl;

	outputs[0]->sim_output_name(init);
    init << "_REG = 0;";
	init << endl;
	
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{}" << endl;

    updatefuncs << "void ";
    sim_output_name(updatefuncs);
    updatefuncs << "_UPDATE(void)" << endl;
    updatefuncs << "{" << endl;

	// If 2 inputs, then the second one is the enable signal
    if (inputs.size() == 2)
    {
        updatefuncs << "  if (";    
        inputs[1]->sim_output_name(updatefuncs);
    }
    else
    {
        updatefuncs << "  if (1";
    }

    updatefuncs << " && ";
    outputs[0]->sim_output_name(updatefuncs);
    updatefuncs << "_REG != ";
    inputs[0]->sim_output_name(updatefuncs);
    updatefuncs << ")" << endl;
    updatefuncs << " {" << endl;
    updatefuncs << "    ";
    outputs[0]->sim_output_name(updatefuncs);
    updatefuncs << "_REG = ";
    inputs[0]->sim_output_name(updatefuncs);
    updatefuncs << ";" << endl;

    updatefuncs << "  SCHEDULE(";
    outputs[0]->sim_output_name(updatefuncs);
    updatefuncs << "_OUTPUTS);" << endl;

    updatefuncs << "  }" << endl;

    updatefuncs << "}" << endl; 

    update << "if (pass == 0)" << endl;
    sim_output_name(update);
    update << "_UPDATE();" << endl;
    update << "else" << endl;
    outputs[0]->sim_output_name(update);
    update << " = ";
    outputs[0]->sim_output_name(update);
    update << "_REG;" << endl;


    init << "SCHEDULE(";
    outputs[0]->sim_output_name(init);
    init << "_OUTPUTS);" << endl;
}

void ct_or_gate::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{" << endl;

    propfuncs << "  ";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << " = ";

    if (inputs.size())
    {
        for (vector<signalw *>::iterator i = inputs.begin(); i!=inputs.end(); i++)
	    {
          if (i!=inputs.begin())
            propfuncs << "|";

          (*i)->sim_output_name(propfuncs);
	    }
    }
    else
    {
        propfuncs << 0;
    }

    propfuncs << ";" << endl;

    propfuncs << "  SCHEDULE(";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << "_OUTPUTS);" << endl;

    propfuncs << "}" << endl;
}

void ct_xor_gate::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{" << endl;

    propfuncs << "  ";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << " = ";

    if (inputs.size())
    {
        for (vector<signalw *>::iterator i = inputs.begin(); i!=inputs.end(); i++)
	    {
          if (i!=inputs.begin())
            propfuncs << "^";

          (*i)->sim_output_name(propfuncs);
	    }
    }
    else
    {
        propfuncs << 0;
    }

    propfuncs << ";" << endl;

    propfuncs << "  SCHEDULE(";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << "_OUTPUTS);" << endl;

    propfuncs << "}" << endl;
}

void ct_multiplexer::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{" << endl;

    for (vector<signalw *>::iterator i = inputs.begin(); i!=inputs.end();)
	{
        propfuncs << "  if(";
        (*(i++))->sim_output_name(propfuncs);
        propfuncs << "==1)" << endl;
        propfuncs << "  {" << endl;
        propfuncs << "    ";
        outputs[0]->sim_output_name(propfuncs);
        propfuncs << " = ";
        (*(i++))->sim_output_name(propfuncs);
        propfuncs << ";";
        propfuncs << "  SCHEDULE(";
        outputs[0]->sim_output_name(propfuncs);
        propfuncs << "_OUTPUTS);" << endl;

        propfuncs << "  }" << endl;
	}
    
    propfuncs << "}" << endl;
}

void ct_inverter::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{" << endl;

    propfuncs << "  " << endl;
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << " = !";
    inputs[0]->sim_output_name(propfuncs);
    propfuncs << ";" << endl;

    propfuncs << "  SCHEDULE(";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << "_OUTPUTS);" << endl;

    propfuncs << "}" << endl;

}

void ct_and_gate::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{" << endl;

    propfuncs << "  ";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << " = ";

    if (inputs.size())
    {
        for (vector<signalw *>::iterator i = inputs.begin(); i!=inputs.end(); i++)
	    {
          if (i!=inputs.begin())
            propfuncs << "&";

          (*i)->sim_output_name(propfuncs);
	    }
    }
    else
    {
        propfuncs << 0;
    }

    propfuncs << ";" << endl;

    propfuncs << "  SCHEDULE(";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << "_OUTPUTS);" << endl;

    propfuncs << "}" << endl;
}

void ct_adder::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{" << endl;

    propfuncs << "  ";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << " = (";
    inputs[0]->sim_output_name(propfuncs);
    propfuncs << " + ";
    inputs[1]->sim_output_name(propfuncs);
    propfuncs << ") & " << (1<<width)-1 << ";" << endl;

    propfuncs << "  SCHEDULE(";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << "_OUTPUTS);" << endl;

    propfuncs << "}" << endl;
}

void ct_subtractor::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
}

void ct_equal::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{" << endl;

    propfuncs << "  ";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << " = (";
    inputs[0]->sim_output_name(propfuncs);
    propfuncs << " == ";
    inputs[1]->sim_output_name(propfuncs);
    propfuncs << ");" << endl;

    propfuncs << "  SCHEDULE(";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << "_OUTPUTS);" << endl;

    propfuncs << "}" << endl;
}

void ct_unequal::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{" << endl;

    propfuncs << "  ";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << " = (";
    inputs[0]->sim_output_name(propfuncs);
    propfuncs << " != ";
    inputs[1]->sim_output_name(propfuncs);
    propfuncs << ");" << endl;

    propfuncs << "  SCHEDULE(";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << "_OUTPUTS);" << endl;

    propfuncs << "}" << endl;
}

void ct_cat::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{" << endl;

    propfuncs << "  ";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << " = (";
    inputs[0]->sim_output_name(propfuncs);
    propfuncs << "<<" << inputs[1]->get_width() << ")+";
    inputs[1]->sim_output_name(propfuncs);
    propfuncs << ";" << endl;

    propfuncs << "  SCHEDULE(";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << "_OUTPUTS);" << endl;

    propfuncs << "}" << endl;
}

void ct_select::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{" << endl;

    propfuncs << "  ";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << " = (";
    inputs[0]->sim_output_name(propfuncs);
    propfuncs << "&" << (1<<(top+1))-1 << ") >> " << bottom;
    propfuncs << ";" << endl;

    propfuncs << "  SCHEDULE(";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << "_OUTPUTS);" << endl;

    propfuncs << "}" << endl;
}

void ct_port_in::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
}
/*
void ct_port_in::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
	vhdl_output_name(ports);
    ports << " : " << "IN" << " std_logic_vector(" << width-1 << " downto 0);" << endl;

	outputs[0]->vhdl_output_name(body);
	body << " <= ";
	vhdl_output_name(body);
	body << ";" << endl;

	for(unsigned i=0; i<plist->size(); i++)
	{
      pins << "Pinout ( '";
      vhdl_output_name(pins);
      pins << "[" << plist->size()-i-1 << "]";
      pins << "' toPin 'A.";
      widenum_output(*(*plist)[i],pins);
      pins << "' )" << endl;
	}
}
*/
void ct_port_out::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{" << endl;

    propfuncs << "  cout << ";
    inputs[0]->sim_output_name(propfuncs);
    propfuncs << " << endl;" << endl;

    propfuncs << "}" << endl;

}

/*
void ct_port_out::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
	vhdl_output_name(ports);
    ports << " : " << "OUT" << " std_logic_vector(" << width-1 << " downto 0);" << endl;

	vhdl_output_name(body);
	body << " <= ";
	inputs[0]->vhdl_output_name(body);
	body << ";" << endl;

	for(unsigned i=0; i<plist->size(); i++)
	{
      pins << "Pinout ( '";
      vhdl_output_name(pins);
      pins << "[" << plist->size()-i-1 << "]";
      pins << "' toPin 'A.";
      widenum_output(*(*plist)[i],pins);
      pins << "' )" << endl;
	}
}
*/

void ct_port_inout::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
}

/*
void ct_port_inout::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
}
*/

void ct_ram::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
    propfuncs << "void ";
    sim_output_name(propfuncs);
    propfuncs << "_PROPAGATE(void)" << endl;
    propfuncs << "{" << endl;

    propfuncs << "  if (!";
    inputs[2]->sim_output_name(propfuncs);
    propfuncs << ")" << endl;
    propfuncs << "  {" << endl;
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << " = ";
    sim_output_name(propfuncs);
    propfuncs << "[";
    inputs[0]->sim_output_name(propfuncs);
    propfuncs << "%" << (1<<size) << "];" << endl;

    propfuncs << "  SCHEDULE(";
    outputs[0]->sim_output_name(propfuncs);
    propfuncs << "_OUTPUTS);" << endl;

    propfuncs << "  }" << endl;
    
    propfuncs << "}" << endl;

    updatefuncs << "void ";
    sim_output_name(updatefuncs);
    updatefuncs << "_UPDATE(void)" << endl;
    updatefuncs << "{" << endl;

    updatefuncs << "  if (";
    inputs[2]->sim_output_name(updatefuncs);
    updatefuncs << ")" << endl;
    updatefuncs << "  {" << endl;

    sim_output_name(updatefuncs);
    updatefuncs << "[";
    inputs[0]->sim_output_name(updatefuncs);
    updatefuncs << "%" << (1<<size) << "]";
    updatefuncs << " = ";
    inputs[1]->sim_output_name(updatefuncs);
    updatefuncs << ";" << endl;

    updatefuncs << "  }" << endl;

    updatefuncs << "}" << endl;

    sim_output_name(update);
    update << "_UPDATE();" << endl;
    
    sigs << "unsigned ";
    sim_output_name(sigs);
    sigs << "[" << (1<<size) << "];" << endl;
}
/*
void ct_ram::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
	defs << "#ifndef RAM_" << size << "_" << width << endl;
	defs << "#define RAM_" << size << "_" << width << endl;

	defs << "#define VHDL_PRM_NAME adpram_prl_" << size << "_" << width << endl;
	defs << "#define VHDL_PRM_AWIDTH " << size << endl;
	defs << "#define VHDL_PRM_DWIDTH " << width << endl;

	defs << "#include \"adpram.def\"" << endl;

	defs << "#undef VHDL_PRM_DWIDTH" << endl;
	defs << "#undef VHDL_PRM_AWIDTH" << endl;
	defs << "#undef VHDL_PRM_NAME" << endl;

	defs << "#endif" << endl;


	body << "#define VHDL_PRM_DEFNAME adpram_prl_" << size << "_" << width << endl;
	body << "#define VHDL_PRM_NAME ";
  
	vhdl_output_name(body);
  
	body << endl;
	body << "#define VHDL_PRM_AWIDTH " << size << endl;
	body << "#define VHDL_PRM_DWIDTH " << width << endl;
	body << "#define VHDL_PRM_AIN ";
	inputs[0]->vhdl_output_name(body);
	body << endl;
	body << "#define VHDL_PRM_AOUT ";
	inputs[0]->vhdl_output_name(body);
	body << endl;
	body << "#define VHDL_PRM_DIN ";
	inputs[1]->vhdl_output_name(body);
	body << endl;
	body << "#define VHDL_PRM_DOUT ";
	outputs[0]->vhdl_output_name(body);
	body << endl;

	body << "#define VHDL_PRM_OEN ";
	inputs[2]->vhdl_output_name(body);
	body << endl;
	body << "#define VHDL_PRM_WEN (NOT ";
	inputs[2]->vhdl_output_name(body);
	body << ") OR GLOBAL_ENABLE" << endl;

	body << "#include \"adpram.ins\"" << endl;

	body << "#undef VHDL_PRM_WEN" << endl;
	body << "#undef VHDL_PRM_OEN" << endl;
	body << "#undef VHDL_PRM_DOUT" << endl;
	body << "#undef VHDL_PRM_DIN" << endl;
	body << "#undef VHDL_PRM_AOUT" << endl;
	body << "#undef VHDL_PRM_AIN" << endl;
	body << "#undef VHDL_PRM_DWIDTH" << endl;
	body << "#undef VHDL_PRM_AWIDTH" << endl;
	body << "#undef VHDL_PRM_NAME" << endl;
	body << "#undef VHDL_PRM_DEFNAME" << endl;
}
*/

void ct_par_join::sim_output
    (ostream &propfuncs,
     ostream &updatefuncs,
     ostream &sigs,
     ostream &sigdeps_decl,
     ostream &sigdeps_def,
     ostream &update,
     ostream &init)
{
	/* ct_par_join will be expanded into other components, so no need to simulate it, provided that simulation output is generated after
	 * expanding */
}

/*
void ct_par_join::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
}
*/