#ifndef _ANIVISION_DEPGRAPH_H_
#define _ANIVISION_DEPGRAPH_H_ 1

#include <hlib/linkedlist.h>


class DepGraphNode;


class DepGraphNodeList
{
	static const int ents_per_chunk=32;
	public:
		struct Chunk : LinkedListBase<Chunk>
		{
			// Entries are filled first-to-last. 
			// ALWAYS, the first nents entries are 
			// present, the other are NULL. 
			DepGraphNode *ent[ents_per_chunk];
			int nents;
		};
		// List of chunks: 
		LinkedList<Chunk> chunks;
		// Total number of entries in all chunks. 
		int nnodes;
		// Number of chunks: 
		int nchunks;
		
		// Merge the passed two chunks if possible. 
		inline void _TryMerge(Chunk *c,Chunk *n);
	private:
	public:
		DepGraphNodeList();
		~DepGraphNodeList();
		
		// Return total number of nodes in list. 
		int NNodes() const
			{  return(nnodes);  }
		
		// Add a node to the list. 
		// May only be called if it is certain, that the node 
		// is not yet in the list. 
		void AddNode(DepGraphNode *dgn);
		
		// Remove a node; returns 1 if succesful and 0 it not. 
		int RemoveNode(DepGraphNode *dgn);
		
		// Check if the passed node is in the list. 
		// 1 -> yes; 0 -> no. 
		int IsNodeInList(DepGraphNode *dgn);
};

class DepGraphNode
{
	private:
		DepGraphNodeList depends;
		DepGraphNodeList provides;
		DepGraphNodeList solved;
	public: _CPP_OPERATORS
		DepGraphNode() : depends(),provides(),solved() {}
		~DepGraphNode() {}
		
		// Store the information that *this node depends on *dn. 
		// Alternatively: That *dn provides for *this. 
		// Return value: 
		//  0 -> OK
		//  1 -> was already in list
		// -2 -> the node was not added because it is in 
		//       our provide list, i.e. dn already depends on 
		//       *this. 
		int DependsOn(DepGraphNode *dgn);
		
		// Tell the graph that the dependency was solved. 
		// I.e. *this depends on dgn and dgn is solved. 
		// Hence, put dgn from depends into solved list. 
		// Return value: 
		//  >=0 -> number of left entries in the depends list 
		//   -2 -> dgn was not in depends list. 
		int DepSolved(DepGraphNode *dgn);
};

#endif  /* _ANIVISION_DEPGRAPH_H_ */
