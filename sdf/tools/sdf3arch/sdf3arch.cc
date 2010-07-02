/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   arch.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 30, 2006
 *
 *  Function        :   Generate random architecture graphs.
 *
 *  History         :
 *      30-11-06    :   Initial version.
 *
 * $Id: sdf3arch.cc,v 1.1.1.1 2007/10/02 10:59:45 sander Exp $
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

#include "sdf3arch.h"
#include "base/base.h"

struct _Node;

typedef struct _Edge
{
    CString name;
    struct _Node *srcNode;
    struct _Node *dstNode;
} Edge;
typedef vector<Edge*>           Edges;
typedef Edges::iterator         EdgesIter;
typedef Edges::const_iterator   EdgesCIter;

typedef struct _Node
{
    CString name;
    uint id;
    uint x;
    uint y;
    Edges inEdges;
    Edges outEdges;
} Node;
typedef vector<Node*>           Nodes;
typedef Nodes::iterator         NodesIter;
typedef Nodes::const_iterator   NodesCIter;

typedef struct _ArchGraph
{
    Nodes nodes;
    Edges edges;
} ArchGraph;

typedef struct _Settings
{
    // options file
    CString optionsFile;
    
    // output file
    CString outputFile;
    
    // Architecture
    CString type;
    uint dimX;
    uint dimY;
} Settings;

// Settings
Settings settings;

// Architecture graph
ArchGraph *archGraph = NULL;

// Random generator
MTRand mtRand;

/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void helpMessage(ostream &out)
{
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << endl;
    out << endl;
    out << "Usage: " << TOOL << " --type <type> <N> <M> [--output <file>]";
    out << endl;
    out << "   --type <type> <X> <Y>  topology: mesh, tree, torus " << endl;
    out << "                          with dimensions X and Y" << endl;
    out << "   --output <file>        output file (default: stdout)" << endl;
}

/**
 * parseCommandLine ()
 * The function parses the command line arguments and add info to the
 * supplied settings structure.
 */
void parseCommandLine(int argc, char **argv)
{
    int arg = 1;
    
    while (arg < argc)
    {
        if (argv[arg] == CString("--output") && arg+1<argc)
        {
            arg++;
            settings.outputFile = argv[arg];
        }
        else if (argv[arg] == CString("--type") && arg+3<argc)
        {
            arg++;
            settings.type = argv[arg];
            arg++;
            settings.dimX = CString(argv[arg]);
            arg++;
            settings.dimY = CString(argv[arg]);
        }
        else
        {
            helpMessage(cerr);
            throw CException("");
        }
        
        // Next argument
        arg++;
    }
}

/**
 * setDefaults ()
 * Set all settings at their default value.
 */
void setDefaults()
{
    settings.type = "mesh";
    settings.dimX = 0;
    settings.dimY = 0;
}

/**
 * initSettings ()
 * The function initializes the program settings.
 */
bool initSettings(int argc, char **argv)
{
    // Defaults
    setDefaults();
    
    // Parse the command line
    parseCommandLine(argc, argv);

    return true;
}

/**
 * lengthShortestPath ()
 * The function returns the length of the shortest path from a node src
 * to a node dst.
 */
uint lengthShortestPath(Node *src, Node *dst)
{
    Nodes Q;
    Node *n, *m;
    Edge *l;
    NodesIter iterN;

    // Initialization
    v_uint d(archGraph->nodes.size(), UINT_MAX);
    d[src->id] = 0;
    Q = archGraph->nodes;

    while (!Q.empty())
    {
        // Extract min from Q
        n = NULL;
        for (NodesIter iter = Q.begin(); iter != Q.end(); iter++)
        {
            m = *iter;
            
            if (n == NULL || d[m->id] < d[n->id])
            {
                n = m;
                iterN = iter;
            }
        }
        Q.erase(iterN);
        
        // Reached destination?
        if (n == dst)
            return d[n->id];
        
        for (EdgesIter iter = n->outEdges.begin();
                iter != n->outEdges.end(); iter++)
        {
            l = *iter;
            m = l->dstNode;
            
            if (d[m->id] > d[n->id] + 1)
                d[m->id] = d[n->id] + 1;
        }
    }
    
    return UINT_MAX;
}

/**
 * printArchitectureGraph ()
 * The function outputs the architecture graph to the supplied stream as
 * an XML document.
 */
