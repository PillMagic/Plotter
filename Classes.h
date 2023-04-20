#ifndef SOURCE2_H
#define SOURCE2_H
#include <vector>
#include <map>
#include <cmath>
#include <string>

typedef double floatN;
typedef std::map<char, floatN> mapCD;
typedef std::pair<char, floatN> pairCD;
typedef unsigned int uint;
class Unknown;
struct Num;
class Layout;

#define UNKNOWNSIGN 'u'
#define NUMSIGN 'd'
#define LAYOUTSIGN 'l'
#define OPERATSIGN 'o'
#define BINOP 'b'
//common type
class ABC_Variable
{
protected:
    char id;
public:
	ABC_Variable();
	ABC_Variable(char id);
    virtual ~ABC_Variable() = 0;
    char getId()const { return id; }
	friend Unknown;
	friend Num;
	friend Layout;
};
template<class T>
struct fast_ptr
{
    T*ptr;
    fast_ptr(T*ptr2):ptr(ptr2){}
    ~fast_ptr(){delete ptr;}
};
//operation, such as +, *, /
struct operation : public ABC_Variable
{
	char op;
	operation(char ch);
    operation(char ch, char a): op(ch), ABC_Variable(a){}
    operation() :ABC_Variable(OPERATSIGN) { op = 0; }
    ~operation()override{}
};
//number
struct Num : public ABC_Variable //floatN
{
    floatN numD;
    Num() : ABC_Variable(NUMSIGN),numD(0) { }
    Num(floatN n) : ABC_Variable(NUMSIGN),numD(n) { }
    floatN&getValue() { return numD; }
    ~Num()override {}
};
//unkown, such as 'x', 'y'
class Unknown :public ABC_Variable
{
    floatN qty;
    char sign;
	ABC_Variable*power;
public:
    Unknown()
        :ABC_Variable(UNKNOWNSIGN), power(nullptr), sign(0) {}
    Unknown(char sign, const Num& pow, floatN qty = 1)
        :ABC_Variable(UNKNOWNSIGN),  power(new Num(pow)),
          sign(sign), qty(qty) {}
    Unknown(char sign, const Unknown& pow, floatN qty = 1)
        :ABC_Variable('u'), power(new Unknown(pow)),
          sign(sign), qty(qty) {}
	void inilisated(char sign, ABC_Variable& pow);
    void inilisated(char sign, ABC_Variable& pow, floatN qw);
    floatN get_value(floatN x)const
	{
		if (power == nullptr)
            throw
"Unknown::get_value(floatN x). In Unknown power is nullptr\n";
        return pow(x, ((Num*)power)->getValue()) * qty;
	}
	char getSign()const { return sign; }
    ~Unknown() override;
    friend Num;
};

class Layout : public ABC_Variable,
        public std::vector<ABC_Variable*>
{
public:
    Layout() :std::vector<ABC_Variable*>(),
        ABC_Variable(LAYOUTSIGN) {}
	~Layout();
};

#define sizeOperationsNames 8
#define sizeConstants 6
#define sizeBinOpNames 2
#ifdef DEFINE_DATA_SOLVER_EQ
#define MYEXTERN
#else
#define MYEXTERN extern
#define DEFINE_DATA_SOLVER_EQ
#endif
namespace forSolverEq
{
double mysqrt(double num, double n);
MYEXTERN double(*operations[sizeOperationsNames])(double);
MYEXTERN double constants[sizeConstants];
MYEXTERN const char* operationsNames[sizeOperationsNames];
MYEXTERN const char* binOpNames[sizeBinOpNames];//{"sqrt", "pow", "^"}
MYEXTERN const char* constantsNames[sizeConstants];
MYEXTERN double (*binops[sizeBinOpNames])(double, double);
struct data //temperary data for calcul()
{
    const mapCD*search;
    char op;
};
MYEXTERN data*d; //We need it while calcul(). It's temperary data
MYEXTERN double exactSqrt;
}
class SolverEq  //solver equation
{
	Layout lay; //array, when all numbers are in groups
    floatN expression(Layout&l);
    floatN mult(Layout&l, uint&i);
    floatN getValue(Layout&l, uint&ind);
public:
    SolverEq(){}
    floatN calcul(mapCD&c);
	void read(char*t);
    void clear(); //we have to clear SolverEquation after every equation
    bool empty(){return lay.size() == 0; }
};;

#endif
