#include <iostream>

using std::ostream;

#include "circuit.h"

void circuit::vhdl_output(ostream &defs, ostream &ports,
			ostream &signals, ostream &body, ostream &pins)
{
	{for(list<signalw *>::iterator i = sigs.begin(); 
		i != sigs.end(); i++)
	{
		(*i)->vhdl_output(defs,ports,signals,body,pins);
	}}

	{for(list<component *>::iterator i = comps.begin(); 
		i != comps.end(); i++)
	{
		(*i)->vhdl_output(defs,ports,signals,body,pins);
	}}
}

void signalw::vhdl_output_name(ostream &os)
{
	os << name;
}

void signalw::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
	sigs << "#define VHDL_PRM_NAME ";
	vhdl_output_name(sigs);
	sigs << endl;
	sigs << "#define VHDL_PRM_WIDTH " << width << endl;
       
	sigs << "#include \"SIGNAL.INS\"" << endl;
        
	sigs << "#undef VHDL_PRM_WIDTH" << endl;
	sigs << "#undef VHDL_PRM_NAME" << endl;

	if (valueset)
	{
		vhdl_output_name(body);
		body << " <= ";
		widenum_output(value,body);
		body << ";" << endl;
	}
}

void component::vhdl_output_name(ostream &os)
{
	os << name;
}

void ct_register::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
    defs << "#ifndef REGISTER_" << width << endl;
    defs << "#define REGISTER_" << width << endl;

    defs << "#define VHDL_PRM_NAME dffIen_prl_" << width << endl;
    defs << "#define VHDL_PRM_WIDTH " << width << endl;

    defs << "#include \"DTYPE.DEF\"" << endl;

    defs << "#undef VHDL_PRM_WIDTH" << endl;
    defs << "#undef VHDL_PRM_NAME" << endl;

    defs << "#endif" << endl;


	body << "#define VHDL_PRM_DEFNAME dffIen_prl_" << width << endl;
    body << "#define VHDL_PRM_NAME ";
	vhdl_output_name(body);
    body << endl;
    body << "#define VHDL_PRM_WIDTH " << width << endl;
    body << "#define VHDL_PRM_IN ";
    inputs[0]->vhdl_output_name(body);
    body << endl;
    body << "#define VHDL_PRM_OUT ";
    outputs[0]->vhdl_output_name(body);
    body << endl;
    body << "#define VHDL_PRM_RESET GLOBAL_RESET" << endl;

	if (inputs.size() == 2)
	{
		body << "#define VHDL_PRM_ENABLE GLOBAL_ENABLE AND ";
		inputs[1]->vhdl_output_name(body);
		body << endl;
	}
	else
	{
		body << "#define VHDL_PRM_ENABLE GLOBAL_ENABLE" << endl;
	}

    body << "#define VHDL_PRM_CLK GLOBAL_CLOCK" << endl;

    body << "#include \"DTYPE.INS\"" << endl;

    body << "#undef VHDL_PRM_CLK" << endl;
    body << "#undef VHDL_PRM_ENABLE" << endl;
    body << "#undef VHDL_PRM_RESET" << endl;
    body << "#undef VHDL_PRM_OUT" << endl;
    body << "#undef VHDL_PRM_IN" << endl;
    body << "#undef VHDL_PRM_WIDTH" << endl;
    body << "#undef VHDL_PRM_NAME" << endl;
    body << "#undef VHDL_PRM_DEFNAME" << endl;
}

void ct_or_gate::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
    outputs[0]->vhdl_output_name(body);
    body << " <= ";

    for (vector<signalw *>::iterator i = inputs.begin(); i!=inputs.end(); i++)
	{
      if (i!=inputs.begin())
        body << " " << "OR";

      body << " ";
      (*i)->vhdl_output_name(body);
	}

    body << ";" << endl;
}

void ct_xor_gate::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
    outputs[0]->vhdl_output_name(body);
    body << " <= ";

    for (vector<signalw *>::iterator i = inputs.begin(); i!=inputs.end(); i++)
	{
      if (i!=inputs.begin())
        body << " " << "XOR";

      body << " ";
      (*i)->vhdl_output_name(body);
	}

    body << ";" << endl;
}

