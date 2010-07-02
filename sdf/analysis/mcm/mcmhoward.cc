/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mcmhoward.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 8, 2005
 *
 *  Function        :   Compute the MCM for an HSDF graph using Howard's
 *                      algorithm implemented in Max-Plus algebra.
 *
 *  Disclaimer      :   This code is based on 'Howard Policy Iteration Algorithm
 *                      for Max Plus Matrices' written by Stephane Gaubert
 *                      (Stephane.Gaubert@inria.fr). The max-plus version of
 *                      Howard's algorithm is described in the paper:
 *                      'Numerical computation of spectral elements in max-plus
 *                      algebra'. Jean Cochet-Terrasson, Guy Cohen, Stephane
 *                      Gaubert, Michael Mc Gettrick, Jean-Pierre Quadrat
 *                      IFAC Workshop on System Structure and Control,
 *                      Nantes, July 1997.
 *
 *  History         :
 *      08-11-05    :   Initial version.
 *
 * $Id: mcmhoward.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "mcmgraph.h"
#include "../../base/hsdf/check.h"
#include "../../base/algo/components.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

/* 
 * Howard terminates with an error if MAX_NIterations occur.
 */
#define MAX_NIterations 1000

#define EPSILON -HUGE_VAL

static int *ij;
static double *a;
static int nnodes;
static int narcs;
static double *chi;
static double *v;
static int *pi;
static int *NIterations;
static int *NComponents;

static int *newpi=NULL; /*  new policy */

/* The inverse policy is coded by a linearly chained list.
 * piinv_idx[i]= pointer to the chain of inverses of node i.
 */
static 	int *piinv_idx=NULL;

/* piinv_idx[j]= pointer to the next inverse */
static	int *piinv_succ=NULL;

/* corresponding node  */
static	int *piinv_elem=NULL;

/* piinv_last[i]= last inverse of i */
static	int *piinv_last=NULL;

static	double *c=NULL;
static	double *vaux=NULL;
static	double *newc=NULL;
static	double *newchi=NULL;
static 	int *visited=NULL;
static	int *component=NULL;
static  double lambda=0;
static  double epsilon=0;
static  int color=1;


/**
 * Epsilon ()
 * The termination tests are performed up to an epsilon constant, which is fixed
 * heuristically by the following routine.
 */
static
void Epsilon(double *a,int narcs,double *epsilon)
{ 
    int i;
    double MAX,MIN;

    MAX=a[0];
    MIN=a[0];

    for (i=1;i<narcs; i++)
    {
        if (a[i]> MAX)
            MAX=a[i];
        if (a[i]<MIN)
            MIN=a[i];
    }

    *epsilon=(MAX-MIN)*0.000000001;
}

/**
 * Initial_Policy
 * Build an admissible policy pi and its associated cost vector c from ij and A.
 * Reasonable greedy rule to determine the first policy. pi(node i) = arc with 
 * maximal weigth starting from i for full random matrices, this choice of 
 * initial policy seems to cut the number of iterations by a factor 1.5, by 
 * comparison with a random initial policy.
 */ 
static
void Initial_Policy()
{
    int i;  

    /* we loose a O(nnodes) time here ... */
    /* we use the auxiliary variable vaux to compute the row max of A */ 
    for (i=0; i< nnodes; i++) 
        vaux[i] = EPSILON;

    for (i=0; i<narcs; i++)
    {
        if (vaux[ij[i*2]] <= a[i]) 
        {
            pi[ij[i*2]] =  ij[i*2+1];
            c[ij[i*2]] = a[i];
            vaux[ij[i*2]] = a[i];
        }
    }
}

static 
void New_Build_Inverse()
{
    int i,j,locus;
    int ptr=0;

    for (i=0;i<nnodes;i++)
    {
        piinv_idx[i] =-1;
        piinv_last[i]=-1;
    }
    
    for (i=0;i<nnodes;i++)
    {
        j=pi[i];
        if (piinv_idx[j]==-1) 
        {
            piinv_succ[ptr]=-1;
            piinv_elem[ptr]=i;
            piinv_last[j]=ptr;
            piinv_idx[j]=ptr;
            ptr++;
        }
        else
        {
            piinv_succ[ptr]=-1;
            piinv_elem[ptr]=i;
            locus=piinv_last[j];
            piinv_succ[locus]=ptr;
            piinv_last[j]=ptr;
            ptr++;
        };
    }
}

