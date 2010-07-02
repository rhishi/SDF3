/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3analyze.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   30 September 2007
 *
 *  Function        :   SADF Graph Analysis Functionality
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

#include "sdf3analyze.h"
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
    out << "   --graph  <file>      input SADF graph" << endl;
    out << "   --output <file>      output file (default: stdout)" << endl;
    out << "   --compute <property> compute requested property for graph:" << endl;
    out << "       number_of_states[(resolve_non_determinism)]" << endl;
    out << "       throughput[(<process>)]" << endl;
    out << "       inter_firing_latency(minimum|maximum|average|variance[,process(<process>)])" << endl;
    out << "       response_delay(minimum|maximum|expected[,process(<process>)])" << endl;
    out << "       deadline_miss(response|periodic,process(<process>),deadline(<value>))" << endl;
    out << "       buffer_occupancy(maximum|average|variance[,channel(<channel>)])" << endl;
//    out << "       buffer_size[(<channel>)]" << endl;
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
        else if (argv[arg] == CString("--compute") && arg+1<argc)
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
 * SADF_PrintPerformanceResultForProcess()
 * The function that performs the printing of the result
 */

void SADF_PrintPerformanceResultForProcess(ostream& out, const CString &ProcessName, CId ProcessType, CDouble Result, CSize NumberOfConfigurations, CTimer T) {

	if (ProcessType == SADF_KERNEL)
		out << "Kernel '";
	else
		out << "Detector '";
	
	out << ProcessName << "': " << Result << "	(#States: " << NumberOfConfigurations << ", Analysis Time: ";
	printTimer(out, &T);
	out << ")" << endl;
}

void SADF_PrintPerformanceResultForChannel(ostream& out, const CString &ChannelName, CId ChannelType, CDouble Result, CSize NumberOfConfigurations, CTimer T) {

	if (ChannelType == SADF_DATA_CHANNEL)
		out << "Data channel '";
	else
		out << "Control channel '";
	
	out << ChannelName << "': " << Result << "	(#States: " << NumberOfConfigurations << ", Analysis Time: ";
	printTimer(out, &T);
	out << ")" << endl;
}

/**
 * SADF_AnalyzeGraph()
 * The function that calls the actual analysis functions
 */

