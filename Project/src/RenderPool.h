#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <glm/glm.hpp>

#include "Scene.h"
#include "Integrator.h"

class RenderJob {
    std::vector<glm::vec2> GenerateUniformRandomSamples(int numS);

public:
    RenderJob(glm::uvec2 startPixel, glm::uvec2 windowSize);
    void render(Scene* scene, Integrator* integrator);

public:
    const glm::uvec2 startPixel;
    const glm::uvec2 windowSize;

private:
    //std::vector<glm::vec3> _result;

};

class RenderPool {

private:

    Scene* _scene;
    Integrator* _integrator;
    std::vector<std::thread> _threads;
    std::mutex _mutex;
    std::condition_variable _condition;
    size_t _nextJob;
    std::vector<RenderJob*>& _jobQueue;
    std::vector<RenderJob*> _completedJobs;

public:

    RenderPool(Scene* scene, Integrator* integrator, int numThreads, std::vector<RenderJob*>& jobs);
    ~RenderPool();
    void getCompletedJobs(std::vector<RenderJob*>& completedJobs);

private:

    static void threadMain(RenderPool* pool);

};
