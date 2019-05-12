#ifndef widenum_h
#define widenum_h

#include <string>
#include<iostream>

using std::string;
using std::ostream;

// This is a class that will eventually support numbers > 64 bits wide.
// For the time being it only supports numbers <= 32 bits wide
class widenum
{
public:
	widenum(void) : value(0) {}
	widenum(string &s);
	widenum(int v) : value(v) {}

public:
	int value;
};

bool widenum_positive(const widenum &wn);
bool widenum_gte(const widenum &wn1, const widenum &wn2);
unsigned widenum_unsigned(const widenum &wn);
widenum widenum_subtract(const widenum &wn1, const widenum &wn2);
widenum widenum_invert(const widenum &wn, unsigned width);
unsigned widenum_requiredwidth(const widenum &wn);
bool widenum_equal(const widenum &wn1, const widenum &wn2);
bool widenum_zero(const widenum &wn);
bool widenum_ones(const widenum &wn, unsigned width);

void widenum_output(const widenum &wn, ostream &os);

#endif