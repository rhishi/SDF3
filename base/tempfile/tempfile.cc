/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tempfile.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   August 1, 2005
 *
 *  Function        :   Temporary file C++ interface
 *
 *  History         :
 *      01-08-05    :   Initial version.
 *
 * $Id: tempfile.cc,v 1.2 2008/11/01 16:04:00 sander Exp $
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
 
#include "tempfile.h"
#include <cstdlib>

/**
 * tempFileName ()
 * The function returns the name of a temporary file.
 */
CString tempFileName(const CString &dir, const CString &prefix)
{
    CString nameStr;
    char *name;
    
    name = tempnam(dir.c_str(), prefix.c_str());
    nameStr = CString(name);
    free(name);
    
    return nameStr;
}
