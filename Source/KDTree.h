#ifndef KDTREE_H
#define KDTREE_H

#include "CoreTypes.h"
#include "Transform.h"
#include "Interaction.h"

struct KDTreeNode {
    KDTreeNode* left;
    KDTreeNode* right;
    std::vector<Shape*> mPrimitives;
    Bounds3f mBounds;
    KDTreeNode() : left(nullptr), right(nullptr), mPrimitives(std::vector<Shape*>()) { }
};


class KDTree {
public:
    KDTree() { }
    KDTreeNode* Build(std::vector<Shape*>& shapes, Bounds3f& boundsRoot, int depth);
    bool Intersect(Ray& ray, KDTreeNode* root, Float* tmin, Interaction* intr);
};

#endif