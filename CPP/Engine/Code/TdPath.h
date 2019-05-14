#pragma once
#include "TdArray.h"

namespace eng {

struct AStar;
struct AStarNode
{
	AStarNode* parent;
	int32 index;
	float G;
	float H;
};

typedef void (*AStarGetAdjacents)(AStar*, int32);
typedef void (*AStarCalcG)(AStar*, AStarNode*);
typedef void (*AStarCalcH)(AStar*, AStarNode*);

struct AStar
{
	int32 target;
	TdArray<AStarNode> open_list;
	TdArray<AStarNode> closed_list;
	TdArray<int32> temp_adjacents;
	AStarGetAdjacents get_adjacents;
	AStarCalcG calc_g;
	AStarCalcH calc_h;
};

void AStarSearch(AStar*, int32 start_index, int32 target_index);
void AStarInit(AStar*, int32 list_max, int32 max_adjacents, AStarGetAdjacents, AStarCalcG, AStarCalcH);

}