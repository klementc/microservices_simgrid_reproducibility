import networkx as nx
import argparse
from graphviz import render
from networkx.drawing.nx_agraph import write_dot
import os
import sys
import codeGen

# copied from https://stackoverflow.com/questions/3041986/apt-command-line-interface-like-yes-no-input, I find it fun
def query_yes_no(question, default="yes"):
    """Ask a yes/no question via raw_input() and return their answer.

    "question" is a string that is presented to the user.
    "default" is the presumed answer if the user just hits <Enter>.
        It must be "yes" (the default), "no" or None (meaning
        an answer is required of the user).

    The "answer" return value is True for "yes" or False for "no".
    """
    valid = {"yes": True, "y": True, "ye": True,
             "no": False, "n": False}
    if default is None:
        prompt = " [y/n] "
    elif default == "yes":
        prompt = " [Y/n] "
    elif default == "no":
        prompt = " [y/N] "
    else:
        raise ValueError("invalid default answer: '%s'" % default)

    while True:
        sys.stdout.write(question + prompt)
        choice = input().lower()
        if default is not None and choice == '':
            return valid[default]
        elif choice in valid:
            return valid[choice]
        else:
            sys.stdout.write("Please respond with 'yes' or 'no' "
                             "(or 'y' or 'n').\n")


def cleanNames(G,nodeList):
    s=""
    for n in nodeList:
        s+="\n\t- %s"%(G.nodes[n]["label"])
    return s

def verifyNoMissingServs(G1, G2):
    l1 = []
    l2 = []
    for n in G1.nodes():
        l1.append(G1.nodes[n]["label"])
    for n in G2.nodes():
        l2.append(G2.nodes[n]["label"])

    print("MISSING:"+str(set(l1) ^ set(l2)))

def showGraphStrFromF(fName):
    G = nx.drawing.nx_agraph.read_dot(fName)
    showGraphStr(G)

def showGraphStr(G):
    #G = graph
    print("Nodes and their attributes:")
    for node_id in G.nodes:
        print("%s: %s"%(node_id,G._node[node_id]))

def toImage(fName, outName):
    print("Render %s to %s"%(fName, outName))
    render('dot', 'png', fName)

def dfsTest(graph):
    #G = nx.drawing.nx_agraph.read_dot(fName)
    G = graph
    print("------------ Nodes in a DFS traversal:")
    print(list(G.nodes))
    print(list(G.edges))
    root = [n for n,d in G.in_degree() if d==0] [0]
    nodes = nx.dfs_edges(G, source = root)
    for n in nodes:
        #print(n)
        print("%s -> %s"%(G.nodes[n[0]],G.nodes[n[1]]))
    print("------------")

def toDotAndMakeImg(graph, fName, imgName=None):
    graph.graph['graph']={'rankdir':'LR'}
    write_dot(graph, fName)
    # compacted image:
    if(imgName == None):
        imgName = fName+".png"

    toImage(fName, imgName)

def getSumOfTimes(graph):
    sum = 0
    for n in graph.nodes:
        sum += int(graph._node[n]["dur"])

    return sum


def groupNodesWithSameServ(graph, root, already_list=[]):
    '''
    Decide and modify the structure of the graph depending on the need or not
    for parallel subtasks
    '''
    graphOut = nx.DiGraph()
    subg = codeGen.create_subgraph(graph, root)

    # add root and then add dst of each edge
    rootServ = graph.nodes[root]["label"]
    rootId = rootServ
    while(rootId in already_list):
        rootId += "_"
    already_list.append(rootId)
    graphOut.add_node(rootId)
    nx.set_node_attributes(graphOut,{rootId:{"serv":rootServ,"label":rootServ,"id":rootId,"dur":int(graph.nodes[root]["dur"])}})

    currentServ = {"serv":rootServ,"id":rootId}


    edges = nx.dfs_edges(subg, source = root)
    for src, dst in edges:
        srcChildNodes = list(subg.successors(src))
        if(len(srcChildNodes)>1):
            # add grouped graph for each child
            for succ in srcChildNodes:
                gChild, alistCh = groupNodesWithSameServ(graph, succ, already_list=already_list)
                already_list += alistCh
                graphOut.add_nodes_from(gChild.nodes(data=True))
                graphOut.add_edges_from(gChild.edges(data=True))
                # add link between src and succ
                rootChild =  [n for n,d in gChild.in_degree() if d==0][0]
                graphOut.add_edge(currentServ["id"],gChild.nodes[rootChild]["id"])
            return (graphOut, already_list)
        else:
            # already in this service, add duration
            if(graph.nodes[dst]["label"] == currentServ["serv"]):
                graphOut.nodes[currentServ["id"]]["dur"] += int(graph.nodes[dst]["dur"])
            # new service, add service and edge
            else:
                dstServ = graph.nodes[dst]["label"]
                dstId = dstServ
                while(dstId in already_list):
                    dstId += "_"
                already_list.append(dstId)
                graphOut.add_node(dstId)
                nx.set_node_attributes(graphOut,{dstId:{"serv":dstServ,"label":dstServ,"id":dstId,"dur":int(graph.nodes[dst]["dur"])}})
                graphOut.add_edge(currentServ["id"], dstId)
                currentServ = {"serv":dstServ,"id":dstId}

    return (graphOut, already_list)

