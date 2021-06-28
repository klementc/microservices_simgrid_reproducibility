import networkx as nx
import argparse
import os
import sys
import graphReader
import pandas as pd
import re


reqNames = []


def addSizes(mapReqs):
  '''
  Generate the code to set the correct sizes in output functions
  '''
  DEFAULTOUTSIZE = 100
  d = {}
  code="""\nint outSizes(RequestType t, std::string nextServ) {
    switch (t) {\n"""
  for k in mapReqs:
    graph = mapReqs[k]
    fromFile = graphReader.query_yes_no("Do you want to add output sizes for request %s from a size file? (Otherwise use default value: %d bytes)"%(k, DEFAULTOUTSIZE), default="no")
    if(fromFile):
      pathSizes = input("Please give the path to your size file: ")
      # for allowing a name in dot I replaced - by _ in the graph, do the same here for the container names
      d = {row[0].replace("-", "_") : row[2] for _, row in pd.read_csv(pathSizes).iterrows()}
    else:
      print("Using default size %d for all messages"%(DEFAULTOUTSIZE))

    outServs = {}
    root = [n for n,d in graph.in_degree() if d==0] [0]
    for e in nx.dfs_edges(graph,source = root):
      if(not graph.nodes[e[1]]["serv"] in outServs):
        outServs[graph.nodes[e[1]]["serv"]] = 1
      else:
        outServs[graph.nodes[e[1]]["serv"]] += 1

    switchVals = []
    print(d)
    for r in reqNames:
      val = re.split(r'(_\d)', r, maxsplit = 1)[0]
      print(str(val),"<->", str(k))
      if(val == k):
        switchVals.append(r)
    for r in switchVals:
      code += "\tcase RequestType::%s :\n"%(r)
      for o in outServs:
        # dirty, but to avoid missing names because of prefix
        present = False
        for kd in d:
          if(o in kd):
            present = kd
        if(present):
          print("found in d: amount: %d, value=%d/%d"%(outServs[o],int(d[present]),outServs[o]))
          code += "\tif (nextServ == \"%s\") {return %d;}\n"%(o,int(d[present]/outServs[o])) # we divide by the amount of instance of this service in the request
        else:
          print("%s not in d, use default"%(o))
          code += "\tif (nextServ == \"%s\") {return %d;}\n"%(o,DEFAULTOUTSIZE)
      code += "\tbreak;\n"
  code+="""}return -1; // WE SHOULD NEVER GET TO THAT POINT\n}\n"""

  return code

