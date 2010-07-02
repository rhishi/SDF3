/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   ctg.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 3, 2006
 *
 *  Function        :   Generate random communication traffic.
 *
 *  History         :
 *      03-02-06    :   Initial version.
 *
 * $Id: sdf3ctg.cc,v 1.1.1.1 2007/10/02 10:59:45 sander Exp $
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

#include "sdf3ctg.h"
#include "base/base.h"

typedef unsigned int TTime;

class Message
{
public:
    // Constructor
    Message(CId id) : id(id) {};
    
    // Destructor
    ~Message() {};
    
    // Properties
    CId getId() const { return id; };
    CId getSrcNodeId() const { return srcNodeId; };
    void setSrcNodeId(const CId n) { srcNodeId = n; };
    CId getDstNodeId() const { return dstNodeId; };
    void setDstNodeId(const CId n) { dstNodeId = n; };
    CId getStreamId() const { return streamId; };
    void setStreamId(const CId n) { streamId = n; };
    ulong getSeqNr() const { return seqNr; };
    void setSeqNr(const ulong n) { seqNr = n; };
    TTime getStartTime() const { return startTime; };
    void setStartTime(const TTime t) { startTime = t;};
    TTime getDuration() const { return duration; };
    void setDuration(const TTime d) { duration = d; };
    CSize getSize() const { return size; };
    void setSize(const CSize sz) { size = sz; };

private:    
    CId id;
    CId srcNodeId;
    CId dstNodeId;
    CId streamId;
    ulong seqNr;
    TTime startTime;
    TTime duration;
    CSize size;
};

typedef list<Message*>              Messages;
typedef Messages::iterator          MessagesIter;
typedef Messages::const_iterator    MessagesCIter;

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
    // settings file
    CString settingsFile;
    
    // output file
    CString outputFile;
    
    // Traffic
    CString type;
    uint nrHotspots;
    double hotspotProp;
        
    // Architecture
    CNode *xmlArchGraph;
        
    // Streams
    uint nrStreams;
    TTime period;
    TTime startTimeAvg;
    TTime startTimeVar;
    TTime startTimeMin;
    TTime startTimeMax;
    TTime durationAvg;
    TTime durationVar;
    TTime durationMin;
    TTime durationMax;
    TTime sizeAvg;
    TTime sizeVar;
    TTime sizeMin;
    TTime sizeMax;
    CSize lengthAvg;
    CSize lengthVar;
    CSize lengthMin;
    CSize lengthMax;

    // Jitter
    TTime startTimeJAvg;
    TTime startTimeJVar;
    TTime startTimeJMin;
    TTime startTimeJMax;
    TTime durationJAvg;
    TTime durationJVar;
    TTime durationJMin;
    TTime durationJMax;
    TTime sizeJAvg;
    TTime sizeJVar;
    TTime sizeJMin;
    TTime sizeJMax;
} Settings;

// Settings
Settings settings;

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
    out << "Usage: " << TOOL << " [--settings <file> --output <file>]";
    out << endl;
    out << "   --settings  <file>  generator settings (default: sdf3.opt)";
    out << endl;
    out << "   --output <file>     output file (default: stdout)" << endl;
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
        else if (argv[arg] == CString("--settings") && arg+1<argc)
        {
            arg++;
            settings.settingsFile = argv[arg];
        }
        else
        {
            helpMessage(cerr);
            throw CException("");
        }
        
        // Next argument
        arg++;
    };
}

/**
 * loadArchitectureGraphFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the platform.
 */
CNode *loadArchitectureGraphFromFile(CString &file, CString &module)
{
    CNode *archGraphNode, *sdf3Node;
    CDoc *archGraphDoc;
    
    // Open file
    archGraphDoc = CParseFile(file);
    if (archGraphDoc == NULL)
        throw CException("Failed loading architecture from '" + file + "'.");

    // Locate the sdf3 root element and check module type
    sdf3Node = CGetRootNode(archGraphDoc);
    if (CGetAttribute(sdf3Node, "type") != module)
    {
        throw CException("Root element in file '" + file + "' is not "
                         "of type '" + module + "'.");
    }
    
    // Get architecture graph node
    archGraphNode = CGetChildNode(sdf3Node, "architectureGraph");
    if (archGraphNode == NULL)
        throw CException("No architecture graph in '" + file + "'.");
    
    return archGraphNode;
}

