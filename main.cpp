#include <iostream>
#include <fstream>

using namespace std;

#include <stdio.h>
#include <string.h>

#include "compilestate.h"
#include "typecheck.h"

int yyparse(void);
void yyrestart(FILE *f);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr,"Usage:hcfront [-vhdl] [-sim] filename\n");
        exit(1);
    }

	FILE *f = fopen(argv[2],"r");
	
	yyrestart(f);

	if (!yyparse())
	{
		cout << "Parsed okay" << endl;

		state.hcast->typecheck();

		if (state.typecheckgood)
		{
			state.hcast->compile(state.startsig);

            gatecount g;
            
            state.c.estimate(g);
            g.show(cout);
            g.reset();

			{
				ofstream graphos("graph.gv");
				graphos << "digraph {" << endl;
				state.c.graph_output(graphos);
				graphos << "}" << endl;
			}
			
            cout << "Expanding..." << endl;

			state.c.expand();

            state.c.estimate(g);
            g.show(cout);
            g.reset();

            cout << "Optimising..." << endl;

			bool opt = true;
			while(opt)
			{
				cout << "Calling dup component" << endl;
				opt = state.c.optimise_dup_component();
				state.c.check_circuit();
				cout << "Calling redundant component" << endl;
				opt = opt || state.c.optimise_redundant();
				state.c.check_circuit();
				cout << "Calling optimise input" << endl;
				opt = opt || state.c.optimise_input();
				state.c.check_circuit();
			}

            state.c.estimate(g);
            g.show(cout);
            g.reset();

			{
				ofstream graphos("graph_opt.gv");
				graphos << "digraph {" << endl;
				state.c.graph_output(graphos);
				graphos << "}" << endl;
			}
			
            if (!strcmp(argv[1],"-vhdl"))
            {
	            ofstream defsfile("compdefs.vhi");
	            ofstream portsfile("ports.vhi");
	            ofstream sigsfile("signals.vhi");
	            ofstream bodyfile("body.vhi");
	            ofstream pinsfile("pins.rci");

			    state.c.vhdl_output(
				    defsfile,
				    portsfile,
				    sigsfile,
				    bodyfile,
				    pinsfile
				    );
            }
            
            if (!strcmp(argv[1],"-sim"))
            {

                ofstream propfile("prop.cpi");
                ofstream updatefuncfile("updatefunc.cpi");
                ofstream sigfile("sigs.cpi");
                ofstream sigdepfile("sigdep.cpi");
                ofstream sigdepdeffile("sigdepdef.cpi");
                ofstream updatefile("update.cpi");
                ofstream initfile("init.cpi");

                state.c.sim_output(
                    propfile,
                    updatefuncfile,
                    sigfile,
                    sigdepfile,
                    sigdepdeffile,
                    updatefile,
                    initfile
                    );
            }
		}
		else
		{
			cout << "Type check failed" << endl;
		}
	}
	else
	{
		cout << " line " << LineNumber << endl;
	}

	return 0;
}