import json
import networkx as nx
import graphReader


def zipkingJSONFormatToDot(trace_path):
  '''
    this function transforms a zipking trace exported as json into a dot
    file that is accepted by the code generator
  '''
  with open(trace_path) as json_file:
    trace = json.load(json_file)
    outGraph = nx.DiGraph()
    #print(data)
    for span in trace:
      #print(span["id"])
      outGraph.add_node(span["id"])
      attributes = {span["id"]:
        {"label": span["localEndpoint"]["serviceName"],
          "serv":span["localEndpoint"]["serviceName"],
          "dur":span["duration"],
          "id":span["localEndpoint"]["serviceName"]}}
      #print(attributes)
      nx.set_node_attributes(outGraph, attributes)
      if("parentId" in span):
        outGraph.add_edge(span["parentId"], span["id"])
    return outGraph

g = zipkingJSONFormatToDot("traceZipkinExample.json")
graphReader.toDotAndMakeImg(g, "zipkinout.dot")