void SADF_AnalyzeGraph(SADF_Graph* Graph, CPairs &analyze, ostream &out) {

	if (analyze.front().key == "number_of_states") {

		// Determine metric type
	
	    CPairs options = parseSwitchArgument(analyze.front().value);

		bool ResolveNonDeterminism = false;

		for (CPairsIter i = options.begin(); i != options.end(); i++)
			if ((*i).key == "resolve_non_determinism")
				ResolveNonDeterminism = true;

		if (ResolveNonDeterminism)
			out << "SADF graph '" << Graph->getName() << "' implies a Markov chain with " << SADF_Analyse_NumberOfStates_Resolved(Graph) << " unique states." << endl;
		else
			out << "SADF graph '" << Graph->getName() << "' implies a Markov Decision Process with " << SADF_Analyse_NumberOfStates(Graph) << " unique states." << endl;
	
	} else if (analyze.front().key == "inter_firing_latency") {

		// Determine metric type
	
	    CPairs options = parseSwitchArgument(analyze.front().value);

		CString MetricType = "";
		CString ProcessName = "";

		for (CPairsIter i = options.begin(); i != options.end(); i++) {
			if ((*i).key == "minimum" || (*i).key == "maximum" || (*i).key == "average" || (*i).key == "variance")
				MetricType = (*i).key;
			if ((*i).key == "process")
				ProcessName = (*i).value;
		}

		if (MetricType == "")
			throw CException("Error: Missing type of inter_firing_latency [minimum, maximum, average, variance].");

		// Determine process to monitor
	
		CId ProcessType = SADF_UNDEFINED;
		CId ProcessID = SADF_UNDEFINED;

		SADF_Process* Process = Graph->getKernel(ProcessName);
	
		if (Process == NULL)
			Process = Graph->getDetector(ProcessName);
	
		if (Process != NULL) {
			ProcessType = Process->getType();
			ProcessID = Process->getIdentity();
		} else
			if (ProcessName != "")
				throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not have a kernel or detector named '" + ProcessName + "'.");

		CDouble Average = 0;
		CDouble Variance = 0;
		CDouble Minimum = 0;
		CDouble Maximum = 0;

		CSize NumberOfConfigurations = 0;

		if (Process == NULL) {

			if (MetricType == "average")
				out << "---- Average inter-firing latencies of all processes for SADF graph '" << Graph->getName() << "' ----" << endl;
			else if (MetricType == "variance")
				out << "---- Variance in inter-firing latencies of all processes for SADF graph '" << Graph->getName() << "' ----" << endl;
			else if (MetricType == "minimum")
				out << "---- Minimum inter-firing latencies of all processes for SADF graph '" << Graph->getName() << "' ----" << endl;
			else
				out << "---- Maximum inter-firing latencies of all processes for SADF graph '" << Graph->getName() << "' ----" << endl;
				
			for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {
			
				CTimer T;
				startTimer(&T);
				
				if (MetricType == "average" || MetricType == "variance")
					NumberOfConfigurations = SADF_Analyse_LongRunInterFiringLatency(Graph, SADF_KERNEL, i, Average, Variance);
				else
					NumberOfConfigurations = SADF_Analyse_ExtreemInterFiringLatency(Graph, SADF_KERNEL, i, Minimum, Maximum);
				
				stopTimer(&T);

				if (MetricType == "average")
					SADF_PrintPerformanceResultForProcess(out, Graph->getKernel(i)->getName(), SADF_KERNEL, Average, NumberOfConfigurations, T);
				else if (MetricType == "variance")
					SADF_PrintPerformanceResultForProcess(out, Graph->getKernel(i)->getName(), SADF_KERNEL, Variance, NumberOfConfigurations, T);
				else if (MetricType == "minimum")
					SADF_PrintPerformanceResultForProcess(out, Graph->getKernel(i)->getName(), SADF_KERNEL, Minimum, NumberOfConfigurations, T);
				else
					SADF_PrintPerformanceResultForProcess(out, Graph->getKernel(i)->getName(), SADF_KERNEL, Maximum, NumberOfConfigurations, T);
			}

			for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {
			
				CTimer T;
				startTimer(&T);
				
				if (MetricType == "average" || MetricType == "variance")
					NumberOfConfigurations = SADF_Analyse_LongRunInterFiringLatency(Graph, SADF_DETECTOR, i, Average, Variance);
				else
					NumberOfConfigurations = SADF_Analyse_ExtreemInterFiringLatency(Graph, SADF_DETECTOR, i, Minimum, Maximum);
				
				stopTimer(&T);

				if (MetricType == "average")
					SADF_PrintPerformanceResultForProcess(out, Graph->getDetector(i)->getName(), SADF_DETECTOR, Average, NumberOfConfigurations, T);
				else if (MetricType == "variance")
					SADF_PrintPerformanceResultForProcess(out, Graph->getDetector(i)->getName(), SADF_DETECTOR, Variance, NumberOfConfigurations, T);
				else if (MetricType == "minimum")
					SADF_PrintPerformanceResultForProcess(out, Graph->getDetector(i)->getName(), SADF_DETECTOR, Minimum, NumberOfConfigurations, T);
				else
					SADF_PrintPerformanceResultForProcess(out, Graph->getDetector(i)->getName(), SADF_DETECTOR, Maximum, NumberOfConfigurations, T);
			}

		} else {

			if (MetricType == "average")
				out << "---- Average inter-firing latency of one process for SADF graph '" << Graph->getName() << "' ----" << endl;
			else if (MetricType == "variance")
				out << "---- Variance in inter-firing latency of one process for SADF graph '" << Graph->getName() << "' ----" << endl;
			else if (MetricType == "minimum")
				out << "---- Minimum inter-firing latency of one process for SADF graph '" << Graph->getName() << "' ----" << endl;
			else
				out << "---- Maximum inter-firing latency of one process for SADF graph '" << Graph->getName() << "' ----" << endl;

			CTimer T;
			startTimer(&T);
	
			if (MetricType == "average" || MetricType == "variance")
				NumberOfConfigurations = SADF_Analyse_LongRunInterFiringLatency(Graph, ProcessType, ProcessID, Average, Variance);
			else
				NumberOfConfigurations = SADF_Analyse_ExtreemInterFiringLatency(Graph, ProcessType, ProcessID, Minimum, Maximum);
			
			stopTimer(&T);

			if (MetricType == "average")
				SADF_PrintPerformanceResultForProcess(out, Process->getName(), ProcessType, Average, NumberOfConfigurations, T);
			else if (MetricType == "variance")
				SADF_PrintPerformanceResultForProcess(out, Process->getName(), ProcessType, Variance, NumberOfConfigurations, T);
			else if (MetricType == "minimum")
				SADF_PrintPerformanceResultForProcess(out, Process->getName(), ProcessType, Minimum, NumberOfConfigurations, T);
			else
				SADF_PrintPerformanceResultForProcess(out, Process->getName(), ProcessType, Maximum, NumberOfConfigurations, T);
		}
		
	} else if (analyze.front().key == "throughput") {

		// Determine metric type
	
	        CPairs options = parseSwitchArgument(analyze.front().value);

		CString MetricType = "";
		CString ProcessName = "";

		for (CPairsIter i = options.begin(); i != options.end(); i++)
			if ((*i).key != "")
				ProcessName = (*i).key;

		// Determine process to monitor
	
		CId ProcessType = SADF_UNDEFINED;
		CId ProcessID = SADF_UNDEFINED;

		SADF_Process* Process = Graph->getKernel(ProcessName);
	
		if (Process == NULL)
			Process = Graph->getDetector(ProcessName);
	
		if (Process != NULL) {
			ProcessType = Process->getType();
			ProcessID = Process->getIdentity();
		} else
			if (ProcessName != "")
				throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not have a kernel or detector named '" + ProcessName + "'.");

		CDouble Average = 0;
		CDouble Variance = 0;

		CSize NumberOfConfigurations = 0;

		if (Process == NULL) {

			out << "---- Throughput of all processes for SADF graph '" << Graph->getName() << "' ----" << endl;
				
			for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {
			
				CTimer T;
				startTimer(&T);
				NumberOfConfigurations = SADF_Analyse_LongRunInterFiringLatency(Graph, SADF_KERNEL, i, Average, Variance);
				stopTimer(&T);
				SADF_PrintPerformanceResultForProcess(out, Graph->getKernel(i)->getName(), SADF_KERNEL, 1 / Average, NumberOfConfigurations, T);
			}

			for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {
			
				CTimer T;
				startTimer(&T);
				NumberOfConfigurations = SADF_Analyse_LongRunInterFiringLatency(Graph, SADF_DETECTOR, i, Average, Variance);
				stopTimer(&T);
				SADF_PrintPerformanceResultForProcess(out, Graph->getDetector(i)->getName(), SADF_DETECTOR, 1 / Average, NumberOfConfigurations, T);
			}

		} else {

			out << "---- Throughput of one process for SADF graph '" << Graph->getName() << "' ----" << endl;

			CTimer T;
			startTimer(&T);
			NumberOfConfigurations = SADF_Analyse_LongRunInterFiringLatency(Graph, ProcessType, ProcessID, Average, Variance);
			stopTimer(&T);
			SADF_PrintPerformanceResultForProcess(out, Process->getName(), ProcessType, 1 / Average, NumberOfConfigurations, T);
		}

	} else 	if (analyze.front().key == "response_delay") {

		// Determine metric type
	
	        CPairs options = parseSwitchArgument(analyze.front().value);

		CString MetricType = "";
		CString ProcessName = "";

		for (CPairsIter i = options.begin(); i != options.end(); i++) {
			if ((*i).key == "minimum" || (*i).key == "maximum" || (*i).key == "expected")
				MetricType = (*i).key;
			if ((*i).key == "process")
				ProcessName = (*i).value;
		}

		if (MetricType == "")
			throw CException("Error: Missing type of response_delay [minimum, maximum, expected].");

		// Determine process to monitor
	
		CId ProcessType = SADF_UNDEFINED;
		CId ProcessID = SADF_UNDEFINED;

		SADF_Process* Process = Graph->getKernel(ProcessName);
	
		if (Process == NULL)
			Process = Graph->getDetector(ProcessName);
	
		if (Process != NULL) {
			ProcessType = Process->getType();
			ProcessID = Process->getIdentity();
		} else
			if (ProcessName != "")
				throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not have a kernel or detector named '" + ProcessName + "'.");

		CDouble Expected = 0;
		CDouble Minimum = 0;
		CDouble Maximum = 0;

		CSize NumberOfConfigurations = 0;

		if (Process == NULL) {

			if (MetricType == "expected")
				out << "---- Expected response delay of all processes for SADF graph '" << Graph->getName() << "' ----" << endl;
			else if (MetricType == "minimum")
				out << "---- Minimum response delays of all processes for SADF graph '" << Graph->getName() << "' ----" << endl;
			else
				out << "---- Maximum response delays of all processes for SADF graph '" << Graph->getName() << "' ----" << endl;
				
			for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {
			
				CTimer T;
				startTimer(&T);
				
				NumberOfConfigurations = SADF_Analyse_ResponseDelay(Graph, SADF_KERNEL, i, Expected, Minimum, Maximum);
				
				stopTimer(&T);

				if (MetricType == "expected")
					SADF_PrintPerformanceResultForProcess(out, Graph->getKernel(i)->getName(), SADF_KERNEL, Expected, NumberOfConfigurations, T);
				else if (MetricType == "minimum")
					SADF_PrintPerformanceResultForProcess(out, Graph->getKernel(i)->getName(), SADF_KERNEL, Minimum, NumberOfConfigurations, T);
				else
					SADF_PrintPerformanceResultForProcess(out, Graph->getKernel(i)->getName(), SADF_KERNEL, Maximum, NumberOfConfigurations, T);
			}

			for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {
			
				CTimer T;
				startTimer(&T);
				
				NumberOfConfigurations = SADF_Analyse_ResponseDelay(Graph, SADF_DETECTOR, i, Expected, Minimum, Maximum);
				
				stopTimer(&T);

				if (MetricType == "expected")
					SADF_PrintPerformanceResultForProcess(out, Graph->getDetector(i)->getName(), SADF_DETECTOR, Expected, NumberOfConfigurations, T);
				else if (MetricType == "minimum")
					SADF_PrintPerformanceResultForProcess(out, Graph->getDetector(i)->getName(), SADF_DETECTOR, Minimum, NumberOfConfigurations, T);
				else
					SADF_PrintPerformanceResultForProcess(out, Graph->getDetector(i)->getName(), SADF_DETECTOR, Maximum, NumberOfConfigurations, T);
			}

		} else {

			if (MetricType == "expected")
				out << "---- Expected response delay of one process for SADF graph '" << Graph->getName() << "' ----" << endl;
			else if (MetricType == "minimum")
				out << "---- Minimum response delay of one process for SADF graph '" << Graph->getName() << "' ----" << endl;
			else
				out << "---- Maximum response delay of one process for SADF graph '" << Graph->getName() << "' ----" << endl;

			CTimer T;
			startTimer(&T);
	
			NumberOfConfigurations = SADF_Analyse_ResponseDelay(Graph, ProcessType, ProcessID, Expected, Minimum, Maximum);
			
			stopTimer(&T);

			if (MetricType == "expected")
				SADF_PrintPerformanceResultForProcess(out, Process->getName(), ProcessType, Expected, NumberOfConfigurations, T);
			else if (MetricType == "minimum")
				SADF_PrintPerformanceResultForProcess(out, Process->getName(), ProcessType, Minimum, NumberOfConfigurations, T);
			else
				SADF_PrintPerformanceResultForProcess(out, Process->getName(), ProcessType, Maximum, NumberOfConfigurations, T);
		}

	} else if (analyze.front().key == "deadline_miss") {

		// Determine metric type
	
	        CPairs options = parseSwitchArgument(analyze.front().value);

		CString MetricType = "";
		CString ProcessName = "";
		CDouble Deadline = 0;
		bool DeadlineDefined = false;
		
		for (CPairsIter i = options.begin(); i != options.end(); i++) {
			if ((*i).key == "periodic" || (*i).key == "response")
				MetricType = (*i).key;
			if ((*i).key == "process")
				ProcessName = (*i).value;
			if ((*i).key == "deadline") {
				Deadline = (*i).value;
				DeadlineDefined = true;
			}
		}

		if (MetricType == "")
			throw CException("Error: Missing type of deadline_miss [response, periodic].");

		if (ProcessName == "")
			throw CException("Error: Missing process name.");

		if (!DeadlineDefined)
			throw CException("Error: No deadline specified.");
		else if (Deadline <= 0)
			throw CException("Error: Deadline must be positive.");

		// Determine process to monitor
	
		CId ProcessType = SADF_UNDEFINED;
		CId ProcessID = SADF_UNDEFINED;

		SADF_Process* Process = Graph->getKernel(ProcessName);
	
		if (Process == NULL)
			Process = Graph->getDetector(ProcessName);
	
		if (Process != NULL) {
			ProcessType = Process->getType();
			ProcessID = Process->getIdentity();
		} else
			if (ProcessName != "")
				throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not have a kernel or detector named '" + ProcessName + "'.");

		CDouble Probability = 0;
		
		CSize NumberOfConfigurations = 0;

		if (MetricType == "periodic")
			out << "---- Periodic deadline miss probability of one process for SADF graph '" << Graph->getName() << "' ----" << endl;
		else
			out << "---- Response deadline miss probability of one process for SADF graph '" << Graph->getName() << "' ----" << endl;

		CTimer T;
		startTimer(&T);
	
		if (MetricType == "periodic")
			NumberOfConfigurations = SADF_Analyse_PeriodicDeadlineMissProbability(Graph, ProcessType, ProcessID, Deadline, Probability);
		else
			NumberOfConfigurations = SADF_Analyse_ResponseDeadlineMissProbability(Graph, ProcessType, ProcessID, Deadline, Probability);			

		stopTimer(&T);
		SADF_PrintPerformanceResultForProcess(out, Process->getName(), ProcessType, Probability, NumberOfConfigurations, T);

	} else 	if (analyze.front().key == "buffer_occupancy") {

		// Determine metric type
	
	        CPairs options = parseSwitchArgument(analyze.front().value);

		CString MetricType = "";
		CString ChannelName = "";

		for (CPairsIter i = options.begin(); i != options.end(); i++) {
			if ((*i).key == "maximum" || (*i).key == "average" || (*i).key == "variance")
				MetricType = (*i).key;
			if ((*i).key == "channel")
				ChannelName = (*i).value;
		}

		if (MetricType == "")
			throw CException("Error: Missing type of buffer_occupancy [maximum, average, variance].");

		// Determine channel to monitor
	
		CId ChannelType = SADF_UNDEFINED;
		CId ChannelID = SADF_UNDEFINED;

		SADF_Channel* Channel = Graph->getChannel(ChannelName);
	
		if (Channel != NULL) {
			ChannelType = Channel->getType();
			ChannelID = Channel->getIdentity();
		} else
			if (ChannelName != "")
				throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not have a channel named '" + ChannelName + "'.");

		CDouble Maximum = 0;
		CDouble Average = 0;
		CDouble Variance = 0;

		CSize NumberOfConfigurations = 0;

		if (Channel == NULL) {

			if (MetricType == "maximum")
				out << "---- Maximum buffer occupancy of all channels for SADF graph '" << Graph->getName() << "' ----" << endl;
			else if (MetricType == "average")
				out << "---- Average buffer occupancy of all channels for SADF graph '" << Graph->getName() << "' ----" << endl;
			else
				out << "---- Variance in buffer occupancy of all channels for SADF graph '" << Graph->getName() << "' ----" << endl;
				
			for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++) {
			
				CTimer T;
				startTimer(&T);
				
				if (MetricType == "maximum")
					NumberOfConfigurations = SADF_Analyse_MaximumBufferOccupancy(Graph, SADF_DATA_CHANNEL, i, Maximum);
				else
					NumberOfConfigurations = SADF_Analyse_LongRunBufferOccupancy(Graph, SADF_DATA_CHANNEL, i, Average, Variance);
				
				stopTimer(&T);

				if (MetricType == "maximum")
					SADF_PrintPerformanceResultForChannel(out, Graph->getDataChannel(i)->getName(), SADF_DATA_CHANNEL, Maximum, NumberOfConfigurations, T);
				else if (MetricType == "average")
					SADF_PrintPerformanceResultForChannel(out, Graph->getDataChannel(i)->getName(), SADF_DATA_CHANNEL, Average, NumberOfConfigurations, T);
				else
					SADF_PrintPerformanceResultForChannel(out, Graph->getDataChannel(i)->getName(), SADF_DATA_CHANNEL, Variance, NumberOfConfigurations, T);
			}

			for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++) {
			
				CTimer T;
				startTimer(&T);
				
				if (MetricType == "maximum")
					NumberOfConfigurations = SADF_Analyse_MaximumBufferOccupancy(Graph, SADF_CONTROL_CHANNEL, i, Maximum);
				else
					NumberOfConfigurations = SADF_Analyse_LongRunBufferOccupancy(Graph, SADF_CONTROL_CHANNEL, i, Average, Variance);

				stopTimer(&T);

				if (MetricType == "maximum")
					SADF_PrintPerformanceResultForChannel(out, Graph->getControlChannel(i)->getName(), SADF_CONTROL_CHANNEL, Maximum, NumberOfConfigurations, T);
				else if (MetricType == "average")
					SADF_PrintPerformanceResultForChannel(out, Graph->getControlChannel(i)->getName(), SADF_CONTROL_CHANNEL, Average, NumberOfConfigurations, T);
				else
					SADF_PrintPerformanceResultForChannel(out, Graph->getControlChannel(i)->getName(), SADF_CONTROL_CHANNEL, Variance, NumberOfConfigurations, T);
			}

		} else {

			if (MetricType == "maximum")
				out << "---- Maximum buffer occupancy of one channel for SADF graph '" << Graph->getName() << "' ----" << endl;
			else if (MetricType == "average")
				out << "---- Average buffer occupancy of one channel for SADF graph '" << Graph->getName() << "' ----" << endl;
			else
				out << "---- Variance in buffer occupancy of one channel for SADF graph '" << Graph->getName() << "' ----" << endl;

			CTimer T;
			startTimer(&T);
	
			if (MetricType == "maximum")
				NumberOfConfigurations = SADF_Analyse_MaximumBufferOccupancy(Graph, ChannelType, ChannelID, Maximum);
			else
				NumberOfConfigurations = SADF_Analyse_LongRunBufferOccupancy(Graph, ChannelType, ChannelID, Average, Variance);
			
			stopTimer(&T);

			if (MetricType == "maximum")
				SADF_PrintPerformanceResultForChannel(out, ChannelName, ChannelType, Maximum, NumberOfConfigurations, T);
			else if (MetricType == "average")
				SADF_PrintPerformanceResultForChannel(out, ChannelName, ChannelType, Average, NumberOfConfigurations, T);
			else
				SADF_PrintPerformanceResultForChannel(out, ChannelName, ChannelType, Variance, NumberOfConfigurations, T);
		}

