#include "TdPath.h"

namespace eng {

void AStarFinishSearch(AStar* a_star)
{
	assert(a_star);

	while (a_star->open_list.count > 0)
	{
		int32 lowest_f_index = -1;
		float lowest_f = FLT_MAX;
		for (int32 i = 0; i < a_star->open_list.count; ++i)
		{
			float cost = a_star->open_list[i].G + a_star->open_list[i].H;
			if (cost < lowest_f)
			{
				lowest_f = cost;
				lowest_f_index = i;
			}
		}
		AStarNode* node = tdArrayPush(a_star->closed_list);
		memcpy(node, a_star->open_list.ptr + lowest_f_index, sizeof(AStarNode));
		if (node->index == a_star->target)
			break; // path found

		tdArrayRemoveAt(a_star->open_list, lowest_f_index);
		tdArrayClear(a_star->temp_adjacents);
		a_star->get_adjacents(a_star, node->index);

		for (int32 i = 0; i < a_star->temp_adjacents.count; ++i)
		{
			int32 adjacent = a_star->temp_adjacents[i];
			int32 ci_found = -1;
			for (int32 j = 0; j < a_star->closed_list.count; ++j)
			{
				if (a_star->closed_list[j].index == adjacent)
				{
					ci_found = j;
					break;
				}
			}
			if (ci_found < 0)
			{
				int32 open_list_index = -1;
				for (int32 j = 0; j < a_star->open_list.count; ++j)
				{
					if (a_star->open_list[j].index == adjacent)
					{
						open_list_index = j;
						break;
					}
				}
				if (open_list_index >= 0)
				{
					AStarNode* adj_node = a_star->open_list.ptr + open_list_index;
					AStarNode n = {node, adjacent};
					a_star->calc_g(a_star, &n);
					if (n.G < adj_node->G)
					{
						adj_node->parent = node;
						a_star->calc_g(a_star, adj_node);
					}
				}
				else
				{
					AStarNode child = {node, adjacent};
					a_star->calc_g(a_star, &child);
					a_star->calc_h(a_star, &child);
					tdArrayAdd(a_star->open_list, child);
				}
			}
		}
	}
}

void AStarStartSearch(AStar* a_star, int32 start_index, int32 target_index)
{
	assert(a_star);
	tdArrayClear(a_star->open_list);
	tdArrayClear(a_star->closed_list);
	tdArrayClear(a_star->temp_adjacents);
	a_star->target = target_index;
	a_star->get_adjacents(a_star, start_index);
	AStarNode* parent_node = tdArrayPush(a_star->closed_list);
	parent_node->parent = nullptr;
	parent_node->index = start_index;
	for (int32 i = 0; i < a_star->temp_adjacents.count; ++i)
	{
		AStarNode node = {parent_node, a_star->temp_adjacents[i]};
		a_star->calc_g(a_star, &node);
		a_star->calc_h(a_star, &node);
		tdArrayAdd(a_star->open_list, node);
	}
}

void AStarSearch(AStar* a_star, int32 start_index, int32 target_index)
{
	TIMED_BLOCK(AStarSearch);
	AStarStartSearch(a_star, start_index, target_index);
	AStarFinishSearch(a_star);
}

void AStarInit(AStar* a_star, int32 list_max, int32 max_adjacents, AStarGetAdjacents get_adjacents, AStarCalcG calc_g, AStarCalcH calc_h)
{
	assert(a_star);
	assert(get_adjacents);
	tdArrayInit(a_star->open_list, list_max);
	tdArrayInit(a_star->closed_list, list_max);
	tdArrayInit(a_star->temp_adjacents, max_adjacents);
	a_star->get_adjacents = get_adjacents;
	a_star->calc_g = calc_g;
	a_star->calc_h = calc_h;
}
	
}