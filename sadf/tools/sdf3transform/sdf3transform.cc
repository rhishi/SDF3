/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3transform.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   30 September 2007
 *
 *  Function        :   SADF Transformation Functionality
 *
 *  History         :
 *      30-09-07    :   Initial version.
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

#include "sdf3transform.h"
#include "../../sadf.h"

typedef struct _CPair 
{
    CString key;
    CString value;
} CPair;

typedef list<CPair>         CPairs;
typedef CPairs::iterator    CPairsIter;

/**
 * Settings
 * Struct to store program settings.
 */
typedef struct _Settings
{
    // Input file with graph
    CString graphFile;
    
    // Output file
    CString outputFile;
    
    // Switch argument(s) given to analysis algorithm
    CPairs arguments;
    
    // Application graph
    CNode *xmlAppGraph;
} Settings;

/**
 * settings
 * Program settings.
 */
Settings settings;

/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void helpMessage(ostream &out)
{
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << endl;
    out << endl;
    out << "Usage: " << TOOL << " --graph <file> --transform <list>";
    out << " [--output <file>]" << endl;
    out << "   --graph  <file>     input SADF graph" << endl;
    out << "   --output <file>     output file (default: stdout)" << endl;
    out << "   --transform <list>  comma separated list with graph transformations:" << endl;
    out << "       fix_scenarios(<subscenariolist>)" << endl;
    out << "       execution_times(minimize|average|maximize)" << endl;
}

/**
 * parseSwitchArgument ()
 * The function parses the string 'arguments' into a sequence of (arg, value)
 * pairs. The syntax as as follows:
 *
 * pair := key(value)
 * arg := pair,pair,...
 *
 * Note: value may be a pair itself, but this is not expanded into a set of
 * pairs (i.e. nested pairs are not supported).
 */
CPairs parseSwitchArgument(CString arguments)
{
    CPairs pairs;
    CPair p;
    
    while (arguments.size() != 0)
    {
        char c;
        p.key = "";
        p.value = "";
        
        // Get key from argument string
        do
        {
            c = arguments[0];
            arguments = arguments.substr(1);
            if (c == ',' || c== '(')
                break;
            p.key += c;
        } while (arguments.size() != 0);

        // Is next part of argument a value?
        if (c == '(')
        {
            CString::size_type ePos = 0;
            int level = 1;
            
            // Find the matching closing brace
            while (level != 0 && arguments.size() != 0)
            {
                if (arguments[ePos] == ')')
                    level--;
                else if (arguments[ePos] == '(')
                    level++;
                
                // Next
                ePos++;
            }
            
            // Closing brace found?
            if (level != 0)
                throw CException("Missing closing brace in value of argument.");
              
            // Get value  
            p.value = arguments.substr(0, ePos-1);
            
            // More arguments left?
            if (arguments.size() > ePos)
                arguments = arguments.substr(ePos+1); 
            else
                arguments = "";
        }

        // Add pair to list of pairs
        pairs.push_back(p);
    }

    return pairs;
}

/**
 * parseCommandLine ()
 * The function parses the command line arguments and add info to the
 * supplied settings structure.
 */
