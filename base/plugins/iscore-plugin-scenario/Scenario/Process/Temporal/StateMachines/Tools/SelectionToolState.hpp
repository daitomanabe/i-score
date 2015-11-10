#pragma once
#include <Scenario/Process/Temporal/StateMachines/Tools/ScenarioToolState.hpp>
#include <iscore/selection/SelectionDispatcher.hpp>

class ScenarioSelectionState;

class MoveConstraintState;
class MoveEventState;
class MoveTimeNodeState;

// TODO rename file.
namespace Scenario
{
class SelectionAndMoveTool final : public ScenarioTool
{
    public:
        SelectionAndMoveTool(ScenarioStateMachine& sm);

        void on_pressed() override;
        void on_moved() override;
        void on_released() override;

    private:
        ScenarioSelectionState* m_state{};
        MoveConstraintState* m_moveConstraint{};
        MoveEventState* m_moveEvent{};
        MoveTimeNodeState* m_moveTimeNode{};

        bool m_nothingPressed{true};
};
}