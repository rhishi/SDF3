/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   SDF graph
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: sdf.h,v 1.5 2008/03/20 16:16:17 sander Exp $
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

#ifndef SDF_SDF_H_INCLUDED
#define SDF_SDF_H_INCLUDED

// Basic untimed SDF graph
#include "base/untimed/graph.h"

// Timed SDF graph
#include "base/timed/graph.h"

// Basic graph algorithms
#include "base/algo/acyclic.h"
#include "base/algo/components.h"
#include "base/algo/connected.h"
#include "base/algo/cycle.h"
#include "base/algo/dfs.h"
#include "base/algo/repetition_vector.h"
#include "base/hsdf/check.h"

// Analysis algorithms
#include "analysis/analysis.h"

// Generate random SDFGs
#include "generate/generate.h"

// Output SDFG in different format
#include "output/buffer_throughput/buffy.h"
#include "output/dot/dot.h"
#include "output/hapi/hapi.h"
#include "output/html/html.h"
#include "output/schedule/schedule.h"
#include "output/xml/xml.h"

// Resource allocation (MP-SoC architecture)
#include "resource_allocation/mpsoc_arch/arch_types.h"
#include "resource_allocation/mpsoc_arch/binding.h"
#include "resource_allocation/mpsoc_arch/component.h"
#include "resource_allocation/mpsoc_arch/connection.h"
#include "resource_allocation/mpsoc_arch/graph.h"
#include "resource_allocation/mpsoc_arch/memory.h"
#include "resource_allocation/mpsoc_arch/networkinterface.h"
#include "resource_allocation/mpsoc_arch/processor.h"
#include "resource_allocation/mpsoc_arch/tile.h"
#include "resource_allocation/mpsoc_arch/xml.h"

// Resource allocation (NoC scheduling)
#include "resource_allocation/noc_allocation/scheduler/classic.h"
#include "resource_allocation/noc_allocation/scheduler/greedy.h"
#include "resource_allocation/noc_allocation/scheduler/knowledge.h"
#include "resource_allocation/noc_allocation/problem/problem.h"
#include "resource_allocation/noc_allocation/scheduler/random.h"
#include "resource_allocation/noc_allocation/scheduler/ripup.h"

// Resource allocation (Design flow)
#include "resource_allocation/flow/flow.h"

// Resource allocation (SDFG scheduling)
#include "resource_allocation/scheduling/scheduling.h"

// Resource allocation (Tile allocation)
#include "resource_allocation/tile_allocation/binding.h"
#include "resource_allocation/tile_allocation/loadbalance.h"

// Graph transformations
#include "transform/model/autoconc.h"
#include "transform/model/buffersize.h"
#include "transform/to_apg/apg.h"
#include "transform/to_apg/sdftoapg.h"
#include "transform/hsdf/hsdf.h"
#include "transform/hsdf/unfold.h"
#include "transform/misc/reverse_channels.h"

#endif

