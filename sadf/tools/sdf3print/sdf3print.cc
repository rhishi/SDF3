/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3print.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   30 September 2007
 *
 *  Function        :   SADF Graph Print Functionality
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

#include "sdf3print.h"
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
    out << "Usage: " << TOOL << " --graph <file> --format <type>";
    out << " [--output <file>]" << endl;
    out << "   --graph  <file>     input SADF graph" << endl;
    out << "   --output <file>     output file (default: stdout)" << endl;
    out << "   --format <type>     output format:" << endl;
    out << "       dot" << endl;
    out << "       html[(dot(<location>))]" << endl;			// Actually: html[(dot(<location>),target(<location>),url(<URL>)]
    out << "       php[(dot(<location>))]" << endl;			// Actually: html[(dot(<location>),target(<location>),url(<URL>)]
    out << "       poosl[(settings(<file>),log(<file>))]" << endl;
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
        else if (argv[arg] == CString("--format") && arg+1<argc)
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
 * SADF_PrintGraph()
 * The function that calls the actual printing functions
 */
void SADF_PrintGraph(SADF_Graph *Graph, CPairs &format, ostream &out) {

	if (format.front().key == "dot")
		SADF2DOT(Graph,out);
	else if (format.front().key == "html") {
		
		CString url = "";
		CString dot = "";
		CString target = "";

		CPairs options = parseSwitchArgument(format.front().value);

		// The "url" and "target" options are a hidden (used for on-line visualisation)

		for (CPairsIter i = options.begin(); i != options.end(); i++) {
			if ((*i).key == "url")
				url = (*i).value;
			if ((*i).key == "dot")
				dot = (*i).value;
			if ((*i).key == "target")
				target = (*i).value;
		}
	
		SADF2HTML(Graph, false, dot, target, url, out);
		
	} else if (format.front().key == "php") {
		
		CString url = "";
		CString dot = "";
		CString target = "";

		CPairs options = parseSwitchArgument(format.front().value);

		// The "url" and "target" options are a hidden (used for on-line visualisation)

		for (CPairsIter i = options.begin(); i != options.end(); i++) {
			if ((*i).key == "url")
				url = (*i).value;
			if ((*i).key == "dot")
				dot = (*i).value;
			if ((*i).key == "target")
				target = (*i).value;
		}
	
		SADF2HTML(Graph, true, dot, target, url, out);
		
	} else if (format.front().key == "poosl") {

		CString LogFile = "results.log";
		CString SettingsFile = "";

		CPairs options = parseSwitchArgument(format.front().value);

		for (CPairsIter i = options.begin(); i != options.end(); i++) {
			if ((*i).key == "settings")
				SettingsFile = (*i).value;
			if ((*i).key == "log")
				LogFile = (*i).value;
		}

		SADF_SimulationSettings* Settings = new SADF_SimulationSettings(Graph);

		if (SettingsFile != "") {

			CDoc* SettingsDoc = CParseFile(SettingsFile);
			CNode* SettingsNode = CGetRootNode(SettingsDoc);
			
			if (SettingsNode == NULL)
				throw CException("Invalid XML file - missing settings");
			
			Settings = SADF_ParseSimulationSettings(Graph, Settings, SettingsNode);
		}

		SADF2POOSL(Graph, Settings, LogFile, out);
		
		delete Settings;
	} else
		throw CException("Unknown output format.");
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
	SADF_PrintGraph(Graph, settings.arguments, out);

	delete Graph;

    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
