#pragma once
#include "TdBase.h"
#include "TdPoint.h"

namespace eng {

struct TdQuadTree
{
	void* data;
	TdQuadTree* nw;
	TdQuadTree* ne;
	TdQuadTree* sw;
	TdQuadTree* se;
	TdQuadTree* parent;
	TdPoint2 min;
	TdPoint2 max;
};

struct TdQuadTreeSearch
{
	TdQuadTree *leaf;
	int32 dir;
	int32 curr_size;
	int32 dir_size;
	int32 last_up_size;
	int32 last_right_size;
	int32 last_down_size;
	int32 last_left_size;
	int32 max_distance;
	int32 leaf_size;
	TdQuadTree *root;
	Vector2 center;
};

void tdQuadTreeInit(TdQuadTree* qt, TdQuadTree* parent, TdPoint2 min, TdPoint2 max);
void tdQuadTreeDivide(TdMemoryArena* arena, TdQuadTree* qt, int32 until_size = 0);
TdQuadTree* tdQuadTreeGetRoot(TdQuadTree* qt);
TdQuadTree* tdQuadTreeGetLeaf(TdQuadTree* qt, TdPoint2 p);
ALWAYS_INLINE TdQuadTree* tdQuadTreeGetLeaf(TdQuadTree* qt, Vector2 p) { return tdQuadTreeGetLeaf(qt, TdPoint2((int32)p.x, (int32)p.y)); }
TdQuadTree *tdQuadTreeGetNeighbourUp(TdQuadTree *leaf, TdQuadTree *root);
TdQuadTree *tdQuadTreeGetNeighbourRight(TdQuadTree *leaf, TdQuadTree *root);
TdQuadTree *tdQuadTreeGetNeighbourDown(TdQuadTree *leaf, TdQuadTree *root);
TdQuadTree *tdQuadTreeGetNeighbourLeft(TdQuadTree *leaf, TdQuadTree *root);
TdQuadTree *tdQuadTreeGetNextLeaf(TdQuadTreeSearch *search);
TdQuadTree *tdQuadTreeInitSearch(TdQuadTreeSearch *search, TdQuadTree *root, Vector2 center, int32 max_distance);
void tdQuadTreeGetLeafArray(TdQuadTree *root, TdQuadTree** array);
int32 tdQuadTreeGetLeaves(TdQuadTree** leaves, int32 leaves_per_row, TdQuadTree** result, TdPoint2 top_left, TdPoint2 bottom_right);

}