static 
void Init_Depth_First()
{
    int j;
    
    for (j=0;j<nnodes;j++)
    {
        visited[j]=0;
        component[j]=0;
    }
}



/**
 *
 * Given the value of v at initial point i, we compute v[j] for all predecessor 
 * j of i, according to the spectral equation, v[j]+ lambda = A(arc from j to i)
 * v[i] the array visited is changed by side effect.
 */
static
void New_Depth_First_Label(int i)
{ 
    int nexti,a;
    a=piinv_idx[i];
    while (a != -1 && visited[piinv_elem[a]]==0)
    {
        nexti=piinv_elem[a];
        visited[nexti]=1;
        v[nexti]= -lambda + c[nexti]+ v[i];
        component[nexti]=color;
        chi[nexti]= lambda;
        New_Depth_First_Label(nexti);
        a=piinv_succ[a];
    }
}

static
void Visit_From(int initialpoint,int color)
{
    int index,newindex,i;
    double weight;
    int length;

    index=initialpoint;
    component[index]=color;
    newindex=pi[index];

    while (component[newindex]==0)
    {
        component[newindex]=color;
        index=newindex;
        newindex=pi[index];
    }

    /* a cycle has been detected, since newindex is already visited */ 
    weight=0;
    length=0;
    i=index;
    do
    {
        weight+=c[i];
        length++;
        i=pi[i];
    }
    while (i !=index);

    lambda=weight/length;
    v[i]=vaux[i]; /* keeping the previous value */
    chi[i]=lambda;
    New_Depth_First_Label(index);
}

/**
 * Value()
 * Computes the value (v,chi) associated with a policy pi.
 */
static
void Value()
{
    int initialpoint;
    color=1;

    Init_Depth_First();
    initialpoint=0;

    do
    { 
        Visit_From(initialpoint,color);
        while ((initialpoint<nnodes) && (component[initialpoint] !=0))
            initialpoint++;
        color++;
    }
    while (initialpoint<nnodes);
    
    *NComponents=--color;
}

static
void Init_Improve()
{
  int i;

    for (i=0;i<nnodes; i++)
    {
        newchi[i]=chi[i];
        vaux[i]=v[i];
        newpi[i]=pi[i];
        newc[i]=c[i];
    }
}

static
void First_Order_Improvement(int *improved)
{
    int i;
    for (i=0;i<narcs; i++)
    {
        if (chi[ij[i*2+1]]>newchi[ij[i*2]])
        { 
            *improved=1;
            newpi[ij[i*2]]=ij[i*2+1];
            newchi[ij[i*2]]=chi[ij[i*2+1]];
            newc[ij[i*2]]=a[i];
        }
    }
}

static
void Second_Order_Improvement(int *improved)
{
    int i;     
    double w;
    if (*NComponents >1)
    {
        for (i=0;i<narcs; i++)
        {
            /* arc i is critical */
            if (chi[ij[i*2+1]]==newchi[ij[i*2]])
            {
                w=a[i]+ v[ij[i*2+1]]-chi[ij[i*2+1]];
                if (w>vaux[ij[i*2]] + epsilon)
                {
                    *improved=1;
                    vaux[ij[i*2]]=w;
                    newpi[ij[i*2]]=ij[i*2+1];
                    newc[ij[i*2]]=a[i];
                }
            }
        }
    }
    else
    {
        /* we know that all the arcs realize the max in the 
        first order improvement */
        for (i=0;i<narcs; i++)
        {
            w=a[i]+ v[ij[i*2+1]]-chi[ij[i*2+1]];
            if (w>vaux[ij[i*2]] + epsilon)
            {
                *improved=1;
                vaux[ij[i*2]]=w;
                newpi[ij[i*2]]=ij[i*2+1];
                newc[ij[i*2]]=a[i];
            }
        }
    }
}

static
void Improve(int *improved)
{
    *improved=0;
    Init_Improve();
    
    /* a first order policy improvement may occur */
    if (*NComponents>1)
        First_Order_Improvement(improved);
    
    if (*improved ==0)
        Second_Order_Improvement(improved);
}

