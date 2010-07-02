/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   cstring.cc
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
 * $Id: cstring.cc,v 1.1.1.1 2007/10/02 10:59:48 sander Exp $
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

#include "cstring.h"
#include <stdio.h>
#include <algorithm>
#include <cctype> 

/**
 * CString ()
 * Constructor.
 */
CString::CString() : std::string()
{
}

/**
 * CString ()
 * Constructor.
 */
CString::CString(const char s) : std::string(1, s)
{
}

/**
 * CString ()
 * Constructor.
 */
CString::CString(const char *s) : std::string(s)
{
}

/**
 * CString ()
 * Constructor.
 */
CString::CString(const std::string &s) : std::string (s)
{
}

/**
 * CString ()
 * Constructor.
 */
CString::CString(const CString &s) : std::string (s)
{
}

/**
 * CString ()
 * Constructor.
 */
CString::CString(const int n) : std::string()
{
    char str[32];
    sprintf(&str[0],"%i",n);
    append(std::string(str));
} 

/**
 * CString ()
 * Constructor.
 */
CString::CString(const unsigned int n) : std::string()
{
    char str[32];
    sprintf(&str[0],"%u",n);
    append(std::string(str));
} 

/**
 * CString ()
 * Constructor.
 */
CString::CString(const long int n) : std::string()
{
    char str[32];
    sprintf(&str[0],"%ld",n);
    append(std::string(str));
} 

/**
 * CString ()
 * Constructor.
 */
CString::CString(const unsigned long int n) : std::string()
{
    char str[32];
    sprintf(&str[0],"%ld",n);
    append(std::string(str));
} 

/**
 * CString ()
 * Constructor.
 */
CString::CString(const long long int n) : std::string()
{
    char str[32];
    sprintf(&str[0],"%lld",n);
    append(std::string(str));
} 

/**
 * CString ()
 * Constructor.
 */
CString::CString(const double n) : std::string()
{
    char str[32];
    sprintf(&str[0],"%g",n);
    append(std::string(str));
} 

/**
 * ~CString ()
 * Destructor.
 */
CString::~CString()
{
}

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const CString &s)
{
    append(s);
    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const char c)
{
    push_back(c);
    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const int n)
{
    CString str(n);
    append(str);
    
    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const unsigned int n)
{
    CString str(n);
    append(str);
    
    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const long int n)
{
    CString str(n);
    append(str);
    
    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const unsigned long int n)
{
    CString str(n);
    append(str);
    
    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const long long int n)
{
    CString str(n);
    append(str);
    
    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const double n)
{
    CString str(n);
    append(str);
    
    return *this;
}

/**
 * operator const char* ()
 * Type conversion to constant character pointer.
 */
CString::operator const char* () const
{
    return c_str();
}

/**
 * operator int ()
 * Type conversion to integer.
 */
CString::operator int () const
{
    return strtol(c_str(), NULL, 0);
}

/**
 * operator uint ()
 * Type conversion to unsigned integer.
 */
CString::operator uint () const
{
    return strtoul(c_str(), NULL, 0);
}

/**
 * operator double ()
 * Type conversion to double.
 */
CString::operator double () const
{
    return strtod(c_str(), NULL);
}

/**
 * operator long ()
 * Type conversion to long.
 */
CString::operator long () const
{
    return strtol(c_str(), NULL, 0);
}

/**
 * operator unsigned long ()
 * Type conversion to unsigned long.
 */
CString::operator unsigned long () const
{
    return strtoul(c_str(), NULL, 0);
}

/**
 * operator long long ()
 * Type conversion to long long.
 */
CString::operator long long () const
{
    return strtoll(c_str(), NULL, 0);
}

/**
 * operator unsigned long long ()
 * Type conversion to unsigned long long.
 */
CString::operator unsigned long long () const
{
    return strtoull(c_str(), NULL, 0);
}

/**
 * trim ()
 * Remove whitespace from left-hand and right-hand side of string.
 */
CString &CString::trim()
{
    ltrim();
    rtrim();
    
    return *this;
}

/**
 * ltrim ()
 * Remove whitespace from left-hand side of string.
 */
CString &CString::ltrim()
{
    
    CString::size_type startPos = 0;

    // Find first non-whitespace character in the string (from left)
    while (startPos<length() && isspace(at(startPos))) startPos++;

    if (startPos == length())
        assign("");
    else
        assign(substr(startPos, length()));
    return *this;
}

/**
 * rtrim ()
 * Remove whitespace from right-hand side of string.
 */
CString &CString::rtrim()
{
    CString::size_type endPos = length() - 1;
        
    // Find first non-whitespace character in the string (from right)
    while (endPos < length() && isspace(at(endPos))) endPos--;
    
    if (endPos > length())
        assign("");
    else
        assign(substr(0, endPos+1));

    return *this;
}

/**
 * split ()
 * Split the string on all occurances of delim character
 */
CStrings CString::split(const char delim) const
{
    CStrings strings;
    CString::size_type curDelim, nextDelim = 0;
    
    do {
        // Position the delimitors
        curDelim = nextDelim;
        nextDelim = find(delim, curDelim);
        
        // Add substring to list
        if (nextDelim - curDelim != 0 && curDelim != length())
            strings.push_back(substr(curDelim,(nextDelim-curDelim)));
        
        // Advance nextDelim position to always make progress
        if (nextDelim != CString::npos)
            nextDelim++;
    } while (nextDelim != CString::npos);

    return strings;
}

/**
 * replace ()
 * Replace first n occurances of string s1 with string s2 starting from 
 * position sPos. If n is equal to zero, all occurances are replaced
 */
CString &CString::replace(const CString &s1, const CString &s2, 
        const size_type sPos, const uint n)
{
    size_type pos = sPos;
    uint nrReplaced = 0;
    
    for (pos = find(s1,pos); pos != CString::npos; pos = find(s1,pos+s2.length()))
    {
        std::string::replace(pos, s1.size(), s2.c_str());
        nrReplaced++;
        
        if (n > 0 && nrReplaced == n)
            break;
    }
    
    return *this;
}

/**
 * istok ()
 * The function returns true if the character is a valid token, else
 * the function returns false.
 */
bool istok(const char c, const char *tok)
{
    return (strchr(tok, c) != NULL);
}

/**
 * stringtok ()
 * Split a string into tokens using the given token delimitor.
 */
void stringtok(CStrings &l, const CString &str, const char *tok)
{
    const CString::size_type  S = str.size();
          CString::size_type  i = 0;

    // Clear list of strings
    l.clear();
    
    // Split string
    while (i < S) {
        // eat leading whitespace
        while ((i < S) && (istok(str[i],tok)))  ++i;
        if (i == S)  return;  // nothing left but WS

        // find end of word
        CString::size_type  j = i+1;
        while ((j < S) && (!istok(str[j],tok)))  ++j;

        // add word
        l.push_back(str.substr(i,j-i));

        // set up for next loop
        i = j+1;
    }
}

/**
 * toLower ()
 * The function converts the string to lower-case.
 */
CString &CString::toLower()
{
    CString s = *this;
    std::transform (s.begin(),s.end(), s.begin(), (int(*)(int))std::tolower);
    assign(s);
    
    return *this;
}

/**
 * toUpper ()
 * The function converts the string to upper-case.
 */
CString &CString::toUpper()
{
    CString s = *this;
    std::transform (s.begin(),s.end(), s.begin(), (int(*)(int))std::toupper);
    assign(s);

    return *this;
}

