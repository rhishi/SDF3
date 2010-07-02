/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SDF3 - SADF Graph
 *
 *  History         :
 *      29-08-06    :   Initial version.
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

#ifndef SADF_H_INCLUDED
#define SADF_H_INCLUDED

// XML Parser for SADF Graphs
#include "base/parser/sadf_parser.h"

// Verification of SADF Graphs
#include "verification/simple/sadf_simple.h"
#include "verification/simple/sadf_ergodic.h"
#include "verification/boundedness/sadf_boundedness.h"
#include "verification/moc/sadf_moc.h"

// Analysis of SADF Graphs
#include "analysis/graph/sadf_state_space.h"
#include "analysis/process/sadf_inter_firing_latency.h"
#include "analysis/process/sadf_response_delay.h"
#include "analysis/process/sadf_deadline_miss.h"
#include "analysis/channel/sadf_buffer_occupancy.h"

// Simulation of SADF Graphs
#include "simulation/settings/sadf_settings.h"

// Printing Facilities for SADF Graphs
#include "print/xml/sadf2xml.h"
#include "print/dot/sadf2dot.h"
#include "print/html/sadf2html.h"
#include "print/poosl/sadf2poosl.h"

// Transformation of SADF Graphs
#include "transformation/timing/sadf_timing.h"
#include "transformation/scenario/sadf_fix_scenario.h"

// Conversion of SADF Graphs
#include "transformation/sdf/sadf2sdf.h"
#include "transformation/sdf/sdf2sadf.h"
#include "transformation/csdf/sadf2csdf.h"
#include "transformation/csdf/csdf2sadf.h"
#include "transformation/scenario/sadf_collection.h"

#endif
