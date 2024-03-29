#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Constants.h"
#include "Scene.h"

#include "SceneLoader.h"

class SceneLoader {

private:

    RTCDevice _embreeDevice;
    IntegratorType _integratorType = IntegratorType::kRayTracerIntegrator;
    glm::uvec2 _imageSize = glm::uvec2(1280, 720);
    int _maxDepth = 5;
    int _spp = 1;
    std::string _outputFileName = "out.png";
    glm::vec3 _cameraOrigin = glm::vec3(-1.0f, 0.0f, 0.0f);
    glm::vec3 _cameraLookAt = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 _cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float _cameraFieldOfView = 45.0f;
    std::vector<glm::mat4> _sphereTransforms;
    std::vector<material_t> _sphereMaterials;
    std::vector<glm::vec3> _rawVertices;
    std::vector<glm::uvec3> _indices;
    std::vector<glm::vec3> _vertices;
    std::vector<material_t> _triMaterials;
    glm::mat4 curTransform = glm::mat4(1.0f);
    std::vector<glm::mat4> _transformStack;
    std::vector<directionalLight_t> _directionalLights;
    std::vector<pointLight_t> _pointLights;
    std::vector<quadLight_t> _quadLights;
    std::vector<glm::uvec4> _quadIndices;
    std::vector<glm::vec3> _quadVertices;
    std::vector<material_t> _quadMaterials;
    bool _lightStratify = false;
    bool _NEE = false;
    bool _RR = false;
    int _numLightSamples = 1;
    glm::vec3 _curAttenuation = glm::vec3(1.0f, 0.0f, 0.0f);
    material_t _curMaterial = {
        glm::vec3(0.0f),  // diffuse
        glm::vec3(0.0f),  // specular
        1.0f,  // shininess
        glm::vec3(0.0f),  // emission
        glm::vec3(0.2f, 0.2f, 0.2f)  // ambient
    };

public:

    SceneLoader(RTCDevice embreeDevice);
    glm::vec3 loadVec3(const std::vector<std::string>& arguments, size_t startIndex = 0);
    glm::uvec3 loadUVec3(const std::vector<std::string>& arguments, size_t startIndex = 0);
    void executeCommand(const std::string& command, const std::vector<std::string>& arguments);
    void loadSceneData(const std::string& filePath);
    Integrator* createIntegrator();
    void loadEmbreeTriangles(RTCScene embreeScene);
    void loadEmbreeSpheres(RTCScene embreeScene);
    void loadEmbreeQuadLights(RTCScene embreeScene);
    RTCScene createEmbreeScene();
    Scene* commitSceneData(IntegratorType& integratorType);

};

SceneLoader::SceneLoader(RTCDevice embreeDevice)
    : _embreeDevice(embreeDevice)
{
}

glm::vec3 SceneLoader::loadVec3(const std::vector<std::string>& arguments, size_t startIndex)
{
    return glm::vec3(
        std::stof(arguments[startIndex]),
        std::stof(arguments[startIndex + 1]),
        std::stof(arguments[startIndex + 2]));
}

glm::uvec3 SceneLoader::loadUVec3(const std::vector<std::string>& arguments, size_t startIndex)
{
    return glm::uvec3(
        std::stoi(arguments[startIndex]),
        std::stoi(arguments[startIndex + 1]),
        std::stoi(arguments[startIndex + 2]));
}

