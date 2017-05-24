#include "graph.h"
/**
* Add a node to the topology data structure
* @param struct topology*  pointer to the topology data structure
* @param const char* string containing the id of the new node
* @return 1 on success, 0 otherwise
*/
int add_node(struct ng_topology * topo, const char *id)
{
	struct ng_node *temp = topo->first;
	topo->first=(struct ng_node*)malloc(sizeof(struct ng_node));
	topo->first->next=temp;
	topo->first->id=strdup(id);
	topo->first->neighbor_list=0;
	return 1;
}

/**
* Find a node in the topology data structure
* @param struct topology*  pointer to the topology data structure
* @param const char* string containing the id of the searched node
* @return pointer to the node on success, 0 otherwise
*/
struct ng_node* find_node(struct ng_topology *topo,const char *id)
{
	struct ng_node *punt;
	for(punt=topo->first; punt!=0; punt=punt->next){
		if(strcmp(punt->id, id)==0){
			return punt;
		}
	}
	return 0;
}

/**
* Add a neighbor to the node
* @param struct topology*  pointer to the topology data structure
* @param const char* string containing the id of the source node
* @param const char* string containing the id of the target node
* @param const double  cost of the edge
* @return 1 on success, 0 otherwise
*/
int add_edge(struct ng_topology *topo, const char *source, const char *id, const double weight)
{
	struct ng_neighbor *temp;
	struct ng_node* n;
	if((n=find_node(topo, source))==0)
		return 0;
	temp=n->neighbor_list;
	n->neighbor_list=(struct ng_neighbor*)malloc(sizeof(struct ng_neighbor));
	if((n->neighbor_list->id=find_node(topo, id))==0)
		return 0;
	n->neighbor_list->weight=weight;
	n->neighbor_list->next=temp;
	return 1;

}

/**
* Initialize the topology data structure
* @param int number of chars of the id (0 ipv6, 1 ipv4)
* @return pointer to the topology
*/
struct ng_topology * _init_topo(int type)
{
	struct ng_topology *topo = (struct ng_topology*)malloc(sizeof(struct ng_topology));
	if(type==0){
		topo->id_lenght=39;
	}else if(type ==1){
		topo->id_lenght=15;
	}
	topo->first=0;
	return topo;
}


/**
* Destroy topology and dealloc
* @param struct topology * pointer to the structure
**/
void destroy_topo(struct ng_topology *topo)
{
	struct ng_node *n_temp, *punt=topo->first;
	while(punt){
		struct ng_neighbor *n=punt->neighbor_list;
		while(n){
			struct ng_neighbor *temp=n->next;
			free(n);
			n=temp;
		}
		free(punt->id);
		n_temp=punt->next;
		free(punt);
		punt=n_temp;
	}
	free(topo->protocol);
	free(topo->self_id);
	free(topo);
}

/*
*Compose a NetJSON object and return its string representation1
*/

const char * compose_netjson(struct ng_topology * c_topo){
	struct ng_node *punt;
	cJSON *topo = cJSON_CreateObject();
	cJSON *nodes;
	cJSON *node;
	cJSON_AddItemToObject(topo, "nodes", nodes = cJSON_CreateArray());
	//compose the list of nodes
	for(punt=c_topo->first; punt!=0; punt=punt->next){
		cJSON_AddItemToArray(nodes, node = cJSON_CreateObject());
		cJSON_AddStringToObject(node, "id", punt->id);
	}
	cJSON *edges;
	cJSON *edge;
	cJSON_AddItemToObject(topo, "edges", edges = cJSON_CreateArray());
	//compose the list of edges
	for(punt=c_topo->first; punt!=0; punt=punt->next){
		struct ng_neighbor* neigh;
		for(neigh=punt->neighbor_list; neigh!=0; neigh=neigh->next){
			const char* source = punt->id;
			const char* target = neigh->id->id;
			double cost = neigh->weight;
			cJSON_AddItemToArray(edges, edge = cJSON_CreateObject());
			cJSON_AddStringToObject(edge, "source", source);
			cJSON_AddStringToObject(edge, "target", target);
			cJSON_AddNumberToObject(edge, "cost", cost);
    }
	}
	return cJSON_PrintUnformatted(topo);
}

char * uint_to_string(uint32_t address){
	char *id = malloc(sizeof(char)*16);
	sprintf(id, "%d.%d.%d.%d",
	((address >> 24) & 0xff),
	((address >> 16) & 0xff),
	((address >> 8) & 0xff),
	(address & 0xff));
	return id;
}