def sequentializeOrNot(graph, root, seqLeafs=None):
    '''
    Group tasks that are neighboor as a single node
    '''
    graphOut = nx.DiGraph()
    #print("enter seqornot:",graph.nodes(data=True))
    #print("root: ",root)
    subg = codeGen.create_subgraph(graph, root)
    leafList = []

    if(subg.number_of_nodes() == 1):
        print("Only one node, add it and return")
        leaf = [nodeKey for nodeKey in subg.nodes()][0]
        leafList.append(leaf)
        return subg, leafList
    else:
        # it means the subgraph has edges, process for each edge
        edges = nx.dfs_edges(subg, source = root)
        for src,dst in edges:
            graphOut.add_node(src)
            nx.set_node_attributes(graphOut,{src:graph.nodes[src]})
            #print(src+"->"+dst)
            srcChildNodes = list(subg.successors(src))
            if(len(srcChildNodes)==1):
                print("Sequence, no problem add: %s %s",graph.nodes[dst],list(graph.successors(dst)))
                graphOut.add_node(dst)
                nx.set_node_attributes(graphOut,{dst:graph.nodes[dst]})
                graphOut.add_edge(src,dst)
                # if no child to dst, add it to the leaflist
                if(len(list(graph.successors(dst)))==0):
                    #print("ADDD LEAFF")
                    leafList.append(dst)
            else:
                print("FORK %d nodes"%(len(srcChildNodes)))
                # create subgraph for all childs and link them together
                prevChildLeaves = []
                parallel = query_yes_no("Execute in parallel?\nNode: %s\nChilds: %s\n"%(subg.nodes[src]["label"], cleanNames(subg, srcChildNodes)))
                for succ in srcChildNodes:
                    # create subgraph
                    #print("create subgraph: ",graph.nodes[succ])
                    gChild, leavesChild = sequentializeOrNot(graph, succ)
                    #print("Fork adds: %s"%(gChild.nodes(data=True)))
                    graphOut.add_nodes_from(gChild.nodes(data=True))
                    graphOut.add_edges_from(gChild.edges(data=True))

                    # if in parallel, attach child graph to the src node
                    if(parallel):
                        #print("Add edge: %s -> %s"%(src, succ))
                        graphOut.add_edge(src, succ)
                    else:
                        # TODO
                        # if first child:
                        #   add to src
                        # elif only one leaf in previous child:
                        #   add new child to previous leaf and new leaf = child.leaf
                        # else: (multiple leaves in previous child)
                        #   choose node to add to  (or take longest)
                        if(len(prevChildLeaves) == 0):
                            graphOut.add_edge(src,succ)
                            prevChildLeaves = leavesChild
                            #print("CHHHH: %s"%(prevChildLeaves))
                        elif(len(prevChildLeaves) == 1):
                            graphOut.add_edge(prevChildLeaves[0], succ)
                            prevChildLeaves = leavesChild
                            #print("CIIIIIII: %s"%(prevChildLeaves))
                        else:
                            longest = nx.dag_longest_path(gChild,weight="dur")
                            #print("LONGEST: %s"%(longest))
                            query_yes_no("TODO, WILL ADD TO %s "%prevChildLeaves[0])
                            graphOut.add_edge(longest[-1], succ)
                            prevChildLeaves = longest[-1]
                # GET LONGEST PATH  IN CURRENT GRAPH IF REQUIRED IN PARENT CALL
                longest = nx.dag_longest_path(graphOut,weight="dur")[-1]
                return graphOut, [longest] # TODO RETURN CORRECT CHILDS

    return graphOut, leafList

