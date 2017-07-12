#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_

#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include <stdint.h>
/* topology structures definition*/
struct ng_topology{
	int id_lenght;
	char *protocol;
	char *self_id;
	struct ng_node *first;

};

struct ng_metrics{
	uint16_t metric;
	uint32_t network;
	uint32_t netmask;
	struct ng_metrics *next;
};

struct ng_node{
	char *id;
	struct ng_metrics *metric_list;
	struct ng_neighbor *neighbor_list;
	struct ng_node *next;
};


struct ng_neighbor{
	struct ng_node *id;
	float weight;
	struct ng_neighbor *next;
};

const char * compose_netjson(struct ng_topology * c_topo, uint32_t router_id);

int add_node(struct ng_topology * topo, const char *id);
struct ng_topology * _init_topo(int type);
int add_edge(struct ng_topology *topo, const char *source, const char *id, uint32_t network);
void destroy_topo(struct ng_topology *topo);
struct ng_node* find_node(struct ng_topology *topo, const char *id);
char * uint_to_string(uint32_t address);
void destroy_topo(struct ng_topology *topo);

#endif /* SRC_PARSER_H_ */
