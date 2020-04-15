#ifndef KDTREE_H
#define KDTREE_H

#include "CoreTypes.h"
#include "Transform.h"
#include "Interaction.h"


class KDTree {
    struct KDTreeNode {
    KDTreeNode* left;
    KDTreeNode* right;
    std::vector<Shape*> mPrimitives;
    Bounds3f mBounds;
    KDTreeNode() : left(nullptr), right(nullptr), mPrimitives(std::vector<Shape*>()) { }
};
public:
    KDTree() { }
    void Build(std::vector<Shape*>& shapes, Bounds3f boundsRoot) {
        boundsRoot = Expand(boundsRoot,.01);
        mRoot = Build(shapes, boundsRoot, 1);
    }
    bool Intersect(Ray& ray, Float* tmin, Interaction* intr) {
        return Intersect(ray, mRoot, tmin, intr);
    }
private:
    KDTreeNode* Build(std::vector<Shape*>& shapes, Bounds3f& boundsRoot, int depth);
    bool Intersect(Ray& ray, KDTreeNode* root, Float* tmin, Interaction* intr);
private:
    KDTreeNode* mRoot;
};

#endif