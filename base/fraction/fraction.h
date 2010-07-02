/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   fraction.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   Fraction class
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: fraction.h,v 1.3 2008/11/01 16:00:26 sander Exp $
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * 
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */

#ifndef BASE_FRACTION_FRACTION_H
#define BASE_FRACTION_FRACTION_H

#include "../basic_types.h"
#include "../math/cmath.h"
#include <climits>

class CFraction
{
public:
    // Constructor
    CFraction(const int num = 0, const int den = 1) 
    { 
        this->num = num; 
        this->den = den; 
        fraction = true; 
        val = (double)(num) / (double)(den); 
    };
    CFraction(const double v)
    {
        doubleToFraction(v, INFINITY);
    };
    CFraction(CString &f)
    {
        fraction = true;
        if (f.find('/') == CString::npos) 
            { num = f; den = 1; }
        else 
            { num = CString(f.substr(0,f.find('/'))); 
                den = CString(f.substr(f.find('/')+1)); }
        val = num / den;
    };
    
    // Destructor
    ~CFraction() {};
    
    // Fraction contains a real fraction?
    bool isFraction() const { return fraction; };
    
    // Fractional parts
    long long int numerator() const { return num; };
    long long int denominator() const { return den; };
    
    // Real value
    double value() const { return val; };

    // Convert double to fraction
    void doubleToFraction(const double v, const double precision = 1e-15) 
    { 
        if (precision == INFINITY)
        {
            fraction = false; 
            num = 0; 
            den = 1; 
            val = v; 
        }
        else
        {
            long long int i, a = 0, b = 0;

            // Floating point number
            val = v;
            
            // Seperate integer part from the fractional part
            long long int whole = (long long int)(val+precision);
            val -= whole;
            val = fabs(val);
            double frac = val;
            double diff = frac;
            num = 1;
            den = 0;

            // Compute fraction as sum of reciprocals
            while (diff >= precision) 
            {
                val = 1.0 / val;
                i = (long long int)(val + precision);
                val -= i;
                if (a)
                    num = i * num + b;
                den = (long long int)(num / frac + 0.5);
                diff = fabs((double)num / den - frac);
                b = a;
                a = num;
            }

            if (num == den)
            {
                whole++;
                num = 0;
                den = 1;
            }
            else if (den == 0)
            {
                num = 0;
                den = 1;
            }        

            // Add integer part to numerator
            num = whole * den + num;

            // Compute double value based on fraction
            val = (double)(num) / (double)(den); 
            fraction = true;
        }
    };
    
    // Lowest term
    CFraction lowestTerm() const
    {
        CFraction tmp;
        int g = gcd(num, den);
        tmp.num = num / g;
        tmp.den = den / g;
        tmp.val = tmp.num / tmp.den;
        return tmp;
    }
    
    // Operators
    CFraction operator+(const CFraction &rhs)
    {
        CFraction tmp;

        if (!isFraction() || !rhs.isFraction())
            return CFraction(value() + rhs.value());
        
        tmp.num = rhs.den * num + den * rhs.num;
        tmp.den = den * rhs.den;
        return tmp.lowestTerm();
    }

    CFraction operator-(const CFraction &rhs)
    {
        CFraction tmp;

        if (!isFraction() || !rhs.isFraction())
            return CFraction(value() - rhs.value());
        
        tmp.num = rhs.den * num - den * rhs.num;
        tmp.den = den * rhs.den;
        return tmp.lowestTerm();
    }

    CFraction operator*(const CFraction &rhs)
    {
        CFraction tmp;

        if (!isFraction() || !rhs.isFraction())
            return CFraction(value() * rhs.value());
        
        tmp.num = num * rhs.num;
        tmp.den = den * rhs.den;
        return tmp.lowestTerm();
    }

    CFraction operator/(const CFraction &rhs)
    {
        CFraction tmp;

        if (!isFraction() || !rhs.isFraction())
            return CFraction(value() / rhs.value());
        
        tmp.num = num * rhs.den;
        tmp.den = den * rhs.num;
        return tmp.lowestTerm();
    }

    bool operator==(const CFraction &rhs) const
    {
        if (!isFraction() || !rhs.isFraction())
            return value() == rhs.value();
        
        if (den == 0 || rhs.den == 0)
            return false;
    
        CFraction r = rhs.lowestTerm();
        CFraction l = lowestTerm();
        
        if (r.den == l.den && r.num == l.num)
            return true;
        
        return false;
    }

    bool operator!=(const CFraction &rhs)
    {
        if (*this == rhs)
            return false;
        
        return true;
    }

    bool operator>(const CFraction &rhs)
    {
        int l = lcm(den, rhs.den);

        if (!isFraction() || !rhs.isFraction())
            return value() > rhs.value();
        
        if (den == 0 || rhs.den == 0)
            return false;
        if (num == LONG_LONG_MAX)
            return true;
        if ((num * (l/den)) > (rhs.num * (l/rhs.den)))
            return true;
        
        return false;
    }
    
    bool operator<(const CFraction &rhs)
    {
        if (*this == rhs || *this > rhs)
            return false;
            
        return true;
    }
    
    ostream &print(ostream &out)
    {
        if (isFraction())
        {
            if (denominator() == 0)
                out << "NaN";
            else if (denominator() == 1)
                out << numerator();
            else
                out << numerator() << "/" << denominator();
        }
        else
        {
            out << value();
        }
                
        return out;
    }

    friend ostream &operator<<(ostream &out, CFraction &f)
        { return f.print(out); };
    
private:
    bool fraction;
    double val;
    long long int num;
    long long int den;
};

typedef vector<CFraction>           CFractions;
typedef CFractions::iterator        CFractionsIter;
typedef CFractions::const_iterator  CFractionsCIter;

#endif