void ct_multiplexer::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
    vector<signalw *>::iterator i;

	body << "PROCESS(";

    for (i = inputs.begin(); i!=inputs.end();)
    {
		(*i++)->vhdl_output_name(body);

		if (i!=inputs.end())
		{
			body << ",";
		}
	}

	body << ")" << endl << "BEGIN" << endl;

	outputs[0]->vhdl_output_name(body);
    body << " <= \"";
	
	for(unsigned j=0; j<width;j++)
	{
		body << "X";
	}
		
	body << "\";" << endl;

    for (i = inputs.begin(); i!=inputs.end();)
    {
		body << "IF (";
		(*i++)->vhdl_output_name(body);
		body << " = '1') THEN ";

		outputs[0]->vhdl_output_name(body);
	    body << " <= ";

		(*i++)->vhdl_output_name(body);
		body << ";" << endl;

		body << "END IF;" << endl;
    }
	
	body << "END PROCESS;" << endl;
}

void ct_inverter::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
	outputs[0]->vhdl_output_name(body);
	body << " <= NOT ";
	inputs[0]->vhdl_output_name(body);
	body << ";" << endl;
}

void ct_and_gate::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
    outputs[0]->vhdl_output_name(body);
    body << " <= ";

    for (vector<signalw *>::iterator i = inputs.begin(); i!=inputs.end(); i++)
	{
      if (i!=inputs.begin())
        body << " " << "AND";

      body << " ";
      (*i)->vhdl_output_name(body);
	}

    body << ";" << endl;
}

void ct_adder::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
    defs << "#ifndef ADDER_" << width << endl;
    defs << "#define ADDER_" << width << endl;

    defs << "#define VHDL_PRM_NAME addUnsigned_" << width << endl;
    defs << "#define VHDL_PRM_WIDTH " << width << endl;

    defs << "#include \"ADDERU.DEF\"" << endl;

    defs << "#undef VHDL_PRM_WIDTH" << endl;
    defs << "#undef VHDL_PRM_NAME" << endl;

    defs << "#endif" << endl;


	body << "#define VHDL_PRM_DEFNAME addUnsigned_" << width << endl;
    body << "#define VHDL_PRM_NAME ";    
	vhdl_output_name(body);  
	body << endl;

    body << "#define VHDL_PRM_WIDTH " << width << endl;
    body << "#define VHDL_PRM_INA ";
    inputs[0]->vhdl_output_name(body);
    body << endl;
    body << "#define VHDL_PRM_INB ";
    inputs[1]->vhdl_output_name(body);
    body << endl;

    body << "#define VHDL_PRM_OUT ";
    outputs[0]->vhdl_output_name(body);
    body << endl;

    body << "#include \"ADDERU.INS\"" << endl;

    body << "#undef VHDL_PRM_OUT" << endl;
    body << "#undef VHDL_PRM_INB" << endl;
    body << "#undef VHDL_PRM_INA" << endl;
    body << "#undef VHDL_PRM_WIDTH" << endl;
    body << "#undef VHDL_PRM_NAME" << endl;
    body << "#undef VHDL_PRM_DEFNAME" << endl;
}

void ct_subtractor::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
}

void ct_equal::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
    body << "  PROCESS (";
    inputs[0]->vhdl_output_name(body);
	body << ",";
    inputs[1]->vhdl_output_name(body);
    body << ")" << endl << "  BEGIN" << endl;

    body << "    IF (";
    inputs[0]->vhdl_output_name(body);
    body << "=";
    inputs[1]->vhdl_output_name(body);
    body << ") THEN" << endl;
    body << "    ";
    outputs[0]->vhdl_output_name(body);
    body << " <= \"1\";" << endl;
    body << "    ELSE" << endl;
    body << "    ";
    outputs[0]->vhdl_output_name(body);
    body << " <= \"0\";" << endl;
    body << "    END IF;" << endl;
    body << "  END PROCESS;" << endl;
}