ostream &printArchitectureGraph(ostream &out)
{
    uint id;
    
    out << "<?xml version=\"1.0\"?>" << endl;
    out << "<!--" << endl;
    out << "  Architecture Graph Generator (v1.0)" << endl;
    out << "" << endl;
    out << "  Settings:" << endl; 
    out << "    type:   " << settings.type << endl;
    out << "    X:      " << settings.dimX << endl;
    out << "    Y:      " << settings.dimY << endl;
    out << "-->" << endl;
    out << "<sdf3 type='sdf' version='1.0'" << endl;
    out << "     xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'" << endl;
    out << "      xsi:noNamespaceSchemaLocation='http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd'>" << endl;
    out << "  <architectureGraph name='random_graph'>" << endl;
    
    
    for (NodesIter iter = archGraph->nodes.begin();
            iter != archGraph->nodes.end(); iter++)
    {
        Node *n = *iter;

        out << "    <tile name='tile_" << n->id << "'>" << endl;
        out << "      <processor name='proc' type='proc_0'>" << endl;
        out << "        <arbitration type='TDMA' wheelsize='100000'/>" << endl;
        out << "      </processor>" << endl;
        out << "      <memory name='mem' size='100000'/>" << endl;
        out << "      <networkInterface name='ni' nrConnections='8'";
        out << " inBandwidth='96' outBandwidth='96'/>" << endl;
        out << "    </tile>" << endl;
    }
    
    id = 0;
    for (NodesIter iter = archGraph->nodes.begin();
            iter != archGraph->nodes.end(); iter++)
    {
        Node *n = *iter;

        for (NodesIter iter = archGraph->nodes.begin();
                iter != archGraph->nodes.end(); iter++)
        {
            Node *m = *iter;

            if (n != m)
            {
                out << "    <connection name='con_" << id << "' ";
                out << "srcTile='tile_" << n->id << "' ";
                out << "dstTile='tile_" << m->id << "' ";
                out << "delay='" << (lengthShortestPath(n, m) + 2) << "'/>";
                out << endl;
                
                // Next
                id++;
            }
        }
    }
    
    out << "    <network slotTableSize='8' packetHeaderSize='32' flitSize='96'";
    out << " reconfigurationTimeNI='10'>" << endl;

    for (NodesIter iter = archGraph->nodes.begin();
            iter != archGraph->nodes.end(); iter++)
    {
        Node *n = *iter;
        
        out << "      <router name='r" << n->id <<"'/>" << endl;
    }

    id = 0;
    for (NodesIter iter = archGraph->nodes.begin();
            iter != archGraph->nodes.end(); iter++)
    {
        Node *n = *iter;
        
        out << "      <link name='l" << id << "' ";
        out << "src='tile_" << n->id << "' ";
        out << "dst='" << n->name << "'/>" << endl;

        out << "      <link name='l" << (id + 1) << "' ";
        out << "src='" << n->name << "' ";
        out << "dst='tile_" << n->id << "'/>" << endl;
        
        // Next
        id += 2;
    }

    for (EdgesIter iter = archGraph->edges.begin();
            iter != archGraph->edges.end(); iter++)
    {
        Edge *e = *iter;
        
        out << "      <link name='l" << id << "' ";
        out << "src='" << e->srcNode->name << "' ";
        out << "dst='" << e->dstNode->name << "'/>" << endl;
        
        // Next
        id++;
    }

    out << "    </network>" << endl;
    out << "  </architectureGraph>" << endl;
    out << "</sdf3>" << endl;

    return out;
}

/**
 * createNode ()
 * The function adds a node to the architecture graph.
 */
Node *createNode(uint x, uint y)
{
    Node *n = new Node;
    n->name = CString("r")  + CString(archGraph->nodes.size());
    n->id = archGraph->nodes.size();
    n->x = x;
    n->y = y;
    
    archGraph->nodes.push_back(n);
    
    return n;
}

/**
 * getNode ()
 * The function returns a pointer to the node in the architecture graph
 */
Node *getNode(uint x, uint y)
{
    for (NodesIter iter = archGraph->nodes.begin();
            iter != archGraph->nodes.end(); iter++)
    {
        Node *n = *iter;
        
        if (n->x == x && n->y == y)
            return n;
    }
    
    return NULL;
}

/**
 * createEdge ()
 * The function adds an edge from the supplied source to the supplied 
 * destination node in the architecture graph.
 */
Edge *createEdge(Node *src, Node *dst)
{
    Edge *e = new Edge;
    e->name = CString("l_") + src->name + "_" + dst->name;
    e->srcNode = src;
    e->dstNode = dst;
    
    src->outEdges.push_back(e);
    dst->inEdges.push_back(e);
    archGraph->edges.push_back(e);
    
    return e;
}