/**
 * parseSettingsFile ()
 * The function parses all settings from the settings file.
 */
void parseSettingsFile(CString module, CString type)
{
    CNode *sdf3Node, *settingsNode, *trafficNode, *archGraphNode, *streamsNode;
    CNode *startTimeNode, *durationNode, *sizeNode, *lengthNode, *jitterNode;
    CDoc *settingsDoc;
    CString file;

    // Open settings file and get root node
    settingsDoc = CParseFile(settings.settingsFile);
    sdf3Node = CGetRootNode(settingsDoc);
    if (sdf3Node == NULL)
    {
        throw CException("Failed opening '" + settings.settingsFile + "'.");
    }
    
    // Is the node of the correct type?
    if (CGetAttribute(sdf3Node, "type") != module)
    {
        throw CException("Root element in file '" + settings.settingsFile 
                         + "' is not of type '" + module + "'.");
    }
    
    // Get the settings element of the tool
    for (settingsNode = CGetChildNode(sdf3Node, "settings");
            settingsNode != NULL; 
                settingsNode = CNextNode(settingsNode, "settings"))
    {
        if (CGetAttribute(settingsNode, "type") == type)
        {
            break;
        }
    }
    
    // Found the correct settings element?
    if (settingsNode == NULL)
    {
        throw CException("File '" + settings.settingsFile + "' contains no "
                         " settings of type '" + type + "'.");
    }

    // Traffic
    trafficNode = CGetChildNode(settingsNode, "traffic");
    if (trafficNode == NULL)
        throw CException("Missing traffic element in settings.");
    if (!CHasAttribute(trafficNode, "type"))
        throw CException("Missing type attribute on traffic.");
    settings.type = CGetAttribute(trafficNode, "type");
    if (settings.type == "hotspots")
    {
        if (!CHasAttribute(trafficNode, "nrHotspots"))
            throw CException("Missing nrHotspots attribute on traffic.");
        if (!CHasAttribute(trafficNode, "hotspotProp"))
            throw CException("Missing hotspotProp attribute on traffic.");
    
        settings.nrHotspots = CGetAttribute(trafficNode, "nrHotspots");
        settings.hotspotProp = CGetAttribute(trafficNode, "hotspotProp");
    }
    
    // Architecture graph
    archGraphNode =  CGetChildNode(settingsNode, "architectureGraph");
    if (archGraphNode == NULL)
        throw CException("No architectureGraph specified.");
    if (!CHasAttribute(archGraphNode, "file"))
        throw CException("Missing file attribute on architectureGraph.");
    file = CGetAttribute(archGraphNode, "file");
    settings.xmlArchGraph = loadArchitectureGraphFromFile(file, module);

    // Streams
    streamsNode = CGetChildNode(settingsNode, "streams");
    if (streamsNode == NULL)
        throw CException("Missing streams element in settings.");
    if (!CHasAttribute(streamsNode, "n"))
        throw CException("Missing n attribute on streams.");
    settings.nrStreams = (uint)CGetAttribute(streamsNode, "n");
    if (!CHasAttribute(streamsNode, "period"))
        throw CException("Missing period attribute on streams.");
    settings.period = (uint)CGetAttribute(streamsNode, "period");

    // Start time
    startTimeNode = CGetChildNode(streamsNode, "startTime");
    if (startTimeNode == NULL)
        throw CException("Missing startTime element in settings.");
    if (!CHasAttribute(startTimeNode, "avg"))
        throw CException("Missing avg attribute on startTime.");
    settings.startTimeAvg = (TTime)CGetAttribute(startTimeNode, "avg");
    if (!CHasAttribute(startTimeNode, "var"))
        throw CException("Missing var attribute on startTime.");
    settings.startTimeVar = (TTime)CGetAttribute(startTimeNode, "var");
    if (!CHasAttribute(startTimeNode, "min"))
        throw CException("Missing min attribute on startTime.");
    settings.startTimeMin = (TTime)CGetAttribute(startTimeNode, "min");
    if (!CHasAttribute(startTimeNode, "max"))
        throw CException("Missing max attribute on startTime.");
    settings.startTimeMax = (TTime)CGetAttribute(startTimeNode, "max");

    // Duration
    durationNode = CGetChildNode(streamsNode, "duration");
    if (durationNode == NULL)
        throw CException("Missing duration element in settings.");
    if (!CHasAttribute(durationNode, "avg"))
        throw CException("Missing avg attribute on duration.");
    settings.durationAvg = (TTime)CGetAttribute(durationNode, "avg");
    if (!CHasAttribute(durationNode, "var"))
        throw CException("Missing var attribute on duration.");
    settings.durationVar = (TTime)CGetAttribute(durationNode, "var");
    if (!CHasAttribute(durationNode, "min"))
        throw CException("Missing min attribute on duration.");
    settings.durationMin = (TTime)CGetAttribute(durationNode, "min");
    if (!CHasAttribute(durationNode, "max"))
        throw CException("Missing max attribute on duration.");
    settings.durationMax = (TTime)CGetAttribute(durationNode, "max");

    // Size
    sizeNode = CGetChildNode(streamsNode, "size");
    if (sizeNode == NULL)
        throw CException("Missing size in settings");
    if (!CHasAttribute(durationNode, "avg"))
        throw CException("Missing avg attribute on size.");
    settings.sizeAvg = (TTime)CGetAttribute(sizeNode, "avg");
    if (!CHasAttribute(durationNode, "var"))
        throw CException("Missing var attribute on size.");
    settings.sizeVar = (TTime)CGetAttribute(sizeNode, "var");
    if (!CHasAttribute(durationNode, "min"))
        throw CException("Missing min attribute on size.");
    settings.sizeMin = (TTime)CGetAttribute(sizeNode, "min");
    if (!CHasAttribute(durationNode, "max"))
        throw CException("Missing max attribute on size.");
    settings.sizeMax = (TTime)CGetAttribute(sizeNode, "max");

    // Length
    lengthNode = CGetChildNode(streamsNode, "length");
    if (lengthNode == NULL)
        throw CException("Missing length in settings");
    if (!CHasAttribute(durationNode, "avg"))
        throw CException("Missing avg attribute on length.");
    settings.lengthAvg = (TTime)CGetAttribute(lengthNode, "avg");
    if (!CHasAttribute(durationNode, "var"))
        throw CException("Missing var attribute on length.");
    settings.lengthVar = (TTime)CGetAttribute(lengthNode, "var");
    if (!CHasAttribute(durationNode, "min"))
        throw CException("Missing min attribute on length.");
    settings.lengthMin = (TTime)CGetAttribute(lengthNode, "min");
    if (!CHasAttribute(durationNode, "max"))
        throw CException("Missing max attribute on length.");
    settings.lengthMax = (TTime)CGetAttribute(lengthNode, "max");

    // Streams (jitter)
    if (CHasChildNode(streamsNode, "jitter"))
    {
        jitterNode = CGetChildNode(streamsNode, "jitter");

        if (CHasChildNode(jitterNode, "startTime"))
        {
            startTimeNode = CGetChildNode(jitterNode, "startTime");
            if (!CHasAttribute(startTimeNode, "avg"))
                throw CException("Missing avg attribute on startTime.");
            settings.startTimeJAvg = (TTime)CGetAttribute(startTimeNode, "avg");
            if (!CHasAttribute(startTimeNode, "var"))
                throw CException("Missing var attribute on startTime.");
            settings.startTimeJVar = (TTime)CGetAttribute(startTimeNode, "var");
            if (!CHasAttribute(startTimeNode, "min"))
                throw CException("Missing min attribute on startTime.");
            settings.startTimeJMin = (TTime)CGetAttribute(startTimeNode, "min");
            if (!CHasAttribute(startTimeNode, "max"))
                throw CException("Missing max attribute on startTime.");
            settings.startTimeJMax = (TTime)CGetAttribute(startTimeNode, "max");
        }
        
        if (CHasChildNode(jitterNode, "duration"))
        {
            durationNode = CGetChildNode(jitterNode, "duration");
            if (!CHasAttribute(durationNode, "avg"))
                throw CException("Missing avg attribute on duration.");
            settings.durationJAvg = (TTime)CGetAttribute(durationNode, "avg");
            if (!CHasAttribute(durationNode, "var"))
                throw CException("Missing var attribute on duration.");
            settings.durationJVar = (TTime)CGetAttribute(durationNode, "var");
            if (!CHasAttribute(durationNode, "min"))
                throw CException("Missing min attribute on duration.");
            settings.durationJMin = (TTime)CGetAttribute(durationNode, "min");
            if (!CHasAttribute(durationNode, "max"))
                throw CException("Missing max attribute on duration.");
            settings.durationJMax = (TTime)CGetAttribute(durationNode, "max");
        }
        
        if (CHasChildNode(jitterNode, "size"))
        {
            sizeNode = CGetChildNode(jitterNode, "size");
            if (!CHasAttribute(sizeNode, "avg"))
                throw CException("Missing avg attribute on size.");
            settings.sizeJAvg = (TTime)CGetAttribute(sizeNode, "avg");
            if (!CHasAttribute(sizeNode, "var"))
                throw CException("Missing var attribute on size.");
            settings.sizeJVar = (TTime)CGetAttribute(sizeNode, "var");
            if (!CHasAttribute(sizeNode, "min"))
                throw CException("Missing min attribute on size.");
            settings.sizeJMin = (TTime)CGetAttribute(sizeNode, "min");
            if (!CHasAttribute(sizeNode, "max"))
                throw CException("Missing max attribute on size.");
            settings.sizeJMax = (TTime)CGetAttribute(sizeNode, "max");
        }
    }
}

