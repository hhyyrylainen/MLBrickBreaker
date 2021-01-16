#include "Match.h"

using namespace mlbb;

constexpr auto NEXT_BALL_DELAY = 0.2f;

void Match::Update(float elapsed, const Input& input)
{
    if(elapsed <= 0)
        return;

    TotalElapsed += elapsed;
    CurrentStateElapsed += elapsed;

    switch(State) {
    case MatchState::Starting: {
        // TODO: create bricks
        // State = MatchState::Serving;
        break;
    }
    case MatchState::Serving: {

        break;
    }
    case MatchState::WaitingForNextServe: {
        if(CurrentStateElapsed > NEXT_BALL_DELAY) {
            State = MatchState::Serving;
        }
        break;
    }
    case MatchState::Playing: {
        break;
    }
    case MatchState::Ended: break;
    }
}
