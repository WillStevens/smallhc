#ifndef types_h
#define types_h

typedef enum {tp_basic_type,tp_array_type} typetype;

class type
{
public:
	type(typetype t) : ttp(t) {}
	virtual ~type(void) {}

	typetype get_type(void) {return ttp;}

	virtual bool get_sign(void) = 0;
	virtual unsigned get_width(void) = 0;
	virtual unsigned get_size(void) = 0;
	virtual type *get_subtype(unsigned n = 0) = 0;
protected:
	typetype ttp;
};

class basic_type : public type
{
public:
	basic_type(void) : type(tp_basic_type) {}
	~basic_type(void) {}

	basic_type(bool is, unsigned w)
		: type(tp_basic_type), issigned(is), width(w) {}

	basic_type(const basic_type &bt, unsigned w)
		: type(tp_basic_type), issigned(bt.issigned), width(w) {}

	bool get_sign(void) {return issigned;}
	unsigned get_width(void) {return width;}
	unsigned get_size(void) {return 1;}
	type *get_subtype(unsigned n = 0) {return this;}
private:
	bool issigned;
	unsigned width;
};

class array_type : public type
{
public:
	array_type(type *etp, unsigned s)
		: type(tp_array_type), elementtype(etp), size(s) {}
	~array_type(void) {}

	bool get_sign(void) {return elementtype->get_sign();}
	unsigned get_width(void) {return elementtype->get_width();}
	unsigned get_size(void) {return size;}
	type *get_subtype(unsigned n = 0) {return elementtype;}
private:
	type *elementtype;
	unsigned size;
};

bool equal_types(type *t1, type *t2);

#endif