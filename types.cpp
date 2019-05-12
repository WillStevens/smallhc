#include <assert.h>

#include "types.h"

bool equal_types(type *t1, type *t2)
{
	if (!t1 || !t2)
		return true;

	if (t1->get_type() == t2->get_type())
	{
		switch(t1->get_type())
		{
		case tp_basic_type:
			return t1->get_width() == t2->get_width()
				&& t1->get_sign() == t2->get_sign();
			break;
		case tp_array_type:
			return equal_types(t1->get_subtype(),t2->get_subtype());
		default:
			assert(0);
			return false;
			break;
		}
	}
	else
	{
		return false;
	}
}
