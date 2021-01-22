#include "AITrainer.h"

#include <genome.h>
#include <population.h>

using namespace mlbb;

AITrainer::AITrainer(int width, int height, int startingPopulationSize) :
    MatchWidth(width), MatchHeight(height), StartingPopulationSize(startingPopulationSize)
{}

AITrainer::~AITrainer()
{
    // TODO: stop threading here once it is added
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

void AITrainer::Update(float delta)
{
    bool generationFinished = true;

    // Update any current generation AIs
    // TODO: add threading here to speed this up
    // TODO: also add a parameter to limit the concurrently running AIs (so that first N are
    // ran until completion and then the next ones, or rather this loop breaks whenever N
    // number of AIs have ran)
    for(auto& run : CurrentRuns) {
        if(run.PlayingMatch->HasEnded())
            continue;

        generationFinished = false;

        // We don't have winners yet...
        run.AI->winner = false;

        ProgrammaticInput input;
        PerformAIThinking(run.AI, *run.PlayingMatch, input);
        run.PlayingMatch->Update(delta, input);

        // Time out the AI after some time
        if(run.PlayingMatch->GetElapsedTime() >= MAX_AI_PLAY_TIME)
            run.PlayingMatch->Forfeit();

        // Record score when match ends
        // TODO: proper scoring, for now how long the AI was alive is tracked
        if(run.PlayingMatch->HasEnded()) {
            run.AI->fitness = run.PlayingMatch->GetElapsedTime();
        }
    }

    // Start a new generation if needed
    if(generationFinished) {
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
    std::array<double, 5> inputs = {// paddle x
        GetScaledPaddleX(match),
        // ball x
        ballX,
        // ball y
        ballY,
        // lowest brick x
        brickX,
        // lowest brick y
        brickY};

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

std::shared_ptr<Match> AITrainer::GetAIMatch()
{
    // For now just the first (alive one is shown, or the last one if nothing else)
    for(const auto& run : CurrentRuns) {
        if(!run.PlayingMatch->HasEnded())
            return run.PlayingMatch;
    }

    if(!CurrentRuns.empty())
        return CurrentRuns.back().PlayingMatch;

    // No good match to show
    return nullptr;
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
