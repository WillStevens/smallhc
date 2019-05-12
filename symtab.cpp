#include "symtab.h"

scope::scope(void) : parent(NULL)
{
	currentsubscope = subscopes.end();
}

scope::scope(scope *p) : parent(p)
{
	currentsubscope = subscopes.end();
}

scope *scope::exitscope(void)
{
	if (parent)
		return parent;
	else
	{
		assert(0);
		return NULL;
	}
}

scope *scope::enterscope(void)
{
	if (currentsubscope == subscopes.end())
	{
		subscopes.push_back(new scope(this));
		currentsubscope = subscopes.end();
		return subscopes.back();
	}
	else
	{
		return *(currentsubscope++);
	}
}

void scope::reset(void)
{
	currentsubscope = subscopes.begin();

	for(list<scope *>::iterator i = subscopes.begin();
			i!=subscopes.end(); i++)
	{
		(*i)->reset();
	}
}

bool scope::exists(string &id)
{
	if (symbols.find(id) != symbols.end())
		return true;
	else if (parent)
	{
		return parent->exists(id);
	}
	else
	{
		return false;
	}
}

symtab_entry &scope::find(string &id)
{
	if (symbols.find(id) != symbols.end())
	{
		return symbols[id];
	}
	else 
		return parent->find(id);
}
