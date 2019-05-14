#include "TdQuadTree.h"

namespace eng {

void tdQuadTreeInit(TdQuadTree* qt, TdQuadTree* parent, TdPoint2 min, TdPoint2 max)
{
	assert(qt);
	assert(max.x > min.x);
	assert(max.y > min.y);
	assert(max.x - min.x == max.y - min.y);
	assert((max.x - min.x) % 2 == 0);
	assert((max.y - min.y) % 2 == 0);
	qt->parent = parent;
	qt->min = min;
	qt->max = max;
}

void tdQuadTreeDivide(TdMemoryArena* arena, TdQuadTree* qt, int32 until_size)
{
	assert(qt);
	assert(arena);

	TdQuadTree* nw = tdMalloc<TdQuadTree>(arena);
	TdQuadTree* ne = tdMalloc<TdQuadTree>(arena);
	TdQuadTree* sw = tdMalloc<TdQuadTree>(arena);
	TdQuadTree* se = tdMalloc<TdQuadTree>(arena);

	TdPoint2 min = qt->min;
	TdPoint2 max = qt->max;

	int32 w = (max.x - min.x) >> 1;
	max.x = min.x + w;
	max.y = min.y + w;
	tdQuadTreeInit(nw, qt, min, max);
	min.x += w;
	max.x += w;
	tdQuadTreeInit(ne, qt, min, max);
	min.x -= w;
	max.x -= w;
	min.y += w;
	max.y += w;
	tdQuadTreeInit(sw, qt, min, max);
	min.x += w;
	max.x += w;
	tdQuadTreeInit(se, qt, min, max);

	qt->nw = nw;
	qt->ne = ne;
	qt->sw = sw;
	qt->se = se;

	if (until_size > 0 && w > until_size)
	{
		tdQuadTreeDivide(arena, nw, until_size);
		tdQuadTreeDivide(arena, ne, until_size);
		tdQuadTreeDivide(arena, sw, until_size);
		tdQuadTreeDivide(arena, se, until_size);
	}
}

TdQuadTree* tdQuadTreeGetLeaf(TdQuadTree* qt, TdPoint2 p)
{
	assert(qt);
	if (p.x >= qt->min.x && p.x < qt->max.x && p.y >= qt->min.y && p.y < qt->max.y)
	{
		if (!qt->nw) return qt;
		TdQuadTree* leaf = tdQuadTreeGetLeaf(qt->nw, p);
		if (leaf) return leaf;
		leaf = tdQuadTreeGetLeaf(qt->ne, p);
		if (leaf) return leaf;
		leaf = tdQuadTreeGetLeaf(qt->sw, p);
		if (leaf) return leaf;
		leaf = tdQuadTreeGetLeaf(qt->se, p);
		return leaf;
	}
	return nullptr;
}

TdQuadTree* tdQuadTreeGetRoot(TdQuadTree* qt)
{
	assert(qt);
	TdQuadTree* root = qt;
	TdQuadTree* parent = qt->parent;
	while (parent)
	{
		root = parent;
		parent = parent->parent;
	}
	return root;
}

TdQuadTree* tdQuadTreeGetNeighbourUp(TdQuadTree* leaf, TdQuadTree* root)
{
	TdQuadTree *parent = leaf->parent;
	if (leaf == parent->sw) return parent->nw;
	else if (leaf == parent->se) return parent->ne;
	return tdQuadTreeGetLeaf(root, TdPoint2(leaf->min.x, leaf->min.y - 1));
}

TdQuadTree* tdQuadTreeGetNeighbourRight(TdQuadTree* leaf, TdQuadTree* root)
{
	TdQuadTree *parent = leaf->parent;
	if (leaf == parent->nw) return parent->ne;
	else if (leaf == parent->sw) return parent->se;
	return tdQuadTreeGetLeaf(root, TdPoint2(leaf->max.x + 1, leaf->min.y));
}

TdQuadTree* tdQuadTreeGetNeighbourDown(TdQuadTree* leaf, TdQuadTree* root)
{
	TdQuadTree *parent = leaf->parent;
	if (leaf == parent->nw) return parent->sw;
	else if (leaf == parent->ne) return parent->se;
	return tdQuadTreeGetLeaf(root, TdPoint2(leaf->min.x, leaf->max.y + 1));
}

TdQuadTree* tdQuadTreeGetNeighbourLeft(TdQuadTree* leaf, TdQuadTree* root)
{
	TdQuadTree *parent = leaf->parent;
	if (leaf == parent->ne) return parent->nw;
	else if (leaf == parent->se) return parent->sw;
	return tdQuadTreeGetLeaf(root, TdPoint2(leaf->min.x - 1, leaf->min.y));
}

TdQuadTree* tdQuadTreeGetNextLeafCore(TdQuadTreeSearch* search)
{
	// dir
	// 0 - up
	// 1 - right
	// 2 - down
	// 3 - left
	TdQuadTree *leaf = search->leaf;

	if (search->dir == 0)
	{
		leaf = tdQuadTreeGetNeighbourUp(leaf, search->root);
		if (!leaf || --search->curr_size == 0)
		{
			search->dir = 1;
			search->last_right_size = search->curr_size = search->dir_size;
		}
		return search->leaf = leaf;
	}
	else if (search->dir == 1)
	{
		leaf = tdQuadTreeGetNeighbourRight(leaf, search->root);
		if (!leaf || --search->curr_size == 0)
		{
			search->dir = 2;
			++search->dir_size;
			search->last_down_size = search->curr_size = search->dir_size;
		}
		return search->leaf = leaf;
	}
	else if (search->dir == 2)
	{
		leaf = tdQuadTreeGetNeighbourDown(leaf, search->root);
		if (!leaf || --search->curr_size == 0)
		{
			search->dir = 3;
			search->last_left_size = search->curr_size = search->dir_size;
		}
		return search->leaf = leaf;
	}
	else
	{
		leaf = tdQuadTreeGetNeighbourLeft(leaf, search->root);
		if (!leaf || --search->curr_size == 0)
		{
			search->dir = 0;
			++search->dir_size;
			search->last_up_size = search->curr_size = search->dir_size;
		}
		return search->leaf = leaf;
	}
}

TdQuadTree* tdQuadTreeGetNextLeaf(TdQuadTreeSearch* search)
{
	TdPoint2 p = search->leaf->min;
	TdQuadTree *result = tdQuadTreeGetNextLeafCore(search);
	int32 out_of_bounds_count = 4;

	while (!result && out_of_bounds_count--)
	{
		if (search->dir == 0)
		{
			p.y -= search->leaf_size * search->curr_size;
			result = tdQuadTreeGetLeaf(search->root, p);
			if (!result) p.y = 0;
			search->dir = 1;
			++search->last_right_size;
			search->curr_size = search->last_right_size;
		}
		else if (search->dir == 1)
		{
			p.x += search->leaf_size * search->curr_size;
			result = tdQuadTreeGetLeaf(search->root, p);
			if (!result) p.x = search->root->max.x - search->leaf_size;
			search->dir = 2;
			++search->dir_size;
			++search->last_down_size;
			search->curr_size = search->last_down_size;
		}
		else if (search->dir == 2)
		{
			p.y += search->leaf_size * search->curr_size;
			result = tdQuadTreeGetLeaf(search->root, p);
			if (!result) p.y = search->root->max.y - search->leaf_size;
			search->dir = 3;
			++search->last_left_size;
			search->curr_size = search->last_left_size;
		}
		else
		{
			p.x -= search->leaf_size * search->curr_size;
			result = tdQuadTreeGetLeaf(search->root, p);
			if (!result) p.x = 0;
			search->dir = 0;
			++search->dir_size;
			++search->last_up_size;
			search->curr_size = search->last_up_size;
		}
		search->leaf = result;
	}
	return result;
}

TdQuadTree* tdQuadTreeInitSearch(TdQuadTreeSearch* search, TdQuadTree* root, Vector2 center, int32 max_distance)
{
	assert(search);
	assert(root);

	search->root = root;
	search->center = center;
	search->max_distance = max_distance;
	search->dir = 0;
	search->dir_size = 1;
	search->curr_size = 1;
	search->last_up_size = 1;
	search->last_right_size = 0;
	search->last_down_size = 0;
	search->last_left_size = 0;
	search->leaf_size = 0;
	search->leaf = tdQuadTreeGetLeaf(root, center);
	if (search->leaf) search->leaf_size = search->leaf->max.x - search->leaf->min.x;
	return search->leaf;
}

void tdQuadTreeGetLeafArray(TdQuadTree *root, TdQuadTree** array)
{
	assert(root);
	assert(array);

	TdQuadTree *leaf = tdQuadTreeGetLeaf(root, root->min);
	if (!leaf) return;
	
	int32 x_size = leaf->max.x - leaf->min.x;
	int32 y_size = leaf->max.y - leaf->min.y;
	TdPoint2 p;

	for (int32 y = root->min.y; y < root->max.y; y += y_size)
	{
		p.y = y;
		for (int32 x = root->min.x; x < root->max.x; x += x_size)
		{
			p.x = x;
			leaf = tdQuadTreeGetLeaf(root, p);
			*array++ = leaf;
		}
	}
}

int32 tdQuadTreeGetLeaves(TdQuadTree** leaves, int32 leaves_per_row, TdQuadTree** result, TdPoint2 top_left, TdPoint2 bottom_right)
{
	assert(leaves);
	assert(result);

	int minx = leaves[0]->min.x;
	int miny = leaves[0]->min.y;
	int maxx = leaves[leaves_per_row - 1]->max.x;
	int maxy = leaves[leaves_per_row * leaves_per_row - 1]->max.x;
	if (bottom_right.x < minx || bottom_right.y < miny || top_left.x > maxx || top_left.y > maxy) return 0;

	if (top_left.x < minx) top_left.x = minx;
	if (top_left.y < miny) top_left.y = miny;
	if (bottom_right.x > maxx) bottom_right.x = maxx;
	if (bottom_right.y > maxy) bottom_right.y = maxy;

	int32 leaf_size = leaves[0]->max.x - minx;
	int32 start_y = (top_left.y - miny) / leaf_size;
	int32 start_x = (top_left.x - minx) / leaf_size;
	int32 end_y = (bottom_right.y - miny) / leaf_size + 1;
	int32 end_x = (bottom_right.x - minx) / leaf_size + 1;
	int32 count = 0;

	for (int32 y = start_y; y < end_y; ++y)
	{
		int iy = y * leaves_per_row;
		for (int32 x = start_x; x < end_x; ++x)
		{
			result[count++] = leaves[x + iy];
		}
	}

	return count;
}

}