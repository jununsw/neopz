/* Generated by Together */

#include "TPZBoostGraph.h"
#include <fstream>

#ifdef USING_BOOST

#include "pzlog.h"

#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("boostgraph"));
#endif

using namespace boost;
using namespace std;

TPZBoostGraph::TPZBoostGraph(int NElements, int NNodes) : TPZRenumbering(NElements,NNodes),fGType(KMCExpensive)
{
  m_Graph.clear();
/*  Graph lG(NElements);
  m_Graph = lG;*/
  //m_Edges.resize(NElements);
  //Creating two iterators over the vertices
}


TPZBoostGraph::~TPZBoostGraph()
{
  //Performe termination tasks
}


void TPZBoostGraph::SetNodeWeights(TPZVec<int> &weights)
{
}
void TPZBoostGraph::ClearDataStructures()
{
  m_Edges.clear();
}
void TPZBoostGraph::SetElementGraph(TPZVec<int> &elgraph, TPZVec<int> &elgraphindex)
{
  fElementGraph = elgraph;
  fElementGraphIndex = elgraphindex;
  
}

void TPZBoostGraph::Resequence(TPZVec<int> &perm, TPZVec<int> &inverseperm)
{
#ifdef DEBUG
    std::cout << "TPZBoostGraph::Resequence started \n";
#endif
#ifdef LOG4CXX
  {
    std::stringstream sout;
    Print(fElementGraph,fElementGraphIndex,"Element graph when entering Resequence",sout);
    LOG4CXX_DEBUG(logger,sout.str())
  }
#endif
  Graph G;
  size_type i;
  size_type elgraphsize = fElementGraphIndex.NElements()-1;
  std::vector<int> nconnects(fNNodes,0);
  for(i=0; i < elgraphsize; i++)
  {
    int first, second;
    first = fElementGraphIndex[i];
    second = fElementGraphIndex[i+1];
    int j,k;
    for(j=first; j< second; j++)
    {
      nconnects[fElementGraph[j]]++;
      for(k=j+1; k<second; k++)
      {
        add_edge(fElementGraph[j],fElementGraph[k],G);
      }
    }
  }
  for(i=0; i< nconnects.size(); i++)
  {
    if(!nconnects[i]) 
    {
      add_edge(i,i,G);
    }
  }

  graph_traits<Graph>::vertex_iterator ui, ui_end;

  property_map<Graph,vertex_degree_t>::type deg = get(vertex_degree, G);
  for (boost::tie(ui, ui_end) = vertices(G); ui != ui_end; ++ui)
    deg[*ui] = degree(*ui, G);

#ifdef DEBUG
 std::cout << "NNodes " << fNNodes << std::endl;
 std::cout << "NElements " << fNElements << std::endl;

  std::cout << "Original Bandwidth: " << bandwidth(G) << std::endl;
  std::cout << " Original profile: " 
      << profile(G)
      << std::endl;
#endif
/*  std::cout << " Original max_wavefront: " 
      << max_wavefront(G)
      << std::endl;
  std::cout << " Original aver_wavefront: " 
      << aver_wavefront(G)
      << std::endl;
  std::cout << " Original rms_wavefront: " 
      << rms_wavefront(G)
      << std::endl;*/
//   std::cout << "Number of Vertices " << num_vertices(G) << std::endl;
//   std::cout << "Number of Edges " << num_edges(G) << std::endl;
  int nVertices = num_vertices(G);
//  std::cout << "Number of Vertices " << nVertices << std::endl;
  std::vector<Vertex> inv_perm(nVertices);
  std::vector<size_type> l_perm(nVertices);
  for(size_type i = 0; i < l_perm.size(); i++) l_perm[i]=-1;
  for(size_type i = 0; i < l_perm.size(); i++) inv_perm[i]=0;
  perm.Resize(fNNodes,-1);
  switch(fGType)
  {
    case KMC:
  
    {
      Vertex s = vertex(0, G);
      //reverse cuthill_mckee_ordering
      cuthill_mckee_ordering(G, s, inv_perm.begin(), get(vertex_color, G),
                            get(vertex_degree, G));   
#ifdef DEBUG 
      cout << "Reverse Cuthill-McKee ordering:" << endl;
#endif

    }
    break;
    
    case KMCExpensive:
    {
      cuthill_mckee_ordering(G, inv_perm.begin(), get(vertex_color, G),
                            get(vertex_degree, G));
#ifdef DEBUG
      cout << "Reverse Expensive Cuthill-McKee ordering:" << endl;
#endif

    }
    break;
    case Sloan:
    {
#ifdef DEBUG
      cout << "Sloan ordering:" << endl;
#endif      
    
    //Setting the start node
      Vertex s = vertex(0, G);
      int ecc;   //defining a variable for the pseudoperipheral radius
    
    //Calculating the pseudoeperipheral node and radius
      Vertex e = pseudo_peripheral_pair(G, s, ecc, get(vertex_color, G), get(vertex_degree, G) );
#ifdef DEBUG
      cout << "Sloan Starting vertex: " << s << endl;
      cout << "Sloan Pseudoperipheral vertex: " << e << endl;
      cout << "Sloan Pseudoperipheral radius: " << ecc << endl << endl;
#endif      
    //Sloan ordering
      sloan_ordering(G, s, e, inv_perm.begin(), get(vertex_color, G), 
                     get(vertex_degree, G), get(vertex_priority, G));
    }
    break;

  }
    for (size_type c = 0; c != inv_perm.size(); ++c)
    {
      l_perm[inv_perm[c]] = c;
    }
        
/*
    std::cout << "l_perm = ";
    for (size_type i = 0; i < l_perm.size(); ++i)
    {
      std::cout << i << "/" << l_perm[i] << " ";
    }
    std::cout << std::endl;
  */  
    property_map<Graph, vertex_index_t>::type
        index_map = get(vertex_index, G);
    
/*    std::cout << "  bandwidth: "
       << bandwidth(G, make_iterator_property_map(&l_perm[0], index_map, l_perm[0]))
       << std::endl;
    std::cout << "  profile: " 
        << profile(G, make_iterator_property_map(&l_perm[0], index_map, l_perm[0]))
        << std::endl;
    std::cout << "  max_wavefront: " 
        << max_wavefront(G, make_iterator_property_map(&l_perm[0], index_map, l_perm[0]))
        << std::endl;
    std::cout << "  aver_wavefront: " 
        << aver_wavefront(G, make_iterator_property_map(&l_perm[0], index_map, l_perm[0]))
        << std::endl;
    std::cout << "  rms_wavefront: " 
        << rms_wavefront(G, make_iterator_property_map(&l_perm[0], index_map, l_perm[0]))
        << std::endl;
    std::cout << "  inverse map bandwidth: "
        << bandwidth(G, make_iterator_property_map(&inv_perm[0], index_map, inv_perm[0]))
        << std::endl;
    std::cout << "  inverse map profile: " 
        << profile(G, make_iterator_property_map(&inv_perm[0], index_map, inv_perm[0]))
        << std::endl;
    std::cout << "  inverse map max_wavefront: " 
        << max_wavefront(G, make_iterator_property_map(&inv_perm[0], index_map, inv_perm[0]))
        << std::endl;
    std::cout << "  inverse map aver_wavefront: " 
        << aver_wavefront(G, make_iterator_property_map(&inv_perm[0], index_map, inv_perm[0]))
        << std::endl;
    std::cout << "  inverse map rms_wavefront: " 
        << rms_wavefront(G, make_iterator_property_map(&inv_perm[0], index_map, inv_perm[0]))
        << std::endl;*/
    perm.Resize(l_perm.size());
    inverseperm.Resize(inv_perm.size());
    for(i=0; i<l_perm.size(); i++)
    {
      perm[i] = l_perm[i];
      inverseperm[i] = inv_perm[i];
    }
#ifdef DEBUG
  std::cout << "TPZBoostGraph::Resequence finished \n";
#endif

}
/*

      std::cout << "l_perm[index_map[inv_perm[c]]] " <<
l_perm[index_map[inv_perm[c]]] << " index_map[inv_perm[c]] " <<
index_map[inv_perm[c]] <<
        " Inv_Perm[c] " << inv_perm[c] << "\n";


void TPZBoostGraph::Resequence(TPZVec<int> &permuta, TPZVec<int> &inverseperm)
{
  //Creating a graph and adding the edges from above into it
  Graph G;//(10);
  for (int i = 0; i < m_Edges.size(); ++i)
    add_edge(m_Edges[i].first, m_Edges[i].second, G);

  //Creating two iterators over the vertices
  graph_traits<Graph>::vertex_iterator ui, ui_end;

  //Creating a property_map with the degrees of the degrees of each vertex
  property_map<Graph,vertex_degree_t>::type deg = get(vertex_degree, G);
  for (boost::tie(ui, ui_end) = vertices(G); ui != ui_end; ++ui)
    deg[*ui] = degree(*ui, G);

  //Creating a property_map for the indices of a vertex
  property_map<Graph, vertex_index_t>::type index_map = get(vertex_index, G);

  std::cout << "original bandwidth: " << bandwidth(G) << std::endl;
  std::cout << "original profile: " << profile(G) << std::endl;
  std::cout << "original max_wavefront: " << max_wavefront(G) << std::endl;
  std::cout << "original aver_wavefront: " << aver_wavefront(G) << std::endl;
  std::cout << "original rms_wavefront: " << rms_wavefront(G) << std::endl;
  

  //Creating a vector of vertices  
  std::vector<Vertex> sloan_order(num_vertices(G));
  //Creating a vector of size_type  
  std::vector<size_type> perm(num_vertices(G));
  permuta.Resize(perm.size());
    {
      //sloan_ordering
      sloan_ordering(G, sloan_order.begin(), 
                        get(vertex_color, G),
                        make_degree_map(G), 
                        get(vertex_priority, G) );
      
      cout << endl << "Sloan ordering without a start-vertex:" << endl;
      cout << "  ";
      for (std::vector<Vertex>::const_iterator i=sloan_order.begin();
           i != sloan_order.end(); ++i)
    {
      cout << index_map[*i] << " ";
      permuta[*i]=index_map[*i];
    }
      cout << endl;
      
      for (size_type c = 0; c != sloan_order.size(); ++c)
        perm[index_map[sloan_order[c]]] = c;
      std::cout << "  bandwidth: " 
                << bandwidth(G, make_iterator_property_map(&perm[0], index_map, perm[0]))
                << std::endl;
      std::cout << "  profile: " 
                << profile(G, make_iterator_property_map(&perm[0], index_map, perm[0]))
                << std::endl;
      std::cout << "  max_wavefront: " 
                << max_wavefront(G, make_iterator_property_map(&perm[0], index_map, perm[0]))
                << std::endl;
      std::cout << "  aver_wavefront: " 
                << aver_wavefront(G, make_iterator_property_map(&perm[0], index_map, perm[0]))
                << std::endl;
      std::cout << "  rms_wavefront: " 
                << rms_wavefront(G, make_iterator_property_map(&perm[0], index_map, perm[0]))
                << std::endl;
    }

  {
    
    //Setting the start node
    Vertex s = vertex(0, G);
    int ecc;   //defining a variable for the pseudoperipheral radius
    
    //Calculating the pseudoeperipheral node and radius
    Vertex e = pseudo_peripheral_pair(G, s, ecc, get(vertex_color, G), get(vertex_degree, G) );

    cout << endl;
    cout << "Starting vertex: " << s << endl;
    cout << "Pseudoperipheral vertex: " << e << endl;
    cout << "Pseudoperipheral radius: " << ecc << endl << endl;



    //Sloan ordering
    sloan_ordering(G, s, e, sloan_order.begin(), get(vertex_color, G), 
                           get(vertex_degree, G), get(vertex_priority, G));
    
    cout << "Sloan ordering starting at: " << s << endl;
    cout << "  ";    
    
    for (std::vector<Vertex>::const_iterator i = sloan_order.begin();
         i != sloan_order.end(); ++i)
    {
      cout << index_map[*i] << " ";
      permuta[*i]=index_map[*i];
    }
    cout << endl;

    for (size_type c = 0; c != sloan_order.size(); ++c)
      perm[index_map[sloan_order[c]]] = c;
    std::cout << "  bandwidth: " 
              << bandwidth(G, make_iterator_property_map(&perm[0], index_map, perm[0]))
              << std::endl;
    std::cout << "  profile: " 
              << profile(G, make_iterator_property_map(&perm[0], index_map, perm[0]))
              << std::endl;
    std::cout << "  max_wavefront: " 
              << max_wavefront(G, make_iterator_property_map(&perm[0], index_map, perm[0]))
              << std::endl;
    std::cout << "  aver_wavefront: " 
              << aver_wavefront(G, make_iterator_property_map(&perm[0], index_map, perm[0]))
              << std::endl;
    std::cout << "  rms_wavefront: " 
              << rms_wavefront(G, make_iterator_property_map(&perm[0], index_map, perm[0]))
              << std::endl;
  }

}
*/
/*
{
  //perm.Resize(fNNodes+1);
  Graph G;
  for (int i = 0; i < m_Edges.size(); ++i)
    add_edge(m_Edges[i].first, m_Edges[i].second, G);

  graph_traits<Graph>::vertex_iterator ui, ui_end;

  property_map<Graph,vertex_degree_t>::type deg = get(vertex_degree, G);
  for (boost::tie(ui, ui_end) = vertices(G); ui != ui_end; ++ui)
    deg[*ui] = degree(*ui, G);

  property_map<Graph, vertex_index_t>::type
    index_map = get(vertex_index, G);

  std::cout << "original bandwidth: " << bandwidth(G) << std::endl;
  std::cout << "Number of Vertices " << num_vertices(G) << std::endl;
  std::cout << "Number of Edges " << num_edges(G) << std::endl;

  std::vector<Vertex> inv_perm(num_vertices(G));
  std::vector<size_type> l_perm(num_vertices(G));

  {
    //reverse cuthill_mckee_ordering
//    cuthill_mckee_ordering(G, inv_perm.rbegin(), get(vertex_color, G),
//      make_degree_map(G));
    cuthill_mckee_ordering(G, inv_perm.begin(), get(vertex_color, G),
      make_degree_map(G));  
    cout << "Reverse Cuthill-McKee ordering:" << endl;
    cout << "  ";
    perm.Resize(l_perm.size(),0);
    int orignode = 0;
    for (std::vector<Vertex>::const_iterator i=inv_perm.begin();i != inv_perm.end();++i)
    {
      cout << *i << ": "  << index_map[*i] << " ";
      perm[orignode] = index_map[*i];
      orignode++;
    }
    cout << endl;

    for (size_type c = 0; c != inv_perm.size(); ++c)
    {
      l_perm[index_map[inv_perm[c]]] = c;
      std::cout << "l_perm[index_map[inv_perm[c]]] " <<  
l_perm[index_map[inv_perm[c]]] << " index_map[inv_perm[c]] " << 
index_map[inv_perm[c]] <<
	" Inv_Perm[c] " << inv_perm[c] << "\n";
    }
    std::cout << "  bandwidth: " 
       << bandwidth(G, make_iterator_property_map(&l_perm[0], index_map, l_perm[0]))
       << std::endl;
  }



}
*/
void TPZBoostGraph::SetElementGraphOld(TPZVec<int> &elgraph, TPZVec<int> &elgraphindex)
{

  int i = 0;
  int j = 0;
  TPZVec<int> nodegraph(0),nodegraphindex(0);
  ConvertGraph(elgraph,elgraphindex,nodegraph,nodegraphindex);

  std::ofstream outfile("adjacencia.txt");
  //Criar pares e adicionar ao vetor m_Edges
  for(i=0;i<nodegraphindex.NElements()-1;i++)
  {
    for(j = nodegraphindex[i]; j< nodegraphindex[i+1];j++)
    {
      //std::cout << "Creating Entry: " << i << ":" << nodegraph[j] << std::endl;
      Pair node(i,nodegraph[j]);
      outfile << i << " " << nodegraph[j] << std::endl;
      m_Edges.push_back(node);
    }
  }
  outfile.close();
  
  for (int i = 0; i < (int)m_Edges.size(); ++i)
  {
    add_edge(m_Edges[i].first, m_Edges[i].second, m_Graph);
  }
  
  //boost::graph_traits<Graph>::vertex_iterator ui, ui_end;

  //Creating a property_map with the degrees of the degrees of each vertex
  //m_Degrees = get(vertex_degree, m_Graph);
  //for (boost::tie(ui, ui_end) = vertices(m_Graph); ui != ui_end; ++ui)
  //{
  //  m_Degrees[*ui] = degree(*ui, m_Graph);
  //}

  //Creating a property_map for the indices of a vertex
  //property_map<Graph, vertex_index_t>::type 
  //m_Index_map = get(vertex_index, m_Graph);
  //= get(vertex_index, G);
  std::cout << "Original bandwidth: " << bandwidth(m_Graph) << std::endl;
  std::cout << "Original profile: " << profile(m_Graph) << std::endl;
  std::cout << "Original max_wavefront: " << max_wavefront(m_Graph) << std::endl;
  std::cout << "Original aver_wavefront: " << aver_wavefront(m_Graph) << std::endl;
  std::cout << "Original rms_wavefront: " << rms_wavefront(m_Graph) << std::endl;
  
  
}

/*
  std::vector<Vertex> inv_perm(num_vertices(m_Graph));
  std::vector<size_type> l_perm(num_vertices(m_Graph));
  {
    Vertex s = vertex(0, m_Graph);
    //reverse cuthill_mckee_ordering
    cuthill_mckee_ordering(m_Graph, s, inv_perm.rbegin(), get(vertex_color, m_Graph),
                           get(vertex_degree, m_Graph));
    for (std::vector<Vertex>::const_iterator i=inv_perm.begin();
       i != inv_perm.end(); ++i)
      cout << m_Index_map[*i] << " ";
    cout << endl;

    for (size_type c = 0; c != inv_perm.size(); ++c)
      l_perm[m_Index_map[inv_perm[c]]] = c;
    std::cout << "  bandwidth: " 
       << bandwidth(m_Graph, make_iterator_property_map(&l_perm[0], m_Index_map, l_perm[0]))
       << std::endl;
  }
  int i;
  int nVertex = l_perm.size();
  perm.Resize(nVertex);
  inverseperm.Resize(nVertex);
  for(i = 0; i < nVertex; i++)
  {
    perm[i]=l_perm[i];
    inverseperm[i]=inv_perm[i];
  }

*/
#endif
