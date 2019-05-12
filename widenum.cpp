#include "widenum.h"

widenum::widenum(string &s)
{
    if (s.length()>2 && s[1]=='x')
    {
        value = 0;

        for(unsigned i = 2; i<s.length(); i++)
        {
            value = value * 16;

            if (s[i] >= '0' && s[i] <= '9')
                value += s[i]-'0';
            if (s[i] >= 'a' && s[i] <= 'f')
                value += s[i]-'a'+10;
            if (s[i] >= 'A' && s[i] <= 'F')
                value += s[i]-'A'+10;
        }
    }
    else
    {
        value = atoi(s.c_str());
    }
}

bool widenum_positive(const widenum &wn)
{
	return wn.value >= 0;
}

bool widenum_gte(const widenum &wn1, const widenum &wn2)
{
	return wn1.value >= wn2.value;
}

unsigned widenum_unsigned(const widenum &wn)
{
	return (unsigned)wn.value;
}

widenum widenum_subtract(const widenum &wn1, const widenum &wn2)
{
	return widenum(wn1.value - wn2.value);
}

widenum widenum_invert(const widenum &wn, unsigned width)
{
	return widenum(wn.value ^ ((1<<width)-1)); 
}

unsigned widenum_requiredwidth(const widenum &wn)
{
	int v;
	unsigned w = 0;

	v = wn.value;

	while(v)
	{
		w++;
		v>>=1;
		v &= 0x7fffffff;
	}

	return w;
}

bool widenum_equal(const widenum &wn1, const widenum &wn2)
{
	return wn1.value == wn2.value;
}

bool widenum_zero(const widenum &wn)
{
	return wn.value == 0;
}

bool widenum_ones(const widenum &wn, unsigned width)
{
	int v = wn.value;
	
	while(width--)
	{
		if (!(v&1))
			return false;
		
		v>>=1;
	}
	
	return true;
}

void widenum_output(const widenum &wn, ostream &os)
{
	os << wn.value;
}