/**
 * createMeshArchitecture ()
 * The function creates a mesh architecture with the specified dimensions.
 */
void createMeshArchitecture(const uint dimX, const uint dimY)
{
    // Create graph
    archGraph = new ArchGraph;
    
    // Create nodes
    for (uint x = 0; x < dimX; x++)
    {
        for (uint y = 0; y < dimY; y++)
        {
            createNode(x,y);
        }
    }
    
    // Create edges
    for (uint x = 0; x < dimX; x++)
    {
        for (uint y = 0; y < dimY; y++)
        {
            // Link to left?
            if (x != 0)
                createEdge(getNode(x,y), getNode(x-1,y));
                
            // Link to right?
            if (x != dimX-1)
                createEdge(getNode(x,y), getNode(x+1,y));
            
            // Link to top?
            if (y != 0)
                createEdge(getNode(x,y), getNode(x,y-1));
            
            // Link to bottom?
            if (y != dimY-1)
                createEdge(getNode(x,y), getNode(x,y+1));
        }
    }    
}

/**
 * createTorusArchitecture ()
 * The function creates a torus architecture with the specified dimensions.
 */
void createTorusArchitecture(const uint dimX, const uint dimY)
{
    // Create graph
    archGraph = new ArchGraph;
    
    // Create nodes
    for (uint x = 0; x < dimX; x++)
    {
        for (uint y = 0; y < dimY; y++)
        {
            createNode(x,y);
        }
    }
    
    // Create edges
    for (uint x = 0; x < dimX; x++)
    {
        for (uint y = 0; y < dimY; y++)
        {
            // Link to left?
            if (x != 0)
                createEdge(getNode(x,y), getNode(x-1,y));
            else
                createEdge(getNode(x,y), getNode(dimX-1,y));
                
            // Link to right?
            if (x != dimX-1)
                createEdge(getNode(x,y), getNode(x+1,y));
            else
                createEdge(getNode(x,y), getNode(0,y));
            
            // Link to top?
            if (y != 0)
                createEdge(getNode(x,y), getNode(x,y-1));
            else
                createEdge(getNode(x,y), getNode(x,dimY-1));
                        
            // Link to bottom?
            if (y != dimY-1)
                createEdge(getNode(x,y), getNode(x,y+1));
            else
                createEdge(getNode(x,y), getNode(x,0));
        }
    }    
}

/**
 * createChildrenTreeNode ()
 * Recursive creation of tree with specified depth and degree per node.
 */
void createChildrenTreeNode(Node *parent, uint depth,
        const uint maxDepth, const uint degree)
{
    Node *n;
    
    if (depth >= maxDepth)
        return;
    
    for (uint i = 0; i < degree; i++)
    {
        n = createNode(depth+1, i);
        createEdge(parent, n);
        createEdge(n, parent);
        createChildrenTreeNode(n, depth + 1, maxDepth, degree);
    }
}

/**
 * createTreeArchitecture ()
 * The function creates a tree architecture with the specified dimensions.
 */
void createTreeArchitecture(const uint dimX, const uint dimY)
{
    Node *n;
    
    // Create graph
    archGraph = new ArchGraph;
    
    // Create root node
    n = createNode(0,0);
    
    // Create children of root node
    createChildrenTreeNode(n, 0, dimX, dimY);
}

/**
 * generateArchitecture ()
 * The function output an architecture of the given type with the
 * specified sizes to the supplied stream.
 */
void generateArchitecture(CString &type, uint dimX, uint dimY, ostream &out)
{
    // Construct architecture graph
    if (type == "mesh")
        createMeshArchitecture(dimX, dimY);
    else if (type == "torus")
        createTorusArchitecture(dimX, dimY);
    else if (type == "tree")
        createTreeArchitecture(dimX, dimY);
    else
        throw CException("[ERROR] Unknown topology");
    
    // Print architecture
    printArchitectureGraph(out);
}

/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv)
{
    int exit_status = 0;
    ofstream out;

    try
    {
        // Initialize the program
        if (!initSettings(argc, argv))
            return 1;
            
        // Set output stream
        if (!settings.outputFile.empty())   
            out.open(settings.outputFile.c_str());
        else
            ((ostream&)(out)).rdbuf(cout.rdbuf());

        // Generate architectur
        generateArchitecture(settings.type, settings.dimX, settings.dimY, out);
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