void SceneLoader::executeCommand(
    const std::string& command,
    const std::vector<std::string>& arguments)
{
    if (command == "size") {

        _imageSize = glm::uvec2(std::stoi(arguments[0]), std::stoi(arguments[1]));

    } else if (command == "maxdepth") {

        _maxDepth = std::stoi(arguments[0]);
        if (_maxDepth == -1) _maxDepth = std::numeric_limits<int>::max();

    } else if (command == "spp") {
        _spp = std::stoi(arguments[0]);
    } else if (command == "output") {
        _outputFileName = arguments[0];
    } else if (command == "lightsamples") {
        _numLightSamples = std::stoi(arguments[0]);
    } else if (command == "lightstratify") {
        _lightStratify = true;
    } else if (command == "nexteventestimation") {
        if (arguments[0]== "on")
            _NEE = true;
    } else if (command == "russianroulette") {
        if (arguments[0]=="on") _RR = true;
    } else if (command == "integrator") {
        if (arguments[0] == "analyticdirect")
            _integratorType = IntegratorType::kAnalyticIntegrator;
        else if (arguments[0] == "direct")
            _integratorType = IntegratorType::kDirectIntegrator;
        else if (arguments[0] == "pathtracer")
            _integratorType = IntegratorType::kPathTracerIntegrator;
        else
            _integratorType = IntegratorType::kRayTracerIntegrator;
    } else if (command == "camera") {

        _cameraOrigin = loadVec3(arguments, 0);
        _cameraLookAt = loadVec3(arguments, 3);
        _cameraUp = loadVec3(arguments, 6);
        _cameraFieldOfView = std::stof(arguments[9]);

    } else if (command == "sphere") {

        glm::vec3 center = loadVec3(arguments, 0);
        float radius = std::stof(arguments[3]);

        glm::mat4 transform = glm::mat4(1.0f);
        transform = curTransform * transform;
        transform = glm::translate(transform, center);
        transform = glm::scale(transform, glm::vec3(radius));

        _sphereTransforms.push_back(transform);

        _sphereMaterials.push_back(_curMaterial);

    } else if (command == "maxverts") {

        // ignore since we are using std::vector

    } else if (command == "vertex") {

        _rawVertices.push_back(loadVec3(arguments));

    } else if (command == "tri") {

        glm::uvec3 rawIndices = loadUVec3(arguments);

        _indices.push_back(glm::uvec3(
            _vertices.size(),
            _vertices.size() + 1,
            _vertices.size() + 2));

        _vertices.push_back(glm::vec3(curTransform * glm::vec4(_rawVertices[rawIndices.x], 1.0f)));
        _vertices.push_back(glm::vec3(curTransform * glm::vec4(_rawVertices[rawIndices.y], 1.0f)));
        _vertices.push_back(glm::vec3(curTransform * glm::vec4(_rawVertices[rawIndices.z], 1.0f)));

        _triMaterials.push_back(_curMaterial);

    } else if (command == "translate") {

        glm::vec3 translation = loadVec3(arguments);
        curTransform = glm::translate(curTransform, translation);

    } else if (command == "rotate") {

        glm::vec3 axis = loadVec3(arguments, 0);
        float radians = std::stof(arguments[3]) * PI / 180.0f;
        curTransform = glm::rotate(curTransform, radians, axis);

    } else if (command == "scale") {

        glm::vec3 scale = loadVec3(arguments);
        curTransform = glm::scale(curTransform, scale);

    } else if (command == "pushTransform") {

        _transformStack.push_back(curTransform);

    } else if (command == "popTransform") {

        curTransform = _transformStack.back();
        _transformStack.pop_back();

    } else if (command == "directional") {

        directionalLight_t light;
        light.toLight = glm::normalize(loadVec3(arguments, 0));
        light.brightness = loadVec3(arguments, 3);

        _directionalLights.push_back(light);

    } else if (command == "point") {

        pointLight_t light;
        light.point = loadVec3(arguments, 0);
        light.brightness = loadVec3(arguments, 3);
        light.attenuation = _curAttenuation;

        _pointLights.push_back(light);

    } else if (command == "quadLight") {
        quadLight_t light;
        light._a = loadVec3(arguments, 0);
        light._ab = loadVec3(arguments, 3);
        light._ac = loadVec3(arguments, 6);
        auto cross = glm::cross(light._ab, light._ac);
        light._area = glm::length(cross);
        light._normal = glm::normalize(cross);
        light._b = light._ab + light._a;
        light._c = light._ac + light._a;
        light._d = light._ab + light._ac + light._a;
        light._intensity = loadVec3(arguments, 9);

        _quadIndices.push_back(glm::uvec4(
            _quadVertices.size(),
            _quadVertices.size() + 1,
            _quadVertices.size() + 3,
            _quadVertices.size() + 2));

        // Following the convention that Light sources are not being multiplied by transformation
        _quadVertices.push_back(light._a);
        _quadVertices.push_back(light._b);
        _quadVertices.push_back(light._c);
        _quadVertices.push_back(light._d);

        _quadLights.push_back(light);

        material_t material;
        material.emission = light._intensity;
        material.isLightSource = true;
        _quadMaterials.emplace_back(material);
    } else if (command == "attenuation") {

        _curAttenuation = loadVec3(arguments);

    } else if (command == "ambient") {

        _curMaterial.ambient = loadVec3(arguments);

    } else if (command == "diffuse") {

        _curMaterial.diffuse = loadVec3(arguments);

    } else if (command == "specular") {

        _curMaterial.specular = loadVec3(arguments);

    } else if (command == "shininess") {

        _curMaterial.shininess = std::stof(arguments[0]);

    } else if (command == "emission") {

        _curMaterial.emission = loadVec3(arguments);

    } else {

        std::cerr << "Unknown command in scene file: '" << command << "'" << std::endl;

    }
}

