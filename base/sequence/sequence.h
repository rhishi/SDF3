/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sequence.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   May 7, 2007
 *
 *  Function        :   Sequence class
 *
 *  History         :
 *      07-05-07    :   Initial version.
 *
 * $Id: sequence.h,v 1.5 2008/11/01 16:03:11 sander Exp $
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

#ifndef BASE_SEQUENCE_SEQUENCE_H_INCLUDED
#define BASE_SEQUENCE_SEQUENCE_H_INCLUDED

#include "../basic_types.h"

/**
 * CSequence
 * Sequence class template
 */
template <class T>
class CSequence : public vector<T>
{
public:
    // Constructor
    CSequence() { };
    CSequence(T t) { push_back(t); };
    CSequence(size_t n, const T &t) { resize(n, t); };
    CSequence(const CString &t) { 
        CStrings ta;

        // Split string on comma
        ta = t.split(',');

        // Create vector
        for (CStringsIter iter = ta.begin(); iter != ta.end(); iter++) 
        {
            CString element = (*iter).trim();
            CString counter = "";

            char c;
            bool counterFound = false;

            do 
            {
                c = element[0];
                element = element.substr(1);
                
                if (c == '*') 
                {
                    counterFound = true;
                    break;
                }
                
                counter += c;
            
            } 
            while(element.size() != 0);
                
            if (counterFound && element.trim().size() == 0)
                throw CException("[Error] Invalid rate sequence.");

            if (counterFound)
            {
                for (uint i = 0; i != (uint)(strtol(counter, NULL, 10)); i++)
                {
                    push_back((T)(element));
                }
            } 
            else
            {
                push_back((T)(counter));
            }
        }
    };
        
    // Destructor
    ~CSequence() {};

    // Size
    size_t size() const { return ((vector<T>)(*this)).size(); };

    // operator[]
    T &operator[] (const int i) {
        return at((i+size()) % size());
    }
    
    // Print sequence
    ostream &print(ostream &out)
    {
        for (uint i = 0; i < size(); i++)
        {
            if (i != 0)
                out << ",";
            out << (*this)[i];
        }
                
        return out;
    }

    friend ostream &operator<<(ostream &out, CSequence &s)
        { return s.print(out); };
};

#endif

