#include "Classes.h" //это наш прошлый заголовок
#include <cstring> //заголовок для strcmp()
namespace forSolverEq
{
//
double mysqrt(double num, double n)
{
    double exp = pow(forSolverEq::exactSqrt, n);
    if(n == 0) return 1;//x^0 == 1
    else if(n<0) return 1 / mysqrt(num, -n); // x^(1/-2) == 1 / x^(1/2)
    else if(n<1) return pow(num, 1 / n);// x^(1/2) == pow(x, 1/2)
    else if(num < 0 &&  fmod(n,2) == 0)
        return -nan("");//sqrt from negative is impossible
    double onedivn = 1 / n, nm1 = n - 1,
            rt  = (num + 1) / 2;
    if(exp == 0) exp = 0.00000000001;
    long double powed = pow(rt,n);
    if(std::isinf(powed)){
        rt =2;
        powed = pow(rt, n);
    }
    //way searching roots by Newton
    int i = 1;
    while(fabs(powed - num) > exp)
    {
        rt  =onedivn* (nm1 * rt  + num / (powed / rt));
        if(std::isinf(powed) || rt == 0) rt = i;
        powed = pow(rt,n);
        if(i>500) break;
        i++;
    }
    return rt;
}
double(*operations[sizeOperationsNames])(double) {fabs,sin, cos, tan, asin, acos, atan,sqrt };
double constants[sizeConstants]
{
    3.141592653589,6.283180779586, 2.718281828459,
    4.669201609102, 1.618033988749, 57.295779513082
};
const char* operationsNames[sizeOperationsNames]
{
    "abs","sin", "cos", "tan", "asin", "acos", "atan", "sqrt"
};
const char* constantsNames[sizeConstants]{
    "pi", "tau", "e", "delta", "phi", "rad"
};
const char* binOpNames[sizeBinOpNames]{"rt", "pow"};
double (*binops[sizeBinOpNames])(double, double){mysqrt, pow};
data*d; //We need it while calcul(). It's temperary data
double exactSqrt = 0.1;
}
ABC_Variable::ABC_Variable() :id(0) {  }
ABC_Variable::ABC_Variable(char id) : id(id) {  }
ABC_Variable::~ABC_Variable(){}

//Unknown
void Unknown::inilisated(char sign, ABC_Variable& pow)
{
    this->sign = sign;
    if (pow.id == NUMSIGN)
        power = new Num(static_cast<Num&>(pow));
}//
void Unknown::inilisated(char sign, ABC_Variable& pow, double qty2)
{
    this->sign = sign;
    qty = qty2;
    if (pow.id == NUMSIGN)
        power = new Num(static_cast<Num&>(pow));
}
Unknown::~Unknown()
{
    delete power;
}

//operation
operation::operation(char ch) :op(ch), ABC_Variable('o') { }

//Layout
Layout::~Layout()
{
    for (auto it = begin(); it != end(); it++)
        delete *(it);
}
void SolverEq::read(char*str)
{
    std::vector<Layout*>stack; //stack of Layout*.
    int size{};
    {
        int maxBrackets = 0,rightBr = -1, leftBr = -1;
        for (char ch = str[0];; ch = str[size])
        {
            if (ch == '(')
                leftBr++;
            else if (ch == ')')
                rightBr++;
            else if (ch == '\0')
                break;
            if (leftBr > maxBrackets)
                maxBrackets = leftBr;
            size++;
        }
        if(rightBr != leftBr)
            throw "lack of brackets";
        stack.reserve(maxBrackets + 1);//and reserve. +1 for main equation
    }
    stack.push_back(&lay);
    Layout* thisLay = &lay;
    for (int i = 0; i < size; i++)
    {
        char ch = str[i];
        if(ch >= 'a' && ch <= 'z')
        {
            char* str2 = &str[i];
            for (;; i++)
            {
                if(str[i] < 'a' || str[i] > 'z')
                    break;
            }
            ch = str[i];
            str[i] = '\0';
            if(strcmp(str2, "x") == 0)
                thisLay->push_back(new Unknown('x', Num(1)));
            else if(strcmp(str2, "y") == 0)
                thisLay->push_back(new Unknown('y', Num(1)));
            else{
                for(int indJ = 0;indJ < sizeConstants;indJ++)
                    if (strcmp(forSolverEq::constantsNames[indJ], str2) == 0)
                    {
                        thisLay->push_back(new Num(forSolverEq::constants[indJ]));
                        goto noErr;
                    }
                for (int indJ = 0; indJ < sizeOperationsNames; indJ++)
                    if (strcmp(forSolverEq::operationsNames[indJ], str2) == 0)
                    {
                        thisLay->push_back(new operation(static_cast<char>(indJ)));
                        goto noErr;
                    }
                for(int indJ = 0; indJ < sizeBinOpNames; indJ++)
                    if(strcmp(forSolverEq::binOpNames[indJ], str2) == 0)
                    {
                        thisLay->push_back(new operation(static_cast<char>(indJ), 'b'));
                        goto noErr;
                    }
                throw
                "Unknown variable/function";
            }
noErr:
            str[i] = ch;
            i--;
        }
        else if (ch >= '0' && ch <= '9')
        {
            char* str2 = &str[i];
            for (;; i++)
            {
                if ((str[i] < '0' || str[i] > '9') && (str[i] != '.'))
                    break;
            }
            ch = str[i];
            str[i] = '\0';
            thisLay->push_back(new Num(atof(str2)));
            str[i] = ch;
            i--;
        }
        else {
            switch (ch)
            {
            case ' ':
                break;
            case '-':
            case '+':
            case '/':
            case '*':
                thisLay->push_back(new operation(ch));
                break;
            case ',':
            case ')':
                stack.pop_back();
                thisLay = stack.back();
                if(ch == ')')break;
            case '(':
            {
                Layout*newL = new Layout;
                stack.push_back(newL);
                thisLay->push_back(newL);
                thisLay = newL;
                break;
            }
            case '^':
                thisLay->push_back(new operation('^', OPERATSIGN));
                std::swap(thisLay->back(), *(&(thisLay->back()) - 1));
                break;
            default:
                    throw "Unknown variable2/function";
                };
            }
        }
    }