void SceneLoader::loadSceneData(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) throw std::runtime_error("Could not open file: '" + filePath + "'");

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream tokenStream(line);

        std::string command;
        tokenStream >> command;

        if (command.size() == 0 || command[0] == '#') continue;

        std::vector<std::string> arguments;
        std::string argument;
        while (tokenStream >> argument) {
            arguments.push_back(argument);
        }

        executeCommand(command, arguments);
    }
}

void SceneLoader::loadEmbreeTriangles(RTCScene embreeScene)
{
    RTCGeometry embreeTriangles = rtcNewGeometry(_embreeDevice, RTC_GEOMETRY_TYPE_TRIANGLE);

    glm::vec3* embreeVertices = reinterpret_cast<glm::vec3*>(rtcSetNewGeometryBuffer(
        embreeTriangles,
        RTC_BUFFER_TYPE_VERTEX,
        0,
        RTC_FORMAT_FLOAT3,
        sizeof(glm::vec3),
        _vertices.size()));
    std::memcpy(embreeVertices, _vertices.data(), _vertices.size() * sizeof(glm::vec3));

    glm::uvec3* embreeIndices = reinterpret_cast<glm::uvec3*>(rtcSetNewGeometryBuffer(
        embreeTriangles,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT3,
        sizeof(glm::uvec3),
        _indices.size()));
    std::memcpy(embreeIndices, _indices.data(), _indices.size() * sizeof(glm::uvec3));

    rtcCommitGeometry(embreeTriangles);
    //rtcAttachGeometry(embreeScene, embreeTriangles);
    rtcAttachGeometryByID(embreeScene, embreeTriangles, geometryID_t::kTriangle);
    rtcReleaseGeometry(embreeTriangles);
}

void SceneLoader::loadEmbreeSpheres(RTCScene embreeScene)
{
    RTCScene embreeSphereScene = rtcNewScene(_embreeDevice);

    RTCGeometry embreeSphere = rtcNewGeometry(_embreeDevice, RTC_GEOMETRY_TYPE_SPHERE_POINT);

    glm::vec4* embreeSpherePoint = reinterpret_cast<glm::vec4*>(rtcSetNewGeometryBuffer(
        embreeSphere,
        RTC_BUFFER_TYPE_VERTEX,
        0,
        RTC_FORMAT_FLOAT4,
        sizeof(glm::vec4),
        1));
    *embreeSpherePoint = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    rtcCommitGeometry(embreeSphere);
    //rtcAttachGeometry(embreeSphereScene, embreeSphere);
    rtcAttachGeometryByID(embreeSphereScene, embreeSphere, geometryID_t::kSphere);
    rtcReleaseGeometry(embreeSphere);
    rtcCommitScene(embreeSphereScene);

    for (glm::mat4 transform : _sphereTransforms) {
        RTCGeometry embreeSphereInstance = rtcNewGeometry(_embreeDevice, RTC_GEOMETRY_TYPE_INSTANCE);
        rtcSetGeometryInstancedScene(embreeSphereInstance, embreeSphereScene);
        rtcSetGeometryTimeStepCount(embreeSphereInstance, 1);
        rtcSetGeometryTransform(
            embreeSphereInstance,
            0,
            RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR,
            glm::value_ptr(transform));
        rtcCommitGeometry(embreeSphereInstance);
        rtcAttachGeometry(embreeScene, embreeSphereInstance);
        rtcReleaseGeometry(embreeSphereInstance);
    }

    rtcReleaseScene(embreeSphereScene);
}

