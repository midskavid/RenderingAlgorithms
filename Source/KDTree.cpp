#include "KDTree.h"
#include "Shape.h"

KDTreeNode* KDTree::Build(std::vector<Shape*>& shapes, Bounds3f& boundsRoot, int depth) {
    auto root = new KDTreeNode();
    root->mPrimitives = shapes;
    root->mBounds = boundsRoot;

    if (shapes.size()<10) {
        // Do not recurse..
        return root;
    }

    // Get centroid of all primitives..
    Point3f centroidAll(0,0,0);
    for (auto& sh:shapes) {
        centroidAll += sh->GetCentroid();
    }
    centroidAll /= shapes.size();
    //TODO : [mkaviday] remove safety check..
    if (!Inside(centroidAll, boundsRoot)) std::cout<<"WTF!\n";
    int splitAxis = boundsRoot.MaximumExtent();

    Bounds3f leftBounds, rightBounds;
    switch (splitAxis)
    {
    case 0:
        leftBounds = Bounds3f(boundsRoot.pMin,Point3f(centroidAll[0], boundsRoot.pMax[1], boundsRoot.pMax[2]));
        rightBounds = Bounds3f(boundsRoot.pMax,Point3f(centroidAll[0], boundsRoot.pMin[1], boundsRoot.pMin[2]));        
        break;
    case 1:
        leftBounds = Bounds3f(boundsRoot.pMin,Point3f(boundsRoot.pMax[0], centroidAll[1], boundsRoot.pMax[2]));
        rightBounds = Bounds3f(boundsRoot.pMax,Point3f(boundsRoot.pMin[0], centroidAll[1], boundsRoot.pMin[2]));        
        break;
    case 2:
        leftBounds = Bounds3f(boundsRoot.pMin,Point3f(boundsRoot.pMax[0], boundsRoot.pMax[1], centroidAll[2]));
        rightBounds = Bounds3f(boundsRoot.pMax,Point3f(boundsRoot.pMin[0], boundsRoot.pMin[1], centroidAll[2]));        
        break;    
    }

    std::vector<Shape*> leftNodes;
    std::vector<Shape*> rightNodes;
    int common = 0;
    bool leftpushed;
    for (auto& sh:shapes) {
        leftpushed = false;
        if (Overlaps(leftBounds, sh->GetWorldBounds())) {
            leftNodes.emplace_back(sh);
            leftpushed = true;
        }
        if (Overlaps(rightBounds, sh->GetWorldBounds())) {
            rightNodes.emplace_back(sh);
            if (leftpushed) ++common;
        }
    }

    if (common/shapes.size()>.7) {
        // Dont recurse..
        return root;
    }
    root->left = Build(leftNodes, leftBounds, depth+1);
    root->right = Build(rightNodes, rightBounds, depth+1);
    return root;
}


bool KDTree::Intersect(Ray& ray, KDTreeNode* root, Float* tmin, Interaction* intr){
    if (!root) return false;
    Float t;
    if (root->mBounds.IntersectP(ray,&t)) {
        if (t>*tmin) return true; // Short-circuit
        if (!root->left) {
            // base case..
            bool hit = false;
            for (auto& shape:root->mPrimitives) {
                auto its = shape->Intersect(ray);
                if (its.IsHit()) {    
                    Float dist = Distance(ray.o,its.GetPosition());
                    if (dist < *tmin) {
                        *tmin = dist;
                        *intr = its;
                    }
                    hit = true;
                }
            }
            return hit;
        }
        else {
            return Intersect(ray, root->left, tmin, intr) || Intersect(ray, root->right, tmin, intr);
        }
    }
    return false;
}
