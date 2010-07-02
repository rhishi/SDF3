/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3verify.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   30 September 2007
 *
 *  Function        :   SADF Graph Verification Functionality
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

#include "sdf3verify.h"
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
    out << "Usage: " << TOOL << " --graph <file> --check <proporty>";
    out << " [--output <file>]" << endl;
    out << "   --graph  <file>     input SADF graph" << endl;
    out << "   --output <file>     output file (default: stdout)" << endl;
    out << "   --check <property>  verify requested property for graph:" << endl;
    out << "       connected_graph" << endl;
    out << "       is_timed" << endl;
//    out << "       is_bounded" << endl;
//    out << "       strong_consistency" << endl;
//    out << "       weak_consistency" << endl;
    out << "       ergodicity" << endl;
    out << "       moc" << endl;
    out << "       is_hsdf" << endl;
    out << "       is_sdf" << endl;
    out << "       is_csdf" << endl;
    out << "       simple_sdf_collection" << endl;
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
        else if (argv[arg] == CString("--check") && arg+1<argc)
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
 * SADF_VerifyGraph()
 * The function that calls the actual verification functions
 */

void SADF_VerifyGraph(SADF_Graph* Graph, CPairs &property, ostream &out) {

	if (property.front().key == "connected_graph")
		out << "SADF graph '" << Graph->getName() << ((SADF_Verify_SingleComponent(Graph)) ? "' is " : "' is not ") << "connected." << endl;

	else if (property.front().key == "is_timed")
		out << "SADF graph '" << Graph->getName() << ((SADF_Verify_Timed(Graph)) ? "' is " : "' is not ") << "timed." << endl;

//	else if (property.front().key == "is_bounded")
//		out << "SADF graph '" << Graph->getName() << "' is " << ((SADF_Verify_Boundedness(Graph)) ? "bounded." : "unbounded.") << endl;

//	else if (property.front().key == "strong_consistency")
//		out << "SADF graph '" << Graph->getName() << ((SADF_Verify_StrongConsistency(Graph)) ? "' is " : "' is not ") << "strongly consistent." << endl;

//	else if (property.front().key == "weak_consistency")
//		out << "SADF graph '" << Graph->getName() << ((SADF_Verify_WeakConsistency(Graph)) ? "' is " : "' is not ") << "weakly consistent." << endl;

	else if (property.front().key == "ergodicity") {
	
		if (SADF_Verify_SimpleErgodicity(Graph))
			out << "SADF graph '" << Graph->getName() << "' is ergodic." << endl;
		else
			out << "Ergodicity of SADF graph '" << Graph->getName() << "' could not be established for sure based on currently implemented (simple) test." << endl;

	} else if (property.front().key == "moc") {

		CString MoC = SADF_Determine_MoC(Graph);
		if (MoC == "HSDF" || MoC == "SDF")	
			out << "SADF graph '" << Graph->getName() << "' actually represents an " << MoC << " graph." << endl;
		else if (MoC == "CSDF")
			out << "SADF graph '" << Graph->getName() << "' actually represents a CSDF graph." << endl;
		else
			out << "SADF graph '" << Graph->getName() << "' is really an SADF graph." << endl;

	} else if (property.front().key == "is_hsdf")
		out << "SADF graph '" << Graph->getName() << ((SADF_Verify_HSDF(Graph)) ? "' represents " : "' does not represent ") << "an HSDF graph." << endl;

	else if (property.front().key == "is_sdf")
		out << "SADF graph '" << Graph->getName() << ((SADF_Verify_SDF(Graph)) ? "' represents " : "' does not represent ") << "an SDF graph." << endl;

	else if (property.front().key == "is_csdf")
		out << "SADF graph '" << Graph->getName() << ((SADF_Verify_CSDF(Graph)) ? "' represents " : "' does not represent ") << "a CSDF graph." << endl;

	else if (property.front().key == "simple_sdf_collection") {
		
            list<SDFgraph*> Collection;
            
            out << "SADF graph '" << Graph->getName() << ((SADF_Verify_SimpleStronglyConsistentSDFCollection(Graph, Collection)) ? "' is " : "' isn't ");
            out << "a strongly consistent SADF graph with " << Graph->getNumberOfDetectors() << " Detector(s) and fixed execution times per scenario." << endl;
        }

	else
		throw CException("Unknown property.");
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
	SADF_VerifyGraph(Graph, settings.arguments, out);

	delete Graph;

    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