def genOutputFunctionSwitchCode(graph, requestType, hop = 1):
  '''
  Generates the switch case for each node of the graph
  to be pasted in the output function (redirecting taskRequests
  to the next service depending on the hop value and actual service)
  '''
  reqNames.append(requestType)
  mapCode={}
  print(graph)
  root = [n for n,d in graph.in_degree() if d==0] [0]
  edges = nx.dfs_edges(graph,source = root)

  # take care of lonely node after fork
  if(graph.number_of_nodes() == 1):
    servName = list(graph.nodes(data=True))[0][1]["serv"]
    #print("ENNDDDD %s "%(list(graph.nodes)))
    # TODO clean DELETIONS of spans
    code = """
    \ncase RequestType::%s:
    if (td->nbHops == %d) {
      XBT_DEBUG("Output Function %s (FINAL NODE, NO CHILD!)");
      // TODO DELETE
              for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it)
  {
    if ((*it)->get() != NULL) {
    auto t2 = std::chrono::seconds(946684800)+std::chrono::microseconds(int(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
    (*it)->get()->Log({{"end",t2.count()}});
    (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});
    (*it)->reset();
    (*it) == NULL;
    }
  }
    }
    """ % (requestType, hop, servName)
    if(not servName in mapCode):
      mapCode[servName] = code
    else:
      mapCode[servName] += code


  for edge in edges:
    print("edge: ",edge)
    # if more than one successor, it means we have parallel spans
    # in this case, add the output code for this requesttype, and call each of the successors
    if(len(list(graph.successors(edge[0]))) > 1 and not "seen" in graph.nodes[list(graph.successors(edge[0]))[0]]):
      srcServName = graph.nodes[edge[0]]["serv"]
      # generate the code for this node with one put per parallel child
      code = """
      if (td->nbHops == %d) {
        XBT_DEBUG("Output Function for %s (PARALLEL CHILDS)");
        std::vector<simgrid::s4u::CommPtr> comV;
        simgrid::s4u::CommPtr com;
      """ % (hop, edge[0])
      # foreach successor, add its put code
      childs = list(graph.successors(edge[0]))

      # use this to avoid using twice the same name for a mailbox on the same service (if multiple child calls to the same ouput service)
      servAlreadyUsed = []
      for i in range(len(childs)):
        dst = graph.nodes[childs[i]]
        graph.nodes[childs[i]]["seen"]=True
        dstMBName = dst["serv"]
        while(dstMBName in servAlreadyUsed):
          dstMBName += "_"
        # code to be used
        servAlreadyUsed.append(dstMBName)
        code += """
          XBT_DEBUG("Put to %s");
          s4u_Mailbox* m_%s = s4u_Mailbox::by_name("%s");
          sg_microserv::TaskDescription* td_%d = new sg_microserv::TaskDescription(*td);
          td_%d->requestType = RequestType::%s_%d;
          com = m_%s->put_async(td_%d, outSizes(td_%d->requestType, \"%s\"));
          comV.push_back(com);
        """ % (dst["serv"], dstMBName, dst["serv"], i, i, requestType,i,dstMBName,i,i,dst["serv"])
      if(not srcServName in mapCode):
        mapCode[srcServName] = "\ncase RequestType::%s:"%(requestType)
      mapCode[srcServName] += code
      mapCode[srcServName]+="\nsimgrid::s4u::Comm::wait_all(&comV); // wait for communications to finish\nbreak;\n}\n"

      # return the result of the maps of childrens and the current map
      for i in range(len(childs)):
        # ugly but easiest way I found for now
        subgraph = create_subgraph(graph,childs[i])
        graphReader.showGraphStr(subgraph)
        mapCode = mergeCodeMaps([mapCode, genOutputFunctionSwitchCode(subgraph,requestType+"_"+str(i),hop+1)])
      return mapCode
    # while we have sequential spans, no need to do something complex
    else:
      outputServName = graph.nodes[edge[0]]["serv"]
      dstName = graph.nodes[edge[1]]["serv"]
      print("generate code for: %s request: %s"%(edge[0], requestType))
      print("%s (serv %s) sends to %s"%(edge[0],  outputServName, edge[1]))

      # code to be used
      code = """
      if (td->nbHops == %d) {
        XBT_DEBUG("Output Function for %s, put to %s");
        s4u_Mailbox* m_%s = s4u_Mailbox::by_name("%s");
        m_%s->put(td, outSizes(td->requestType, \"%s\"));

      """ % (hop, requestType, dstName,dstName, dstName, dstName, dstName)

      # add it to the map
      if(not outputServName in mapCode):
        mapCode[outputServName] = "\ncase RequestType::%s:"%(requestType)

      mapCode[outputServName] += code
      mapCode[outputServName]+="\nbreak;\n}\n"
      # used to differentiate multiple calls to the same service
      hop+=1

      # if the dst node is the last node of the sequence, add its code now
      # TODO: clean deletes of spans
      if(len(list(graph.successors(edge[1]))) == 0):
        #print("%s is the last node"%(edge[1]))
        # code to be used
        code ="""
      if (td->nbHops == %d) {
        XBT_DEBUG("Output Function for %s, Final service, DELETE");
        for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it)
  {
    if ((*it)->get() != NULL) {
    auto t2 = std::chrono::seconds(946684800)+std::chrono::microseconds(int(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
    (*it)->get()->Log({{"end",t2.count()}});
    (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});
    (*it)->reset();
    (*it) == NULL;
    }
  }
        delete td;
      }
      """% (hop, requestType)
        if(not graph.nodes[edge[1]]["serv"] in mapCode):
          mapCode[graph.nodes[edge[1]]["serv"]] = "\ncase RequestType::%s:"%(requestType)

        mapCode[graph.nodes[edge[1]]["serv"]] += code

  # add break;
  for k in mapCode:
    print("add break to %s"%(k))
    mapCode[k]+="""\nbreak;\n"""

  return mapCode