void ct_unequal::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
    body << "  PROCESS (";
    inputs[0]->vhdl_output_name(body);
	body << ",";
    inputs[1]->vhdl_output_name(body);
    body << ")" << endl << "  BEGIN" << endl;

    body << "    IF (";
    inputs[0]->vhdl_output_name(body);
    body << "=";
    inputs[1]->vhdl_output_name(body);
    body << ") THEN" << endl;
    body << "    ";
    outputs[0]->vhdl_output_name(body);
    body << " <= \"0\";" << endl;
    body << "    ELSE" << endl;
    body << "    ";
    outputs[0]->vhdl_output_name(body);
    body << " <= \"1\";" << endl;
    body << "    END IF;" << endl;
    body << "  END PROCESS;" << endl;
}

void ct_cat::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
	unsigned leftsz = inputs[0]->get_width();
	unsigned rightsz = inputs[1]->get_width();
	unsigned totsz = outputs[0]->get_width();

	outputs[0]->vhdl_output_name(body);
	body << "(" << totsz-1 << " downto " << totsz - leftsz << ")" << " <= ";
	inputs[0]->vhdl_output_name(body);
	body << ";" << endl;

	outputs[0]->vhdl_output_name(body);
	body << "(" << rightsz-1 << " downto " << 0 << ")" << " <= ";
	inputs[1]->vhdl_output_name(body);
	body << ";" << endl;
}

void ct_select::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
	outputs[0]->vhdl_output_name(body);
	body << " <= ";
	inputs[0]->vhdl_output_name(body);
	body << "(" << top << " downto " << bottom << ");" << endl;
}

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

void ct_port_inout::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
	vhdl_output_name(ports);
    ports << " : " << "INOUT" << " std_logic_vector(" << width-1 << " downto 0);" << endl;

    body << "  PROCESS (";
    inputs[0]->vhdl_output_name(body);
	body << ",";
    inputs[1]->vhdl_output_name(body);
	body << ",";
    vhdl_output_name(body);
    body << ")" << endl << "  BEGIN" << endl;
    body << "    IF (";
    inputs[1]->vhdl_output_name(body);
    body << "='1'";
    body << ") THEN" << endl;
    body << "    ";
    vhdl_output_name(body);
    body << " <= ";
    inputs[0]->vhdl_output_name(body);
    body << ";" << endl;
    body << "    ELSE" << endl;
    body << "    ";
    vhdl_output_name(body);
    body << " <= \"Z\";" << endl;
    body << "    END IF;" << endl;
	outputs[0]->vhdl_output_name(body);
	body << " <= ";
	vhdl_output_name(body);
	body << ";" << endl;
    body << "  END PROCESS;" << endl;

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

void ct_ram::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
	defs << "#ifndef RAM_" << size << "_" << width << endl;
	defs << "#define RAM_" << size << "_" << width << endl;

	defs << "#define VHDL_PRM_NAME sdpram_prl_" << size << "_" << width << endl;
	defs << "#define VHDL_PRM_AWIDTH " << size << endl;
	defs << "#define VHDL_PRM_DWIDTH " << width << endl;

	defs << "#include \"sdpram.def\"" << endl;

	defs << "#undef VHDL_PRM_DWIDTH" << endl;
	defs << "#undef VHDL_PRM_AWIDTH" << endl;
	defs << "#undef VHDL_PRM_NAME" << endl;

	defs << "#endif" << endl;


	body << "#define VHDL_PRM_DEFNAME sdpram_prl_" << size << "_" << width << endl;
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
	body << ")" << endl;
    body << "#define VHDL_PRM_CLK GLOBAL_CLOCK" << endl;

	body << "#include \"sdpram.ins\"" << endl;

	body << "#undef VHDL_PRM_CLK" << endl;
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

void ct_par_join::vhdl_output(ostream &defs, ostream &ports,
			ostream &sigs, ostream &body, ostream &pins)
{
}
