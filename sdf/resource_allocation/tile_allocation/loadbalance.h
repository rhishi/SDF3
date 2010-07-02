/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   loadbalance.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 11, 2006
 *
 *  Function        :   Load-balance algorithm
 *
 *  History         :
 *      11-04-06    :   Initial version.
 *
 * $Id: loadbalance.h,v 1.3 2008/03/06 10:49:45 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_TILE_ALLOCATION_LOADBALANCE_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_TILE_ALLOCATION_LOADBALANCE_H_INCLUDED

#include "binding.h"

/**
 * LoadBalanceBinding ()
 * Load balance resource assignment algorithm.
 */
class LoadBalanceBinding : public Binding
{
public:
    // Constructor
    LoadBalanceBinding(SDFflowType flowType);
    
    // Destructor
    ~LoadBalanceBinding();
    
    // Binding algorithm
    bool bind();
    bool bindSDFGtoTiles();
    bool constructStaticOrderScheduleTiles();
    bool allocateTDMAtimeSlices();
    bool optimizeStorageSpaceAllocations();

    // Binding check
    bool bindingCheck();

    // Constants used in tile cost function
    void getConstantsTileCostFunction(double &a, double &b,
        double &c, double &d, double &e, double &f, double &g, double &k, 
        double &l, double &m, double &n, double &o, double &p, double &q);
    void setConstantsTileCostFunction(double a, double b,
        double c, double d, double e, double f, double g, double k, double l,
        double m, double n, double o, double p, double q);
    
    // Application graph (overload)
    void setAppGraph(TimedSDFgraph *g);

private:
    // Check bindings
    bool isActorBound(const SDFactor *a) const;
    bool isChannelBound(const SDFchannel *c) const;
    bool isChannelBoundToTile(const SDFchannel *c) const;
    bool isChannelBoundToConnection(const SDFchannel *c) const;

    // Resource management
    void releaseResources();
    void releaseConnectionResources(TimedSDFchannel *c);
    void releaseResources(TimedSDFactor *a, Tile *t);
    bool allocateConnectionResources(TimedSDFchannel *c);
    bool allocateResources(TimedSDFactor *a, Tile *t);

    // Actor binding
    bool bindActorsToTiles();
    void optimizeActorToTileBindings();
    bool moveActorBinding(TimedSDFactor *a, bool allowExistingTile);

    // Channel bandwidth allocation
    bool changeBandwidthAllocation(TimedSDFchannel *c, double bw);

    // Static order scheduling
    void constructStaticOrderSchedules();
    void reconstructStaticOrderSchedules();

    // Time slice allocation
    bool changeSlotAllocation(Tile *t, CSize sz);
    void reserveTimeSlices (double fraction);
    void releaseTimeSlices();
    bool minimizeTimeSlices(double step, const double minStep);
    void optimizeTimeSlices();
    bool optimizeTimeSlices(vector<CSize> minSlice, vector<CSize> maxSlice);

    // Storage space allocation
    void minimizeStorageSpace();
    void updateStorageSpaceAllocation(TimedSDFgraph *mappedAppGraph,
            StorageDistribution *d);

    // Resource estimation
    void estimateMaxCycleMean();
    double computeLoadOfChannelToConnectionBinding(SDFactor *a, Tile *t);
    double bwChannelsMappedToInConnection(SDFactor *a, Tile *t);
    double bwChannelsMappedToOutConnection(SDFactor *a, Tile *t);
    CSize memLoadChannelsOnTile(SDFactor *a, Tile *t);
    int nrChannelsMappedToConnection(SDFactor *a, Tile *t);
    SDFactors sortActorsOnCriticality();
    void sortTilesOnCommunicationOverhead(TimedSDFactor *a, Tiles &tiles);
    void sortTilesOnLoad(TimedSDFactor *a, Tiles &tiles,
        double const_a, double const_b, double const_c, double const_d, 
        double const_e, double const_f, double const_g, double const_k, 
        double const_l, double const_m, double const_n, double const_o, 
        double const_p, double const_q);

    // Tile load
    void initTileLoad();
    double actorLoadOnTile(TimedSDFactor *a, Tile *t);
    void increaseLoadTile(TimedSDFactor *a, Tile *t);
    void decreaseLoadTile(TimedSDFactor *a, Tile *t);
  
private:
    // Binding of actors to tiles
    Tiles actorTileBinding;

    // Allowed bindings of actors to tiles    
    vector<Tiles> actorTileBindingOptions;
    
    // Estimated cycle mean of actors
    double *maxCycleMean;
   
    // Computation load of tiles
    double *tileLoad;
    
    // Repetition vector application graph
    RepetitionVector repVec;
    
    // Constants used in tile sort function
    double cnst_a, cnst_b, cnst_c, cnst_d, cnst_e, cnst_f, cnst_g;
    double cnst_k, cnst_l, cnst_m, cnst_n, cnst_o, cnst_p, cnst_q;    
};

#endif

