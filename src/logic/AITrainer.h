#pragma once

#include "Match.h"

namespace NEAT {
class Population;
class Genome;
class Organism;
} // namespace NEAT

#include <memory>

namespace mlbb {

//! \brief Holds the AI simulation state and matches the AI is playing
class AITrainer {
    struct RunningAI {
    public:
        NEAT::Organism* AI;
        std::shared_ptr<Match> PlayingMatch;
    };

public:
    AITrainer(int width, int height, int startingPopulationSize);
    ~AITrainer();

    void Begin();

    void Update(float delta);

    std::shared_ptr<Match> GetAIMatch();

private:
    void SetupGenerationMatches();
    void PerformAIThinking(
        NEAT::Organism* organism, const Match& match, ProgrammaticInput& output);

    // AI input finding
    double GetScaledPaddleX(const Match& match) const;
    std::tuple<double, double> GetScaledBallPos(const Match& match) const;
    std::tuple<double, double> GetScaledLowestBrickPos(const Match& match) const;

private:
    const int MatchWidth;
    const int MatchHeight;
    const int StartingPopulationSize;

    std::vector<RunningAI> CurrentRuns;

    std::unique_ptr<NEAT::Population> AIPopulation;
    std::unique_ptr<NEAT::Genome> InitialGenes;

    int CurrentGeneration = -1;
};
} // namespace mlbb