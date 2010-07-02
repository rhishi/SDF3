/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   storage_distribution.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   amrch 3. 2008
 *
 *  Function        :   Definition of a storage-distribution (set)
 *
 *  History         :
 *      08-03-03    :   Initial version.
 *
 * $Id: storage_distribution.h,v 1.1 2008/03/06 10:49:42 sander Exp $
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

#ifndef SDF_ANALYSIS_BUFFERSIZING_STORAGE_DISTRIBUTION_H_INCLUDED
#define SDF_ANALYSIS_BUFFERSIZING_STORAGE_DISTRIBUTION_H_INCLUDED

#include "../../basic_types.h"

/**
 * StorageDistribution
 * A storage distribution.
 */
typedef struct _StorageDistribution
{
    TBufSize sz;
    TBufSize *sp;
    TDtime thr;
    bool *dep;
    struct _StorageDistribution *prev;
    struct _StorageDistribution *next;
} StorageDistribution;

/**
 * StorageDistributionSet
 * A container for a linked-list of storage distributions with the same size.
 * The container can also be used to build a linked-list of distributions of
 * different size.
 */
typedef struct _StorageDistributionSet
{
    TDtime thr;
    TBufSize sz;
    StorageDistribution *distributions;
    struct _StorageDistributionSet *prev;
    struct _StorageDistributionSet *next;
} StorageDistributionSet;

#endif