def genETMSwitchFunction(graph, requestType, hop = 1):
  '''
  Generates the code to be pasted in the switch function given to ETMs
  so that they know the amount of computation to perform depending on the
  request type and hop value
  '''
  mapCode={}

  # take care of lonely nodes after fork
  if(graph.number_of_nodes() == 1):
    servName = list(graph.nodes(data=True))[0][1]["serv"]
    dur=int(list(graph.nodes(data=True))[0][1]["dur"])
    code = """
    \ncase RequestType::%s:
    if (td->nbHops == %d) {XBT_DEBUG("Entered cost Function for %s"); return %d;}
    """ % (requestType, hop, servName,int(dur)*1000)
    if(not servName in mapCode):
      mapCode[servName] = code
    else:
      mapCode[servName] += code

  root = [n for n,d in graph.in_degree() if d==0] [0]
  edges = nx.dfs_edges(graph,source = root)

  for edge in edges:
    #print(edge)
    # if more than one successor, it means we have parallel spans
    # in this case, add the output code for this requesttype, and call each of the successors
    print("%d : %s"%(len(list(graph.successors(edge[0]))),list(graph.successors(edge[0]))))
    if(len(list(graph.successors(edge[0]))) > 1):
      outputServName = graph.nodes[edge[0]]["serv"]
      dur = int(graph.nodes[edge[0]]["dur"])

      code ="""
      if (td->nbHops == %d) {XBT_DEBUG("Entered cost Function for %s"); return %d;\n}\nbreak;\n
      """% (hop, outputServName, int(dur)*1000)
      if(not graph.nodes[edge[0]]["serv"] in mapCode):
        mapCode[graph.nodes[edge[0]]["serv"]] =  "\ncase RequestType::%s:"%(requestType)
      mapCode[graph.nodes[edge[0]]["serv"]] += code

      # return the result of the maps of childrens and the current map
      childs = list(graph.successors(edge[0]))
      for i in range(len(childs)):
        # ugly but easiest way I found for now
        subgraph = create_subgraph(graph,childs[i])
        graphReader.showGraphStr(subgraph)
        mapCode = mergeCodeMaps([mapCode, genETMSwitchFunction(subgraph,requestType+"_"+str(i),hop+1)])
      return mapCode

    # while we have sequential spans, no need to do something complex
    else:
      outputServName = graph.nodes[edge[0]]["serv"]
      dur = int(graph.nodes[edge[0]]["dur"])
      #print("generate flop amount code for: %s request: %s"%(edge[0], requestType))
      #print("%s (service: %s cost: %s)"%(edge[0],  outputServName, dur))

      # code to be used
      code = """
      if (td->nbHops == %d) {XBT_DEBUG("Entered cost Function for %s"); return %s;}
      """ % (hop, requestType, dur*1000)

      # add it to the map
      if(not outputServName in mapCode):
        mapCode[outputServName] = "\ncase RequestType::%s:"%(requestType)

      mapCode[outputServName] += code

      hop += 1

      # if the dst node is the last node of the sequence, add its code now
      if(len(list(graph.successors(edge[1]))) == 0):
        #print("%s is the last node"%(edge[1]))
        # code to be used
        code ="""
      if (td->nbHops == %d) {XBT_DEBUG("Entered cost Function for %s (FINAL NODE, NO CHILD!)"); return %s;}
      """% (hop, requestType, int(graph.nodes[edge[1]]["dur"])*1000)
        if(not graph.nodes[edge[1]]["serv"] in mapCode):
          mapCode[graph.nodes[edge[1]]["serv"]] =  "\ncase RequestType::%s:"%(requestType)

        mapCode[graph.nodes[edge[1]]["serv"]] += code

  # add break;
  for k in mapCode:
    print("add break to %s"%(k))
    mapCode[k]+="\nbreak;\n"

  return mapCode

