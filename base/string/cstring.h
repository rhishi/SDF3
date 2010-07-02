/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   cstring.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   September 26, 2003
 *
 *  Function        :   String class
 *
 *  History         :
 *      26-09-03    :   Initial version.
 *
 * $Id: cstring.h,v 1.2 2008/11/01 16:03:45 sander Exp $
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

#ifndef BASE_STRING_CSTRING_H
#define BASE_STRING_CSTRING_H

#include <cstring>
#include "../basic_types.h"

// Forward class definition
class CString;

// Types
typedef list<CString>        CStrings;
typedef CStrings::iterator   CStringsIter;

/**
 * CString
 * String container class.
 * Derived from STL library class string.
 */
class CString : public std::string
{
public:
    // Constructor
    CString();
    CString(const char s);
    CString(const char *s);
    CString(const std::string &s);
    CString(const CString &s);
    
    // Constructor (integer number)
    CString(const int n);
    CString(const unsigned int n);
    CString(const long int n);
    CString(const unsigned long int n);
    CString(const long long int n);

    // Constructor (floating number)
    CString(const double n);
    
    // Destructor
    ~CString();
    
    // Assignment
    CString &operator+=(const CString &s);
    CString &operator+=(const char c);
    CString &operator+=(const int n);
    CString &operator+=(const unsigned int n);
    CString &operator+=(const long int n);
    CString &operator+=(const unsigned long int n);
    CString &operator+=(const long long int n);
    CString &operator+=(const double n);
    
    // Character access
    char operator[](int n) { return (c_str())[n]; };
    
    // Type conversion
    operator const char*() const;
    operator int() const;
    operator uint() const;
    operator double() const;
    operator long() const;
    operator unsigned long() const;
    operator long long() const;
    operator unsigned long long() const;
    
    // Whitespace
    CString &trim();
    CString &ltrim(); // left-hand side
    CString &rtrim(); // right-hand side
    
    // Split
    CStrings split(const char delim) const;
    
    // Replacement
    CString &replace(const CString &s1, const CString &s2, 
            const size_type sPos = 0, const uint n = 0);
    
    // Case
    CString &toLower();
    CString &toUpper();
};

/**
 * operator+
 * Append operator for CString class.
 */
inline CString operator+ (const CString &lhs, const CString &rhs)
{
    CString str (lhs);
    str.append(rhs);
    return str;
}

inline CString operator+ (const CString &lhs, const std::string &rhs)
{
    CString str (lhs);
    str.append(rhs);
    return str;
}

inline CString operator+ (const CString &lhs, const char *rhs)
{
    CString str (lhs);
    str.append(rhs);
    return str;
}

inline CString operator+ (const std::string &lhs, const CString &rhs)
{
    CString str (lhs);
    str.append(rhs);
    return str;
}

inline CString operator+ (const char *lhs, const CString &rhs)
{
    CString str (lhs);
    str.append(rhs);
    return str;
}

// Tokenize a string
void stringtok(CStrings &l, const CString &str, const char *tok = " \t\n");

#endif