static
void Allocate_Memory()
{
    newpi=(int *)calloc(nnodes, sizeof(int));
    piinv_idx=(int *)calloc(nnodes, sizeof(int));
    piinv_succ=(int *)calloc(nnodes, sizeof(int));
    piinv_elem=(int *)calloc(nnodes, sizeof(int));
    piinv_last=(int *)calloc(nnodes, sizeof(int));
    visited=(int *)calloc(nnodes, sizeof(int));
    component=(int *)calloc(nnodes, sizeof(int));
    c=(double *)calloc(nnodes, sizeof(double));
    newc=(double *)calloc(nnodes, sizeof(double));
    vaux=(double *)calloc(nnodes, sizeof(double));
    newchi=(double *)calloc(nnodes, sizeof(double));
    
    if ((newchi==NULL)|| (vaux==NULL) || (newc==NULL)|| (c==NULL) 
            || (component==NULL) || (visited==NULL)|| (piinv_idx==NULL)
            ||(piinv_succ== NULL) ||( piinv_elem==NULL)|| (  piinv_last==NULL)
            || (newpi==NULL))
    {
        throw CException("Failed allocation memory");
    }
}

static
void Free_Memory()
{
    free(newpi);
    free(piinv_idx);
    free(piinv_succ);
    free(piinv_elem);
    free(piinv_last);
    free(visited);
    free(component);
    free(c);
    free(newc);
    free(vaux);
    free(newchi);
}

static void Check_Rows()
{ 
    int i;
    int *u=NULL;
    u=(int *)calloc(nnodes,sizeof(int));

    for (i=0; i<narcs;i++ )
        u[ij[2*i]]=1;

    for (i=0; i<nnodes;i++ )
    {
        if (u[i]==0)
            throw CException("Failed check on rows in Howard's MCM algorithm.");
    }

    free(u);
}

static
void Security_Check()
{
    if (nnodes<1)
        throw CException("Howard: number of nodes must be a positive integer.");

    if (narcs<1)
        throw CException("Howard: number of arcs must be a positive integer.");

    Check_Rows();
}

static
void Import_Arguments(int *IJ, double *A,int NNODES, int NARCS, double *CHI,
        double *V, int *POLICY, int *NITERATIONS, int *NCOMPONENTS)
{
    ij=IJ;
    a=A;
    nnodes=NNODES;
    narcs=NARCS;
    chi=CHI;
    v=V;
    pi=POLICY;
    NIterations=NITERATIONS;
    NComponents=NCOMPONENTS;
}

static 
void Update_Policy()
{
    register int i;

    for (i=0;i<nnodes;i++)
    {
        pi[i]=newpi[i];
        c[i]=newc[i];
        vaux[i]=v[i]; /* Keep a copy of the current value function */
    }
}

static
void End_Message()
{
    if (*NIterations ==MAX_NIterations)
        throw CException("Howard: exceeded maximum number of iterations.");
}

/**
 * Howard ()
 * Howard Policy Iteration Algorithm for Max Plus Matrices.
 *
 * INPUT of Howard Algorithm:
 *      ij,A,nnodes,narcs : sparse description of a matrix.
 * 
 * OUTPUT:
 *      chi cycle time vector
 *      v bias
 *      pi optimal policy
 *      NIterations: Number of iterations of the algorithm
 *      NComponents: Number of connected components of the optimal policy
 *
 * REQUIRES: O(nnodes) SPACE
 * One iteration requires: O(narcs+nnodes) TIME
 *  
 * The following variables should be defined in the environment from which the
 * Howard routine is called.
 *
 * INPUT VARIABLES
 * int NNODES;  number of nodes of the graph 
 * int NARCS;   number of arcs of the graph 
 * int *IJ;     array of integers of size 2*narcs 
 *              for (0 <=k <narcs), the arc numbered k  goes from 
 *              IJ[k][0] =(IJ[2k]) to IJ[k][1] (=IJ[2k+1])
 * double *A;   array of double of size narcs
 *              A[k]=weight of the arc numbered k 
 *
 * OUTPUT VARIABLES
 * double *V;   array of double of size nnodes (the bias vector)
 * double *CHI; array of double of size nnodes (the cycle time vector)
 * int *POLICY; array of integer of size nnodes (an optimal policy)
 * int NITERATIONS; the number of iterations of the algorithm
 * int NCOMPONENTS; the number of connected components of the optimal
 *               policy which is returned.
 */
