#include "AITrainer.h"

#include <genome.h>
#include <population.h>

#include <array>
#include <functional>

using namespace mlbb;

AITrainer::AITrainer(int width, int height, int startingPopulationSize) :
    MatchWidth(width), MatchHeight(height), StartingPopulationSize(startingPopulationSize)
{}

AITrainer::~AITrainer()
{
    // Tell all threads to quit
    {
        std::unique_lock<std::mutex> lock(TaskListMutex);
        EnsureRightThreadCount(0, lock);
        TaskWait.notify_all();
    }

    // Wait for all of the threads to quit
    for(auto& thread : TaskThreads) {
        thread.join();
    }
}

void AITrainer::Begin()
{
    // Load the initial genome
    {
        std::ifstream genomeReader("config/startgenes");

        // So apparently we need to manually read the first line
        std::string genomeWord;
        int id;

        genomeReader >> genomeWord >> id;

        if(!genomeReader.good())
            throw std::runtime_error("can't read startgenes file");

        InitialGenes = std::make_unique<NEAT::Genome>(id, genomeReader);
    }

    // Prepare population
    AIPopulation =
        std::make_unique<NEAT::Population>(InitialGenes.get(), StartingPopulationSize);

    if(!AIPopulation->verify())
        throw std::runtime_error("failed to verify initial AI population after creation");

    CurrentGeneration = 1;
    SetupGenerationMatches();
}

void AITrainer::Update(float delta, int iterations, int threads)
{
    std::unique_lock<std::mutex> lock(TaskListMutex);
    EnsureRightThreadCount(threads > 1 ? threads : 0, lock);
    ReadyTasks = 0;

    // Update any current generation AIs
    const int simulationsNeeded = CountActiveAIMatches();

    // TODO: add a parameter to limit the concurrently running AIs (so that first N are
    // ran until completion and then the next ones, but not on the same game update as that
    // wouldn't help anything)

    if(simulationsNeeded <= 1 || threads <= 1) {
        // Just a single AI needs to be ran, run in current thread (or single thread mode is
        // used)
        for(auto& run : CurrentRuns) {
            if(run.PlayingMatch->HasEnded())
                continue;

            RunSingleAI(run, delta, iterations);
        }

        // Make sure threads do get notified about quit tasks
        TaskWait.notify_one();
    } else {
        // Find AIs that are still alive to build a list of tasks to run
        const int aisPerThread = std::max(1, simulationsNeeded / threads);

        AliveRuns.clear();
        // Need to reserve here to not break pointers with allocations
        AliveRuns.reserve(simulationsNeeded);

        ReadyTasks = 0;
        int submittedTasks = 0;

        RunningAI** chunkStart = nullptr;
        int currentChunk = 0;

        // Create and populate the tasks list
        for(auto& run : CurrentRuns) {
            if(run.PlayingMatch->HasEnded())
                continue;

            AliveRuns.push_back(&run);

            if(chunkStart == nullptr)
                chunkStart = &AliveRuns.back();
            ++currentChunk;

            if(currentChunk >= aisPerThread) {
                TaskList.emplace(chunkStart, currentChunk, iterations, delta);

                ++submittedTasks;
                chunkStart = nullptr;
                currentChunk = 0;
            }
        }

        TaskWait.notify_all();
        lock.unlock();

        // Wait for AIs to end
        while(ReadyTasks != submittedTasks) {
            // Could maybe sleep here, but this guarantees that we'll detect faster that
            // things have finished
            std::this_thread::yield();
        }

        lock.lock();
    }

    // Start a new generation when all matches are finished
    if(CountActiveAIMatches() < 1) {
        // Finish the current generation
        for(auto species : AIPopulation->species) {
            species->compute_average_fitness();
            species->compute_max_fitness();
        }

        // This creates the next generation
        AIPopulation->epoch(CurrentGeneration);

        ++CurrentGeneration;

        SetupGenerationMatches();
    }
}

void AITrainer::SetupGenerationMatches()
{
    CurrentRuns.clear();

    // Create one match for each of the organisms
    for(auto organism : AIPopulation->organisms) {
        CurrentRuns.push_back({organism, std::make_shared<Match>(MatchWidth, MatchHeight)});
    }
}

