/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   scheduling.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 27, 2005
 *
 *  Function        :   Scheduling algorithms for SDF graphs
 *
 *  History         :
 *      27-07-05    :   Initial version.
 *
 * $Id: scheduling.h,v 1.3 2008/09/25 10:49:58 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_SCHEDULING_SCHEDULING_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_SCHEDULING_SCHEDULING_H_INCLUDED

#include "static_order_schedule.h"

/**
 * List scheduler
 * Constructs a set of static-order schedules for the processors in the
 * architecture platform. Each schedule orders the firings of the actors
 * running on the processor. A list scheduler is used to construct the
 * static-order schedules.
 */
#include "list_scheduler.h"

/**
 * Priority list scheduler
 * Constructs a set of static-order schedules for the processors in the
 * architecture platform. Each schedule orders the firings of the actors
 * running on the processor. A priority-base list scheduler is used to construct 
 * the static-order schedules. The actor priorities depend on the number of
 * occurances of an actor in the abstract dependency graph.
 */
#include "priority_list_scheduler.h"

/**
 * Static-Periodic Scheduler
 * Compute a static-periodic schedule with maximal throughput for the supplied
 * SDFG.
 */
#include "static_periodic_scheduler.h"
#include "static_periodic_scheduler_chao.h"

#endif