void SolverEq::clear()
{
    if(lay.size() != 0)
        lay.clear();
}
floatN SolverEq::expression(Layout&l)
{
    using forSolverEq::d;
    uint i = 0, siz = l.size();
    floatN result = mult(l, i);
    for (; i < siz;)
	{
		switch (d->op)
		{
		case '+':
            result += mult(l, ++i);
			break;
		case '-':
			result -= mult(l, ++i);
			break;
		default:
			return result;
		}
	}
	return result;
}
floatN SolverEq::mult(Layout&l, uint&ind)
{
    using forSolverEq::d;
    floatN result;
	result = getValue(l, ind);
    uint siz = l.size();
    for (; ind < siz;)
	{
        forSolverEq::d->op = static_cast<operation*>(l[ind])->op;
        switch (forSolverEq::d->op)
		{
		case '*':
			result *= getValue(l, ++ind);
			break;
		case '/': {
            floatN d = getValue(l, ++ind);
            if(d == 0)
				throw "Zero dividing";
			result /= d;
        }
            break;
		default:
			return result;
		}
	}
	d->op = 0;
	return result;
}
floatN SolverEq::getValue(Layout&l, uint&ind)
{
	ABC_Variable&abc = *(l[ind]);
	char ch = abc.getId();
	ind++;
	switch (ch)
	{
	case 'l':
        return expression(static_cast<Layout&>(abc));
	case 'u': {
        auto&t = static_cast<Unknown&>(abc);
        return t.get_value(forSolverEq::d->search->at(t.getSign()));
	}
	case 'd':
        return static_cast<Num&>(abc).numD;
	case 'o':
	{
		operation&o = static_cast<operation&>(abc);
        if (o.op == '-')
			return -getValue(l, ind);
        else if (o.op < sizeOperationsNames)
            return forSolverEq::operations[o.op](getValue(l, ind));
        else if(o.op == '^'){
            floatN one = getValue(l, ind);
            floatN two = getValue(l, ind);
            return pow(one, two);
        }
        break;
	}
    case 'b':
    {
        int n= static_cast<operation&>(abc).op;
        floatN one = getValue(l, ind);
        floatN two = getValue(l, ind);
        return forSolverEq::binops[n](one, two);
    }
	default:
		break;
	}
}
floatN SolverEq::calcul(mapCD&c)
{
using forSolverEq::data;
using forSolverEq::d;
    d = new data;
    fast_ptr<data> delD(d);// for delete d;
    d->search = &c;
    floatN r = expression(lay);
	return r;
}
/*                for(int i = thisLay->size() - 3;;--i)
                {
                    if(i < 0)break;
                    ABC_Variable*a = (*thisLay)[i];
                    if(a->getId() == 'o' || a->getId() == 'b')
                    {
                        char ch = static_cast<operation*>(a)->op;
                        if((ch < sizeOperationsNames || ch == '^'))
                            std::swap(a,(*thisLay)[i+1] );
                    }
                    else break;
                }
                break;*/
