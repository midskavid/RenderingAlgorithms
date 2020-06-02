#include <iostream>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <chrono>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <embree3/rtcore.h>
#include <lodepng/lodepng.h>

#include "Scene.h"
#include "SceneLoader.h"
#include "Integrator.h"
#include "RenderPool.h"

#include "Engine.h"

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<float>;

const unsigned int WINDOW_DIM = 32;
const glm::vec3 LIGHT_BLUE = {0.5,0.5,1.0};
const glm::vec3 LIGHT_RED = {1.0, 0.5, 0.5};

static unsigned char convertColorChannel(float channel)
{
    return static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, 255.0f * channel)));
}

static void saveImage(
    const std::vector<glm::vec3>& imageData,
    glm::uvec2 imageSize,
    const std::string& fileName)
{
    std::vector<unsigned char> imageByteData(imageSize.y * imageSize.x * 3);
    for (size_t y = 0; y < imageSize.y; y++) {
        for (size_t x = 0; x < imageSize.x; x++) {

            glm::vec3 color = imageData[y * imageSize.x + x];

            size_t outPixelBase = 3 * (y * imageSize.x + x);
            imageByteData[outPixelBase + 0] = convertColorChannel(color.r);
            imageByteData[outPixelBase + 1] = convertColorChannel(color.g);
            imageByteData[outPixelBase + 2] = convertColorChannel(color.b);
        }
    }

    unsigned int error = lodepng::encode(
        fileName,
        imageByteData,
        imageSize.x,
        imageSize.y,
        LCT_RGB);
    if (error) {
        throw std::runtime_error(
            "LodePNG error (" + std::to_string(error) + "): "
            + lodepng_error_text(error));
    }
}

static void embreeErrorFunction(void* userPtr, RTCError error, const char* str)
{
    (void) userPtr;
    std::cerr << "Embree error (" << error << "): " << str << std::endl;
}

static void printLoadingBar(float completion, int numBars = 60)
{
    int barsComplete = static_cast<int>(std::floor(numBars * completion));
    int percentComplete = static_cast<int>(std::floor(100 * completion));

    std::ostringstream oss;

    oss << "\r[";
    int j = 1;
    for (; j <= barsComplete; j++) {
        oss << '#';
    }
    for (; j <= numBars; j++) {
        oss << ' ';
    }
    oss << "] " << percentComplete << "%\r";

    std::cout << oss.str() << std::flush;
}

