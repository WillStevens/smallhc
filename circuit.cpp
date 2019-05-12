#include "circuit.h"

string generate_name(void)
{
	static unsigned counter = 0;
	static char namebuffer[20];

	sprintf(namebuffer,"N_%d",counter++);

	return string(namebuffer);
}

void circuit::add_signal(signalw *s)
{
	sigs.push_back(s);
	list<signalw *>::iterator i = sigs.end();
	i--;
	s->setpos(i);
}

void circuit::add_component(component *c)
{
	comps.push_back(c);
	list<component *>::iterator i = comps.end();
	i--;
	c->setpos(i);
}

signalw *circuit::get_constant_signal(unsigned w, const widenum &val)
{
	for(list<signalw *>::iterator i = sigs.begin(); i!=sigs.end();
		i++)
	{
		if ((*i)->is_valueset())
		{
			if ((*i)->get_width() == w && widenum_equal((*i)->get_value(),val))
			{
				return (*i);
			}
		}
		else
		{
			// we want constant signals to be at the front of the signal list, so break here
			break;
		}
	}

	signalw *newsig = new signalw(w,val);
	sigs.push_front(newsig);

	return newsig;
}

// Remove component from circuit by removing it as a sink of its inputs, and removing its output signals
void component::removefromcircuit(circuit *c)
{
	unsigned j = 0;
	{for(vector<signalw *>::iterator i = outputs.begin();
		i != outputs.end(); i++,j++)
	{
		if ((*i)->removesource(this,j))
			(*i)->removefromcircuit(c);
	}}

	j = 0;
	{for(vector<signalw *>::iterator i = inputs.begin();
		i != inputs.end(); i++,j++)
	{
		(*i)->removesink(this,j);
	}}

	c->erasecomponent(pos);
}

void signalw::removefromcircuit(circuit *c)
{
	c->erasesignal(pos);
}