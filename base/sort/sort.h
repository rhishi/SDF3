/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sort.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 20, 2003
 *
 *  Function        :   Sort class
 *
 *  History         :
 *      20-07-06    :   Initial version.
 *
 * $Id: sort.h,v 1.1.1.1 2007/10/02 10:59:48 sander Exp $
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

#ifndef BASE_SORT_SORT_H
#define BASE_SORT_SORT_H

#include <vector>

/**
 * lowerCost ()
 * The function returns true if x has lower cost then y.
 */
template <class T>
bool lowerCost(T x, T y, double *cost, int N, int dimSize)
{
    // Iterate over all dimensions till cost of both elements
    // have a different value in dimension n
    for (int n = 0; n < N; n++)
    {
        if (cost[x->getId() + n * dimSize] 
                < cost[y->getId() + n * dimSize])
        {
            // Cost of element x smaller then cost of element y in dimension n
            return true;
        }
        else if (cost[x->getId() + n * dimSize] 
                > cost[y->getId() + n * dimSize])
        {
            // Cost of element x larger then cost of element y in dimension n
            return false;
        }
    }

    // Cost of x and y in all dimensions equal
    return false;
}

/**
 * sortOnCost ()
 * Sort a vector 'vec' using an N-dimensional cost function. The elements of
 * 'vec' are ordered with a cost from low to high. Each element of the vector
 * must be a pointer to an object that has a 'getId' function. The 'getId'
 * function must return an index into the 'cost' array. The size of one
 * dimension (maximal id returned by getId + 1) is equal to dimSize. The number
 * of dimensions contained in the array is 'N'. The cost of an object located at
 * position i the vector 'vec' in dimension 'd' (d < N) is equal to:
 * cost[vec[i]->getId() + d * dimSize]
 *
 * Note that all dimensions must have the same size. 
 */
template <class T>
void sortOnCost(vector<T> &vec, double *cost, int N = 1, int dimSize = 1)
{
    int i, j;
    T index;

    // A basic insertion sort algorithm...
    for (i=1; i < (int) vec.size(); i++)
    {
        index = vec[i];
        j = i-1;
        while ((j >= 0) && lowerCost(index, vec[j], cost, N, dimSize))
        {
            vec[j+1] = vec[j];
            j = j - 1;
        }
        vec[j+1] = index;
    }
}

/**
 * sortOnCost ()
 * Sort a list 'vec' using an N-dimensional cost function. The elements of
 * 'vec' are ordered with a cost from low to high. Each element of the vector
 * must be a pointer to an object that has a 'getId' function. The 'getId'
 * function must return an index into the 'cost' array. The size of one
 * dimension (maximal id returned by getId + 1) is equal to dimSize. The number
 * of dimensions contained in the array is 'N'. The cost of an object located at
 * position i the vector 'vec' in dimension 'd' (d < N) is equal to:
 * cost[vec[i]->getId() + d * dimSize]
 *
 * Note that all dimensions must have the same size. 
 */
template <class T>
void sortOnCost(list<T> &vec, double *cost, int N = 1, int dimSize = 1)
{
    std::_List_iterator<T> i,j, x;
    T index;

    // A basic insertion sort algorithm...
    for (i = ++vec.begin(); i != vec.end(); i++)
    {
        index = *i;
        j = i; j--;
        while (j != vec.end() && lowerCost(index, *j, cost, N, dimSize))
        {
            x = j; x++;
            *x = *j; 
            j--;
        }
        j++;
        *j = index;
    }
}

#endif