def genETMInitCode(graphs, parDeg=10):
  '''
  ETM constructor for a given list of graphs
  (only 1 ETM if a service is in multiple graphs (i.e. different requests))
  parDeg = maximum amount of parallel execs per inst
  '''
  servList = []
  for graph in graphs:
    # for each graph, add service if not already added to servList
    root = [n for n,d in graph.in_degree() if d==0] [0]
    edges = nx.dfs_edges(graph,source = root)
    for edge in edges:
      s1Name = graph.nodes[edge[0]]["serv"]
      s2Name = graph.nodes[edge[1]]["serv"]
      if(not s1Name in servList):
        servList.append(s1Name)
      if(not s2Name in servList):
        servList.append(s2Name)

  # build config file
  configFname = input("Give a name for the service config file:")
  configDef = {}
  n=0
  for service in servList:
    n+=1
    configDef[service] = {"location":"cb1-%d"%(n), "execRatio":"1", "parDeg":parDeg}
  # add default service in case
  configDef["default"] = {"location":"cb1-1", "execRatio":"1", "parDeg":parDeg}
  generateConfigFile(configDef, configFname)

  # add the code for each service
  print("%d different services"%(len(servList)))
  code = ""
  hostIndex=1
  for service in servList:
    print("Generate constructor for service %s"%(service))
    code += """
    // create ETM for service %s
    std::vector<std::string> v_serv_%s = std::vector<std::string>();
    v_serv_%s.push_back("%s");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_%s = std::make_shared<sg_microserv::ElasticTaskManager>("%s",v_serv_%s);
    serv_%s->setBootDuration(0);
    // serv_%s->setDataSizeRatio(1); NOT USED ANYMORE, KEEP AS COMMENT IN CASE
    // modify this value in the configuration file (parDeg)
    serv_%s->setParallelTasksPerInst(std::stoi(configServices.find(\"%s\")->second.at(2)));
    // modify this value in the  configuration file (execRatio)
    serv_%s->setExecRatio(std::stod(configServices.find(\"%s\")->second.at(1)));
    serv_%s->setOutputFunction(return_%s);
    serv_%s->setExecAmountFunc(pr_%s);
    serv_%s->setReqNames(reqTypeToStr);
    // set the host location in the config file (location)
    serv_%s->addHost(simgrid::s4u::Host::by_name(configServices.find(\"%s\")->second.at(0)));
    simgrid::s4u::Actor::create("etm_%s", simgrid::s4u::Host::by_name(configServices.find(\"%s\")->second.at(0)), [serv_%s] { serv_%s->run(); });

    """%(service,service,service,service,service,service,service,service,service,service,service,service,service,service,service,service,service,service,service,service,service,service,service,service)
    hostIndex+=1
  return code