/**
 * setDefaults ()
 * Set all settings at their default value.
 */
void setDefaults()
{
    // settings file
    settings.settingsFile = "sdf3.opt";
    
    // output file
    settings.outputFile = "";
    
    // Traffic
    settings.type = "";
    settings.nrHotspots = 0;
    settings.hotspotProp = 0;
        
    // Streams
    settings.nrStreams = 0;
    settings.period = 0;
    settings.startTimeAvg = 0;
    settings.startTimeVar = 0;
    settings.startTimeMin = 0;
    settings.startTimeMax = 0;
    settings.durationAvg = 0;
    settings.durationVar = 0;
    settings.durationMin = 0;
    settings.durationMax = 0;
    settings.sizeAvg = 0;
    settings.sizeVar = 0;
    settings.sizeMin = 0;
    settings.sizeMax = 0;
    settings.lengthAvg = 0;
    settings.lengthVar = 0;
    settings.lengthMin = 0;
    settings.lengthMax = 0;

    // Jitter
    settings.startTimeJAvg = 0;
    settings.startTimeJVar = 0;
    settings.startTimeJMin = 0;
    settings.startTimeJMax = 0;
    settings.durationJAvg = 0;
    settings.durationJVar = 0;
    settings.durationJMin = 0;
    settings.durationJMax = 0;
    settings.sizeJAvg = 0;
    settings.sizeJVar = 0;
    settings.sizeJMin = 0;
    settings.sizeJMax = 0;
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

    // Parse settings
    parseSettingsFile(MODULE, SETTINGS_TYPE);
    
    return true;
}

