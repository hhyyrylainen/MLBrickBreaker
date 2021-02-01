#pragma once

#include "Match.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <stack>
#include <thread>
#include <tuple>

namespace NEAT {
class Population;
class Genome;
class Organism;
} // namespace NEAT

namespace mlbb {

enum class AIType { Best };

//! \brief Holds the AI simulation state and matches the AI is playing
class AITrainer {
    struct RunningAI {
    public:
        NEAT::Organism* AI;
        std::shared_ptr<Match> PlayingMatch;
    };

    struct AIRunTask {
    public:
        explicit AIRunTask(bool quit) : Quit(true)
        {
            if(!quit)
                throw std::runtime_error("quit constructor must provide true");
        }

        AIRunTask(RunningAI** tasks, int count, int iterations, float delta) :
            Delta(delta), Iterations(iterations), Count(count), TaskArray(tasks)
        {}

        // TODO: actually use this (though this might need atomic, or a more complicated
        // design...)
        bool AnyStillActive = false;

        // If true tells task thread to quit
        bool Quit = false;

        float Delta = 0.1f;

        int Iterations = 1;

        // Would be nice to use a view here, but those are AFAIK C++20
        int Count = 0;
        RunningAI** TaskArray = nullptr;
    };

public:
    AITrainer(int width, int height, int startingPopulationSize);
    ~AITrainer();

    void Begin();

    void Update(float delta, int iterations, int threads);

    //! \brief Returns a match for the user to view, second value is the AI identifier
    //! (basically just an index for now)
    std::tuple<std::shared_ptr<Match>, int> GetAIMatch(
        std::vector<std::shared_ptr<Match>>* ghostMatches = nullptr, int ghosts = 10) const;

    void WriteSpeciesToFile(const std::string& fileName, AIType ai) const;
    void WriteOrganismToFile(const std::string& fileName, AIType ai) const;

    int CountActiveAIMatches() const;

    int GetGenerationNumber() const
    {
        return CurrentGeneration;
    }

    //! \brief Sets a oneshot generation end callback
    void SetGenerationEndCallback(std::function<void()> callback)
    {
        GenerationEndCallback = callback;
    }

private:
    void SetupGenerationMatches();
    void PerformAIThinking(
        NEAT::Organism* organism, const Match& match, ProgrammaticInput& output);

    // AI input finding
    double GetScaledPaddleX(const Match& match) const;
    std::tuple<double, double> GetScaledBallPos(const Match& match) const;
    std::tuple<double, double> GetScaledLowestBrickPos(const Match& match) const;

    void EnsureRightThreadCount(int threads, std::unique_lock<std::mutex>& lock);

    void RunTaskThread();
    void ProcessTask(AIRunTask& task);
    void RunSingleAI(RunningAI& run, float delta, int iterations);

private:
    const int MatchWidth;
    const int MatchHeight;
    const int StartingPopulationSize;

    std::vector<RunningAI> CurrentRuns;

    std::unique_ptr<NEAT::Population> AIPopulation;
    std::unique_ptr<NEAT::Genome> InitialGenes;

    int CurrentGeneration = -1;

    std::function<void()> GenerationEndCallback;

    // Threading variables
    // TODO: find a better way to turn off threads (right now we don't know which thread
    // closes, so we can't really clean them up easily)
    std::vector<std::thread> TaskThreads;
    int ActiveWorkerCount = 0;

    std::mutex TaskListMutex;
    std::stack<AIRunTask> TaskList;
    std::condition_variable TaskWait;

    std::atomic<int> ReadyTasks;

    std::vector<RunningAI*> AliveRuns;
};
} // namespace mlbb