void render(const std::string& sceneFilePath)
{

    std::cout << "Loading scene..." << std::endl;

    RTCDevice embreeDevice = rtcNewDevice(nullptr);
    if (!embreeDevice) throw std::runtime_error("Could not initialize Embree device.");

    rtcSetDeviceErrorFunction(embreeDevice, embreeErrorFunction, nullptr);

    Scene* scene;
    IntegratorType integratorType;
    loadScene(sceneFilePath, embreeDevice, &scene, integratorType);

    Integrator* integrator = nullptr;
    switch (integratorType)
    {
    case IntegratorType::kRayTracerIntegrator:
        integrator = new RayTracerIntegrator;
        break;
    case IntegratorType::kAnalyticIntegrator:
        integrator = new AnalyticIntegrator;
        break;
    case IntegratorType::kDirectIntegrator :
        integrator = new DirectIntegrator(scene->lightStratify, scene->numLightSamples);
        break;
    case IntegratorType::kPathTracerIntegrator :
        integrator = new PathTracerIntegrator(scene->maxDepth, scene->lightStratify, scene->numLightSamples);
        break;
    }
    integrator->setScene(scene);
    //scene->maxDepth = std::min(scene->maxDepth,2);
    //if (scene->outputFileName == "dragon.png") {scene->NEE = true;scene->MIS = false;}
    //if (scene->MIS&&scene->maxDepth>1)
    std::cout << "Preparing render jobs..." << std::endl;

#ifdef NDEBUG
    int numThreads = std::thread::hardware_concurrency();
#else    
    int numThreads = 1;
#endif
    scene->adaptiveSampler = new AMLD (scene->spp, scene->imageSize.x, scene->imageSize.y);
    TimePoint startTime = Clock::now();
    std::vector<glm::vec3> imageData(scene->imageSize.y * scene->imageSize.x);
    for (int itr=0;itr<4;++itr) {
        std::vector<RenderJob*> jobs;
        for (unsigned int y = 0; y < scene->imageSize.y; y += WINDOW_DIM) {
            for (unsigned int x = 0; x < scene->imageSize.x; x += WINDOW_DIM) {
                glm::uvec2 startPixel = glm::uvec2(x, y);
                glm::uvec2 windowSize = glm::uvec2(
                    std::min(x + WINDOW_DIM, scene->imageSize.x) - x,
                    std::min(y + WINDOW_DIM, scene->imageSize.y) - y);
                jobs.push_back(new RenderJob(startPixel, windowSize));
            }
        }

        std::cout
            << "Rendering... ("
            << jobs.size() << " jobs, "
            << numThreads << " threads)"
            << std::endl;

        
        {
            RenderPool pool(scene, integrator, numThreads, jobs);

            size_t numCompletedJobs = 0;
            while (numCompletedJobs < jobs.size()) {

                std::vector<RenderJob*> completedJobs;
                pool.getCompletedJobs(completedJobs);

                // for (RenderJob* job : completedJobs) {
                //     std::vector<glm::vec3> result = job->getResult();
                //     for (unsigned int wy = 0; wy < job->windowSize.y; wy++) {
                //         unsigned int y = job->startPixel.y + wy;
                //         for (unsigned int wx = 0; wx < job->windowSize.x; wx++) {
                //             unsigned int x = job->startPixel.x + wx;
                //             imageData[y * scene->imageSize.x + x] = glm::vec3(std::pow(result[wy * job->windowSize.x + wx].x,1.0f/scene->gamma), std::pow(result[wy * job->windowSize.x + wx].y,1.0f/scene->gamma), std::pow(result[wy * job->windowSize.x + wx].z,1.0f/scene->gamma));
                //         }
                //     }
                // }
                numCompletedJobs += completedJobs.size();

                printLoadingBar(static_cast<float>(numCompletedJobs) / jobs.size());
            }
        }
        // DUMP Image of samples...
        for (size_t ii=0;ii<scene->imageSize.y;++ii) {
            for (size_t jj=0;jj<scene->imageSize.x;++jj) {
                int pixIdx = ii*scene->imageSize.x+jj;
                auto curSamp = scene->adaptiveSampler->GetNumSamplesAtPixel(pixIdx);
                auto alpha = (65.0f-curSamp)/(65.0f);
                auto pixCol = alpha*LIGHT_BLUE + (1.0f-alpha)*LIGHT_RED;
                imageData[pixIdx] = pixCol;
            }
        }
        saveImage(imageData, scene->imageSize, std::to_string(itr)+".png");
        scene->adaptiveSampler->CreateImportanceMap(itr);
        jobs.clear();
    }

    TimePoint endTime = Clock::now();
    Duration renderTime = endTime - startTime;
    
    std::cout << std::endl;
    std::cout << "Render time: " << renderTime.count() << "s" << std::endl;

    for (size_t ii=0;ii<imageData.size();++ii) {
        glm::vec3 col {0,0,0};
        for (const auto& cc:scene->adaptiveSampler->mPixelColor[ii]) {
            col += cc;
        }
        col = col*(1.0f/scene->adaptiveSampler->mPixelColor[ii].size());
        imageData[ii] = glm::vec3(std::pow(col.x,1.0f/scene->gamma), std::pow(col.y,1.0f/scene->gamma), std::pow(col.z,1.0f/scene->gamma));
    }


    rtcReleaseScene(scene->embreeScene);
    rtcReleaseDevice(embreeDevice);

    saveImage(imageData, scene->imageSize, scene->outputFileName);
    std::cout << "Image saved as '" << scene->outputFileName << "'" << std::endl;
}