void parseCommandLine(int argc, char ** argv)
{
    int arg = 1;
    
    while (arg < argc)
    {
        // Configuration file
        if (argv[arg] == CString("--graph") && arg+1<argc)
        {
            arg++;
            settings.graphFile = argv[arg];
        }
        else if (argv[arg] == CString("--output") && arg+1<argc)
        {
            arg++;
            settings.outputFile = argv[arg];
        }
        else if (argv[arg] == CString("--transform") && arg+1<argc)
        {
            arg++;
            settings.arguments = parseSwitchArgument(argv[arg]);
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
 * loadApplicationGraphFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the SDFG.
 */
CNode *loadApplicationGraphFromFile(CString &file, CString module)
{
    CNode *appGraphNode, *sdf3Node;
    CDoc *appGraphDoc;
    
    // Open file
    appGraphDoc = CParseFile(file);
    if (appGraphDoc == NULL)
        throw CException("Failed loading SADF Graph from '" + file + "'.");

    // Locate the sdf3 root element and check module type
    sdf3Node = CGetRootNode(appGraphDoc);
    if (CGetAttribute(sdf3Node, "type") != module)
    {
        throw CException("Root element in file '" + file + "' is not "
                         "of type '" + module + "'.");
    }
    
    // Get application graph node
    appGraphNode = CGetChildNode(sdf3Node, "sadf");
    if (appGraphNode == NULL)
        throw CException("No SADF Graph in '" + file + "'.");
    
    return appGraphNode;
}

/**
 * initSettings ()
 * The function initializes the program settings.
 */
void initSettings(int argc, char **argv)
{
    // Parse the command line
    parseCommandLine(argc, argv);

    // Check required settings
    if (settings.graphFile.empty() || settings.arguments.size() == 0)
    {
        helpMessage(cerr);
        throw CException("");
    }

    // Load application graph
    settings.xmlAppGraph = loadApplicationGraphFromFile(settings.graphFile, MODULE);
}

/**
 * SADF_TransformGraph()
 * The function that calls the actual transformation functions
 */
void SADF_TransformGraph(SADF_Graph *Graph, CPairs &transforms, ostream &out) {

	for (CPairsIter iter = transforms.begin(); iter != transforms.end(); iter++) {

		CPair &transform = *iter;

		if (transform.key == "execution_times") {
	
			if (transform.value == "average")
				SADF_Transform_AverageExecutionTimes(Graph);
			else if (transform.value == "minimize")
				SADF_Transform_MinimizeExecutionTimes(Graph);
			else if (transform.value == "maximize")
				SADF_Transform_MaximizeExecutionTimes(Graph);
			else
				throw CException("Unknown transformation for execution times.");
                
        } else if (transform.key == "fix_scenarios") {

    		// Determine subscenarios

            vector<CId> SubScenarios(Graph->getNumberOfDetectors(), SADF_UNDEFINED);
	
    	    CPairs options = parseSwitchArgument(transform.value);

    		for (CPairsIter i = options.begin(); i != options.end(); i++)
	    		if (Graph->getDetector((*i).key) != NULL) {
                    if (Graph->getDetector((*i).key)->getSubScenario((*i).value) != NULL)
                        SubScenarios[Graph->getDetector((*i).key)->getIdentity()] = Graph->getDetector((*i).key)->getSubScenario((*i).value)->getIdentity();
                    else
                        throw CException((CString)("Error: Dectector '") + (*i).key + "' of SADF graph '" + Graph->getName() + "' has no subscenario named '" + (*i).value + "'.");
                } else
                    throw
                        CException((CString)("Error: SADF graph '") + Graph->getName() + "' has no detector named '" + (*i).key + "'.");

            for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
                if (SubScenarios[i] == SADF_UNDEFINED)
                    throw CException((CString)("Error: No subscenario specified for detector '") + Graph->getDetector(i)->getName() + "'.");
        
            if (!SADF_Verify_SDF(Graph)) {

                Graph = SADF_FixScenarios(Graph, SubScenarios);
                
//                SADF_Graph* NewGraph = SADF_FixScenarios(Graph, SubScenarios);
//                delete Graph;
//                Graph = NewGraph;
            }
        
		} else
			throw CException("Unknown transformation.");
	}
    
	SADF2XML(Graph, out);
}

/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv) 
{
	int exit_status = 0;
	ofstream out;
	SADF_Graph* Graph;
    
	try     {
        // Initialize the program
        initSettings(argc, argv);

        // Set output stream
        if (!settings.outputFile.empty())   
            out.open(settings.outputFile.c_str());
        else
            ((ostream&)(out)).rdbuf(cout.rdbuf());
        
    	// Construction SDF graph model
	Graph = SADF_ConstructGraph(settings.xmlAppGraph, 0);

	// Perform the actual function
	SADF_TransformGraph(Graph, settings.arguments, out);

	delete Graph;

    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