/**
 * constructArchitectureGraph ()
 * The function constructs an architecture graph from the XML description.
 */
ArchGraph *constructArchitectureGraph(CNode *archNode)
{
    CNode *networkNode;
    ArchGraph *g;
        
    // Initialize graph
    g = new ArchGraph;
    
    // Create a node for every tile in the architecture
    for (CNode *tileNode = CGetChildNode(archNode, "tile");
            tileNode != NULL; tileNode = CNextNode(tileNode, "tile"))
    {
        Node *n = new Node;
        n->name = CGetAttribute(tileNode, "name");
        
        g->nodes.push_back(n);
    }

    // Network
    networkNode = CGetChildNode(archNode, "network");
    if (networkNode == NULL)
        throw CException("Architecture contains no network.");
    
    // Create a node for every router in the architecture
    for (CNode *routerNode = CGetChildNode(networkNode, "router");
            routerNode != NULL; routerNode = CNextNode(routerNode, "router"))
    {
        Node *n = new Node;
        n->name = CGetAttribute(routerNode, "name");
        
        g->nodes.push_back(n);
    }

    // Create an edge for every link in the architecture
    for (CNode *linkNode = CGetChildNode(networkNode, "link");
            linkNode != NULL; linkNode = CNextNode(linkNode, "link"))
    {
        Edge *e = new Edge;
        e->name = CGetAttribute(linkNode, "name");
        e->srcNode = NULL;
        e->dstNode = NULL;
        
        for (NodesIter iter = g->nodes.begin(); iter != g->nodes.end(); iter++)
        {
            Node *n = *iter;
            
            if (n->name == CGetAttribute(linkNode, "src"))
                e->srcNode = n;
            if (n->name == CGetAttribute(linkNode, "dst"))
                e->dstNode = n;
        }
        
        if (e->srcNode == NULL || e->dstNode == NULL)
            throw CException("Source or destination node of link does not"
                             " exist.");
        
        e->srcNode->outEdges.push_back(e);
        e->dstNode->inEdges.push_back(e);
        g->edges.push_back(e);
    }
    
    return g;
}

