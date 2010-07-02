/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2poosl_data.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF graph in POOSL format (data classes)
 *
 *  History         :
 *      13-09-06    :   Initial version.
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

#include "sadf2poosl_data.h"

void SADF2POOSL_Data(ostream &out) {

	// Generate Non-Primitive Data Class MarkovChain
	
	out << "data class MarkovChain" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "NumberOfStates: Integer, StateSpace: Array, SubScenarios: Array, Transitions: Array, CurrentState: String, Random: Uniform" << endl;
	out << "instance methods" << endl;
	
	out << "init : MarkovChain" << endl << endl;
	out << "NumberOfStates := 0;" << endl;
	out << "StateSpace := new(Array);" << endl;
	out << "SubScenarios := new(Array);" << endl;
	out << "Transitions := new(Array);" << endl;
	out << "Random := new(Uniform) withParameters(0, 1);" << endl;
	out << "return(self)." << endl << endl;

	out << "addState(State: String, SubScenario: Integer) : MarkovChain" << endl << endl;
	out << "NumberOfStates := NumberOfStates + 1;" << endl;
	out << "StateSpace size(NumberOfStates); StateSpace put(NumberOfStates, State);" << endl;
	out << "SubScenarios size(NumberOfStates); SubScenarios put(NumberOfStates, SubScenario);" << endl;
	out << "Transitions size(NumberOfStates); Transitions put(NumberOfStates, new(Array));" << endl;
	out << "return(self)." << endl << endl;
	
	out << "addTransition(State: String, NextState: String, LowerBound: Real, UpperBound: Real) : MarkovChain |i, StateNumber: Integer, T, A: Array|" << endl << endl;
	out << "i := 1;" << endl;
	out << "while i <= StateSpace getSize do" << endl;
	out << "   if StateSpace get(i) = State then StateNumber := i fi;" << endl;
	out << "   i := i + 1" << endl;
	out << "od;" << endl;
	out << "T := Transitions get(StateNumber);" << endl;
	out << "A := new(Array) size(3); A put(1, NextState) put(2, LowerBound) put(3, UpperBound);" << endl;
	out << "T size(T getSize + 1); T put(T getSize, A);" << endl;
	out << "return(self)." << endl << endl;

	out << "setInitialState(State: String) : MarkovChain" << endl << endl;
	out << "CurrentState := State;" << endl;
	out << "return(self)." << endl << endl;
	
	out << "getNextSubScenario : Integer |NextState: String, SubScenario, i, StateNumber: Integer, T: Array, Sample: Real|" << endl << endl;
	out << "i := 1;" << endl;
	out << "while i <= StateSpace getSize do" << endl;
	out << "   if StateSpace get(i) = CurrentState then StateNumber := i fi;" << endl;
	out << "   i := i + 1" << endl;
	out << "od;" << endl;
	out << "T := Transitions get(StateNumber);" << endl;
	out << "Sample := Random sample;" << endl;
	out << "i := 1;" << endl;
	out << "while i <= T getSize do" << endl;
	out << "   if ((Sample > T get(i) get(2)) & (Sample <= T get(i) get(3))) then NextState := T get(i) get(1) fi;" << endl;
	out << "   i := i + 1" << endl;
	out << "od;" << endl;
	out << "i := 1;" << endl;
	out << "while i <= StateSpace getSize do" << endl;
	out << "   if StateSpace get(i) = NextState then SubScenario := SubScenarios get(i) fi;" << endl;
	out << "   i := i + 1" << endl;
	out << "od;" << endl;
	out << "CurrentState := NextState;" << endl;
	out << "return(SubScenario)." << endl << endl;

	// Generate Non-Primitive Data Class ProcessMonitor

	out << "data class ProcessMonitor" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "Name: String, Memory: Integer, Utilisation: Integer, Throughput: LongRunTimeAverage, PreviousUpdateTime: Real, PreviousFireTime: Real, ResponseDelay: Real, ";
	out << "Monitor: Boolean, Trace: Boolean, TraceFile: FileOut, AverageDelay: LongRunSampleAverage, VarianceDelay: LongRunSampleVariance, MaximumDelay: Real, MinimumDelay: Real" << endl << endl;
	out << "instance methods" << endl;

	out << "init(M: Boolean, ID: Integer, T: Boolean, N: String) : ProcessMonitor" << endl << endl;
	out << "Utilisation := 0;" << endl;
	out << "PreviousUpdateTime := nil; PreviousFireTime:= nil;" << endl;
	out << "Name := N;" << endl;
        out << "Monitor := M;" << endl;
	out << "if Monitor then" << endl;
	out << "   ResponseDelay := nil;" << endl;
	out << "   MaximumDelay := 0.0; MinimumDelay = nil;" << endl;
	out << "   Throughput := new(LongRunRateAverage) withParameters(0.95, 0.95);" << endl;
	out << "   AverageDelay := new(LongRunSampleAverage) withParameters(0.95, 0.95);" << endl;
	out << "   VarianceDelay := new(LongRunSampleVariance) withParameters(0.95, 0.95)" << endl;
	out << "fi;" << endl;
        out << "Trace := T;" << endl;
	out << "if Trace then" << endl;
	out << "   TraceFile := new(FileOut) destination(\"Process_\" + Name + \".trace\") open;" << endl;
	out << "   TraceFile writeString(\"Traced Behaviour of Process: \" + Name cr cr)" << endl;
	out << "fi;" << endl;
	out << "return(self)." << endl << endl;

	out << "start(CurrentTime: Real) : ProcessMonitor" << endl << endl;
	out << "if PreviousUpdateTime != nil then" << endl;
	out << "   if Trace & (CurrentTime > PreviousUpdateTime) then TraceFile writeString(\"At time: \" + PreviousUpdateTime printString + \" Utilisation: \" + Utilisation printString cr) fi" << endl;
	out << "fi;" << endl;
	out << "Utilisation := Utilisation + 1;" << endl;
	out << "PreviousUpdateTime := CurrentTime;" << endl;
	out << "return(self)." << endl << endl;

	out << "end(CurrentTime: Real) : ProcessMonitor" << endl << endl;
	out << "if Trace & (CurrentTime > PreviousUpdateTime) then" << endl;
	out << "   TraceFile writeString(\"At time: \" + PreviousUpdateTime printString + \" Utilisation: \" + Utilisation printString cr)" << endl;
	out << "fi;" << endl;
	out << "Utilisation := Utilisation - 1;" << endl;
	out << "if Monitor & (ResponseDelay = nil) then ResponseDelay := CurrentTime fi;" << endl;
	out << "if Monitor & (PreviousFireTime != nil) then" << endl;
	out << "   if (CurrentTime - PreviousFireTime) > MaximumDelay then MaximumDelay := CurrentTime - PreviousFireTime fi;" << endl;
	out << "   if MinimumDelay != nil then" << endl;
	out << "      if (CurrentTime - PreviousFireTime) < MinimumDelay then MinimumDelay := CurrentTime - PreviousFireTime fi" << endl;
	out << "   else" << endl;
	out << "      MinimumDelay := CurrentTime - PreviousFireTime" << endl;
	out << "   fi;" << endl;
	out << "   Throughput rewardBM(1, CurrentTime);" << endl;
	out << "   AverageDelay rewardBM(CurrentTime - PreviousFireTime);" << endl;
	out << "   VarianceDelay rewardBM(CurrentTime - PreviousFireTime)" << endl;
	out << "fi;" << endl;
	out << "PreviousUpdateTime := CurrentTime; PreviousFireTime := CurrentTime;" << endl;
	out << "return(self)." << endl << endl;

	out << "accurate : Boolean" << endl << endl;
	out << "if Monitor then" << endl;
	out << "   return((Throughput accurate) & (AverageDelay accurate))" << endl;
	out << "else" << endl;
	out << "   return(true)" << endl;
	out << "fi." << endl << endl;

	out << "getResults : String |Result: String|" << endl << endl;
	out << "Result := \"-------------------------------\" cr + \"Statistics for Process: \" + Name cr cr;" << endl;
	out << "Result append(\"Observed Response Delay: \" + ResponseDelay printString cr);" << endl;
	out << "Result append(\"Observed Minimum Inter-Firing Latency: \" + MinimumDelay printString cr);" << endl;
	out << "Result append(\"Observed Maximum Inter-Firing Latency: \" + MaximumDelay printString cr cr);" << endl;
	out << "Result append(\"Average Inter-Firing Latency:\" cr + AverageDelay logStatistics cr cr);" << endl;
	out << "Result append(\"Variance in Inter-Firing Latency:\" cr + VarianceDelay logStatistics cr cr);" << endl;
	out << "Result append(\"Throughput:\" cr + Throughput logStatistics cr cr);" << endl;
	out << "return(Result)." << endl << endl;
	
	// Generate Non-Primitive Data Class DataBufferMonitor

	out << "data class DataBufferMonitor" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "Name: String, AvailableTokens: Integer, AverageOccupation: LongRunTimeAverage, VarianceOccupation: LongRunTimeVariance, BufferSize: Integer, MaximumOccupation: Integer, Monitor: Boolean, PreviousTime: Real, ReservedLocations: Integer, TokenSize: Integer, Trace: Boolean, TraceFile: FileOut" << endl << endl;
	out << "instance methods" << endl;

	out << "init(BS, IT, TS: Integer, M: Boolean, ID: Integer, T: Boolean, N: String) : DataBufferMonitor" << endl << endl;
	out << "Name := N;" << endl;
	out << "BufferSize := BS; TokenSize := TS; AvailableTokens := 0; ReservedLocations := 0;" << endl;
	out << "PreviousTime := 0.0;" << endl;
        out << "Monitor := M;" << endl;
	out << "if Monitor then" << endl;
	out << "   MaximumOccupation := 0;" << endl;
	out << "   AverageOccupation := new(LongRunTimeAverage) withParameters(0.95, 0.95);" << endl;
	out << "   VarianceOccupation := new(LongRunTimeVariance) withParameters(0.95, 0.95)" << endl;
	out << "fi;" << endl;
        out << "Trace := T;" << endl;
	out << "if Trace then" << endl;
	out << "   TraceFile := new(FileOut) destination(\"Channel_\" + Name + \".trace\") open;" << endl;
	out << "   TraceFile writeString(\"Traced Buffer Occupancy for Channel: \" + Name cr cr)" << endl;
	out << "fi;" << endl;
        out << "self reserve(IT, 0.0); self write(IT);" << endl;
	out << "return(self)." << endl << endl;

	out << "room(NumberOfTokens: Integer) : Boolean" << endl << endl;
	out << "if BufferSize > 0 then" << endl;
	out << "   return((ReservedLocations + AvailableTokens + NumberOfTokens) <= BufferSize)" << endl;
	out << "else" << endl;
	out << "   return(true)" << endl;
	out << "fi." << endl << endl;

	out << "available(NumberOfTokens: Integer) : Boolean" << endl << endl;
	out << "return(AvailableTokens >= NumberOfTokens)." << endl << endl;

        out << "reserve(NumberOfTokens: Integer, CurrentTime: Real) : DataBufferMonitor |Occupation: Integer|" << endl << endl;
	out << "Occupation := ReservedLocations + AvailableTokens;" << endl;
	out << "if Trace & (CurrentTime > PreviousTime) then" << endl;
	out << "   TraceFile writeString(\"At time: \" + PreviousTime printString + \" Occupation: \" + (Occupation * TokenSize) printString cr)" << endl;
	out << "fi;" << endl;
	out << "ReservedLocations := ReservedLocations + NumberOfTokens; Occupation := Occupation + NumberOfTokens;" << endl;
	out << "if Monitor then" << endl;
	out << "   if (Occupation * TokenSize) > MaximumOccupation then MaximumOccupation := Occupation * TokenSize fi;" << endl;
	out << "   AverageOccupation rewardBM(Occupation * TokenSize, CurrentTime); VarianceOccupation rewardBM(Occupation * TokenSize, CurrentTime)" << endl;
	out << "fi;" << endl;
	out << "PreviousTime := CurrentTime;" << endl;
	out << "return(self)." << endl << endl;

	out << "write(NumberOfTokens: Integer) : DataBufferMonitor" << endl << endl;
	out << "ReservedLocations := ReservedLocations - NumberOfTokens;" << endl;
	out << "AvailableTokens := AvailableTokens + NumberOfTokens;" << endl;
	out << "return(self)." << endl << endl;

	out << "remove(NumberOfTokens: Integer, CurrentTime: Real) : DataBufferMonitor |Occupation: Integer|" << endl << endl;
	out << "Occupation := ReservedLocations + AvailableTokens;" << endl;
	out << "if Trace & (CurrentTime > PreviousTime) then" << endl;
	out << "   TraceFile writeString(\"At time: \" + PreviousTime printString + \" Occupation: \" + (Occupation * TokenSize) printString cr)" << endl;
	out << "fi;" << endl;
	out << "AvailableTokens := AvailableTokens - NumberOfTokens; Occupation := Occupation - NumberOfTokens;" << endl;
	out << "if Monitor then" << endl;
	out << "   AverageOccupation rewardBM(Occupation * TokenSize, CurrentTime); VarianceOccupation rewardBM(Occupation * TokenSize, CurrentTime)" << endl;
	out << "fi;" << endl;
	out << "PreviousTime := CurrentTime;" << endl;
	out << "return(self)." << endl << endl;

	out << "accurate : Boolean" << endl << endl;
	out << "if Monitor then" << endl;
	out << "   return(AverageOccupation accurate)" << endl;
	out << "else" << endl;
	out << "   return(true)" << endl;
	out << "fi." << endl << endl;

	out << "getResults : String |Result: String|" << endl << endl;
	out << "Result := \"-------------------------------\" cr + \"Statistics for Channel: \" + Name cr cr;" << endl;
	out << "Result append(\"Observed Maximum Buffer Occupancy: \" + MaximumOccupation printString cr cr);" << endl;
	out << "Result append(\"Average Buffer Occupancy:\" cr + AverageOccupation logStatistics cr cr);" << endl;
	out << "Result append(\"Variance in Buffer Occupancy:\" cr + VarianceOccupation logStatistics cr cr);" << endl;
	out << "return(Result)." << endl << endl;

	// Generate Non-Primitive Data Class ControlBufferMonitor

	out << "data class ControlBufferMonitor" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "Name: String, AvailableTokens: Integer, AverageOccupation: LongRunTimeAverage, VarianceOccupation: LongRunTimeVariance, BufferSize: Integer, MaximumOccupation: Integer, Monitor: Boolean, PreviousTime: Real, ReservedLocations: Integer, TokenSize: Integer, Trace: Boolean, TraceFile: FileOut, Buffer: Queue" << endl << endl;
	out << "instance methods" << endl;

	out << "init(BS: Integer, NIT, CIT: Queue, TS: Integer, M: Boolean, ID: Integer, T: Boolean, N: String) : ControlBufferMonitor" << endl << endl;
	out << "Name := N;" << endl;
	out << "BufferSize := BS; TokenSize := TS; AvailableTokens := 0; ReservedLocations := 0;" << endl;
	out << "PreviousTime := 0.0;" << endl;
        out << "Monitor := M;" << endl;
	out << "if Monitor then" << endl;
	out << "   MaximumOccupation := 0;" << endl;
	out << "   AverageOccupation := new(LongRunTimeAverage) withParameters(0.95, 0.95);" << endl;
	out << "   VarianceOccupation := new(LongRunTimeVariance) withParameters(0.95, 0.95)" << endl;
	out << "fi;" << endl;
        out << "Trace := T;" << endl;
	out << "if Trace then" << endl;
	out << "   TraceFile := new(FileOut) destination(\"Channel_\" + Name + \".trace\") open;" << endl;
	out << "   TraceFile writeString(\"Traced Buffer Occupancy for Channel: \" + Name cr cr)" << endl;
	out << "fi;" << endl;
	out << "Buffer := new(Queue) init;" << endl;
	out << "while NIT isNotEmpty do" << endl;
	out << "   self reserve(NIT inspect, 0); self write(NIT inspect, CIT inspect);" << endl;
	out << "   NIT remove; CIT remove" << endl;
	out << "od;" << endl;
	out << "return(self)." << endl << endl;

	out << "room(NumberOfTokens: Integer) : Boolean" << endl << endl;
	out << "if BufferSize > 0 then" << endl;
	out << "   return((ReservedLocations + AvailableTokens + NumberOfTokens) <= BufferSize)" << endl;
	out << "else" << endl;
	out << "   return(true)" << endl;
	out << "fi." << endl << endl;

	out << "inspect() : Integer" << endl << endl;
	out << "return(Buffer inspect)." << endl << endl;

        out << "reserve(NumberOfTokens: Integer, CurrentTime: Real) : ControlBufferMonitor |Occupation: Integer|" << endl << endl;
	out << "Occupation := ReservedLocations + AvailableTokens;" << endl;
	out << "if Trace & (CurrentTime > PreviousTime) then" << endl;
	out << "   TraceFile writeString(\"At time: \" + PreviousTime printString + \" Occupation: \" + (Occupation * TokenSize) printString cr)" << endl;
	out << "fi;" << endl;
	out << "ReservedLocations := ReservedLocations + NumberOfTokens; Occupation := Occupation + NumberOfTokens;" << endl;
	out << "if Monitor then" << endl;
	out << "   if (Occupation * TokenSize) > MaximumOccupation then MaximumOccupation := Occupation * TokenSize fi;" << endl;
	out << "   AverageOccupation rewardBM(Occupation * TokenSize, CurrentTime); VarianceOccupation rewardBM(Occupation * TokenSize, CurrentTime)" << endl;
	out << "fi;" << endl;
	out << "PreviousTime := CurrentTime;" << endl;
	out << "return(self)." << endl << endl;

	out << "write(NumberOfTokens: Integer, Scenario: Integer) : ControlBufferMonitor |Counter: Integer|" << endl << endl;
	out << "ReservedLocations := ReservedLocations - NumberOfTokens;" << endl;
	out << "AvailableTokens := AvailableTokens + NumberOfTokens;" << endl;
	out << "Counter := NumberOfTokens;" << endl;
	out << "while (Counter > 0) do" << endl;
	out << "   Buffer put(Scenario);" << endl;
	out << "   Counter := Counter - 1" << endl;
	out << "od;" << endl;
	out << "return(self)." << endl << endl;

	out << "available : Boolean" << endl << endl;
	out << "return(AvailableTokens >= 1)." << endl << endl;

	out << "remove(CurrentTime: Real) : Integer |Occupation: Integer|" << endl << endl;
	out << "Occupation := ReservedLocations + AvailableTokens;" << endl;
	out << "if Trace & (CurrentTime > PreviousTime) then" << endl;
	out << "   TraceFile writeString(\"At time: \" + PreviousTime printString + \" Occupation: \" + (Occupation * TokenSize) printString cr)" << endl;
	out << "fi;" << endl;
	out << "AvailableTokens := AvailableTokens - 1; Occupation := Occupation - 1;" << endl;
	out << "if Monitor then" << endl;
	out << "   AverageOccupation rewardBM(Occupation * TokenSize, CurrentTime); VarianceOccupation rewardBM(Occupation * TokenSize, CurrentTime)" << endl;
	out << "fi;" << endl;
	out << "PreviousTime := CurrentTime;" << endl;
	out << "return(Buffer remove)." << endl << endl;

	out << "accurate : Boolean" << endl << endl;
	out << "if Monitor then" << endl;
	out << "   return(AverageOccupation accurate)" << endl;
	out << "else" << endl;
	out << "   return(true)" << endl;
	out << "fi." << endl << endl;

	out << "getResults : String |Result: String|" << endl << endl;
	out << "Result := \"-------------------------------\" cr + \"Statistics for Channel: \" + Name cr cr;" << endl;
	out << "Result append(\"Observed Maximum Buffer Occupancy: \" + MaximumOccupation printString cr cr);" << endl;
	out << "Result append(\"Average Buffer Occupancy:\" cr + AverageOccupation logStatistics cr cr);" << endl;
	out << "Result append(\"Variance in Buffer Occupancy:\" cr + VarianceOccupation logStatistics cr cr);" << endl;
	out << "return(Result)." << endl << endl;
	
	// Generate Non-Primitive Data Class MonitorStatus

	out << "data class MonitorStatus" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "Status: Array, NumberOfMonitors: Integer, LogFileName: String, Result: String" << endl;
	out << "instance methods" << endl;

	out << "init(N: Integer, LFN: String) : MonitorStatus" << endl << endl;
	out << "NumberOfMonitors := N;" << endl;
	out << "Status := new(Array) size(NumberOfMonitors) putAll(false);" << endl;
	out << "LogFileName := LFN;" << endl;
	out << "Result := \"Summary of Simulation Results\" cr cr;" << endl;
	out << "return(self)." << endl << endl;

	out << "accurate : Boolean |Accurate: Boolean, i: Integer|" << endl;
	out << "Accurate := true; i := 1;";
	out << "while i <= NumberOfMonitors do Accurate := Accurate & Status get(i); i := i + 1 od;" << endl;
	out << "return(Accurate)." << endl << endl;

	out << "register(ID: Integer, S: Boolean) : MonitorStatus" << endl;
	out << "Status put(ID, S);" << endl;
	out << "return(self)." << endl << endl;

	out << "append(R: String) : MonitorStatus" << endl << endl;
	out << "Result append(R);" << endl;
	out << "return(self)." << endl << endl;
	
	out << "log : MonitorStatus |LogFile: FileOut|" << endl << endl;
	out << "LogFile := new(FileOut) destination(LogFileName) open;" << endl;
	out << "LogFile writeString(Result);" << endl;
	out << "LogFile close;" << endl;
	out << "return(self)." << endl << endl;

	// Generate Performance Monitor Classes

	out << "data class ConfidenceInterval" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "ConfidenceLevel: Real, UpperBound: Real, LowerBound: Real" << endl << endl;
	out << "instance methods" << endl;
	out << "extendedTimes(x, y: Real) : Real" << endl << endl;
	out << "   if (x = nil) | (y = nil) then" << endl;
	out << "      return(nil)" << endl;
	out << "   else" << endl;
	out << "      return(x * y)" << endl;
	out << "   fi." << endl << endl;
	out << "extendedPlus(x, y: Real) : Real" << endl;
	out << "   if (x = nil) | (y = nil) then" << endl;
	out << "      return(nil)" << endl;
	out << "   else" << endl;
	out << "      return(x + y)" << endl;
	out << "   fi." << endl << endl;
	out << "*(CI: ConfidenceInterval) : ConfidenceInterval |Lower, Upper, Level: Real|" << endl << endl;
	out << "   if self extendedLowerGreaterEqualZero then" << endl;
	out << "      if CI extendedLowerGreaterEqualZero then" << endl;
	out << "         Lower := self extendedTimes(LowerBound, CI getLowerBound);" << endl;
	out << "         Upper := self extendedTimes(UpperBound, CI getUpperBound)" << endl;
	out << "      else" << endl;
	out << "         if CI extendedUpperLessZero then" << endl;
	out << "            Lower := self extendedTimes(UpperBound, CI getLowerBound);" << endl;
	out << "            Upper := self extendedTimes(LowerBound, CI getUpperBound)" << endl;
	out << "         else" << endl;
	out << "            Lower := self extendedTimes(UpperBound, CI getLowerBound);" << endl;
	out << "            Upper := self extendedTimes(UpperBound, CI getUpperBound)" << endl;
	out << "         fi" << endl;
	out << "      fi" << endl;
	out << "   else" << endl;
	out << "      if self extendedUpperLessZero then" << endl;
	out << "         if CI extendedLowerGreaterEqualZero then" << endl;
	out << "            Lower := self extendedTimes(LowerBound, CI getUpperBound);" << endl;
	out << "            Upper := self extendedTimes(UpperBound, CI getLowerBound)" << endl;
	out << "         else" << endl;
	out << "            if CI extendedUpperLessZero then" << endl;
	out << "               Lower := self extendedTimes(UpperBound, CI getUpperBound);" << endl;
	out << "               Upper := self extendedTimes(LowerBound, CI getLowerBound)" << endl;
	out << "            else" << endl;
	out << "               Lower := self extendedTimes(LowerBound, CI getUpperBound);" << endl;
	out << "               Upper := self extendedTimes(LowerBound, CI getLowerBound)" << endl;
	out << "            fi" << endl;
	out << "         fi" << endl;
	out << "      else" << endl;
	out << "         if CI extendedLowerGreaterEqualZero then" << endl;
	out << "            Lower := self extendedTimes(LowerBound, CI getUpperBound);" << endl;
	out << "            Upper := self extendedTimes(UpperBound, CI getUpperBound)" << endl;
	out << "         else" << endl;
	out << "            if CI extendedUpperLessZero then" << endl;
	out << "               Lower := self extendedTimes(UpperBound, CI getLowerBound);" << endl;
	out << "               Upper := self extendedTimes(LowerBound, CI getLowerBound)" << endl;
	out << "            else" << endl;
	out << "               Lower := self extendedMin(self extendedTimes(LowerBound, CI getUpperBound), self extendedTimes(UpperBound, CI getLowerBound));" << endl;
	out << "               Upper := self extendedMax(self extendedTimes(LowerBound, CI getLowerBound), self extendedTimes(UpperBound, CI getUpperBound))" << endl;
	out << "            fi" << endl;
	out << "         fi" << endl;    
	out << "      fi" << endl;
	out << "   fi;" << endl;
	out << "   Level := ConfidenceLevel + CI getConfidenceLevel - 1;" << endl;
	out << "   return(new(ConfidenceInterval) withParameters(Lower, Upper, Level))." << endl << endl;
	out << "/(CI: ConfidenceInterval) : ConfidenceInterval" << endl << endl;
	out << "   return(self * CI reciprocal)." << endl << endl;
	out << "getUpperBound() : Real" << endl << endl;
	out << "   return(UpperBound)." << endl << endl;
	out << "extendedMin(x, y: Real) : Real" << endl << endl;
	out << "   if (x = nil) | (y = nil) then" << endl;
	out << "      return(nil)" << endl;
	out << "   else" << endl;
	out << "      if x < y then" << endl;
	out << "         return(x)" << endl;
	out << "      else" << endl;
	out << "         return(y)" << endl;
	out << "      fi" << endl;
	out << "   fi." << endl << endl;
	out << "sqr() : ConfidenceInterval |Lower, Upper: Real|" << endl << endl;
	out << "   if self extendedLowerGreaterEqualZero then" << endl;
	out << "      Lower := self extendedTimes(LowerBound, LowerBound);" << endl;
	out << "      Upper := self extendedTimes(UpperBound, UpperBound)" << endl;
	out << "   else" << endl;
	out << "      if self extendedUpperLessZero then" << endl;
	out << "         Lower := self extendedTimes(UpperBound, UpperBound);" << endl;
	out << "         Upper := self extendedTimes(LowerBound, LowerBound)" << endl;
	out << "      else" << endl;
	out << "         Lower := 0.0;" << endl;
	out << "         Upper := self extendedMax(self extendedTimes(LowerBound, LowerBound), self extendedTimes(UpperBound, UpperBound))" << endl;
	out << "      fi" << endl;
	out << "   fi;" << endl;
	out << "   return(new(ConfidenceInterval) withParameters(Lower, Upper, ConfidenceLevel))." << endl << endl;
	out << "+(CI: ConfidenceInterval) : ConfidenceInterval |Lower, Upper, Level: Real|" << endl << endl;
	out << "   Lower := self extendedPlus(LowerBound, CI getLowerBound);" << endl;
	out << "   Upper := self extendedPlus(UpperBound, CI getUpperBound);" << endl;
	out << "   Level := ConfidenceLevel + CI getConfidenceLevel - 1;" << endl;
	out << "   return(new(ConfidenceInterval) withParameters(Lower, Upper, Level))." << endl << endl;
	out << "extendedUpperGreaterZero() : Boolean" << endl << endl;
	out << "   if UpperBound = nil then" << endl;
	out << "      return(true)" << endl;
	out << "   else" << endl;
	out << "      return(UpperBound > 0.0)" << endl;
	out << "   fi." << endl << endl;
	out << "extendedLowerGreaterEqualZero() : Boolean" << endl << endl;
	out << "   if LowerBound = nil then" << endl;
	out << "      return(false)" << endl;
	out << "   else" << endl;
	out << "      return(LowerBound >= 0.0)" << endl;
	out << "   fi." << endl << endl;
	out << "extendedLowerLessZero() : Boolean" << endl << endl;
	out << "   if LowerBound = nil then" << endl;
	out << "      return(true)" << endl;
	out << "   else" << endl;
	out << "      return(LowerBound < 0.0)" << endl;
	out << "   fi." << endl << endl;
	out << "getRelativeError() : Real |Lower, Upper: Real|" << endl << endl;
	out << "   if (LowerBound = nil) | (UpperBound = nil) then" << endl;
	out << "      return(nil)" << endl;
	out << "   else" << endl;
	out << "      if LowerBound > 0.0 then" << endl;
	out << "         return((UpperBound - LowerBound) / (2 * LowerBound))" << endl;
	out << "      else" << endl;
	out << "         if UpperBound < 0.0 then" << endl;
	out << "            return((LowerBound - UpperBound) / (2 * UpperBound))" << endl;
	out << "         else" << endl;
	out << "            return(nil)" << endl;
	out << "         fi" << endl;
	out << "      fi" << endl;
	out << "   fi." << endl << endl;
	out << "extendedUpperLessZero() : Boolean" << endl << endl;
	out << "   if UpperBound = nil then" << endl;
	out << "      return(false)" << endl;
	out << "   else" << endl;
	out << "      return(UpperBound < 0.0)" << endl;
	out << "   fi." << endl << endl;
	out << "getLowerBound() : Real" << endl << endl;
	out << "   return(LowerBound)." << endl << endl;
	out << "reciprocal() : ConfidenceInterval |Lower, Upper: Real|" << endl << endl;
	out << "   if (self extendedLowerLessZero) & (self extendedUpperGreaterZero) then" << endl;
	out << "      Lower := nil;" << endl;
	out << "      Upper := nil" << endl;
	out << "   else" << endl;
	out << "      Lower := self extendedReciprocal(UpperBound);" << endl;
	out << "      Upper := self extendedReciprocal(LowerBound)" << endl;
	out << "   fi;" << endl;
	out << "   return(new(ConfidenceInterval) withParameters(Lower, Upper, ConfidenceLevel))." << endl << endl;
	out << "-(CI : ConfidenceInterval) : ConfidenceInterval" << endl << endl;
	out << "   return(self + CI negate)." << endl << endl;
	out << "extendedNegate(x: Real) : Real" << endl << endl;
	out << "   if x = nil then" << endl;
	out << "      return(nil)" << endl;
	out << "   else" << endl;
	out << "      return(-x)" << endl;
	out << "   fi." << endl << endl;
	out << "extendedReciprocal(x: Real) : Real" << endl << endl;
	out << "   if x = nil then" << endl;
	out << "      return(0.0)" << endl;
	out << "   else" << endl;
	out << "      if x = 0.0 then" << endl;
	out << "         return(nil)" << endl;
	out << "      else" << endl;
	out << "         return(1 / x)" << endl;
	out << "      fi" << endl;
	out << "   fi." << endl << endl;
	out << "accurate(Accuracy: Real) : Boolean |RelativeError: Real|" << endl << endl;
	out << "   RelativeError := self getRelativeError;" << endl;
	out << "   if RelativeError = nil then" << endl;
	out << "      return(false)" << endl;
	out << "   else" << endl;
	out << "      return(RelativeError <= 1 - Accuracy)" << endl;
	out << "   fi." << endl << endl;
	out << "getConfidenceLevel() : Real" << endl << endl;
	out << "   return(ConfidenceLevel)." << endl << endl;
	out << "negate() : ConfidenceInterval |Lower,Upper: Real|" << endl << endl;
	out << "   Lower := self extendedNegate(UpperBound);" << endl;
	out << "   Upper := self extendedNegate(LowerBound);" << endl;
	out << "   return(new(ConfidenceInterval) withParameters(Lower, Upper, ConfidenceLevel))." << endl << endl;
	out << "extendedMax(x, y: Real) : Real" << endl << endl;
	out << "   if (x = nil) | (y = nil) then" << endl;
	out << "      return(nil)" << endl;
	out << "   else" << endl;
	out << "      if x > y then" << endl;
	out << "         return(x)" << endl;
	out << "      else" << endl;
	out << "         return(y)" << endl;
	out << "      fi" << endl;
	out << "   fi." << endl << endl;
	out << "withParameters(Lower, Upper, Level: Real) : ConfidenceInterval" << endl << endl;
	out << "   LowerBound := Lower;" << endl;
	out << "   UpperBound := Upper;" << endl;
	out << "   ConfidenceLevel := Level;" << endl;
	out << "   return(self)." << endl << endl;
	out << "logHeading : String" << endl << endl;
	out << "   return(\"Point Estimation:\" tab + \"Confidence Interval:\" tab tab + \"Confidence Level:\" tab + \"Relative Error:\")." << endl << endl;
	out << "logStatistics : String |RelativeError: Real, LogOut: String|" << endl << endl;
	out << "   if (LowerBound = nil) | (UpperBound = nil) then" << endl;
	out << "      LogOut := \"Not Specified\" tab + \"[\"" << endl;
	out << "   else" << endl;
	out << "      LogOut := (0.5 * (LowerBound + UpperBound)) printString tab tab + \"[\"" << endl;
	out << "   fi;" << endl;
	out << "   if LowerBound = nil then" << endl;
	out << "      LogOut append(\"-inf, \")" << endl;
	out << "   else" << endl;
	out << "      LogOut append(LowerBound printString + \", \")" << endl;
	out << "   fi;" << endl;
	out << "   if UpperBound = nil then" << endl;
	out << "      LogOut append(\"inf]\" tab)" << endl;
	out << "   else" << endl;
	out << "      LogOut append(UpperBound printString + \"]\" tab)" << endl;
	out << "   fi;" << endl;
	out << "   if (LowerBound = nil) & (UpperBound = nil) then" << endl;
	out << "      LogOut tab tab tab" << endl;
	out << "   fi;" << endl;
	out << "   LogOut append(ConfidenceLevel printString tab tab tab);" << endl;
	out << "   RelativeError := self getRelativeError;" << endl;
	out << "   if RelativeError == nil then" << endl;
	out << "      LogOut append(\"inf\" tab tab)" << endl;
	out << "   else" << endl;
	out << "      LogOut append(RelativeError printString)" << endl;
	out << "   fi;" << endl;
	out << "   return(LogOut)." << endl << endl;

	out << "data class PerformanceMonitor" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "ConfidenceLevel: Real, Accuracy: Real, BatchSize: Integer, IntervalEstimation: ConfidenceInterval, LogFile: FileOut" << endl << endl;
	out << "instance methods" << endl;
	out << "setDefaultBatchSize() : PerformanceMonitor" << endl << endl;
	out << "   BatchSize := 1000;" << endl;
	out << "   return(self)." << endl << endl;
	out << "accurate() : Boolean" << endl << endl;
	out << "   return(IntervalEstimation accurate(Accuracy))." << endl << endl;
	out << "logStatistics() : String" << endl << endl;
	out << "   return(IntervalEstimation logHeading tab + \"Accurate:\" cr + IntervalEstimation logStatistics tab + IntervalEstimation accurate(Accuracy) printString)." << endl << endl;

	out << "data class LongRunSampleAverage" << endl;
	out << "extends PerformanceMonitor" << endl << endl;
	out << "instance variables" << endl;
	out << "AverageSumLengthProduct: Real, CurrentSum: Real, CurrentLength: Integer, NumberOfCycles: Integer, AverageLength: Real, AverageSquaredSum: Real, AverageSum: Real, TransientMode: Boolean, AverageSquaredLength: Real, Constant: Real" << endl << endl;
	out << "instance methods" << endl;
	out << "setBatchSize(m: Integer) : LongRunSampleAverage" << endl << endl;
	out << "   if m > 0 then" << endl;
	out << "      BatchSize := m" << endl;
	out << "   else" << endl;
	out << "      self error(\"BatchSize parameter for Long Run Sample Average must be larger than 0\")" << endl;
	out << "   fi;" << endl;
	out << "   return(self)." << endl << endl;
	out << "rewardBM(Reward: Real) : LongRunSampleAverage" << endl << endl;
	out << "   self rewardRC(Reward, (CurrentLength = 0) | (CurrentLength = BatchSize));" << endl;
	out << "   return(self)." << endl << endl;
	out << "withConfidenceLevel(CL: Real) : LongRunSampleAverage" << endl << endl;
	out << "   if (CL < 0.0) | (CL >= 1.0) then" << endl;
	out << "      self error(\"ConfidenceLevel parameter for Long Run Sample Average must be within the interval [0.0, 1.0)\")" << endl;
	out << "   fi;" << endl;
	out << "   ConfidenceLevel := CL;" << endl;
	out << "   IntervalEstimation := new(ConfidenceInterval) withParameters(nil, nil, ConfidenceLevel);" << endl;
	out << "   Constant := (2.0 sqrt) * (self calculateInverseErfC(1.0 - ConfidenceLevel));" << endl;
	out << "   TransientMode := true;" << endl;
	out << "   NumberOfCycles := 0;" << endl;
	out << "   CurrentLength := 0;" << endl;
	out << "   AverageSum := 0.0;" << endl;
	out << "   AverageLength := 0.0;" << endl;
	out << "   AverageSquaredSum := 0.0;" << endl;
	out << "   AverageSquaredLength := 0.0;" << endl;
	out << "   AverageSumLengthProduct := 0.0;" << endl;
	out << "   self setDefaultBatchSize;" << endl;
	out << "   return(self)." << endl << endl;
	out << "getCurrentLength() : Integer" << endl << endl;
	out << "   return(CurrentLength)." << endl << endl;
	out << "getIntervalEstimation() : ConfidenceInterval" << endl << endl;
	out << "   return(IntervalEstimation)." << endl << endl;
	out << "rewardRC(Reward: Real, RecurrenceCondition: Boolean) : LongRunSampleAverage |PointEstimation, StandardDeviation, Variance, Factor, HalfWidth: Real|" << endl << endl;
	out << "   if RecurrenceCondition then" << endl;
	out << "      if TransientMode then" << endl;
	out << "         TransientMode := false" << endl;
	out << "      else" << endl;
	out << "         NumberOfCycles := NumberOfCycles + 1;" << endl;
	out << "         Factor := (NumberOfCycles - 1) / NumberOfCycles;" << endl;
	out << "         AverageSum := (Factor * AverageSum) + (CurrentSum / NumberOfCycles);" << endl;
	out << "         AverageLength := (Factor * AverageLength) + (CurrentLength / NumberOfCycles);" << endl;
	out << "         AverageSquaredSum := (Factor * AverageSquaredSum) + (CurrentSum sqr / NumberOfCycles);" << endl;
	out << "         AverageSquaredLength := (Factor * AverageSquaredLength) + (CurrentLength sqr / NumberOfCycles);" << endl;
	out << "         AverageSumLengthProduct := (Factor * AverageSumLengthProduct) + ((CurrentSum * CurrentLength) / NumberOfCycles);" << endl;
	out << "         if NumberOfCycles > 1 then" << endl;
	out << "            PointEstimation := AverageSum / AverageLength;" << endl;
	out << "            Variance := ((1 / Factor) * (AverageSquaredSum - (2 * PointEstimation * AverageSumLengthProduct) + (PointEstimation sqr * AverageSquaredLength)));" << endl;
	out << "            if Variance > 0 then StandardDeviation := Variance sqrt else StandardDeviation := 0.0 fi;" << endl;
	out << "            HalfWidth := (Constant * StandardDeviation) / (AverageLength * (NumberOfCycles asReal sqrt));" << endl;
	out << "            IntervalEstimation := new(ConfidenceInterval) withParameters(PointEstimation - HalfWidth, PointEstimation + HalfWidth, ConfidenceLevel)" << endl;
	out << "         fi" << endl;
	out << "      fi;" << endl;
	out << "      CurrentSum := Reward;" << endl;
	out << "      CurrentLength := 1" << endl;
	out << "   else" << endl;
	out << "      if TransientMode not then" << endl;
	out << "         CurrentSum := CurrentSum + Reward;" << endl;
	out << "         CurrentLength := CurrentLength + 1" << endl;
	out << "      fi" << endl;
	out << "   fi;" << endl;
	out << "   return(self)." << endl << endl;
	out << "withParameters(A, CL: Real) : LongRunSampleAverage" << endl << endl;
	out << "   if (A <= 0.0) | (A >= 1.0) then" << endl;
	out << "      self error(\"Accuracy parameter for Long Run Sample Average must be within the interval (0.0, 1.0)\")" << endl;
	out << "   fi;" << endl;
	out << "   Accuracy := A;" << endl;
	out << "   self withConfidenceLevel(CL);" << endl;
	out << "   return(self)." << endl << endl;
	out << "calculateInverseErfC(y: Real) : Real |s, t, u, w, x, z: Real|" << endl << endl;
	out << "    z := y;" << endl;
	out << "    w := 0.916461398268964 - z ln;" << endl;
	out << "    u := w sqrt;" << endl;
	out << "    s := (u ln + 0.488826640273108) / w;" << endl;
	out << "    t := 1 / (u + 0.231729200323405);" << endl;
	out << "    x := u * (1 - s * (s * 0.124610454613712 + 0.5)) - " << endl;
	out << "        ((((-0.0728846765585675 * t + 0.269999308670029) * t + " << endl;
	out << "        0.150689047360223) * t + 0.116065025341614) * t + " << endl;
	out << "        0.499999303439796) * t;" << endl;
	out << "    t := 3.97886080735226 / (x + 3.97886080735226);" << endl;
	out << "    u := t - 0.5;" << endl;
	out << "    s := (((((((((0.00112648096188977922 * u + " << endl;
	out << "        0.000105739299623423047) * u - 0.00351287146129100025) * u - " << endl;
	out << "        0.000771708358954120939) * u + 0.00685649426074558612) * u + " << endl;
	out << "        0.00339721910367775861) * u - 0.011274916933250487) * u - " << endl;
	out << "        0.0118598117047771104) * u + 0.0142961988697898018) * u + " << endl;
	out << "        0.0346494207789099922) * u + 0.00220995927012179067;" << endl;
	out << "    s := ((((((((((((s * u - 0.0743424357241784861) * u - " << endl;
	out << "        0.105872177941595488) * u + 0.0147297938331485121) * u + " << endl;
	out << "        0.316847638520135944) * u + 0.713657635868730364) * u + " << endl;
	out << "        1.05375024970847138) * u + 1.21448730779995237) * u + " << endl;
	out << "        1.16374581931560831) * u + 0.956464974744799006) * u + " << endl;
	out << "        0.686265948274097816) * u + 0.434397492331430115) * u + " << endl;
	out << "        0.244044510593190935) * t - " << endl;
	out << "        z * (2.718281828459045 power(x * x - 0.120782237635245222));" << endl;
	out << "    x := x + s * (x * s + 1);" << endl;
	out << "    return(x)." << endl << endl;

	out << "data class LongRunSampleVariance" << endl;
	out << "extends PerformanceMonitor" << endl;
	out << "instance variables" << endl;
	out << "AverageReward: LongRunSampleAverage, AverageSquaredReward: LongRunSampleAverage" << endl << endl;
	out << "instance methods" << endl;
	out << "setBatchSize(m: Integer) : LongRunSampleVariance" << endl << endl;
	out << "   if m > 0 then" << endl;
	out << "      BatchSize := m;" << endl;
	out << "      AverageReward setBatchSize(BatchSize);" << endl;
	out << "      AverageSquaredReward setBatchSize(BatchSize)" << endl;
	out << "   else" << endl;
	out << "      self error(\"BatchSize parameter for Long Run Sample Variance must be larger than 0\")" << endl;
	out << "   fi;" << endl;
	out << "   return(self)." << endl << endl;
	out << "rewardBM(Reward: Real) : LongRunSampleVariance" << endl << endl;
	out << "   self rewardRC(Reward, (AverageReward getCurrentLength = 0) | (AverageReward getCurrentLength = BatchSize));" << endl;
	out << "   return(self)." << endl << endl;
	out << "rewardRC(Reward: Real, RecurrenceCondition: Boolean) : LongRunSampleVariance" << endl << endl;
	out << "   AverageReward rewardRC(Reward, RecurrenceCondition);" << endl;
	out << "   AverageSquaredReward rewardRC(Reward * Reward, RecurrenceCondition);" << endl;
	out << "   if RecurrenceCondition then" << endl;
	out << "      IntervalEstimation := (AverageSquaredReward getIntervalEstimation) - ((AverageReward getIntervalEstimation) sqr)" << endl;
	out << "   fi;" << endl;
	out << "   return(self)." << endl << endl;
	out << "withParameters(A, CL: Real) : LongRunSampleVariance" << endl << endl;
	out << "   if (A <= 0.0) | (A >= 1.0) then" << endl;
	out << "      self error(\"Accuracy parameter for Long Run Sample Variance must be within the interval (0.0, 1.0)\")" << endl;
	out << "   fi;" << endl;
	out << "   Accuracy := A;" << endl;
	out << "   if (CL < 0.0) | (CL >= 1.0) then" << endl;
	out << "      self error(\"ConfidenceLevel parameter for Long Run Sample Variance must be within the interval [0.0, 1.0)\")" << endl;
	out << "   fi;" << endl;
	out << "   ConfidenceLevel := CL;" << endl;
	out << "   AverageReward := new(LongRunSampleAverage) withConfidenceLevel((ConfidenceLevel + 1) / 2);" << endl;
	out << "   AverageSquaredReward := new(LongRunSampleAverage) withConfidenceLevel((ConfidenceLevel + 1) / 2);" << endl;
	out << "   IntervalEstimation := (AverageSquaredReward getIntervalEstimation) - ((AverageReward getIntervalEstimation) sqr);" << endl;
	out << "   self setDefaultBatchSize();" << endl;
	out << "   return(self)." << endl << endl;

	out << "data class LongRunTimeAverage" << endl;
	out << "extends PerformanceMonitor" << endl;
	out << "instance variables" << endl;
	out << "PreviousReward: Real, AverageTime: LongRunSampleAverage, AverageRewardTimeProduct: LongRunSampleAverage, LastTime: Real" << endl << endl;
	out << "instance methods" << endl;
	out << "setBatchSize(m: Integer) : LongRunTimeAverage" << endl << endl;
	out << "   if m > 0 then" << endl;
	out << "      BatchSize := m;" << endl;
	out << "      AverageRewardTimeProduct setBatchSize(BatchSize);" << endl;
	out << "      AverageTime setBatchSize(BatchSize)" << endl;
	out << "   else" << endl;
	out << "      self error(\"BatchSize parameter for Long Run Time Average must be larger than 0\")" << endl;
	out << "   fi;" << endl;
	out << "   return(self)." << endl << endl;
	out << "rewardBM(Reward, CurrentTime: Real) : LongRunTimeAverage" << endl << endl;
	out << "   self rewardRC(Reward, CurrentTime, (AverageRewardTimeProduct getCurrentLength = 0) | (AverageRewardTimeProduct getCurrentLength = BatchSize));" << endl;
	out << "   return(self)." << endl << endl;
	out << "withConfidenceLevel(CL: Real) : LongRunTimeAverage" << endl << endl;
	out << "   if (CL < 0.0) | (CL >= 1.0) then" << endl;
	out << "      self error(\"ConfidenceLevel parameter for Long Run Time Average must be within the interval [0.0, 1.0)\")" << endl;
	out << "   fi;" << endl;
	out << "   ConfidenceLevel := CL;" << endl;
	out << "   AverageRewardTimeProduct := new(LongRunSampleAverage) withConfidenceLevel((ConfidenceLevel + 1) / 2);" << endl;
	out << "   AverageTime := new(LongRunSampleAverage) withConfidenceLevel((ConfidenceLevel + 1) / 2);" << endl;
	out << "   IntervalEstimation := (AverageRewardTimeProduct getIntervalEstimation) / (AverageTime getIntervalEstimation);" << endl;
	out << "   self setDefaultBatchSize();" << endl;
	out << "   return(self)." << endl << endl;
	out << "getCurrentLength() : Integer" << endl << endl;
	out << "   return(AverageRewardTimeProduct getCurrentLength)." << endl << endl;
	out << "getIntervalEstimation() : ConfidenceInterval" << endl << endl;
	out << "   return(IntervalEstimation)." << endl << endl;
	out << "rewardRC(Reward, CurrentTime: Real, RecurrenceCondition: Boolean) : LongRunTimeAverage" << endl << endl;
	out << "   if LastTime != nil then" << endl;
	out << "      AverageRewardTimeProduct rewardRC(PreviousReward * (CurrentTime - LastTime), RecurrenceCondition);" << endl;
	out << "      AverageTime rewardRC(CurrentTime - LastTime, RecurrenceCondition)" << endl;
	out << "   fi;" << endl;
	out << "   PreviousReward := Reward;" << endl;
	out << "   LastTime := CurrentTime;" << endl;
	out << "   if RecurrenceCondition then" << endl;
	out << "      IntervalEstimation := (AverageRewardTimeProduct getIntervalEstimation) / (AverageTime getIntervalEstimation)" << endl;
	out << "   fi;" << endl;
	out << "   return(self)." << endl << endl;
	out << "withParameters(A, CL: Real) : LongRunTimeAverage" << endl << endl;
	out << "   if (A <= 0.0) | (A >= 1.0) then" << endl;
	out << "      self error(\"Accuracy parameter for Long Run Time Average must be within the interval (0.0, 1.0)\")" << endl;
	out << "   fi;" << endl;
	out << "   Accuracy := A;" << endl;
	out << "   self withConfidenceLevel(CL);" << endl;
	out << "   return(self)." << endl << endl;

	out << "data class LongRunRateAverage" << endl;
	out << "extends PerformanceMonitor" << endl;
	out << "instance variables" << endl;
	out << "AverageReward: LongRunSampleAverage, AverageTime: LongRunSampleAverage, LastTime: Real" << endl << endl;
	out << "instance methods" << endl;
	out << "setBatchSize(m: Integer) : LongRunRateAverage" << endl << endl;
	out << "   if m > 0 then" << endl;
	out << "      BatchSize := m;" << endl;
	out << "      AverageReward setBatchSize(BatchSize);" << endl;
	out << "      AverageTime setBatchSize(BatchSize)" << endl;
	out << "   else" << endl;
	out << "      self error(\"BatchSize parameter for Long Run Rate Average must be larger than 0\")" << endl;
	out << "   fi;" << endl;
	out << "   return(self)." << endl << endl;
	out << "rewardBM(Reward, CurrentTime: Real) : LongRunRateAverage" << endl << endl;
	out << "   self rewardRC(Reward, CurrentTime, (AverageReward getCurrentLength = 0) | (AverageReward getCurrentLength = BatchSize));" << endl;
	out << "   return(self)." << endl << endl;
	out << "withConfidenceLevel(CL: Real) : LongRunRateAverage" << endl << endl;
	out << "   if (CL < 0.0) | (CL >= 1.0) then" << endl;
	out << "      self error(\"ConfidenceLevel parameter for Long Run Rate Average must be within the interval [0.0, 1.0)\")" << endl;
	out << "   fi;" << endl;
	out << "   ConfidenceLevel := CL;" << endl;
	out << "   AverageReward := new(LongRunSampleAverage) withConfidenceLevel((ConfidenceLevel + 1) / 2);" << endl;
	out << "   AverageTime := new(LongRunSampleAverage) withConfidenceLevel((ConfidenceLevel + 1) / 2);" << endl;
	out << "   IntervalEstimation := (AverageReward getIntervalEstimation) / (AverageTime getIntervalEstimation);" << endl;
	out << "   self setDefaultBatchSize();" << endl;
	out << "   return(self)." << endl << endl;
	out << "getCurrentLength() : Integer" << endl << endl;
	out << "   return(AverageReward getCurrentLength)." << endl << endl;
	out << "getIntervalEstimation() : ConfidenceInterval" << endl << endl;
	out << "   return(IntervalEstimation)." << endl << endl;
	out << "rewardRC(Reward, CurrentTime: Real, RecurrenceCondition: Boolean) : LongRunRateAverage" << endl << endl;
	out << "   if LastTime != nil then" << endl;
	out << "      AverageReward rewardRC(Reward, RecurrenceCondition);" << endl;
	out << "      AverageTime rewardRC(CurrentTime - LastTime, RecurrenceCondition)" << endl;
	out << "   fi;" << endl;
	out << "   LastTime := CurrentTime;" << endl;
	out << "   if RecurrenceCondition then" << endl;
	out << "      IntervalEstimation := (AverageReward getIntervalEstimation) / (AverageTime getIntervalEstimation)" << endl;
	out << "   fi;" << endl;
	out << "   return(self)." << endl << endl;
	out << "withParameters(A, CL: Real) : LongRunRateAverage" << endl << endl;
	out << "   if (A <= 0.0) | (A >= 1.0) then" << endl;
	out << "      self error(\"Accuracy parameter for Long Run Rate Average must be within the interval (0.0, 1.0)\")" << endl;
	out << "   fi;" << endl;
	out << "   Accuracy := A;" << endl;
	out << "   self withConfidenceLevel(CL);" << endl;
	out << "   return(self)." << endl << endl;

	out << "data class LongRunTimeVariance" << endl;
	out << "extends PerformanceMonitor" << endl;
	out << "instance variables" << endl;
	out << "AverageReward: LongRunTimeAverage, AverageSquaredReward: LongRunTimeAverage" << endl << endl;
	out << "instance methods" << endl;
	out << "setBatchSize(m: Integer) : LongRunTimeVariance" << endl << endl;
	out << "   if m > 0 then" << endl;
	out << "      BatchSize := m;" << endl;
	out << "      AverageReward setBatchSize(BatchSize);" << endl;
	out << "      AverageSquaredReward setBatchSize(BatchSize)" << endl;
	out << "   else" << endl;
	out << "      self error(\"BatchSize parameter for Long Run Time Variance must be larger than 0\")" << endl;
	out << "   fi;" << endl;
	out << "   return(self)." << endl << endl;
	out << "rewardBM(Reward, CurrentTime: Real) : LongRunTimeVariance" << endl << endl;
	out << "   self rewardRC(Reward, CurrentTime, (AverageReward getCurrentLength = 0) | (AverageReward getCurrentLength = BatchSize));" << endl;
	out << "   return(self)." << endl << endl;
	out << "rewardRC(Reward, CurrentTime: Real, RecurrenceCondition: Boolean) : LongRunTimeVariance" << endl << endl;
	out << "   AverageReward rewardRC(Reward, CurrentTime, RecurrenceCondition);" << endl;
	out << "   AverageSquaredReward rewardRC(Reward * Reward, CurrentTime, RecurrenceCondition);" << endl;
	out << "   if RecurrenceCondition then" << endl;
	out << "      IntervalEstimation := (AverageSquaredReward getIntervalEstimation) - ((AverageReward getIntervalEstimation) sqr)" << endl;
	out << "   fi;" << endl;
	out << "   return(self)." << endl << endl;
	out << "withParameters(A, CL: Real) : LongRunTimeVariance" << endl << endl;
	out << "   if (A <= 0.0) | (A >= 1.0) then" << endl;
	out << "      self error(\"Accuracy parameter for Long Run Sample Variance must within the interval (0.0, 1.0)\")" << endl;
	out << "   fi;" << endl;
	out << "   Accuracy := A;" << endl;
	out << "   if (CL < 0.0) | (CL >= 1.0) then" << endl;
	out << "      self error(\"ConfidenceLevel parameter for Long Run Sample Variance must be within the interval [0.0, 1.0)\")" << endl;
	out << "   fi;" << endl;
	out << "   ConfidenceLevel := CL;" << endl;
	out << "   AverageReward := new(LongRunTimeAverage) withConfidenceLevel((ConfidenceLevel + 1) / 2);" << endl;
	out << "   AverageSquaredReward := new(LongRunTimeAverage) withConfidenceLevel((ConfidenceLevel + 1) / 2);" << endl;
	out << "   IntervalEstimation := (AverageSquaredReward getIntervalEstimation) - ((AverageReward getIntervalEstimation) sqr);" << endl;
	out << "   self setDefaultBatchSize();" << endl;
	out << "   return(self)." << endl << endl;

	// Generate Data Class Distribution and Uniform
	
	out << "data class Distribution" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "Random: RandomGenerator" << endl << endl;
	out << "instance methods" << endl;
	out << "initialise : Distribution" << endl << endl;
	out << "  Random := new(RandomGenerator) randomiseSeed;" << endl;
	out << "  return(self)." << endl << endl;
	
	out << "data class Uniform" << endl;
	out << "extends Distribution" << endl;
	out << "instance variables" << endl;
	out << "IntervalLength: Real, LowerBound: Real" << endl << endl;
	out << "instance methods" << endl;
	out << "withParameters(LB, UB: Real) : Uniform" << endl << endl;
	out << "   self initialise;" << endl;
	out << "   LowerBound := LB; IntervalLength := UB - LB;" << endl;
	out << "   return(self)." << endl << endl;
	out << "sample : Real" << endl << endl;
	out << "   return(LowerBound + (Random random * IntervalLength))." << endl << endl;

	// Generate Data Classes for Queues
	
	out << "data class Queue" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "PrimairElement: QueueElement, Occupation: Integer" << endl << endl;
	out << "instance methods" << endl;
	out << "init : Queue" << endl << endl;
	out << "   PrimairElement := new(QueueElement);" << endl;
	out << "   PrimairElement setNext(PrimairElement) setPrevious(PrimairElement);" << endl;
	out << "   Occupation := 0;" << endl;
	out << "   return(self)." << endl << endl;
	out << "put(NewElement: Object) : Object |NQE: QueueElement|" << endl << endl;
	out << "   NQE := new(QueueElement) setElement(NewElement);" << endl;
	out << "   NQE setNext(PrimairElement getNext) setPrevious(PrimairElement);" << endl;
	out << "   PrimairElement setNext(NQE);" << endl;
	out << "   NQE getNext setPrevious(NQE);" << endl;
	out << "   Occupation := Occupation + 1;" << endl;
	out << "   return(self)." << endl << endl;
	out << "inspect : Object |QE: QueueElement|" << endl << endl;
	out << "   QE := PrimairElement getPrevious;" << endl;
	out << "   return(QE getElement)." << endl << endl;
	out << "remove : Object |QE: QueueElement|" << endl << endl;
	out << "   QE := PrimairElement getPrevious;" << endl;
	out << "   QE getNext setPrevious(QE getPrevious);" << endl;
	out << "   QE getPrevious setNext(QE getNext);" << endl;
	out << "   Occupation := Occupation - 1;" << endl;
	out << "   return(QE getElement)." << endl << endl;
	out << "isNotEmpty : Boolean" << endl << endl;
	out << "   return(Occupation != 0)." << endl << endl;

	out << "data class QueueElement" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "Next: QueueElement, Element: Object, Previous: QueueElement" << endl << endl;
	out << "instance methods" << endl;
	out << "setNext(QE: QueueElement): QueueElement" << endl << endl;
	out << "   Next := QE;" << endl;
	out << "   return(self)." << endl << endl;
	out << "getPrevious: QueueElement" << endl << endl;
	out << "   return(Previous)." << endl << endl;
	out << "setElement(E: Object): QueueElement" << endl << endl;
	out << "   Element := E;" << endl;
	out << "   return(self)." << endl << endl;
	out << "getElement: Object" << endl << endl;
	out << "   return(Element)." << endl << endl;
	out << "getNext: QueueElement" << endl << endl;
	out << "   return(Next)." << endl << endl;
	out << "setPrevious(QE: QueueElement): QueueElement" << endl << endl;
	out << "   Previous := QE;" << endl;
	out << "   return(self)." << endl << endl;

	// Generate Primitive Data Class Definitions
	
	out << "data class Object" << endl;
	out << "instance variables" << endl;
	out << "instance methods" << endl;
	out << "error(ErrorMessage: String): Object primitive." << endl;
	out << "deepCopy: Object primitive." << endl;
	out << "=  (Operand2: Object): Boolean primitive." << endl;
	out << "!= (Operand2: Object): Boolean primitive." << endl;
	out << "== (Operand2: Object): Boolean primitive." << endl;
	out << "!==(Operand2: Object): Boolean primitive." << endl;
	out << "printString: String primitive." << endl;
	out << "shallowCopy: Object primitive." << endl << endl;

	out << "data class Number" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "instance methods" << endl;

	out << "data class Integer" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "instance methods" << endl;
	out << "+  (Operand2: Number): Number primitive." << endl;
	out << "/  (Operand2: Number): Number primitive." << endl;
	out << "*  (Operand2: Number): Number primitive." << endl;
	out << "-  (Operand2: Number): Number primitive." << endl;
	out << "<  (Operand2: Number): Boolean primitive." << endl;
	out << ">  (Operand2: Number): Boolean primitive." << endl;
	out << "<= (Operand2: Number): Boolean primitive." << endl;
	out << ">= (Operand2: Number): Boolean primitive." << endl;
	out << "unaryMinus: Integer primitive." << endl;
	out << "abs: Integer primitive." << endl;
	out << "&  (Operand2: Integer): Integer primitive." << endl;
	out << "|  (Operand2: Integer): Integer primitive." << endl;
	out << "not: Integer primitive." << endl;
	out << "sqrt: Number primitive." << endl;
	out << "power(Operand2: Number): Number primitive." << endl;
	out << "sqr: Integer primitive." << endl;
	out << "div(Operand2: Integer): Integer primitive." << endl;
	out << "modulo(Operand2: Number): Number primitive." << endl;
	out << "asInteger: Integer primitive." << endl;
	out << "asReal: Real primitive." << endl;
	out << "asString: String primitive." << endl;
	out << "printString: String primitive." << endl;
	out << "fac: Integer primitive." << endl;
	out << "monus(n: Integer): Integer if n > self then 0 else self - n fi." << endl << endl;

	out << "data class Real" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "instance methods" << endl;
	out << "+  (Operand2: Number): Real primitive." << endl;
	out << "/  (Operand2: Number): Real primitive." << endl;
	out << "*  (Operand2: Number): Real primitive." << endl;
	out << "-  (Operand2: Number): Real primitive." << endl;
	out << "<  (Operand2: Number): Boolean primitive." << endl;
	out << ">  (Operand2: Number): Boolean primitive." << endl;
	out << "<= (Operand2: Number): Boolean primitive." << endl;
	out << ">= (Operand2: Number): Boolean primitive." << endl;
	out << "unaryMinus: Real primitive." << endl;
	out << "acos: Real primitive." << endl;
	out << "asin: Real primitive." << endl;
	out << "atan: Real primitive." << endl;
	out << "atan2(Denom: Real): Real primitive." << endl;
	out << "cos: Real primitive." << endl;
	out << "sin: Real primitive." << endl;
	out << "tan: Real primitive." << endl;
	out << "abs: Real primitive." << endl;
	out << "log: Real primitive." << endl;
	out << "ln: Real primitive." << endl;
	out << "sqrt: Real primitive." << endl;
	out << "exp: Real primitive." << endl;
	out << "power(Operand2: Number): Real primitive." << endl;
	out << "sqr: Real primitive." << endl;
	out << "ceiling: Integer primitive." << endl;
	out << "floor: Integer primitive." << endl;
	out << "round: Integer primitive." << endl;
	out << "div(Operand2: Number): Integer primitive." << endl;
	out << "modulo(Operand2: Number): Real primitive." << endl;
	out << "asInteger: Integer primitive." << endl;
	out << "asReal: Real primitive." << endl;
	out << "asString: String primitive." << endl;
	out << "printString: String primitive." << endl << endl;

	out << "primitive data class Boolean" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "instance methods" << endl;
	out << "&  (Operand2: Boolean): Boolean primitive." << endl;
	out << "|  (Operand2: Boolean): Boolean primitive." << endl;
	out << "not: Boolean primitive." << endl;
	out << "printString: String primitive." << endl << endl;

	out << "primitive data class Char" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "instance methods" << endl;
	out << "printString: String primitive." << endl << endl;

	out << "primitive data class Array" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "instance methods" << endl;
	out << "get(FromPosition: Integer): Object primitive." << endl;
	out << "getSize: Integer primitive." << endl;
	out << "size(newSize: Integer): Array primitive." << endl;
	out << "printString: String | i, s: Integer, Str: String |" << endl;
	out << "   Str := \"Array (\"; i := 0; s := self getSize;" << endl;
	out << "   while i < s do Str append(self get(i := i + 1) printString) append(if i < s then \",\" else \")\" fi) od;" << endl;
	out << "   Str." << endl;
	out << "put(AtPosition: Integer, Element: Object): Array primitive." << endl;
	out << "putAll(Element: Object): Array primitive." << endl << endl;

	out << "primitive data class String" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "instance methods" << endl;
	out << "+  (aString: String): String primitive." << endl;
	out << "<  (Operand2: String): Boolean primitive." << endl;
	out << ">  (Operand2: String): Boolean primitive." << endl;
	out << "<= (Operand2: String): Boolean primitive." << endl;
	out << ">= (Operand2: String): Boolean primitive." << endl;
	out << "append(aString: String): String primitive." << endl;
	out << "copy(From, To: Integer): String primitive." << endl;
	out << "concat(aString: String): String primitive." << endl;
	out << "cr: String primitive." << endl;
	out << "get(FromPosition: Integer): Char primitive." << endl;
	out << "length: Integer primitive." << endl;
	out << "printString: String primitive." << endl;
	out << "put(AtPosition: Integer, aChar: Char): String primitive." << endl;
	out << "tab: String primitive." << endl << endl;

	out << "primitive data class RandomGenerator" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "instance methods" << endl;
	out << "random: Real primitive." << endl;
	out << "randomInt(n: Integer): Integer primitive." << endl;
	out << "randomiseSeed: RandomGenerator primitive." << endl;
	out << "setSeed(aSeed: Integer): RandomGenerator primitive." << endl << endl;

	out << "primitive data class FileOut" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "instance methods" << endl;
	out << "append: FileOut primitive." << endl;
	out << "destination(FileName: String): FileOut primitive." << endl;
	out << "close: Object primitive." << endl;
	out << "open: FileOut primitive." << endl;
	out << "write(anObject: Object): FileOut" << endl;
	out << "   self writeString(anObject printString append(' '))." << endl;
	out << "writeln(anObject: Object): FileOut" << endl;
	out << "   self writeString(anObject printString cr)." << endl;
	out << "writeString(aString: String): FileOut primitive." << endl << endl;

	out << "primitive data class FileIn" << endl;
	out << "extends Object" << endl;
	out << "instance variables" << endl;
	out << "instance methods" << endl;
	out << "open: FileIn primitive." << endl;
	out << "close: Object primitive." << endl;
	out << "readString: String primitive." << endl;
	out << "readInteger: Integer primitive." << endl;
	out << "readNumber: Object primitive." << endl;
	out << "readReal: Real primitive." << endl;
	out << "readBoolean: Boolean primitive." << endl;
	out << "readChar: Char primitive." << endl;
	out << "source(FileName: String): FileIn primitive." << endl;
}