void AITrainer::PerformAIThinking(
    NEAT::Organism* organism, const Match& match, ProgrammaticInput& output)
{
    const auto [ballX, ballY] = GetScaledBallPos(match);
    const auto [brickX, brickY] = GetScaledLowestBrickPos(match);

    // AI inputs, must match what is in startgenes
    std::array<double, 5> inputs;

    // paddle x
    inputs[0] = GetScaledPaddleX(match);
    // ball x
    inputs[1] = ballX;
    // ball y
    inputs[2] = ballY;
    // lowest brick x
    inputs[3] = brickX;
    // lowest brick y
    inputs[4] = brickY;


    auto* network = organism->net;

    network->load_sensors(inputs.data());

    if(!network->activate()) {
        // Apparently the network loops and it is not finished yet?
        // But maybe outputs are still fine so we can continue?
    }

    const auto left = network->outputs[0]->activation > 0.5f;
    const auto right = network->outputs[1]->activation > 0.5f;
    const auto special = network->outputs[2]->activation > 0.5f;

    output = ProgrammaticInput(left, right, special);
}

std::tuple<std::shared_ptr<Match>, int> AITrainer::GetAIMatch() const
{
    int index = 0;

    // For now just the first (alive one is shown, or the last one if nothing else)
    for(const auto& run : CurrentRuns) {
        if(!run.PlayingMatch->HasEnded())
            return std::make_tuple(run.PlayingMatch, index);

        ++index;
    }

    if(!CurrentRuns.empty())
        return std::make_tuple(CurrentRuns.back().PlayingMatch, CurrentRuns.size() - 1);

    // No good match to show
    return std::make_tuple(nullptr, -1);
}

int AITrainer::CountActiveAIMatches() const
{
    int count = 0;

    for(const auto& run : CurrentRuns) {
        if(!run.PlayingMatch->HasEnded())
            ++count;
    }

    return count;
}

double AITrainer::GetScaledPaddleX(const Match& match) const
{
    for(const auto& paddle : match.GetPaddlesForDrawing()) {
        return paddle.X / static_cast<float>(MatchWidth);
    }

    return -1;
}

std::tuple<double, double> AITrainer::GetScaledLowestBrickPos(const Match& match) const
{
    std::optional<std::tuple<double, double>> lowest;

    for(const auto& brick : match.GetBricksForDrawing()) {
        if(!lowest || brick.Y < std::get<1>(*lowest)) {
            lowest = std::make_tuple(brick.X, brick.Y);
        }
    }

    if(lowest) {
        return std::make_tuple(std::get<0>(*lowest) / static_cast<float>(MatchWidth),
            std::get<1>(*lowest) / static_cast<float>(MatchHeight));
    }

    return std::tuple<double, double>(-1, -1);
}

std::tuple<double, double> AITrainer::GetScaledBallPos(const Match& match) const
{
    for(const auto& ball : match.GetBallsForDrawing()) {
        return std::make_tuple(
            ball.X / static_cast<float>(MatchWidth), ball.Y / static_cast<float>(MatchHeight));
    }

    return std::tuple<double, double>(-1, -1);
}

void AITrainer::EnsureRightThreadCount(int threads, std::unique_lock<std::mutex>& lock)
{
    while(ActiveWorkerCount < threads) {
        TaskThreads.emplace_back(std::bind(&AITrainer::RunTaskThread, this));
        ++ActiveWorkerCount;
    }

    while(ActiveWorkerCount > threads) {
        TaskList.emplace(true);
        --ActiveWorkerCount;
    }
}

void AITrainer::RunTaskThread()
{
    std::unique_lock<std::mutex> lock(TaskListMutex);

    while(true) {
        while(!TaskList.empty()) {

            auto task = std::move(TaskList.top());
            TaskList.pop();

            // Unlock while we process it
            lock.unlock();

            if(task.Quit) {
                // Time to exit
                return;
            }

            ProcessTask(task);

            ++ReadyTasks;
            lock.lock();
        }

        // Wait for is used to prevent this getting stuck if the threads hit things just right
        // which shouldn't happen, but I don't want to deal with the chance that it would
        // happen
        TaskWait.wait_for(lock, std::chrono::milliseconds(200));
    }
}

void AITrainer::ProcessTask(AITrainer::AIRunTask& task)
{
    for(int i = 0; i < task.Count; ++i) {
        RunSingleAI(*task.TaskArray[i], task.Delta, task.Iterations);
    }
}

void AITrainer::RunSingleAI(AITrainer::RunningAI& run, float delta, int iterations)
{
    do {
        // We don't have winners yet...
        run.AI->winner = false;

        ProgrammaticInput input;
        PerformAIThinking(run.AI, *run.PlayingMatch, input);
        run.PlayingMatch->Update(delta, input);

        // Time out the AI after some time
        if(run.PlayingMatch->GetElapsedTime() >= MAX_AI_PLAY_TIME)
            run.PlayingMatch->Forfeit();

        // Record score when match ends
        if(run.PlayingMatch->HasEnded()) {
            // fitness is alive time + timed score
            run.AI->fitness =
                run.PlayingMatch->GetTimedScore() + run.PlayingMatch->GetElapsedTime();

            // No need to run any further iterations
            return;
        }
    } while(--iterations > 0);
}