/**
 * computeReachabilityMatrix ()
 * The function computes the minimal length of the route (number of edges)
 * between every pair of source and destination nodes in the architecture
 * graph. If a node n is not reachable from a node m, the distance from
 * m to n is INT_MAX - 1
 */
uint **computeReachabilityMatrix(ArchGraph *g)
{
    uint **d;
    
    // Initialize distance matrix d
    d = new uint* [g->nodes.size()];
    for (uint n = 0; n < g->nodes.size(); n++)
    {
        d[n] = new uint [g->nodes.size()];
        for (uint m = 0; m < g->nodes.size(); m++)
        {
            if (n == m)
            {
                d[n][m] = 0;
            }
            else
            {
                // Assume no direct edge from n to m
                d[n][m] = UINT_MAX - 1;

                // Does a direct edge from n to m exist?
                for (EdgesIter iter = g->nodes[n]->outEdges.begin();
                        iter != g->nodes[n]->outEdges.end(); iter++)
                {
                    Edge *e = *iter;
                    
                    if (g->nodes[m] == e->dstNode)
                    {
                        d[n][m] = 1;
                        break;
                    }
                }
            }
        }
    }
    
    // Main loop of Floyd-Warshall all-pair shortest path algorithm
    for (uint k = 0; k < g->nodes.size(); k++)
        for (uint i = 0; i < g->nodes.size(); i++)
            for (uint j = 0; j < g->nodes.size(); j++)
                if (d[i][j] > d[i][k] + d[k][j])
                    d[i][j] = d[i][k] + d[k][j];

    return d;      
}

/**
 * printPreamble ()
 * The function outputs a preamble to the supplied output stream explaining the
 * file format.
 */