/*	} else if (analyze.front().key == "buffer_size") {

		// Determine metric type
	
	        CPairs options = parseSwitchArgument(analyze.front().value);

		CString ChannelName = "";

		for (CPairsIter i = options.begin(); i != options.end(); i++)
			if ((*i).key != "")
				ChannelName = (*i).key;

		// Determine channel to monitor
	
		CId ChannelType = SADF_UNDEFINED;
		CId ChannelID = SADF_UNDEFINED;

		SADF_Channel* Channel = Graph->getChannel(ChannelName);
	
		if (Channel != NULL) {
			ChannelType = Channel->getType();
			ChannelID = Channel->getIdentity();
		} else
			if (ChannelName != "")
				throw CException((CString)("Error: SADF graph '") + Graph->getName() + "' does not have a channel named '" + ChannelName + "'.");

		CDouble Result = 0;

		CSize NumberOfConfigurations = 0;

		if (Channel == NULL) {

			out << "---- Buffer sizes of all channels for SADF graph '" << Graph->getName() << "' ----" << endl;
				
			for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++) {
			
				CTimer T;
				startTimer(&T);
				NumberOfConfigurations = SADF_Analyse_BufferSize(Graph, SADF_DATA_CHANNEL, i, Result);
				stopTimer(&T);
				SADF_PrintPerformanceResultForChannel(out, Graph->getDataChannel(i)->getName(), SADF_DATA_CHANNEL, Result, NumberOfConfigurations, T);
			}

			for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++) {
			
				CTimer T;
				startTimer(&T);
				NumberOfConfigurations = SADF_Analyse_BufferSize(Graph, SADF_CONTROL_CHANNEL, i, Result);
				stopTimer(&T);
				SADF_PrintPerformanceResultForChannel(out, Graph->getControlChannel(i)->getName(), SADF_CONTROL_CHANNEL, Result, NumberOfConfigurations, T);
			}

		} else {

			out << "---- Buffer size of one channel for SADF graph '" << Graph->getName() << "' ----" << endl;

			CTimer T;
			startTimer(&T);
			NumberOfConfigurations = SADF_Analyse_BufferSize(Graph, ChannelType, ChannelID, Result);
			stopTimer(&T);
			SADF_PrintPerformanceResultForChannel(out, Channel->getName(), ChannelType, Result, NumberOfConfigurations, T);
		}*/
	} else
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
	SADF_AnalyzeGraph(Graph, settings.arguments, out);

	delete Graph;

    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
