/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   html.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 9, 2008
 *
 *  Function        :   Output SDF graph, architecture, mapping, etcetera in 
 *                      HTML format.
 *
 *  History         :
 *      09-03-08    :   Initial version.
 *
 * $Id: html.cc,v 1.5 2008/05/07 11:53:32 sander Exp $
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

#include "html.h"
#include "../dot/dot.h"

/**
 * printStylesheet ()
 * Write the sylesheet to the supplied output stream.
 */
void SDFconvertToHTML::printStylesheet(ostream &out)
{
    out << "        #header a {" << endl;
    out << "            text-decoration: none;" << endl;
    out << "            color: #6889b5;" << endl;
    out << "            font-size: 20pt;" << endl;
    out << "            font-weight: bold;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        #nav-main {" << endl;
    out << "            background-color: #4f6889;" << endl;
    out << "            width: 100%;" << endl;
    out << "            font-weight: bold;" << endl;
    out << "            font-family: 'Luxi Sans', verdana, helvetica, arial, sans-serif;" << endl;
    out << "            font-size: x-small;" << endl;
    out << "            margin-top: 5px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        #nav-main-list {" << endl;
    out << "            height: 20px; /* size of navigation bar */" << endl;
    out << "            list-style: none;" << endl;
    out << "            margin: 0px;" << endl;
    out << "            padding: 0px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        #nav-main-list li {" << endl;
    out << "            float: left;" << endl;
    out << "            display: inline;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        #nav-main-list a {" << endl;
    out << "            color: #fff;" << endl;
    out << "            text-decoration: none;" << endl;
    out << "            vertical-align: middle;" << endl;
    out << "            padding-left: 10px;" << endl;
    out << "            padding-right: 10px;" << endl;
    out << "            line-height: 20px;" << endl;
    out << "            display: block;" << endl;
    out << "        }" << endl;
    out << "         " << endl;
    out << "        #nav-main-list a:hover {" << endl;
    out << "            background-color: #7ca3d6;" << endl;
    out << "        } " << endl;
    out << "        " << endl;
    out << "        #nav-main-list li strong{" << endl;
    out << "            background-color: #6889b5;" << endl;
    out << "            display: block;" << endl;
    out << "        } " << endl;
    out << "        " << endl;
    out << "        * html .nav-corner-br {" << endl;
    out << "        	margin-top: -12px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        * html .nav-corner-bl {" << endl;
    out << "        	margin-top: -12px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        body {" << endl;
    out << "        	margin: 0;" << endl;
    out << "        	padding: 0;" << endl;
    out << "        	text-align: center;" << endl;
    out << "        	font-family: 'Luxi Sans', 'Bitstream Vera Sans', ";
    out << "'Lucida Grande', 'Trebuchet MS', helvetica, verdana, arial, sans-serif;" << endl;
    out << "        	font-size: small;" << endl;
    out << "        	color: #333;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        #mainone {" << endl;
    out << "            min-width: 720px;" << endl;
    out << "            padding-left: 30px;" << endl;
    out << "            padding-right: 30px;" << endl;
    out << "            margin-left: auto;" << endl;
    out << "            margin-right: auto;" << endl;
    out << "            text-align: left;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        #site-left {" << endl;
    out << "            width: 120px;" << endl;
    out << "            float: left;" << endl;
    out << "            margin-right: 20px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        #site-middle-two {" << endl;
    out << "            margin-left: 140px;  /*width of site-left block */" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        #content {" << endl;
    out << "            margin: 10px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        img {" << endl;
    out << "            border: 0px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        div.center {" << endl;
    out << "            text-align: center;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        .command {" << endl;
    out << "            font-family: 'Courier';" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        div.command {" << endl;
    out << "            margin-top: 10px;" << endl;
    out << "            margin-bottom: 10px;" << endl;
    out << "            white-space: pre;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        div.example {" << endl;
    out << "        	display: block;" << endl;
    out << "        	padding: 10px;" << endl;
    out << "        	border: 1px solid #6666cc;" << endl;
    out << "        	color: #000;" << endl;
    out << "        	overflow: auto;" << endl;
    out << "        	margin: 1em 2em;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        code.screen, pre.screen {" << endl;
    out << "            font-family: 'Courier';" << endl;
    out << "        	display: block;" << endl;
    out << "        	padding: 10px;" << endl;
    out << "        	border: 1px solid #bbb;" << endl;
    out << "        	background-color: #eee;" << endl;
    out << "        	color: #000;" << endl;
    out << "        	overflow: auto;" << endl;
    out << "        	margin: 1em 2em;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        pre {" << endl;
    out << "            font-size: 10pt;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        img.right {" << endl;
    out << "            display: block;" << endl;
    out << "            float: right;" << endl;
    out << "            margin: 10px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        img.left {" << endl;
    out << "            display: block;" << endl;
    out << "            float: left;" << endl;
    out << "            margin: 10px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        p.right {" << endl;
    out << "            text-align: right;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        div.caution {" << endl;
    out << "        	display: block;" << endl;
    out << "        	padding: 10px;" << endl;
    out << "        	border: 1px solid #6666cc;" << endl;
    out << "        	background-color: #ccccff;" << endl;
    out << "        	color: #000;" << endl;
    out << "        	overflow: auto;" << endl;
    out << "        	margin: 1em 2em;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        div.info {" << endl;
    out << "        	display: block;" << endl;
    out << "        	padding: 10px;" << endl;
    out << "        	border: 1px solid #6666cc;" << endl;
    out << "        	background-color: #ccccff;" << endl;
    out << "        	color: #000;" << endl;
    out << "        	overflow: auto;" << endl;
    out << "        	margin: 1em 2em;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        th {" << endl;
    out << "            color: #4f6889;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        .header {" << endl;
    out << "            color: #4f6889;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        ul.separate li {" << endl;
    out << "            margin-bottom: 10px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        ol.separate li {" << endl;
    out << "            margin-bottom: 10px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        li.obsolete {" << endl;
    out << "            color: #aaa;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        div.tabs {" << endl;
    out << "            width: 100%;" << endl;
    out << "            height: 20px; /* size of navigation bar */" << endl;
    out << "            list-style: none;" << endl;
    out << "            margin: 0px;" << endl;
    out << "            padding: 0px;" << endl;
    out << "            " << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        div.tabs ul {" << endl;
    out << "            margin: 0px;" << endl;
    out << "            padding: 0px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        div.tabs li {" << endl;
    out << "            float: left;" << endl;
    out << "            display: inline;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        div.tabs a {" << endl;
    out << "            color: #6f6f6f;" << endl;
    out << "            text-decoration: none;" << endl;
    out << "            padding-left: 0px;" << endl;
    out << "            padding-right: 10px;" << endl;
    out << "            line-height: 20px;" << endl;
    out << "            display: block;" << endl;
    out << "        }" << endl;
    out << "         " << endl;
    out << "        div.tabs a:hover {" << endl;
    out << "            text-decoration: underline;" << endl;
    out << "        } " << endl;
    out << "        " << endl;
    out << "        #current{" << endl;
    out << "            font-weight: bold;" << endl;
    out << "            display: block;" << endl;
    out << "        } " << endl;
    out << "        " << endl;
    out << "        .center{" << endl;
    out << "            text-align: center;" << endl;
    out << "        } " << endl;
    out << "        td {" << endl;
    out << "            padding-right: 30px;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        th {" << endl;
    out << "            padding-right: 30px;" << endl;
    out << "        }" << endl;
    out << "        .footer {" << endl;
    out << "        	font-size: x-small;" << endl;
    out << "            color: #777;" << endl;
    out << "            padding-top: 50px;" << endl;
    out << "            padding-bottom: 30px;" << endl;
    out << "            clear: both;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        a.footer {" << endl;
    out << "            text-decoration: none;" << endl;
    out << "            color: #666699;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        a.footer:visited {" << endl;
    out << "            color: #666699;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        a.footer:hover {" << endl;
    out << "            text-decoration: underline;" << endl;
    out << "            color: #666699;" << endl;
    out << "        }" << endl;
}

/**
 * printHeader ()
 * Write the header of the HTML file to the supplied output stream.
 */
void SDFconvertToHTML::printHeader(ostream &out, CString title)
{
    out << "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' ";
    out << "'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>" << endl;
    out << "<html xmlns='http://www.w3.org/1999/xhtml'>" << endl;
    out << "<head>" << endl;
    out << "    <title>SDF3 :: " << title << "</title>" << endl;
    out << "    <meta http-equiv='Content-type' content='text/html;charset=iso-8859-1' />" << endl;
    out << "    <style>";
    printStylesheet(out);
    out << "    </style>" << endl;
    out << "</head>" << endl;
    out << "<body>" << endl;
    out << "<a name='top'/>" << endl;
    out << "<div id='mainone'>" << endl;
    out << endl;
    out << "    <!-- header -->" << endl;
    out << "    <div id='header'>" << endl;
    out << "        <a href='http://www.es.ele.tue.nl/sdf3'>SDF3</a>" << endl;
    out << "    </div>" << endl;
    out << endl;
    out << "    <!-- navigation -->" << endl;  
    out << "    <div id='nav-main'>" << endl;
    out << "        <ul id='nav-main-list'>" << endl;
    out << "            <li><a href='" << getFullName(sdfGraphFilename, "html");
    out << "'>Application</a></li>" << endl;
    
    if (getPlatformGraph() != NULL)
    {
        out << "            <li><a href='";
        out << getFullName(platformGraphFilename, "html");
        out << "'>Platform</a></li>" << endl;
        out << "            <li><a href='";
        out << getFullName(platformMappingFilename, "html");
        out << "'>Platform Mapping</a></li>" << endl;
        out << "            <li><a href='";
        out << getFullName(platformUsageFilename, "html");
        out << "'>Platform Usage</a></li>" << endl;
    }
        
    if (getSetOfNoCScheduleProblems() != NULL)
    {
        out << "            <li><a href='"; 
        out << getFullName(interconnectGraphFilename, "html");
        out << "'>Interconnect</a></li>" << endl;
        out << "            <li><a href='";
        out << getFullName(interconnectMappingFilename, "html");
        out << "'>Interconnect Mapping</a></li>" << endl;
        out << "            <li><a href='";
        out << getFullName(interconnectUsageFilename, "html");
        out << "'>Interconnect Usage</a></li>" << endl;
    }
    
    out << "        </ul>" << endl;
    out << "    </div>" << endl; 
    out << endl;
    out << "    <div id='content'>";
}

/**
 * footerHeader ()
 * Write the footer of the HTML file to the supplied output stream.
 */
void SDFconvertToHTML::printFooter(ostream &out)
{
    out << "    </div>" << endl;
    out << "</div>" << endl;
    out << "</body>" << endl;
    out << "</html>" << endl;
}

/**
 * convertSDFgraphToPNG ()
 * Convert the graph using dot to a PNG image. The function returns an image 
 * map in XHTML format.
 */
CString SDFconvertToHTML::convertSDFgraphToPNG(TimedSDFgraph *g, 
        CString filename)
{
    CString tmpFilename, map;
    ofstream tmpFile;
    ifstream mapFile;
    
    // Temporary file
    tmpFilename = tempFileName("", "sdf3");
    tmpFile.open(tmpFilename);
    
    // Convert graph to dot format
    outputSDFgraphAsDot(g, tmpFile);
    
    // Close the temporary file
    tmpFile.close();
    
    // Run dot to convert the picture
    if (system("dot -Tpng -o " + filename + " " + tmpFilename) != 0)
        throw CException("Failed running dot."); 

    // Run dot to convert the picture
    if (system("dot -Tcmapx -o " + tmpFilename + ".map " + tmpFilename) != 0)
        throw CException("Failed running dot."); 

    // Read the image map from file
    mapFile.open(tmpFilename + ".map");
    
    // Read map from file
    while (!mapFile.eof())
    {
        char c;
        mapFile.get(c);
        
        if (!mapFile.eof())
            map += c;
    }

    // Close the map
    mapFile.close();    

    // Remove temporary file
    remove(tmpFilename);
    remove(tmpFilename + ".map");
    
    // Done
    return map;
}

/**
 * convertPlatformGraphToPNG ()
 * Convert the graph using dot to a PNG image.
 */
CString SDFconvertToHTML::convertPlatformGraphToPNG(PlatformGraph *g,
        CString filename)
{
    CString tmpFilename, map;
    ofstream tmpFile;
    ifstream mapFile;
    
    // Temporary file
    tmpFilename = tempFileName("", "sdf3");
    tmpFile.open(tmpFilename);
    
    // Convert graph to dot format
    outputPlatformGraphAsDot(g, tmpFile);
    
    // Close the temporary file
    tmpFile.close();
    
    // Run dot to convert the picture
    if (system("dot -Tpng -o " + filename + " " + tmpFilename) != 0)
        throw CException("Failed running dot."); 

    // Run dot to convert the picture
    if (system("dot -Tcmapx -o " + tmpFilename + ".map " + tmpFilename) != 0)
        throw CException("Failed running dot."); 

    // Read the image map from file
    mapFile.open(tmpFilename + ".map");
    
    // Read map from file
    while (!mapFile.eof())
    {
        char c;
        mapFile.get(c);
        
        if (!mapFile.eof())
            map += c;
    }

    // Close the map
    mapFile.close();    

    // Remove temporary file
    remove(tmpFilename);
    remove(tmpFilename + ".map");
    
    // Done
    return map;
}

/**
 * convertPlatformMappingToPNG ()
 * Convert the mapping using dot to a PNG image.
 */
CString SDFconvertToHTML::convertPlatformMappingToPNG(TimedSDFgraph *sdfGraph,
        PlatformGraph *platformGraph, CString filename)
{
    CString tmpFilename, map;
    ofstream tmpFile;
    ifstream mapFile;
    
    // Temporary file
    tmpFilename = tempFileName("", "sdf3");
    tmpFile.open(tmpFilename);
    
    // Convert graph to dot format
    outputPlatformMappingAsDot(sdfGraph, platformGraph, tmpFile);
    
    // Close the temporary file
    tmpFile.close();
    
    // Run dot to convert the picture
    if (system("dot -Tpng -o " + filename + " " + tmpFilename) != 0)
        throw CException("Failed running dot."); 

    // Run dot to convert the picture
    if (system("dot -Tcmapx -o " + tmpFilename + ".map " + tmpFilename) != 0)
        throw CException("Failed running dot."); 

    // Read the image map from file
    mapFile.open(tmpFilename + ".map");
    
    // Read map from file
    while (!mapFile.eof())
    {
        char c;
        mapFile.get(c);
        
        if (!mapFile.eof())
            map += c;
    }

    // Close the map
    mapFile.close();    

    // Remove temporary file
    remove(tmpFilename);
    remove(tmpFilename + ".map");
    
    // Done
    return map;
}

/**
 * convertInterconnectGraphToPNG ()
 * Convert the graph using dot to a PNG image. The function returns an image 
 * map in XHTML format.
 */
CString SDFconvertToHTML::convertInterconnectGraphToPNG(InterconnectGraph *g, 
        CString filename)
{
    CString tmpFilename, map;
    ofstream tmpFile;
    ifstream mapFile;
    
    // Temporary file
    tmpFilename = tempFileName("", "sdf3");
    tmpFile.open(tmpFilename);
    
    // Convert graph to dot format
    outputInterconnectGraphAsDot(g, tmpFile);
    
    // Close the temporary file
    tmpFile.close();
    
    // Run dot to convert the picture
    if (system("dot -Tpng -o " + filename + " " + tmpFilename) != 0)
        throw CException("Failed running dot."); 

    // Run dot to convert the picture
    if (system("dot -Tcmapx -o " + tmpFilename + ".map " + tmpFilename) != 0)
        throw CException("Failed running dot."); 

    // Read the image map from file
    mapFile.open(tmpFilename + ".map");
    
    // Read map from file
    while (!mapFile.eof())
    {
        char c;
        mapFile.get(c);
        
        if (!mapFile.eof())
            map += c;
    }

    // Close the map
    mapFile.close();    

    // Remove temporary file
    remove(tmpFilename);
    remove(tmpFilename + ".map");
    
    // Done
    return map;
}

/**
 * outputSDFgraphAsDot ()
 * The function outputs an SDF graph in DOT format.
 */
void SDFconvertToHTML::outputSDFgraphAsDot(SDFgraph *g, ostream &out)
{
    out << "digraph " << g->getName() << " {" << endl;
    out << "    size=\"7,10\";" << endl;
    
    // Output all actors
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        
        out << "    " << a->getName() << " [ label=\"" << a->getName();
        out << "\", href=\"#actor_" << a->getName() << "\" ];" << endl;
    }
    out << endl;
    
    // Output all channels
    for (SDFchannelsIter iter = g->channelsBegin(); 
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        
        // Initial tokens on channel?
        if (c->getInitialTokens() != 0)
        {
            SDFport *srcP = c->getSrcPort();
            SDFactor *srcA = srcP->getActor();
            SDFport *dstP = c->getDstPort();
            SDFactor *dstA = dstP->getActor();
            
            out << "    " << srcA->getName() << " -> " << dstA->getName();
            out << " [ label=\"" << c->getName() << "(";
            out << c->getInitialTokens()<< ")" << "\", taillabel=\"";
            out << srcP->getRate() << "\", headlabel=\"";
            out << dstP->getRate() << "\" href=\"#channel_" << c->getName();
            out << "\" ];" << endl;
        }
        else
        {
            SDFport *srcP = c->getSrcPort();
            SDFactor *srcA = srcP->getActor();
            SDFport *dstP = c->getDstPort();
            SDFactor *dstA = dstP->getActor();
            
            out << "    " << srcA->getName() << " -> " << dstA->getName();
            out << " [ label=\"" << c->getName() << "\", taillabel=\"";
            out << srcP->getRate() << "\", headlabel=\"";
            out << dstP->getRate() << "\" href=\"#channel_" << c->getName();
            out << "\" ];" << endl;
        }
    }
    
    out << "}" << endl;
}

/**
 * outputPlatformGraphAsDot ()
 * The function outputs a platform graph in DOT format.
 */
void SDFconvertToHTML::outputPlatformGraphAsDot(PlatformGraph *g, ostream &out)
{
    out << "digraph " << g->getName() << " {" << endl;
    out << "    size=\"7,10\";" << endl;
    
    // Output all tiles
    for (TilesIter iter = g->tilesBegin();
            iter != g->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        
        out << "    " << t->getName() << " [ label=\"" << t->getName();
        out << "\", shape=\"box\", href=\"#tile_" << t->getName() << "\" ];";
        out << endl;
    }
    out << endl;
    
    // Output all connections
    for (ConnectionsIter iter = g->connectionsBegin(); 
            iter != g->connectionsEnd(); iter++)
    {
        Connection *c = *iter;
        
        out << "    " << c->getSrcTile()->getName() << " -> ";
        out << c->getDstTile()->getName();
        out << " [ label=\"" << c->getName() << "\", href=\"#connection_";
        out << c->getName() << "\" ];" << endl;
    }
    out << endl;
    
    out << "}" << endl;
}

/**
 * outputPlatformMappingAsDot ()
 * The function outputs a mapping in DOT format.
 */
void SDFconvertToHTML::outputPlatformMappingAsDot(TimedSDFgraph *sdfGraph,
        PlatformGraph *platformGraph, ostream &out)
{
    out << "digraph " << platformGraph->getName() << " {" << endl;
    out << "    size=\"7,10\";" << endl;

    // Output all actors
    for (SDFactorsIter iter = sdfGraph->actorsBegin();
            iter != sdfGraph->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        
        out << "    " << a->getName() << " [ label=\"" << a->getName();
        out << "\", href=\"#actor_" << a->getName() << "\" ];" << endl;
    }
    out << endl;
    
    // Output all channels
    for (SDFchannelsIter iter = sdfGraph->channelsBegin(); 
            iter != sdfGraph->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        
        // Initial tokens on channel?
        if (c->getInitialTokens() != 0)
        {
            SDFport *srcP = c->getSrcPort();
            SDFactor *srcA = srcP->getActor();
            SDFport *dstP = c->getDstPort();
            SDFactor *dstA = dstP->getActor();
            
            out << "    " << srcA->getName() << " -> " << dstA->getName();
            out << " [ label=\"" << c->getName() << "(";
            out << c->getInitialTokens()<< ")" << "\", taillabel=\"";
            out << srcP->getRate() << "\", headlabel=\"";
            out << dstP->getRate() << "\" href=\"#channel_" << c->getName();
            out << "\" ];" << endl;
        }
        else
        {
            SDFport *srcP = c->getSrcPort();
            SDFactor *srcA = srcP->getActor();
            SDFport *dstP = c->getDstPort();
            SDFactor *dstA = dstP->getActor();
            
            out << "    " << srcA->getName() << " -> " << dstA->getName();
            out << " [ label=\"" << c->getName() << "\", taillabel=\"";
            out << srcP->getRate() << "\", headlabel=\"";
            out << dstP->getRate() << "\" href=\"#channel_" << c->getName();
            out << "\" ];" << endl;
        }
    }

    // Output all tiles
    for (TilesIter iter = platformGraph->tilesBegin();
            iter != platformGraph->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        
        out << "    " << t->getName() << " [ label=\"" << t->getName();
        out << "\", shape=\"box\", href=\"#tile_" << t->getName() << "\" ];";
        out << endl;
    }
    out << endl;
    
    // Output all connections
    for (ConnectionsIter iter = platformGraph->connectionsBegin(); 
            iter != platformGraph->connectionsEnd(); iter++)
    {
        Connection *c = *iter;
        
        out << "    " << c->getSrcTile()->getName() << " -> ";
        out << c->getDstTile()->getName();
        out << " [ label=\"" << c->getName() << "\", href=\"#connection_";
        out << c->getName() << "\" ];" << endl;
    }
    out << endl;
    
    // Output the mapping of actors to tiles
    for (TilesIter iter = platformGraph->tilesBegin();
            iter != platformGraph->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        CompBindings * bindings = t->getProcessor()->getActorBindings();

        for (ComponentBindingsIter bindingIter = bindings->begin();
                bindingIter != bindings->end(); bindingIter++)
        {
            ComponentBinding *b = *bindingIter;

            if (b->getComponent() != NULL)
            {
                out << "    " << b->getComponent()->getName() << " -> ";
                out << t->getName();
                out << " [ style=\"dashed\" ];" << endl;
            }
        }
    }
    
    out << "}" << endl;
}

/**
 * outputInterconnectGraphAsDot ()
 * The function outputs an interconnect graph in DOT format.
 */
void SDFconvertToHTML::outputInterconnectGraphAsDot(InterconnectGraph *g,
        ostream &out)
{
    out << "digraph interconnect {" << endl;
    out << "    size=\"7,10\";" << endl;
    
    // Output all nodes
    for (NodesIter iter = g->nodesBegin(); iter != g->nodesEnd(); iter++)
    {
        Node *n = *iter;
        
        out << "    " << n->getName() << " [ label=\"" << n->getName();
        out << "\"";
        
        if (n->getType() == "tile")
            out << ", shape=\"box\" ";
        else
            out << ", shape=\"diamond\" ";
            
        out << "];" << endl;
    }
    out << endl;
    
    // Output all links
    for (LinksIter iter = g->linksBegin(); iter != g->linksEnd(); iter++)
    {
        Link *l = *iter;
        
        out << "    " << l->getSrcNode()->getName() << " -> ";
        out << l->getDstNode()->getName();
        out << " [ label=\"" << l->getName() << "\", ";
        out << "href=\"#link_" << l->getName();
        out << "\" ];" << endl;
    }
    
    out << "}" << endl;
}

/**
 * convertSDFgraph ()
 * Convert the SDF graph to HTML format.
 */
void SDFconvertToHTML::convertSDFgraph()
{
    ofstream out;
    
    // No SDF graph given?
    if (getSDFgraph() == NULL)
        return;

    // Create an output stream for the graph
    out.open(getFullName(sdfGraphFilename, "html"));
    if (!out.is_open())
    {
        throw CException("Failed opening '" 
                   + getFullName(sdfGraphFilename, "html") + "'.");  
    }
    
    // Write header to the file
    printHeader(out, "SDF graph");

    // Graph
    out << "<a name='actor_" << sdfGraph->getName() << "'/>" << endl;
    out << "<h2>Graph: " << sdfGraph->getName() << "</h2>" << endl;
    out << endl;

    // Picture
    out << convertSDFgraphToPNG(sdfGraph, getFullName(sdfGraphFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(sdfGraphFilename, "png") << "' usemap='#";
    out << sdfGraph->getName() << "'/></p>" << endl;
    out << endl;

    // Actors
    out << "<p><b>Actors: </b>" << endl;
    for (SDFactorsIter actorIter = sdfGraph->actorsBegin(); 
            actorIter != sdfGraph->actorsEnd(); actorIter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)(*actorIter);

        if (actorIter != sdfGraph->actorsBegin())
            out << ", ";
        
        out << "<a href='#actor_" << a->getName() << "'>";
        out << a->getName() << "</a>";
    }
    out << "</p>" << endl;
    out << endl;

    // Channels
    out << "<p><b>Channel: </b>" << endl;
    for (SDFchannelsIter channelIter = sdfGraph->channelsBegin(); 
            channelIter != sdfGraph->channelsEnd(); channelIter++)
    {
        TimedSDFchannel *c = (TimedSDFchannel*)(*channelIter);

        if (channelIter != sdfGraph->channelsBegin())
            out << ", ";
        
        out << "<a href='#channel_" << c->getName() << "'>";
        out << c->getName() << "</a>";
    }
    out << "</p>" << endl;
    out << endl;

    // Timing constraints
    out << "<p><b class='header'>Throughput constraint:</b> " <<endl;
    sdfGraph->getThroughputConstraint().print(out);
    out << "</p>" << endl;
    out << endl;

    // Actors
    for (SDFactorsIter actorIter = sdfGraph->actorsBegin(); 
            actorIter != sdfGraph->actorsEnd(); actorIter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)(*actorIter);
        
        // Header of actor
        out << "<hr/><a name='actor_" << a->getName() << "'/>" << endl;
        out << "<h3>Actor: " << a->getName() << "</h3>" << endl;
        out << endl;

        // Ports
        out << "<table class='center'>" << endl;
        out << "    <tr>" << endl;
        out << "        <th>port name</th>" << endl;
        out << "        <th>direction</th>" << endl;
        out << "        <th>rate</th>" << endl;
        out << "        <th>channel</th>" << endl;
        out << "    </tr>" << endl;
        for (SDFportsIter portIter = a->portsBegin();
                portIter != a->portsEnd(); portIter++)
        {
            SDFport *p = (*portIter);
            
            out << "    <tr>" << endl;
            out << "        <td>" << p->getName() << "</td>" << endl;
            out << "        <td>" << p->getTypeAsString() << "</td>" << endl;
            out << "        <td>" << p->getRate() << "</td>" << endl;
            out << "        <td><a href='#channel_" << p->getChannel()->getName();
            out << "'>" << p->getChannel()->getName() << "</a></td>" << endl;
            out << "    </tr>" << endl;
        }
        out << "</table>" << endl;

        out << endl;
        out << "<br/>" << endl;
        out << endl;

        // Properties
        out << "<table class='center'>" <<endl;
        out << "    <tr>" << endl;
        out << "        <th>processor type</th>" << endl;
        out << "        <th>execution time</th>" << endl;
        out << "        <th>statesize</th>" << endl;
        out << "        <th>default</th>" << endl;
        out << "    </tr>" << endl;
        for (TimedSDFactor::ProcessorsIter procIter = a->processorsBegin();
                procIter != a->processorsEnd(); procIter++)
        {
            TimedSDFactor::Processor *p = (*procIter);
            
            out << "    <tr>" << endl;
            out << "        <td>" << p->type << "</td>" << endl;
            out << "        <td>" << p->execTime << "</td>" << endl;
            out << "        <td>" << p->stateSize << "</td>" << endl;
            if (a->getDefaultProcessor() == p->type)
                out << "        <td>true</td>" << endl;
            else
                out << "        <td>false</td>" << endl;
            out << "    </tr>" << endl;
        }
        out << "</table>" << endl;
        out << endl;
    
        out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << endl;
        out << endl;
    }

    // Channels
    for (SDFchannelsIter channelIter = sdfGraph->channelsBegin(); 
            channelIter != sdfGraph->channelsEnd(); channelIter++)
    {
        TimedSDFchannel *c = (TimedSDFchannel*)(*channelIter);

        // Header of channel
        out << "<hr/><a name='channel_" << c->getName() << "'/>" << endl;
        out << "<h3>Channel: " << c->getName() << "</h3>" << endl;
        out << endl;

        // Structure
        out << "<table class='center'>" <<endl;
        out << "    <tr>" << endl;
        out << "        <th>src actor</th>" << endl;
        out << "        <th>src port</th>" << endl;
        out << "        <th>dst actor</th>" << endl;
        out << "        <th>dst port</th>" << endl;
        out << "        <th>#initial tokens</th>" << endl;
        out << "    </tr>" << endl;
        out << "    <tr>" << endl;
        out << "        <td><a href='#actor_" << c->getSrcActor()->getName();
        out << "'>" << c->getSrcActor()->getName() << "</a></td>" << endl;
        out << "        <td>" << c->getSrcPort()->getName() << "</td>" << endl;
        out << "        <td><a href='#actor_" << c->getDstActor()->getName();
        out << "'>" << c->getDstActor()->getName() << "</a></td>" << endl;
        out << "        <td>" << c->getDstPort()->getName() << "</td>" << endl;
        out << "        <td>" << c->getInitialTokens() << "</td>" << endl;
        out << "    </tr>" << endl;
        out << "</table>" << endl;

        out << endl;
        out << "<br/>" << endl;
        out << endl;

        // Properties
        out << "<table class='center'>" <<endl;
        out << "    <tr>" << endl;
        out << "        <th>tokensize</th>" << endl;
        out << "        <th>latency</th>" << endl;
        out << "        <th>bandwidth</th>" << endl;
        out << "        <th>buffersize (sz/src/dst/mem)</th>" << endl;
        out << "    </tr>" << endl;
        out << "    <tr>" << endl;
        out << "        <td>" << c->getTokenSize() << "</td>" << endl;
        out << "        <td>" << c->getMinLatency() << "</td>" << endl;
        out << "        <td>" << c->getMinBandwidth() << "</td>" << endl;
        out << "        <td>";
        if (c->getBufferSize().sz != SDF_INFINITE_SIZE)
            out << c->getBufferSize().sz; 
        else
            out << "inf";
        out << "/";
        if (c->getBufferSize().src != SDF_INFINITE_SIZE)
            out << c->getBufferSize().src; 
        else
            out << "inf";
        out << "/";
        if (c->getBufferSize().dst != SDF_INFINITE_SIZE)
            out << c->getBufferSize().dst; 
        else
            out << "inf";
        out << "/";
        if (c->getBufferSize().mem != SDF_INFINITE_SIZE)
            out << c->getBufferSize().mem; 
        else
            out << "inf";
        out << "</td>";
        out << "    </tr>" << endl;
        out << "</table>" << endl;
        out << endl;

        out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << endl;
        out << endl;
    }
    out << endl;
    
    // Write footer to the file
    printFooter(out);
}

/**
 * convertPlatformGraph ()
 * Convert the platform graph to HTML format.
 */
void SDFconvertToHTML::convertPlatformGraph()
{
    ofstream out;
    
    // No SDF graph given?
    if (getPlatformGraph() == NULL)
        return;

    // Create an output stream for the graph
    out.open(getFullName(platformGraphFilename, "html"));
    if (!out.is_open())
    {
        throw CException("Failed opening '"
                 + getFullName(platformGraphFilename, "html") + "'.");  
    }
    
    // Write header to the file
    printHeader(out, "Platform");

    // Graph
    out << "<a name='platform_" << platformGraph->getName() << "'/>" << endl;
    out << "<h2>Platform: " << platformGraph->getName() << "</h2>" << endl;
    out << endl;

    // Picture
    out << convertPlatformGraphToPNG(platformGraph, 
                                    getFullName(platformGraphFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(platformGraphFilename, "png") << "' usemap='#";
    out << platformGraph->getName() << "'/></p>" << endl;
    out << endl;

    // Tiles
    out << "<p><b>Tiles: </b>" << endl;
    for (TilesIter tileIter = platformGraph->tilesBegin(); 
            tileIter != platformGraph->tilesEnd(); tileIter++)
    {
        Tile *t = (*tileIter);

        if (tileIter != platformGraph->tilesBegin())
            out << ", ";
        
        out << "<a href='#tile_" << t->getName() << "'>";
        out << t->getName() << "</a>";
    }
    out << "</p>" << endl;
    out << endl;

    // Connections
    out << "<p><b>Connections: </b>" << endl;
    for (ConnectionsIter connectionIter = platformGraph->connectionsBegin(); 
            connectionIter != platformGraph->connectionsEnd(); connectionIter++)
    {
        Connection *c = (*connectionIter);

        if (connectionIter != platformGraph->connectionsBegin())
            out << ", ";
        
        out << "<a href='#connections_" << c->getName() << "'>";
        out << c->getName() << "</a>";
    }
    out << "</p>" << endl;
    out << endl;
    
    // Tiles
    out << "<hr/><h3>Tiles</h3>" << endl;
    out << "<table class='center'>" <<endl;
    out << "    <tr>" << endl;
    out << "        <th>name</th>" << endl;
    out << "        <th>processor type</th>" << endl;
    out << "        <th>TDMA wheelsize</th>" << endl;
    out << "        <th>memory size</th>" << endl;
    out << "        <th>#connections</th>" << endl;
    out << "        <th>input bandwidth</th>" << endl;
    out << "        <th>output bandwidth</th>" << endl;
    out << "    </tr>" << endl;

    for (TilesIter tileIter = platformGraph->tilesBegin(); 
            tileIter != platformGraph->tilesEnd(); tileIter++)
    {
        Tile *t = (*tileIter);
        
        out << "    <tr>" << endl;
        out << "        <td><a name='tile_" << t->getName() << "'/>";
        out << t->getName() << "</td>" << endl;
        out << "        <td>" << t->getProcessor()->getType() << "</td>" << endl;
        out << "        <td>" << t->getProcessor()->getTimewheelSize() << "</td>" << endl;
        out << "        <td>" << t->getMemory()->getSize() << "</td>" << endl;
        out << "        <td>" << t->getNetworkInterface()->getNrConnections() << "</td>" << endl;
        out << "        <td>" << t->getNetworkInterface()->getInBandwidth() << "</td>" << endl;
        out << "        <td>" << t->getNetworkInterface()->getOutBandwidth() << "</td>" << endl;
        out << "    </tr>" << endl;
    }

    out << "</table>" << endl;
    out << endl;
    out << "<p class='footer'><a href='#top'>Back to top</a></p>" << endl;
    out << endl;
    
    // Connections
    out << "<hr/><h3>Connections</h3>" << endl;
    out << "<table class='center'>" <<endl;
    out << "    <tr>" << endl;
    out << "        <th>name</th>" << endl;
    out << "        <th>src tile</th>" << endl;
    out << "        <th>dst tile</th>" << endl;
    out << "        <th>delay</th>" << endl;
    out << "    </tr>" << endl;
    
    for (ConnectionsIter connectionIter = platformGraph->connectionsBegin(); 
            connectionIter != platformGraph->connectionsEnd(); connectionIter++)
    {
        Connection *c = (*connectionIter);
        
        out << "    <tr>" << endl;
        out << "        <td><a name='connection_" << c->getName() << "'/>";
        out << c->getName() << "</td>" << endl;
        out << "        <td><a name='#tile_" << c->getSrcTile()->getName() << "'/>";
        out << c->getSrcTile()->getName() << "</td>" << endl;
        out << "        <td><a name='#tile_" << c->getDstTile()->getName() << "'/>";
        out << c->getDstTile()->getName() << "</td>" << endl;
        out << "        <td>" << c->getLatency() << "</td>" << endl;
        out << "    </tr>" << endl;
    }

    out << "</table>" << endl;
    out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << endl;
    out << endl;
    
    // Write footer to the file
    printFooter(out);
}

/**
 * convertPlatformMapping ()
 * Convert the mapping to HTML format.
 */
void SDFconvertToHTML::convertPlatformMapping()
{
    ofstream out;
    
    // No SDF graph given or platform graph?
    if (getPlatformGraph() == NULL || getSDFgraph() == NULL)
        return;

    // Create an output stream for the graph
    out.open(getFullName(platformMappingFilename, "html"));
    if (!out.is_open())
    {
        throw CException("Failed opening '"
                 + getFullName(platformMappingFilename, "html") + "'.");
    }
    
    // Write header to the file
    printHeader(out, "Mapping");

    // Graph
    out << "<a name='mapping_" << platformGraph->getName() << "'/>" << endl;
    out << "<h2>Mapping: " << sdfGraph->getName() << " on ";
    out << platformGraph->getName() << "</h2>" << endl;
    out << endl;

    // Picture
    out << convertPlatformMappingToPNG(sdfGraph, platformGraph,
                                getFullName(platformMappingFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(platformMappingFilename, "png") << "' usemap='#";
    out << platformGraph->getName() << "'/></p>" << endl;
    out << endl;

    // Actors
    out << "<p><b>Actors: </b>" << endl;
    for (SDFactorsIter actorIter = sdfGraph->actorsBegin(); 
            actorIter != sdfGraph->actorsEnd(); actorIter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)(*actorIter);

        if (actorIter != sdfGraph->actorsBegin())
            out << ", ";
        
        out << "<a href='#actor_" << a->getName() << "'>";
        out << a->getName() << "</a>";
    }
    out << "</p>" << endl;
    out << endl;

    // Channels
    out << "<p><b>Channel: </b>" << endl;
    for (SDFchannelsIter channelIter = sdfGraph->channelsBegin(); 
            channelIter != sdfGraph->channelsEnd(); channelIter++)
    {
        TimedSDFchannel *c = (TimedSDFchannel*)(*channelIter);

        if (channelIter != sdfGraph->channelsBegin())
            out << ", ";
        
        out << "<a href='#channel_" << c->getName() << "'>";
        out << c->getName() << "</a>";
    }
    out << "</p>" << endl;
    out << endl;

    // Tiles
    out << "<p><b>Tiles: </b>" << endl;
    for (TilesIter tileIter = platformGraph->tilesBegin(); 
            tileIter != platformGraph->tilesEnd(); tileIter++)
    {
        Tile *t = (*tileIter);

        if (tileIter != platformGraph->tilesBegin())
            out << ", ";
        
        out << "<a href='#tile_" << t->getName() << "'>";
        out << t->getName() << "</a>";
    }
    out << "</p>" << endl;
    out << endl;

    // Connections
    out << "<p><b>Connections: </b>" << endl;
    for (ConnectionsIter connectionIter = platformGraph->connectionsBegin(); 
            connectionIter != platformGraph->connectionsEnd(); connectionIter++)
    {
        Connection *c = (*connectionIter);

        if (connectionIter != platformGraph->connectionsBegin())
            out << ", ";
        
        out << "<a href='#connections_" << c->getName() << "'>";
        out << c->getName() << "</a>";
    }
    out << "</p>" << endl;
    out << endl;
    
    // Tiles
    for (TilesIter tileIter = platformGraph->tilesBegin(); 
            tileIter != platformGraph->tilesEnd(); tileIter++)
    {
        Tile *t = (*tileIter);
        StaticOrderSchedule &s = t->getProcessor()->getSchedule();
        CompBindings *bindings;
        
        out << "<hr/><a name='tile_" << t->getName() << "'/>";
        out << "<h3>Tile: " << t->getName() << "</h3>" << endl;

        // No actors on the tile?
        if (s.empty())
        {
            out << "<p>No actors mapped to this tile.</p>" << endl; 
            out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << endl;
            out << endl;
        }
        else
        {
            out << "<p><b class='header'>Actors:</b> " << endl;
            bindings = t->getProcessor()->getActorBindings();
            for (ComponentBindingsIter bindingIter = bindings->begin();
                    bindingIter != bindings->end(); bindingIter++)
            {
                ComponentBinding *b = *bindingIter;

                if (b->getComponent() != NULL)
                {
                    if (bindingIter != bindings->begin())
                        out << ", ";
                    
                    out << "<a name='actor_" << b->getComponent()->getName();
                    out << "'/>" << b->getComponent()->getName();
                }
            }
            out << "</p>" << endl;
            out << endl;
            
            out << "<p><b class='header'>Static-order schedule:</b> " << endl;
            for (uint i = 0; i != s.size(); i++)
            {
                if (s.getStartPeriodicSchedule() ==i)
                    out << "<b>(</b>";
                    
                out << s.getScheduleEntry(i)->actor->getName();
                
                if (i + 1 == s.size())
                    out << "<b>)</b>";
                else
                    out << ", ";
            } 
            out << "</p>" << endl;
            out << endl;

            out << "<p><b class='header'>TDMA slice:</b> ";
            out << t->getProcessor()->getReservedTimeSlice() << endl;
            out << " (";
            out << 100.0 * (t->getProcessor()->getReservedTimeSlice()) /
                    (double)(t->getProcessor()->getTimewheelSize());
            out << "%)</p>" << endl; 

            bindings = t->getMemory()->getActorBindings();
            out << "<table class='center'>" <<endl;
            out << "    <tr>" << endl;
            out << "        <th>actor / channel</th>" << endl;
            out << "        <th>memory usage</th>" << endl;
            out << "    </tr>" << endl;
            for (ComponentBindingsIter bindingIter = bindings->begin();
                    bindingIter != bindings->end(); bindingIter++)
            {
                ComponentBinding *b = *bindingIter;

                if (b->getComponent() != NULL)
                {
                    out << "    <tr>" << endl;
                    out << "        <td>" << b->getComponent()->getName() << "</td>" << endl;
                    out << "        <td>" << b->getValue(0) << "</td>" << endl;
                    out << "    </tr>" << endl;
                }
            }
            bindings = t->getMemory()->getChannelBindings();
            for (ComponentBindingsIter bindingIter = bindings->begin();
                    bindingIter != bindings->end(); bindingIter++)
            {
                ComponentBinding *b = *bindingIter;

                if (b->getComponent() != NULL)
                {
                    out << "    <tr>" << endl;
                    out << "        <td>" << b->getComponent()->getName() << "</td>" << endl;
                    out << "        <td>" << b->getValue(0) << "</td>" << endl;
                    out << "    </tr>" << endl;
                }
            }
            out << "</table>" << endl;
            out << "</p>" << endl;

            out << "<table class='center'>" <<endl;
            out << "    <tr>" << endl;
            out << "        <th>channel</th>" << endl;
            out << "        <th>#connections</th>" << endl;
            out << "        <th>input bandwidth</th>" << endl;
            out << "        <th>output bandwidth</th>" << endl;
            out << "    </tr>" << endl;
            bindings = t->getNetworkInterface()->getBindings();
            for (ComponentBindingsIter bindingIter = bindings->begin();
                    bindingIter != bindings->end(); bindingIter++)
            {
                ComponentBinding *b = *bindingIter;

                if (b->getComponent() != NULL)
                {
                    out << "    <tr>" << endl;
                    out << "        <td>" << b->getComponent()->getName() << "</td>" << endl;
                    out << "        <td>" << b->getValue(NetworkInterface::nrConn) << "</td>" << endl;
                    out << "        <td>" << b->getValue(NetworkInterface::inBw) << "</td>" << endl;
                    out << "        <td>" << b->getValue(NetworkInterface::outBw) << "</td>" << endl;
                    out << "    </tr>" << endl;
                }
            }
            out << "</table>" << endl;

            out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << endl;
            out << endl;

            out << "</p>" << endl;
        }
    }
    
    // Connections
    out << "<hr/><h3>Connections</h3>" << endl;
    out << "<table class='center'>" <<endl;
    out << "    <tr>" << endl;
    out << "        <th>name</th>" << endl;
    out << "        <th>src tile</th>" << endl;
    out << "        <th>dst tile</th>" << endl;
    out << "        <th>channels</th>" << endl;
    out << "    </tr>" << endl;
    
    for (ConnectionsIter connectionIter = platformGraph->connectionsBegin(); 
            connectionIter != platformGraph->connectionsEnd(); connectionIter++)
    {
        Connection *c = (*connectionIter);
        CompBindings *bindings = c->getChannelBindings();
        
        out << "    <tr>" << endl;
        out << "        <td><a name='connection_" << c->getName() << "'/>";
        out << c->getName() << "</td>" << endl;
        out << "        <td><a href='#tile_" << c->getSrcTile()->getName();
        out << "'/>" << c->getSrcTile()->getName() << "</a></td>" << endl;
        out << "        <td><a href='#tile_" << c->getDstTile()->getName();
        out << "'/>" << c->getDstTile()->getName() << "</a></td>" << endl;
        out << "        <td>";
        for (ComponentBindingsIter bindingIter = bindings->begin();
                bindingIter != bindings->end(); bindingIter++)
        {
            ComponentBinding *b = *bindingIter;
            
            if (b->getComponent() != NULL)
            {
                if (bindingIter != bindings->begin())
                    out << ", ";
                
                out << "<a name='#channel_" << b->getComponent()->getName() << "'/>";
                out << b->getComponent()->getName() << "</a>";
            }
        }
        out << "</td>" << endl;
        out << "    </tr>" << endl;
    }

    out << "</table>" << endl;
    out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << endl;
    out << endl;
    
    // Write footer to the file
    printFooter(out);
}

/**
 * convertPlatformUsage ()
 * Convert the platform usage to HTML format.
 */
void SDFconvertToHTML::convertPlatformUsage()
{
    ofstream out;
    
    // No SDF graph given or platform graph?
    if (getPlatformGraph() == NULL || getSDFgraph() == NULL)
        return;

    // Create an output stream for the graph
    out.open(getFullName(platformUsageFilename, "html"));
    if (!out.is_open())
    {
        throw CException("Failed opening '" 
                        + getFullName(platformUsageFilename, "html") + "'.");  
    }

    // Write header to the file
    printHeader(out, "Usage");

    // Graph
    out << "<a name='usage_" << platformGraph->getName() << "'/>" << endl;
    out << "<h2>Platform usage: " << platformGraph->getName() << "</h2>" << endl;
    out << endl;

    // Picture
    out << convertPlatformGraphToPNG(platformGraph, 
                                    getFullName(platformUsageFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(platformUsageFilename, "png");
    out << "' usemap='#";
    out << platformGraph->getName() << "'/></p>" << endl;
    out << endl;

    // Tiles
    out << "<p><b>Tiles: </b>" << endl;
    for (TilesIter tileIter = platformGraph->tilesBegin(); 
            tileIter != platformGraph->tilesEnd(); tileIter++)
    {
        Tile *t = (*tileIter);

        if (tileIter != platformGraph->tilesBegin())
            out << ", ";
        
        out << "<a href='#tile_" << t->getName() << "'>";
        out << t->getName() << "</a>";
    }
    out << "</p>" << endl;
    out << endl;

    // Tiles
    out << "<hr/><h3>Tiles</h3>" << endl;
    out << "<table class='center'>" <<endl;
    out << "    <tr>" << endl;
    out << "        <th>name</th>" << endl;
    out << "        <th>TDMA wheel</th>" << endl;
    out << "        <th>memory</th>" << endl;
    out << "        <th>#connections</th>" << endl;
    out << "        <th>input bandwidth</th>" << endl;
    out << "        <th>output bandwidth</th>" << endl;
    out << "    </tr>" << endl;

    for (TilesIter tileIter = platformGraph->tilesBegin(); 
            tileIter != platformGraph->tilesEnd(); tileIter++)
    {
        Tile *t = (*tileIter);
        
        out << "    <tr>" << endl;
        out << "        <td><a name='tile_" << t->getName() << "'/>";
        out << t->getName() << "</td>" << endl;
        
        out << "        <td>";
        out << 100.0 * (t->getProcessor()->getReservedTimeSlice() 
                            + t->getProcessor()->getOccupiedTimeSlice()) /
                (double)(t->getProcessor()->getTimewheelSize());
        out << "%</td>" << endl; 

        out << "        <td>";
        out << 100.0 * (t->getMemory()->getSize() 
                - t->getMemory()->availableMemorySize()) /
                (double)(t->getMemory()->getSize());
        out << "%</td>" << endl; 

        out << "        <td>";
        out << 100.0 * (t->getNetworkInterface()->getNrConnections() 
                - t->getNetworkInterface()->availableNrConnections()) /
                (double)(t->getNetworkInterface()->getNrConnections());
        out << "%</td>" << endl; 

        out << "        <td>";
        out << 100.0 * (t->getNetworkInterface()->getInBandwidth() 
                - t->getNetworkInterface()->availableInBandwidth()) /
                (double)(t->getNetworkInterface()->getInBandwidth());
        out << "%</td>" << endl; 

        out << "        <td>";
        out << 100.0 * (t->getNetworkInterface()->getOutBandwidth() 
                - t->getNetworkInterface()->availableOutBandwidth()) /
                (double)(t->getNetworkInterface()->getOutBandwidth());
        out << "%</td>" << endl; 

        out << "    </tr>" << endl;
    }

    out << "</table>" << endl;
    out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << endl;
    out << endl;
    
    // Write footer to the file
    printFooter(out);
}

/**
 * convertInterconnectGraph ()
 * Convert the interconnect mapping to HTML format.
 */
void SDFconvertToHTML::convertInterconnectGraph()
{
    InterconnectGraph *interconnectGraph;
    ofstream out;
    
    // No interconnect mapping?
    if (getSetOfNoCScheduleProblems() == NULL 
            || getSetOfNoCScheduleProblems()->nrScheduleProblems() == 0)
    {
        return;
    }
    
    // Create an output stream for the interconnect
    out.open(getFullName(interconnectGraphFilename, "html"));
    if (!out.is_open())
    {
        throw CException("Failed opening '" 
                    + getFullName(interconnectGraphFilename, "html") + "'.");  
    }

    // Write header to the file
    printHeader(out, "Interconnect");

    // Graph
    out << "<a name='interconnect_graph'/>" << endl;
    out << "<h2>Interconnect</h2>" << endl;
    out << endl;

    // Interconnect graph (structure is equal for all problems)
    interconnectGraph = (*getSetOfNoCScheduleProblems()->scheduleProblemsBegin())->getInterconnectGraph();

    // Picture
    out << convertInterconnectGraphToPNG(interconnectGraph, 
                            getFullName(interconnectGraphFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(interconnectGraphFilename, "png");
    out << "' ";
    out << "usemap='#interconnect'/></p>" << endl;
    out << endl;

    // Links
    out << "<p><b class='header'>Links: </b>" << endl;
    for (LinksIter linkIter = interconnectGraph->linksBegin(); 
            linkIter != interconnectGraph->linksEnd(); linkIter++)
    {
        Link *l = (*linkIter);

        if (linkIter != interconnectGraph->linksBegin())
            out << ", ";
        
        out << l->getName();
    }
    out << "</p>" << endl;
    out << endl;
    
    // NoC properties
    out << "<p><b class='header'>Slot-table size: </b>";
    out << interconnectGraph->getSlotTableSize();
    out << "</p>" << endl;

    out << "<p><b class='header'>Packet header size: </b>";
    out << interconnectGraph->getPacketHeaderSize();
    out << "</p>" << endl;

    out << "<p><b class='header'>Flit size: </b>";
    out << interconnectGraph->getFlitSize();
    out << "</p>" << endl;

    out << "<p><b class='header'>Reconfiguration time NI: </b>";
    out << interconnectGraph->getReconfigurationTimeNI();
    out << "</p>" << endl;
    
    // Write footer to the file
    printFooter(out);
}

/**
 * convertInterconnectMapping ()
 * Convert the interconnect mapping to HTML format.
 */
void SDFconvertToHTML::convertInterconnectMapping()
{
    InterconnectGraph *interconnectGraph;
    ofstream out;
    
    // No interconnect mapping?
    if (getSetOfNoCScheduleProblems() == NULL 
            || getSetOfNoCScheduleProblems()->nrScheduleProblems() == 0)
    {
        return;
    }
    
    // Create an output stream for the interconnect
    out.open(getFullName(interconnectMappingFilename, "html"));
    if (!out.is_open())
    {
        throw CException("Failed opening '" 
                + getFullName(interconnectMappingFilename, "html") + "'.");
    }

    // Write header to the file
    printHeader(out, "Interconnect mapping");

    // Graph
    out << "<a name='interconnect_graph'/>" << endl;
    out << "<h2>Interconnect mapping</h2>" << endl;
    out << endl;

    // Interconnect graph (structure is equal for all problems)
    interconnectGraph = (*getSetOfNoCScheduleProblems()->scheduleProblemsBegin())->getInterconnectGraph();

    // Picture
    out << convertInterconnectGraphToPNG(interconnectGraph, 
                            getFullName(interconnectMappingFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(interconnectMappingFilename, "png");
    out << "' ";
    out << "usemap='#interconnect'/></p>" << endl;
    out << endl;

    // Links
    out << "<p><b>Links: </b>" << endl;
    for (LinksIter linkIter = interconnectGraph->linksBegin(); 
            linkIter != interconnectGraph->linksEnd(); linkIter++)
    {
        Link *l = (*linkIter);

        if (linkIter != interconnectGraph->linksBegin())
            out << ", ";
        
        out << "<a href='#link_" << l->getName() << "'>";
        out << l->getName() << "</a>";
    }
    out << "</p>" << endl;
    out << endl;

    // Scheduling problems
    out << "<p><b>Scheduling problems: </b>" << endl;

    for (NoCScheduleProblemsIter problemIter = 
            getSetOfNoCScheduleProblems()->scheduleProblemsBegin();
            problemIter != getSetOfNoCScheduleProblems()->scheduleProblemsEnd();
            problemIter++)
    {
        NoCScheduleProblem *problem = *problemIter;

        if (problemIter != getSetOfNoCScheduleProblems()->scheduleProblemsBegin())
            out << ", ";
        
        out << "<a href='#problem_" << problem->getName() << "'>";
        out << problem->getName() << "</a>";
    }
    out << "</p>" << endl;
    out << endl;
    
    // Scheduling problem switches
    out << "<hr/><h3>Switches between scheduling problems</h3>" << endl;
    out << "<table class='center'>" <<endl;
    out << "    <tr>" << endl;
    out << "        <th>from</th>" << endl;
    out << "        <th>to</th>" << endl;
    out << "        <th>overlap</th>" << endl;
    out << "    </tr>" << endl;

    // Iterate over all scheduling problems
    for (NoCScheduleProblemsIter problemIter = 
            getSetOfNoCScheduleProblems()->scheduleProblemsBegin();
            problemIter != getSetOfNoCScheduleProblems()->scheduleProblemsEnd();
            problemIter++)
    {
        NoCScheduleProblem *problem = *problemIter;
        
        // Iterate over all constraints
        for (NoCScheduleSwitchConstraintsIter constraintIter = 
                problem->scheduleSwitchConstraintsBegin();
                constraintIter != problem->scheduleSwitchConstraintsEnd();
                constraintIter++)
        {
            NoCScheduleSwitchConstraint constraint = *constraintIter;
            
            // Constraint starts from this problem
            if (constraint.from->getName() == problem->getName())
            {
                out << "    <tr>" << endl;
                out << "        <td>" << constraint.from->getName() << "</td>" << endl;
                out << "        <td>" << constraint.to->getName() << "</td>" << endl;
                out << "        <td>" << constraint.overlap << "</td>" << endl;
                out << "    </tr>" << endl;
            }
        }
    }
    out << "</table>" << endl;
    out << "<div><a class='footer' href='#top'>Back to graph</a></div>" << endl;
    out << endl;

    // Iterate over the scheduling problems
    for (NoCScheduleProblemsIter problemIter = 
            getSetOfNoCScheduleProblems()->scheduleProblemsBegin();
            problemIter != getSetOfNoCScheduleProblems()->scheduleProblemsEnd();
            problemIter++)
    {
        NoCScheduleProblem *problem = *problemIter;

        out << "<hr/><a name='problem_" << problem->getName() << "'/>" << endl;;
        out << "<h3>Scheduling problem: " << problem->getName() << "</h3>" << endl;
        out << "<table class='center'>" <<endl;
        out << "    <tr>" << endl;
        out << "        <th>link</th>" << endl;
        
        for (uint i= 0; i < interconnectGraph->getSlotTableSize(); i++)
            out << "        <th>slot " << i << "</th>" << endl;

        out << "    </tr>" << endl;
        
        for (LinksIter linkIter = interconnectGraph->linksBegin();
                linkIter != interconnectGraph->linksEnd(); linkIter++)
        {
            Link *l = *linkIter;
            SlotReservations slotReservations = l->getUsedSlotsInSchedule();
            
            out << "    <tr>"<< endl;
            out << "        <td>" << l->getName() << "</td>" << endl;
            
            for (uint i= 0; i < interconnectGraph->getSlotTableSize(); i++)
            {
                if (slotReservations[i])
                    out << "        <td>x</td>" << endl;
                else
                    out << "        <td>&nbsp;</td>" << endl;
            }                

            out << "    </tr>"<< endl;
        }
        out << "</table>" << endl;
        out << "<div><a class='footer' href='#top'>Back to graph</a></div>";
        out << endl << endl;
    }
    
    // Write footer to the file
    printFooter(out);
}

/**
 * convertInterconnectUsage ()
 * Convert the interconnect usage to HTML format.
 */
void SDFconvertToHTML::convertInterconnectUsage()
{
    InterconnectGraph *interconnectGraph;
    ofstream out;
    
    // No interconnect?
    if (getSetOfNoCScheduleProblems() == NULL 
            || getSetOfNoCScheduleProblems()->nrScheduleProblems() == 0)
    {
        return;
    }
    
    // Create an output stream for the interconnect
    out.open(getFullName(interconnectUsageFilename, "html"));
    if (!out.is_open())
    {
        throw CException("Failed opening '" 
                    + getFullName(interconnectUsageFilename, "html") + "'.");  
    }
    
    // Write header to the file
    printHeader(out, "Interconnect usage");

    // Graph
    out << "<a name='interconnect_graph'/>" << endl;
    out << "<h2>Interconnect usage</h2>" << endl;
    out << endl;

    // Interconnect graph (structure is equal for all problems)
    interconnectGraph = (*getSetOfNoCScheduleProblems()->scheduleProblemsBegin())->getInterconnectGraph();

    // Picture
    out << convertInterconnectGraphToPNG(interconnectGraph, 
                                getFullName(interconnectUsageFilename, "png"));
    out << "<p class='center'><img src='";
    out << getFullName(interconnectUsageFilename, "png ") << "' ";
    out << "usemap='#interconnect'/></p>" << endl;
    out << endl;

    // Links
    out << "<p><b>Links: </b>" << endl;
    for (LinksIter linkIter = interconnectGraph->linksBegin(); 
            linkIter != interconnectGraph->linksEnd(); linkIter++)
    {
        Link *l = (*linkIter);

        if (linkIter != interconnectGraph->linksBegin())
            out << ", ";
        
        out << "<a href='#link_" << l->getName() << "'>";
        out << l->getName() << "</a>";
    }
    out << "</p>" << endl;
    out << endl;

    // Links
    out << "<hr/><h3>Links</h3>" << endl;
    out << "<table class='center'>" <<endl;
    out << "    <tr>" << endl;
    out << "        <th>link</th>" << endl;
    for (uint i= 0; i < interconnectGraph->getSlotTableSize(); i++)
        out << "        <th>slot " << i << "</th>" << endl;
    out << "    </tr>" << endl;

    // Iterate over all links
    for (LinksIter linkIter = interconnectGraph->linksBegin();
            linkIter != interconnectGraph->linksEnd(); linkIter++)
    {
        Link *l = *linkIter;
        SlotReservations freeSlots, freeSlotsOnLinkInProblem;
    
        // Initialize free slot
        freeSlots.resize(interconnectGraph->getSlotTableSize());
        for (uint i= 0; i < interconnectGraph->getSlotTableSize(); i++)
            freeSlots[i] = true;
        
        // Iterate over all problems
        for (NoCScheduleProblemsIter problemIter = 
                getSetOfNoCScheduleProblems()->scheduleProblemsBegin();
                problemIter != getSetOfNoCScheduleProblems()->scheduleProblemsEnd();
                problemIter++)
        {
            NoCScheduleProblem *problem = *problemIter;
            Link *linkInProblem;
            
            // Find link l in this problem
            linkInProblem = problem->getInterconnectGraph()->getLink(l->getId());
        
            // Free slots in this problem on link l (iterate over all slot tables)
            for (SlotTablesIter iter = linkInProblem->slotTableSeqBegin();
                    iter != linkInProblem->slotTableSeqEnd(); iter++)
            {
                SlotTable &s = *iter;
                
                freeSlotsOnLinkInProblem = s.getSlotReservations(NULL);
                
                for (uint i= 0; i < interconnectGraph->getSlotTableSize(); i++)
                    freeSlots[i] = freeSlots[i] && freeSlotsOnLinkInProblem[i];
            }
        }
        
        // Iterate over all slots in the link
        out << "    <tr>"<< endl;
        out << "        <td><a name='link_" << l->getName() << "'/>";
        out << l->getName() << "</td>" << endl;
        
        for (uint i= 0; i < interconnectGraph->getSlotTableSize(); i++)
        {
            if (freeSlots[i])
                out << "        <td>&nbsp;</td>" << endl;
            else
                out << "        <td>x</td>" << endl;
        }                

        out << "    </tr>"<< endl;
    }    
    out << "</table>" << endl;
    out << "<div><a class='footer' href='#top'>Back to graph</a></div>";
    out << endl << endl;
    
    // Write footer to the file
    printFooter(out);
}

/**
 * convert ()
 * Convert the SDF graph and platform graph to HTML format. When mapping
 * or resource usage information is present, this information is also
 * exported to HTML.
 */
void SDFconvertToHTML::convert(const CString prefix)
{
    // Filename prefix
    fileNamePrefix = prefix;
    
    // No SDF graph given?
    if (getSDFgraph() == NULL)
        return;
    
    // Convert the SDF graph to HTML
    convertSDFgraph();
    
    // No platform graph given?
    if (getPlatformGraph() == NULL)
        return;

    // Convert the platform graph to HTML
    convertPlatformGraph();
    
    // Convert the mapping to HTML
    convertPlatformMapping();

    // Convert platform usage to HTML
    convertPlatformUsage();
    
    // Interconnect mapping
    if (getSetOfNoCScheduleProblems() != NULL)
    {
        convertInterconnectGraph();
        convertInterconnectMapping();
        convertInterconnectUsage();
    }    
}