void printPreamble(ostream &out)
{
    out << "<?xml version=\"1.0\"?>" << endl;
    out << "<!--" << endl;
    out << "  Communication Traffic Generator (v2.0)" << endl;
    out << "" << endl;
    out << "  Settings:" << endl; 
    out << "    settingsFile:  " << settings.settingsFile << endl;
    out << "    outputFile:    " << settings.outputFile << endl;
    out << "    type:          " << settings.type << endl;
    out << "    nrHotspots:    " << settings.nrHotspots << endl;
    out << "    hotspotProp:   " << settings.hotspotProp << endl;
    out << "    nrStreams:     " << settings.nrStreams << endl;
    out << "    period:        " << settings.period << endl;
    out << "    startTimeAvg:  " << settings.startTimeAvg << endl;
    out << "    startTimeVar:  " << settings.startTimeVar << endl;
    out << "    startTimeMin:  " << settings.startTimeMin << endl;
    out << "    startTimeMax:  " << settings.startTimeMax << endl;
    out << "    durationAvg:   " << settings.durationAvg << endl;
    out << "    durationVar:   " << settings.durationVar << endl;
    out << "    durationMin:   " << settings.durationMin << endl;
    out << "    durationMax:   " << settings.durationMax << endl;
    out << "    sizeAvg:       " << settings.sizeAvg << endl;
    out << "    sizeVar:       " << settings.sizeVar << endl;
    out << "    sizeMin:       " << settings.sizeMin << endl;
    out << "    sizeMax:       " << settings.sizeMax << endl;
    out << "    lengthAvg:     " << settings.lengthAvg << endl;
    out << "    lengthVar:     " << settings.lengthVar << endl;
    out << "    lengthMin:     " << settings.lengthMin << endl;
    out << "    lengthMax:     " << settings.lengthMax << endl;
    out << "    startTimeJAvg: " << settings.startTimeJAvg << endl;
    out << "    startTimeJVar: " << settings.startTimeJVar << endl;
    out << "    startTimeJMin: " << settings.startTimeJMin << endl;
    out << "    startTimeJMax: " << settings.startTimeJMax << endl;
    out << "    durationJAvg:  " << settings.durationJAvg << endl;
    out << "    durationJVar:  " << settings.durationJVar << endl;
    out << "    durationJMin:  " << settings.durationJMin << endl;
    out << "    durationJMax:  " << settings.durationJMax << endl;
    out << "    sizeJAvg:      " << settings.sizeJAvg << endl;
    out << "    sizeJVar:      " << settings.sizeJVar << endl;
    out << "    sizeJMin:      " << settings.sizeJMin << endl;
    out << "    sizeJMax:      " << settings.sizeJMax << endl;
    out << "-->" << endl;
    out << "<sdf3 type='sdf' version='1.0'" << endl;
    out << "     xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'" << endl;
    out << "      xsi:noNamespaceSchemaLocation='http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd'>" << endl;
    out << "  <messagesSet>" << endl;
    out << "    <messages name='random_sequence' period='";
    out << settings.period << "'>" << endl;
}

/**
 * printPostamble ()
 * The function outputs a postamble to the supplied output stream closing the
 * XML tags.
 */
void printPostamble(ostream &out)
{
    out << "    </messages>" << endl;
    out << "  </messagesSet>" << endl;
    out << "</sdf3>" << endl;
}

/**
 * printMessage ()
 * The function outputs a message to the supplied output stream.
 */
ostream &printMessage(ArchGraph *archGraph, Message *m, ostream &out)
{
    out << "      <message";
    out << " nr='" << m->getId() << "'";
    out << " src='" << archGraph->nodes[m->getSrcNodeId()]->name << "'";
    out << " dst='" << archGraph->nodes[m->getDstNodeId()]->name << "'";
    out << " channel='ch" << m->getStreamId() << "'";
    out << " seqNr='" << m->getSeqNr() << "'";
    out << " startTime='" << m->getStartTime() << "'";
    out << " duration='" << m->getDuration() << "'";
    out << " size='" << m->getSize() << "'";
    out << "/>" << endl;
    
    return out;
}

/**
 * generateStream ()
 * Generate a stream of messages between the source and destination node.
 */