void SceneLoader::loadEmbreeQuadLights(RTCScene embreeScene) {
    RTCGeometry embreeQuads = rtcNewGeometry(_embreeDevice, RTC_GEOMETRY_TYPE_QUAD);

    glm::vec3* embreeVertices = reinterpret_cast<glm::vec3*>(rtcSetNewGeometryBuffer(
        embreeQuads,
        RTC_BUFFER_TYPE_VERTEX,
        0,
        RTC_FORMAT_FLOAT3,
        sizeof(glm::vec3),
        _quadVertices.size()));
    std::memcpy(embreeVertices, _quadVertices.data(), _quadVertices.size() * sizeof(glm::vec3));

    glm::uvec4* embreeIndices = reinterpret_cast<glm::uvec4*>(rtcSetNewGeometryBuffer(
        embreeQuads,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT4,
        sizeof(glm::uvec4),
        _quadIndices.size()));
    std::memcpy(embreeIndices, _quadIndices.data(), _quadIndices.size() * sizeof(glm::uvec4));

    rtcCommitGeometry(embreeQuads);
    //rtcAttachGeometry(embreeScene, embreeTriangles);
    rtcAttachGeometryByID(embreeScene, embreeQuads, geometryID_t::kQuadLight);
    rtcReleaseGeometry(embreeQuads);
}

RTCScene SceneLoader::createEmbreeScene()
{
    RTCScene embreeScene = rtcNewScene(_embreeDevice);
    loadEmbreeTriangles(embreeScene);
    loadEmbreeSpheres(embreeScene);
    loadEmbreeQuadLights(embreeScene);
    rtcCommitScene(embreeScene);
    return embreeScene;
}

Scene* SceneLoader::commitSceneData(IntegratorType& integratorType)
{
    float aspectRatio = static_cast<float>(_imageSize.x) / _imageSize.y;
    glm::vec3 cameraLook = glm::normalize(_cameraLookAt - _cameraOrigin);
    glm::vec3 imagePlaneRight = glm::normalize(glm::cross(cameraLook, _cameraUp));
    glm::vec3 imagePlaneUp = glm::normalize(glm::cross(imagePlaneRight, cameraLook));

    camera_t camera;
    camera.origin = _cameraOrigin;
    camera.imagePlaneTopLeft =
        _cameraOrigin
        + cameraLook / std::tan(PI * _cameraFieldOfView / 360.0f)
        + imagePlaneUp
        - aspectRatio * imagePlaneRight;
    camera.pixelRight = (2.0f * aspectRatio / _imageSize.x) * imagePlaneRight;
    camera.pixelDown = (-2.0f / _imageSize.y) * imagePlaneUp;

    std::vector<glm::mat3> sphereNormalTransforms;
    for (size_t i = 0; i < _sphereTransforms.size(); i++) {
        sphereNormalTransforms.push_back(glm::inverseTranspose(glm::mat3(_sphereTransforms[i])));
    }

    Scene* scene = new Scene();
    scene->imageSize = _imageSize;
    scene->maxDepth = _maxDepth;
    scene->spp = _spp;
    scene->outputFileName = _outputFileName;
    scene->camera = camera;
    scene->sphereNormalTransforms = std::move(sphereNormalTransforms);
    scene->sphereMaterials = std::move(_sphereMaterials);
    scene->triMaterials = std::move(_triMaterials);
    scene->directionalLights = std::move(_directionalLights);
    scene->pointLights = std::move(_pointLights);
    scene->quadLights = std::move(_quadLights);
    scene->quadMaterials = std::move(_quadMaterials);
    scene->embreeScene = createEmbreeScene();
    scene->numLightSamples = _numLightSamples;
    scene->lightStratify = _lightStratify;
    scene->NEE = _NEE;
    scene->RR = _RR;
    integratorType = _integratorType;
    return scene;
}

void loadScene(
    const std::string& filePath,
    RTCDevice embreeDevice,
    Scene** scene, IntegratorType& integratorType)
{
    SceneLoader sceneLoader(embreeDevice);
    sceneLoader.loadSceneData(filePath);
    *scene = sceneLoader.commitSceneData(integratorType);
}