if __name__=="__main__":
    try:
        parser = argparse.ArgumentParser()
        # Adding optional argument
        parser.add_argument("-f", "--filename", help = "dot file to parse", required=False)
        parser.add_argument("-o", "--outFname", help="output image file name", required=False)
        parser.add_argument("-g", "--generate", help="generates code for the given file if set", required=False)
        parser.add_argument("-s", "--sum", help="compare sum of times between uncompacted and compact graphs if set", required=False)
        parser.add_argument("-i", "--interactive", help="Provide your traces and create your simulation code interractively", action='store_true')

        # Read arguments from command line
        args = parser.parse_args()

        graphDict = {}
        # interractive part
        if(args.interactive):
            print("""Welcome to the simulation code generator.\nBefore executing this you need to obtain jaeger traces of the requests you\nwant in your simulator as a dot file. You can obtain them from this modified jaeger-ui: https://github.com/klementc/jaeger-ui""")
            while(query_yes_no("Do you want to add a new trace to your simulator?")):
                print("Enter the request name associated to this trace : ",end="")
                reqName = input()
                print("Enter the path to your file : ", end="")
                path = input()
                G = nx.drawing.nx_agraph.read_dot(path)

                print("Processing dot graph for request %s (file: %s)"%(reqName, path))
                g, _ = sequentializeOrNot(G, root = [n for n,d in G.in_degree() if d==0][0])
                g2, _ = groupNodesWithSameServ(g, [n for n,d in g.in_degree() if d==0][0])

                for n in g.nodes():
                    g.nodes[n]["label"]+=" dur: "+str(g.nodes[n]["dur"])
                for n in g2.nodes():
                    g2.nodes[n]["label"]+=" dur: "+str(g2.nodes[n]["dur"])

                parOrNotPathImg = path+"_seqnot.png"
                parOrNotPathDot = path+"_seqnot.dot"
                processedGraphPathImg = path+"_seqnot_processed.png"
                processedGraphPathDot = path+"_seqnot_processed.dot"

                print("Save seq output graph as image to '%s', dot file to '%s'\nSave processed ouput as image to '%s', dot file to '%s'"%(parOrNotPathImg, parOrNotPathDot, processedGraphPathImg, processedGraphPathDot))
                toDotAndMakeImg(g, parOrNotPathDot, parOrNotPathImg)
                toDotAndMakeImg(g2, processedGraphPathDot, processedGraphPathImg)

                graphDict[reqName] = g2

                print("Sum of times in the original file: %d"%(getSumOfTimes(G)))
                print("Sum of times in the the processed graph: %d"%(getSumOfTimes(g)))
                print("Sum of times in the the final graph: %d"%(getSumOfTimes(g2)))

            print("All traces processed. Now generating code for :")
            for k in graphDict:
                print("\t- Trace %s"%(k))
            print("Please give the name of the code file to produce : ", end="")
            codeFile = input()
            code = codeGen.generateFullSimulationCode(graphDict)

            with open(codeFile, "w") as text_file:
                text_file.write(code)

            print("=--------------------------------------------------=\nCode generated successfully to '%s'"%(codeFile))
            print("You now need to add your dataSources to the simulation code before running it!")
        else:
            G = nx.drawing.nx_agraph.read_dot(args.filename)
            #GCompact = compactGraph(G)[0]
            g, _ = sequentializeOrNot(G, root = [n for n,d in G.in_degree() if d==0][0])
            toDotAndMakeImg(g, "aaa.dot", "aaa.png")
            g2, _ = groupNodesWithSameServ(g, [n for n,d in g.in_degree() if d==0][0])
            for n in g2.nodes():
                g2.nodes[n]["label"]+=" dur: "+str(g2.nodes[n]["dur"])
            toDotAndMakeImg(g2, "aaa2.dot", "aaa2.png")

            print("Sum of times in the original file: %d"%(getSumOfTimes(G)))
            print("Sum of times in the the processed graph: %d"%(getSumOfTimes(g)))
            print("Sum of times in the the grouped graph: %d"%(getSumOfTimes(g2)))

            d = codeGen.generateFullSimulationCode({"COMPOSE": g2})

            with open("generated.cpp", "w") as text_file:
                text_file.write(d)

    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)