void generateStream(Messages &messages, const CId streamId, const CId srcNodeId,
    const CId dstNodeId)
{
    Message *m;
    TTime startTimeStep, durationBase, startTime, duration;
    CSize sizeBase, size;
    uint seqNr = 0;
    
    // Base time step between start of messages
    startTimeStep = (TTime)mtRand.randNorm(settings.startTimeAvg,
                settings.startTimeVar, settings.startTimeMin,
                settings.startTimeMax);
    
    // Base duration of messages
    durationBase = (TTime)mtRand.randNorm(settings.durationAvg,
                settings.durationVar, settings.durationMin,
                settings.durationMax);
    
    // Base size of messages
    sizeBase = (CSize)mtRand.randNorm(settings.sizeAvg, settings.sizeVar,
                settings.sizeMin, settings.sizeMax);
    
    // Select start time first message
    startTime = startTimeStep + (TTime)mtRand.randNorm(settings.startTimeJAvg, 
                    settings.startTimeJVar, settings.startTimeJMin,
                    settings.startTimeJMax);
    
    while (startTime < settings.period)
    {
        // Create a new message
        m = new Message(messages.size());
        
        // Nodes
        m->setSrcNodeId(srcNodeId);
        m->setDstNodeId(dstNodeId);

        // Stream
        m->setStreamId(streamId);
        m->setSeqNr(seqNr);
        
        // Start time
        m->setStartTime(startTime);
        
        // Duration
        duration = durationBase + (TTime)mtRand.randNorm(settings.durationJAvg,
                settings.durationJVar, settings.durationJMin,
                settings.durationJMax);
        m->setDuration(duration);

        // Size
        size = sizeBase + (CSize)mtRand.randNorm(settings.sizeJAvg,
                settings.sizeJVar, settings.sizeJMin, settings.sizeJMax);
        m->setSize(size);
        
        // Add message to set of messages
        messages.push_back(m);
        
        // Next message
        seqNr++;
        
        // Select start time of next message
        startTime = startTime + startTimeStep +
                (TTime)mtRand.randNorm(settings.startTimeJAvg,
                settings.startTimeJVar, settings.startTimeJMin,
                settings.startTimeJMax);
    }
}

/**
 * selectDstNode ()
 * The function returns the id of a randomly choosen node which
 * is at a randomly choosen distance from the given source node
 * in the architecture graph.
 */
CId selectDstNode(const CId srcNodeId, ArchGraph *archGraph, 
        uint **distanceMatrix)
{
    uint length, maxDistance = 0;
    vector<CId> nodes;

    for (uint n = 0; n < archGraph->nodes.size(); n++)
    {
        if (distanceMatrix[srcNodeId][n] > maxDistance 
                && distanceMatrix[srcNodeId][n] != INT_MAX - 1
                && distanceMatrix[srcNodeId][n] != 0)
            maxDistance = distanceMatrix[srcNodeId][n];
    }

    do
    {    
        // Pick random minimum length between source and destination node
        length = (uint)mtRand.randNorm(settings.lengthAvg, settings.lengthVar,
                                    settings.lengthMin, settings.lengthMax);
    } while (length > maxDistance);

    for (uint diff = 0; diff < maxDistance + length; diff++)
    {
        for (uint n = 0; n < archGraph->nodes.size(); n++)
        {
            if (distanceMatrix[srcNodeId][n] == length + diff
                    || distanceMatrix[srcNodeId][n] == length - diff)
            {
                nodes.push_back(n);
            }
        }
        
        // Found at least one node with difference diff wrt randomly
        // choosen lentgh
        if (nodes.size() > 0)
        {
            // Pick random node from the list of nodes
            return nodes[mtRand.randInt(nodes.size()-1)];    
        }
    }

    return srcNodeId;
}

/**
 * selectSrcNode ()
 * The function returns the id of a randomly choosen node which
 * is at a randomly choosen distance from the given destination node
 * in the architecture graph.
 */
CId selectSrcNode(const CId dstNodeId, ArchGraph *archGraph, 
        uint **distanceMatrix)
{
    uint length, maxDistance = 0;
    vector<CId> nodes;

    for (uint n = 0; n < archGraph->nodes.size(); n++)
    {
        if (distanceMatrix[n][dstNodeId] > maxDistance 
                && distanceMatrix[n][dstNodeId] != INT_MAX - 1
                && distanceMatrix[n][dstNodeId] != 0)
            maxDistance = distanceMatrix[n][dstNodeId];
    }

    do
    {    
        // Pick random minimum length between source and destination node
        length = (uint)mtRand.randNorm(settings.lengthAvg, settings.lengthVar,
                                    settings.lengthMin, settings.lengthMax);
    } while (length > maxDistance);

    for (uint diff = 0; diff < maxDistance + length; diff++)
    {
        for (uint n = 0; n < archGraph->nodes.size(); n++)
        {
            if (distanceMatrix[n][dstNodeId] == length + diff
                    || distanceMatrix[n][dstNodeId] == length - diff)
            {
                nodes.push_back(n);
            }
        }
        
        // Found at least one node with difference diff wrt randomly
        // choosen lentgh
        if (nodes.size() > 0)
        {
            // Pick random node from the list of nodes
            return nodes[mtRand.randInt(nodes.size()-1)];    
        }
    }

    return dstNodeId;
}