def generateFullSimulationCode(mapReqNameToGraph):
  '''
  Generates almost ready to run code given a map of requestName->requestGraph
  - each graph must be a compacted graph of a request
  - don't give more than one graph for the same request

  This function gives as an output:
  - the return function
  - the pr functions (used by ETMs and instances to know how much computation is required)
  - construction of the ETMs

  Do by hand:
  - add hosts on the machines you want (1 example on cb1-#number by the automatic tool)
  - add dataSources
  '''
  code = ""

  # add imports
  code += """
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Host.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Comm.hpp>
#include "ElasticPolicy.hpp"
#include "ElasticTask.hpp"
#include "DataSource.hpp"
#include <memory>

XBT_LOG_NEW_DEFAULT_CATEGORY(run_log, "logs of the experiment");

int outSizes(RequestType, std::string);
"""

  # add output return functions
  code += "/* RETURN FUNCTIONS, AUTO GENERATED CODE, MODIFY IF YOU KNOW WHAT YOU WANT */"

  outputFunctionCodes = {}
  for k in mapReqNameToGraph:
    print("generate output code for request %s"%(k))
    cases = genOutputFunctionSwitchCode(mapReqNameToGraph[k], k)
    for serv in cases:
      # create header of func if not already done
      if(not serv in outputFunctionCodes):
        outputFunctionCodes[serv] = """
    void return_%s(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("Return function of service %s");
      switch (td->requestType)
      {"""%(serv,serv)

      # now add the case for this request
      outputFunctionCodes[serv] += cases[serv]
  # close the function
  for f in outputFunctionCodes:
    outputFunctionCodes[f]+="""\t}\n}"""
    code += outputFunctionCodes[f]

  # add pr functions
  code += "\n/* PR FUNCTIONS, AUTO GENERATED CODE, MODIFY IF YOU KNOW WHAT YOU WANT */"
  prFunctionCodes = {}
  for k in mapReqNameToGraph:
    print("fetch pr code for request %s"%(k))
    cases = genETMSwitchFunction(mapReqNameToGraph[k], k)
    for serv in cases:
      # create header of func if not already done
      if(not serv in prFunctionCodes):
        prFunctionCodes[serv] = """
    double pr_%s(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("pr function of service %s");
      switch (td->requestType) {"""%(serv, serv)

      # now add the case for this request
      prFunctionCodes[serv] += cases[serv]
  # close the function
  for f in prFunctionCodes:
    prFunctionCodes[f]+="""\t}//it should never end up here\nreturn -1;\n}"""
    code += prFunctionCodes[f]

  # add out function sizes
  code += addSizes(mapReqNameToGraph)

  # add dummy reqtype to string:
  code += """\nstd::string reqTypeToStr(RequestType t) {
    switch (t) {\n"""
  for k in reqNames:
    code+= "\tcase RequestType::%s : return \"%s\";break;\n"%(k,k)
  code+="""}return "WTF";\n}\n"""


  # add constructors
  code+="""
  void run(std::map<std::string, std::vector<std::string>> configServices) {
    XBT_INFO("Starting run()");
  """
  l = [mapReqNameToGraph[k] for k in mapReqNameToGraph ]
  code+=genETMInitCode(l)

  code+="""

  /* ADD DATASOURCES MANUALLY HERE, SET THE END TIMER AS YOU WISH, AND LAUNCH YOUR SIMULATOR*/

  // kill policies and ETMs
  simgrid::s4u::this_actor::sleep_for(150); /*set it according to your needs*/
  XBT_INFO("Done. Killing policies and etms");
  """
  servList = []
  for graph in l:
    # for each graph, add service if not already added to servList
    root = [n for n,d in graph.in_degree() if d==0] [0]
    edges = nx.dfs_edges(graph,source = root)
    for edge in edges:
      s1Name = graph.nodes[edge[0]]["serv"]
      s2Name = graph.nodes[edge[1]]["serv"]
      if(not s1Name in servList):
        servList.append(s1Name)
      if(not s2Name in servList):
        servList.append(s2Name)
  for serv in servList:
    code+="""
    serv_%s->kill();"""%(serv)

  code+="""
}
int main(int argc, char* argv[]) {
  if (argc <= 2) {
    std::cout << "Wrong execution line" << std::endl;
		std::cout << argv[0] << "<simgrid-platform-file.xml> <services-config-file.csv>" << std::endl;
		exit(1);
	}

  // read the config from the file provided by the user
  std::ifstream input(argv[2]);
  std::map<std::string, std::vector<std::string>> servConfig;
  std::string v;
  // pass header
  input>>v>>v>>v>>v;
  for (std::string servName, servLocation, servExecRatio, servParDeg; input >> servName >> servLocation >> servExecRatio >> servParDeg;) {
        servConfig[servName] = {servLocation, servExecRatio, servParDeg};
  }

	simgrid::s4u::Engine* e = new simgrid::s4u::Engine(&argc, argv);

	e->load_platform(argv[1]);
	simgrid::s4u::Actor::create("main", simgrid::s4u::Host::by_name(servConfig.find(\"default\")->second.at(0)), [&] {run(servConfig);});
	e->run();
	return 0;
}
"""
  return code

def mergeCodeMaps(mapsToMerge):
  '''
  Used to sum the code in different maps (used when having parallel spans)
  '''
  finalMap = {}
  for m in mapsToMerge:
    for k in m:
      if(not k in finalMap):
        finalMap[k] = m[k]
      else:
        finalMap[k] += "\n"+m[k]

  return finalMap


def generateConfigFile(mapServProps, fname="configSim.csv"):
  '''
    Used to obtain a more flexible executable
    using this file, you specify the location and execution ratio in a config file instead of values directly in the code
  '''
  with open(fname, 'w') as target:
    target.write("serviceName serviceLocation executionRatio parDeg\n")
    for serv in mapServProps:
      location = mapServProps[serv]["location"]
      execRatio = mapServProps[serv]["execRatio"]
      parDeg = mapServProps[serv]["parDeg"]

      target.write("%s %s %s %s\n"%(serv, location, str(execRatio), str(parDeg)))

# taken from https://stackoverflow.com/questions/32935510/networkx-digraph-create-subgraph-digraph-by-node
# warning: don't forget to specify the root node, otherwise it is chosen randomly, which gave me a headache :)
def create_subgraph(G, node):
  nodes = nx.single_source_shortest_path(G,node).keys()
  res = G.subgraph(nodes).copy()

  for i in G.out_edges(node, data=True):
    res.update([i])
    res.add_edge(i[0], i[1])
  for n in nodes:
    for j in G.out_edges(n, data=True):
      res.add_edge(j[0], j[1])

  return res