static
void Howard(int *IJ, double *A,int NNODES,int NARCS,double *CHI,double *V,
        int *POLICY,int *NITERATIONS,int *NCOMPONENTS)
{
    int improved = 0;
    *NITERATIONS = 0;
    
    Import_Arguments(IJ,A,NNODES,NARCS,CHI,V,POLICY,NITERATIONS,NCOMPONENTS);
    Security_Check();
    Allocate_Memory();
    Epsilon(a,narcs,&epsilon);
    Initial_Policy();
    New_Build_Inverse();
    
    do
    { 
        Value();
        Improve(&improved);
        Update_Policy();
        New_Build_Inverse();
        (*NIterations)++;
    }
    while ((improved != 0) && *NIterations <MAX_NIterations);
    
    End_Message();
    Free_Memory();
}

/**
 * convertMCMgraphToMatrix ()
 * The function converts a weighted directed graph used in the MCM algorithms
 * to a sparse amtrix input for Howard's algorithm.
 */
static
void convertMCMgraphToMatrix(MCMgraph *g, int *IJ, double *A)
{
    int k = 0;
    uint i = 0, j = 0;
    v_uint mapId(g->nodes.size());
    
    // Re-map the id of all visible nodes back to the range [0, g->nrNodes())
    for (MCMnodesIter iter = g->nodes.begin(); iter != g->nodes.end(); iter++)
    {
        MCMnode *n = *iter;
        
        if (n->visible)
        {
            mapId[i] = j;
            j++;
        }
        i++;
    }
    
    // Create an entry in the matrices for each edge
    for (MCMedgesIter iter = g->edges.begin(); iter != g->edges.end(); iter++)
    {
        MCMedge *e = *iter;
        
        // Is the edge a existing edge in the graph?
        if (e->visible)
        {
            IJ[2*k] = mapId[e->src->id];
            IJ[2*k+1] = mapId[e->dst->id]; 
            A[k] = e->w;
            
            // Next edge
            k++;
        }
    }
}

/**
 * mcmHoward ()
 * The function computes the maximum cycle mean of a HSDF graph using Howard's
 * algorithm.
 */
static
CFraction mcmHoward(TimedSDFgraph *g)
{
    CFraction mcm;
    MCMgraphs components;
    MCMgraph *mcmGraph;
    int nrNodes, nrEdges, nrIterations, nrComponents;
    double *A, *v, *chi;
    int *policy, *IJ;
    
    // Transform the HSDF to a weighted directed graph
    mcmGraph = transformHSDFtoMCMgraph(g);

    // Extract the strongly connected component from the graph
    // According to the Max-Plus book there is exactly one strongly
    // connected component in the graph when starting from a strongly
    // connected (H)SDF graph.
    stronglyConnectedMCMgraph(mcmGraph, components);

    // Number of nodes and edges in the graph
    nrNodes = mcmGraph->nrNodes(); 
    nrEdges = mcmGraph->nrEdges();
    
    // Allocate memory for matrices
    IJ = new int [2*nrEdges];
    A = new double [nrEdges];
    v = new double [nrNodes];
    chi = new double [nrNodes];
    policy = new int [nrNodes];
    
    // Convert the graph to a sparse matrix
    convertMCMgraphToMatrix(mcmGraph, IJ, A);

    // Run Howard's algorithm
    Howard(IJ, A, nrNodes, nrEdges, chi, v, policy, &nrIterations,
            &nrComponents);
    
    // The MCM is equal to maximum entry in the cycle time vector
    mcm = 0;
    for (int i = 0; i < nrNodes; i++)
        if (mcm < chi[i])
            mcm = chi[i];
    
    // Cleanup
    delete [] IJ;
    delete [] A;
    delete [] v;
    delete [] chi;
    delete [] policy;
    delete mcmGraph;
    
    return mcm;
}

/**
 * maximumCycleMeanHoward ()
 * The function computes the maximum cycle mean of a HSDF graph using Howard's
 * algorithm.
 */
CFraction maximumCycleMeanHoward(TimedSDFgraph *g)
{
    CFraction mcmGraph;
    
    // Check that graph g is an HSDF graph
    if (!isHSDFgraph(g))
        throw CException("Graph is not an HSDF graph.");    

    // Check that the graph g is a strongly connected graph
    if (!isStronglyConnectedGraph(g))
        throw CException("Graph is not strongly connected.");
        
    mcmGraph = mcmHoward(g);
    
    return mcmGraph;
}

