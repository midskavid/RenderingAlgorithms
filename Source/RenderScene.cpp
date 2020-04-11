#include "RenderScene.h"
#include "Transform.h"
#include "RGBColor.h"
#include "LightSource.h"
#include "Camera.h"
#include "Film.h"
#include "Shape.h"
#include "Triangle.h"
#include "Sphere.h"

void RightMultiply(std::stack<Transform>& stk, Transform t2) {
    auto& tp = stk.top();
    tp = tp*t2;
}

void RenderScene::Init() { 
    
}

bool RenderScene::ReadVals(std::stringstream &s, const int numvals, Float* values)
{
    for (int i = 0; i < numvals; i++) {
        s >> values[i]; 
        if (s.fail()) {
            std::cout << "Failed reading value " << i << " will skip\n"; 
            return false;
        }
    }
    return true; 
}

void RenderScene::ReadFile() {
    std::string str, cmd; 
    std::ifstream in;
    in.open(mSceneFileName); 
    if (in.is_open()) {
        std::stack <Transform> transfstack; 
        transfstack.push(Transform());  // identity
        
        std::vector<LightSource*> lights;
        std::vector<Shape*> shapes;
        std::array<Float, 3> attenuation {1,0,0};
        std::vector<Point3f> vertices;
        RGBColor ambient;
        RGBColor diffuse;
        RGBColor specular;
        RGBColor emission;
        Float shininess;

        getline (in, str); 
        while (in) {
            if ((str.find_first_not_of(" \t\r\n") != std::string::npos) && (str[0] != '#')) {
                std::stringstream s(str);
                s >> cmd;
                bool validInput = false;
                Float values[10];
                if (cmd == "maxdepth") {
                    validInput = ReadVals(s, 1, values);
                    if (validInput)
                        mMaxDepth = int(values[0]);
                }
                else if (cmd == "output") {
                    s >> mOutFileName;
                }
                else if (cmd == "point") {
                    validInput = ReadVals(s, 6, values); // Position/color for lts.
                    if (validInput) {
                        Point3f pos {values[0], values[1], values[2]};
                        pos = transfstack.top()(pos);
                        RGBColor col {values[3], values[4], values[5]};
                        PointLightSource* light = new PointLightSource(col, pos, attenuation);
                        lights.emplace_back(light);
                    }
                }
                else if (cmd == "directional") {
                    // TODO : [mkaviday] Complete this
                }
                else if (cmd == "ambient") {
                    validInput = ReadVals(s, 3, values); // colors 
                    if (validInput) {
                        ambient = RGBColor(values[0], values[1], values[2]); 
                    }
                } else if (cmd == "diffuse") {
                    validInput = ReadVals(s, 3, values); // colors 
                    if (validInput) {
                        diffuse = RGBColor(values[0], values[1], values[2]); 
                    }
                } else if (cmd == "specular") {
                    validInput = ReadVals(s, 3, values); // colors 
                    if (validInput) {
                        specular = RGBColor(values[0], values[1], values[2]); 
                    }
                } else if (cmd == "emission") {
                    validInput = ReadVals(s, 3, values); // colors 
                    if (validInput) {
                        emission = RGBColor(values[0], values[1], values[2]); 
                    }
                } else if (cmd == "shininess") {
                    validInput = ReadVals(s, 1, values); // float 
                    if (validInput) {
                        shininess = values[0];
                    }
                } else if (cmd == "size") {
                    validInput = ReadVals(s,2,values); 
                    if (validInput) { 
                        mWidth = (int) values[0]; 
                        mHeight = (int) values[1]; 
                    } 
                } else if (cmd == "camera") {
                    validInput = ReadVals(s,10,values); 
                    if (validInput) {
                        Vector3f camPos {values[0], values[1], values[2]};
                        Vector3f camLookAt {values[3], values[4], values[5]};
                        Vector3f camUp {values[6], values[7], values[8]};
                        Float fovy = values[9];
                        mCamera = new Camera(camPos, camLookAt, camUp, fovy, mHeight, mWidth);
                        mFilm = new Film(mHeight, mWidth);
                    }
                }
                else if (cmd == "maxverts") {
                    validInput = ReadVals(s,1,values); 
                    if (validInput) {
                        vertices.clear();
                    }
                }
                else if (cmd == "vertex") {
                    validInput = ReadVals(s,3,values); 
                    if (validInput) {
                        vertices.emplace_back(Point3f(values[0], values[1], values[2]));
                    }
                }
                else if (cmd == "tri") {
                    validInput = ReadVals(s,3,values); 
                    if (validInput) {
                        Point3f vert1 = transfstack.top()(vertices[values[0]]);
                        Point3f vert2 = transfstack.top()(vertices[values[1]]);
                        Point3f vert3 = transfstack.top()(vertices[values[2]]);
                        Shape* shp = new Triangle(vert1, vert2, vert3, transfstack.top(), diffuse, ambient, specular, emission, shininess);
                        shapes.emplace_back(shp);
                    }
                }
                else if (cmd == "sphere") {
                    validInput = ReadVals(s,4,values); 
                    if (validInput) {
                        Point3f cen{values[0], values[1], values[2]};
                        Float rad = values[3];
                        Shape* shp = new Sphere(cen, rad, transfstack.top(), diffuse, ambient, specular, emission, shininess);
                        shapes.emplace_back(shp);
                    }
                }
                else if (cmd == "translate") {
                    validInput = ReadVals(s,3,values); 
                    if (validInput) {
                        RightMultiply(transfstack, Translate(Vector3f (values[0], values[1], values[2])));
                    }
                }
                else if (cmd == "scale") {
                    validInput = ReadVals(s,3,values); 
                    if (validInput) {
                        RightMultiply(transfstack, Scale(values[0], values[1], values[2]));
                    }
                }
                else if (cmd == "rotate") {
                    validInput = ReadVals(s,3,values); 
                    if (validInput) {
                        RightMultiply(transfstack, Rotate(values[3], Vector3f(values[0], values[1], values[2])));
                    }
                }
                else if (cmd == "pushTransform") {
                    transfstack.push(transfstack.top()); 
                } 
                else if (cmd == "popTransform") {
                    if (transfstack.size() <= 1) {
                        std::cerr << "Stack has no elements.  Cannot Pop\n"; 
                    } 
                    else {
                        transfstack.pop(); 
                    }
                }
                else {
                    std::cerr << "Unknown Command: " << cmd << " Skipping \n"; 
                }
            }
            std::getline (in, str); 
        }
    }
    else {
        std::cerr << "Unable to Open Input Data File " << mSceneFileName << "\n"; 
        throw 2; 
    }
}

void RenderScene::Render(std::string outFileName) {
    for (int ii=0;ii<mHeight;++ii) {
        for (int jj=0;jj<mWidth;++jj) {
            auto pt = Point2i(ii,jj);
            auto ray = mCamera->GenerateRay(pt);
            auto col = mScene->GetColor(ray, mMaxDepth);
            mFilm->AddColor(pt, col);
        }
    }
}

int main() {

    return 0;
}