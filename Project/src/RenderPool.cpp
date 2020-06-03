#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <glm/glm.hpp>

#include "Scene.h"
#include "Integrator.h"
#include "AMLD.h"
#include "RenderPool.h"

RenderJob::RenderJob(glm::uvec2 startPixel, glm::uvec2 windowSize)
    : startPixel(startPixel),
      windowSize(windowSize)
{
}

void RenderJob::render(Scene* scene, Integrator* integrator)
{
    for (size_t wy = 0; wy < windowSize.y; wy++) {
        size_t y = startPixel.y + wy;
        for (size_t wx = 0; wx < windowSize.x; wx++) {
            size_t x = startPixel.x + wx;
            int pixIdx = y*scene->imageSize.x + x;
            auto numSamp = scene->adaptiveSampler->GetSPPForPixel(pixIdx);
            auto unifSamples = GenerateUniformRandomSamples(numSamp);

            for (const auto& sp:unifSamples) {
                glm::vec3 target = scene->camera.imagePlaneTopLeft + (x + sp.x) * scene->camera.pixelRight + (y + sp.y) * scene->camera.pixelDown;
                glm::vec3 direction = glm::normalize(target - scene->camera.origin);
                auto outC = integrator->traceRay(scene->camera.origin, direction);
                scene->adaptiveSampler->AddPixelColor(pixIdx, outC);
            }
        }
    }
}

std::vector<glm::vec2> RenderJob::GenerateUniformRandomSamples(int numS) {
    static thread_local std::mt19937 gen; 
    std::uniform_real_distribution<float> distribution(0.0f,1.0f);
    std::vector<glm::vec2> data;
    //std::cout<<distribution(generator);
    for (int ii=0;ii<numS;++ii) {
        data.emplace_back(glm::vec2(distribution(gen), distribution(gen)));
    }
    //std::generate(data.begin(), data.end(), [distribution]() { return glm::vec2 (distribution(generator),distribution(generator)); });
    return data;    
}

RenderPool::RenderPool(Scene* scene, Integrator* integrator, int numThreads, std::vector<RenderJob*>& jobs)
    : _scene(scene), _integrator(integrator), _nextJob(0), _jobQueue(jobs)
{
    for (int i = 0; i < numThreads; i++) {
        _threads.push_back(std::thread(threadMain, this));
    }
}

RenderPool::~RenderPool()
{
    for (std::thread& thread : _threads) {
        thread.join();
    }
}

void RenderPool::getCompletedJobs(std::vector<RenderJob*>& completedJobs)
{
    {
        std::unique_lock<std::mutex> lock(_mutex);

        _condition.wait(lock, [this]{ return _completedJobs.size() > 0; });
        completedJobs = std::move(_completedJobs);
    }
}

void RenderPool::threadMain(RenderPool* pool)
{
    while (true) {

        size_t jobIndex;
        {
            std::unique_lock<std::mutex> lock(pool->_mutex);

            if (pool->_nextJob >= pool->_jobQueue.size()) break;

            jobIndex = pool->_nextJob;
            pool->_nextJob++;
        }

        pool->_jobQueue[jobIndex]->render(pool->_scene, pool->_integrator);

        {
            std::unique_lock<std::mutex> lock(pool->_mutex);

            pool->_completedJobs.push_back(pool->_jobQueue[jobIndex]);
            pool->_condition.notify_all();
        }
    }
}