/**
 * generateMessagesHotspots ()
 * Generate a set of messages with a hotspot traffic distribution.
 */
void generateMessagesHotspots(ArchGraph *archGraph,
        uint **distanceMatrix, Messages &messages)
{
    vector<CId> hotspots, notHotspots;
    CId srcNodeId, dstNodeId;
    uint nrStreams = 0;

    // Create vector with hotspots
    while (hotspots.size() < settings.nrHotspots)
        hotspots.push_back(mtRand.randInt(archGraph->nodes.size()-1));

    // Add all non hotspot nodes to the vector 'notHotspots'
    for (CId i = 0; i < archGraph->nodes.size(); i++)
    {
        bool isHotspot = false;
        
        // Is node i not a hotspot?
        for (uint j = 0; j < hotspots.size() && !isHotspot; j++)
            if (i == j)
                isHotspot = true;
        
        if (!isHotspot)
            notHotspots.push_back(i);
    }

    while (nrStreams < settings.nrStreams)
    {
        do 
        {
            // Src node is hotspot?
            if (mtRand.rand() < settings.hotspotProp)
            {
                srcNodeId = hotspots[mtRand.randInt(hotspots.size()-1)];
                dstNodeId = selectDstNode(srcNodeId, archGraph, distanceMatrix);
            }
            else
            {
                dstNodeId = hotspots[mtRand.randInt(hotspots.size()-1)];
                srcNodeId = selectSrcNode(dstNodeId, archGraph, distanceMatrix);
            }
        } while (srcNodeId == dstNodeId);

        // Generate a stream of messages between the given source and
        // destination node
        generateStream(messages, nrStreams, srcNodeId, dstNodeId);
        
        // Next stream
        nrStreams++;
    }
}

/**
 * generateMessagesUniform ()
 * Generate a set of messages with a uniform traffic distribution.
 */
void generateMessagesUniform(ArchGraph *archGraph, uint **distanceMatrix,
        Messages &messages)
{
    CId srcNodeId, dstNodeId;
    uint nrStreams = 0;
    
    while (nrStreams < settings.nrStreams)
    {
        do 
        {
            // Select random source node
            srcNodeId = mtRand.randInt(archGraph->nodes.size()-1);

            // Select random destination node (not equla to source)
            dstNodeId = selectDstNode(srcNodeId, archGraph, distanceMatrix);
        } while (srcNodeId == dstNodeId);

        // Generate a stream of messages between the given source and
        // destination node
        generateStream(messages, nrStreams, srcNodeId, dstNodeId);

        // Next stream
        nrStreams++;
    }
}

/**
 * generateMessages ()
 * Generate a set of messages organised in streams for the requested traffic
 * pattern. Messages are outputted to the supplied output stream.
 */
void generateMessages(ostream &out)
{
    uint **distanceMatrix;
    ArchGraph *archGraph;
    Messages messages;
    
    // Construct architecture graph and reachability matrix
    archGraph  = constructArchitectureGraph(settings.xmlArchGraph);    
    distanceMatrix = computeReachabilityMatrix(archGraph);

    // Generate messages for correct traffic pattern
    if (settings.type == "uniform")
        generateMessagesUniform(archGraph, distanceMatrix, messages);
    else if (settings.type == "hotspots")
        generateMessagesHotspots(archGraph, distanceMatrix, messages);
    else
        throw CException("[ERROR] Unknown traffic type.");
    
    // Output the generated messages
    printPreamble(out);
    for (MessagesIter iter = messages.begin(); iter != messages.end(); iter++)
    {
        Message *m = *iter;
        
        printMessage(archGraph, m, out);
    }
    printPostamble(out);
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

        // Generate messages
        generateMessages(out);
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
