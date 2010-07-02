/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   matrix.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   (Square) Sparse Matrices (for doubles)
 *
 *  History         :
 *      29-09-06    :   Initial version.
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

#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#include "../basic_types.h"
#include <cmath>

class SparseVectorElement;

class SparseVectorElement {

public:
	// Constructor

	SparseVectorElement() { Next = NULL; };
	SparseVectorElement(const CId I, const CDouble V) { Index = I; Value = V; Next = NULL; };
	
	// Instance Variables

	CId Index;
	CDouble Value;
	SparseVectorElement* Next;
};

class SparseVector {

public:
	// Constructor
	
	SparseVector() { Root = new SparseVectorElement(); };
	
	// Destructor
	
	~SparseVector();
	
	// Access to elements

	void addElement(const CId Index, const CDouble Value);
	SparseVectorElement* getElement(const CId Index);
	SparseVectorElement* getFirstElement() { return Root->Next; };
	void removeFirstElement();

private:
	// Instance Variables

	SparseVectorElement* Root;
};

class SparseMatrix {

public:
    // Constructor
    
    SparseMatrix(const CId Size);
    
    // Destructor

    ~SparseMatrix();
    
    // Access to elements

    void set(const CId i, const CId j, const CDouble Value) { if (Matrix[i]->getElement(j) == NULL) Matrix[i]->addElement(j, Value); else Matrix[i]->getElement(j)->Value = Value; };
    CDouble get(const CId i, const CId j) { return (Matrix[i]->getElement(j) == NULL) ? 0 : Matrix[i]->getElement(j)->Value; };
    
    // Functions
    
    vector<CDouble> computeEigenVector();

private:
    // Instance Variables;
    
    vector<SparseVector*> Matrix;
};

#endif
