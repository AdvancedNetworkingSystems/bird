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
	topo->first->metric_list=0;
	return 1;
}

int add_metric(struct ng_node * node, const uint32_t network, const uint32_t netmask, uint16_t metric){
	struct ng_metrics *temp = node->metric_list;
	node->metric_list = (struct ng_metrics*)malloc(sizeof(struct ng_metrics));
	node->metric_list->network = network;
	node->metric_list->netmask = netmask;
	node->metric_list->metric = metric;
	node->metric_list->next =temp;
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
int add_edge(struct ng_topology *topo, const char *source, const char *id, uint32_t network)
{
	struct ng_neighbor *temp;
	struct ng_node* n;
	if((n=find_node(topo, source))==0)
		return 0;
	temp=n->neighbor_list;
	n->neighbor_list=(struct ng_neighbor*)malloc(sizeof(struct ng_neighbor));
	if((n->neighbor_list->id=find_node(topo, id))==0)
		return 0;
	int weight = find_weight(n,network);
	if(weight==0){
		return 0;
	}
	n->neighbor_list->weight=weight;
	n->neighbor_list->next=temp;
	return 1;
}

int find_weight(struct ng_node *n, const uint32_t network){
	struct ng_metrics *metric;
	for(metric=n->metric_list; metric!=0; metric = metric->next)
		if(metric->network==network)
			return metric->metric;
	return -1;
}

int add_complete_graph_edges(struct ng_topology *topo, unsigned int routers[], int n_nodes,  uint32_t network){
	//count number of nodes in the subnet
	int i, j;
	//build the complete graph
	for(i=0; i<n_nodes; i++){
		for(j=0; j<n_nodes; j++){
			if(i!=j)
				add_edge(topo, uint_to_string(routers[i]), uint_to_string(routers[j]), network);
		}
	}
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
	cJSON_AddStringToObject(topo, "type", "NetworkGraph");
	cJSON_AddStringToObject(topo, "protocol", "OSPFv2");
	cJSON_AddStringToObject(topo, "version", "0.1");
	cJSON_AddStringToObject(topo, "metric", "none");
	cJSON_AddItemToObject(topo, "nodes", nodes = cJSON_CreateArray());
	//compose the list of nodes
	for(punt=c_topo->first; punt!=0; punt=punt->next){
		cJSON_AddItemToArray(nodes, node = cJSON_CreateObject());
		cJSON_AddStringToObject(node, "id", punt->id);
	}
	cJSON *edges;
	cJSON *edge;
	cJSON_AddItemToObject(topo, "links", edges = cJSON_CreateArray());
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
