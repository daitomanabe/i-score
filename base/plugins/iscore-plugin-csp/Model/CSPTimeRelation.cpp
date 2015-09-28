#include "CSPTimeRelation.hpp"
#include "CSPScenario.hpp"
#include <Process/ScenarioInterface.hpp>
#include <Process/ScenarioModel.hpp>
#include <Document/Constraint/ConstraintModel.hpp>
#include <Model/CSPTimeNode.hpp>
#include <kiwi/kiwi.h>

CSPTimeRelation::CSPTimeRelation(CSPScenario& cspScenario, const Id<ConstraintModel>& constraintId)
    :CSPConstraintHolder::CSPConstraintHolder(cspScenario.getSolver(), &cspScenario)
{
    this->setParent(&cspScenario);
    this->setObjectName("CSPTimeRelation");

    m_iscoreMin = &cspScenario.getScenario()->constraint(constraintId).duration.minDuration();
    m_iscoreMax = &cspScenario.getScenario()->constraint(constraintId).duration.maxDuration();

    m_min.setValue(m_iscoreMin->msec());
    m_max.setValue(m_iscoreMax->msec());

    // weight
    //solver.addEditVariable(m_min, kiwi::strength::strong);
    //solver.addEditVariable(m_max, kiwi::strength::medium);

    auto scenario = cspScenario.getScenario();
    auto& constraint = scenario->constraint(constraintId);

    auto& prevTimenodeModel = scenario->timeNode(constraint.startTimeNode());
    auto& nextTimenodeModel = scenario->timeNode(constraint.endTimeNode());

    auto prevCSPTimenode = cspScenario.insertTimenode(prevTimenodeModel.id());
    auto nextCSPTimenode = cspScenario.insertTimenode(nextTimenodeModel.id());

    // apply model constraints
    //Note: min & max can be negative no problemo muchacho
    // 1 - min inferior to max
    PUT_CONSTRAINT(cMinInfMax, m_min <= m_max);

    // 2 - date of end timenode inside min and max
    PUT_CONSTRAINT(cInsideMin, nextCSPTimenode->getDate() >= (prevCSPTimenode->getDate() + m_min));
    PUT_CONSTRAINT(cInsideMax, nextCSPTimenode->getDate() <= (prevCSPTimenode->getDate() + m_max));

    // 3 - min >= 0
    PUT_CONSTRAINT(cMinSupZero, m_min >= 0);

    // if there are sub scenarios, store them
    for(auto& process : constraint.processes)
    {
        if(auto* scenario = dynamic_cast<ScenarioModel*>(&process))
        {
            m_subScenarios.insert(scenario->id(), new CSPScenario(*scenario, scenario));
        }
    }

    // watch over durations edits
    con(constraint.duration, &ConstraintDurations::minDurationChanged, this, &CSPTimeRelation::onMinDurationChanged);
    con(constraint.duration, &ConstraintDurations::maxDurationChanged, this, &CSPTimeRelation::onMaxDurationChanged);

    // watch over potential subscenarios
    con(constraint.processes, &NotifyingMap<Process>::added, this, &CSPTimeRelation::onProcessCreated);
    con(constraint.processes, &NotifyingMap<Process>::removed, this, &CSPTimeRelation::onProcessRemoved);
}

CSPTimeRelation::~CSPTimeRelation()
{
}

kiwi::Variable& CSPTimeRelation::getMin()
{
    return m_min;
}

kiwi::Variable& CSPTimeRelation::getMax()
{
    return m_max;
}

bool CSPTimeRelation::minChanged() const
{
    return m_min.value() != m_iscoreMin->msec();
}

bool CSPTimeRelation::maxChanged() const
{
    return m_max.value() != m_iscoreMax->msec();
}

void CSPTimeRelation::onMinDurationChanged(const TimeValue& min)
{
    m_min.setValue(min.msec());
}

void CSPTimeRelation::onMaxDurationChanged(const TimeValue& max)
{
    if(max.isInfinite())
    {
        //TODO : ??? remove constraints on max?
    }else
    {
        m_max.setValue(max.msec());
    }
}

void CSPTimeRelation::onProcessCreated(const Process& process)
{
    if(auto scenario = dynamic_cast<const ScenarioModel*>(&process))
    {
        m_subScenarios.insert(scenario->id(), new CSPScenario(*scenario, const_cast<ScenarioModel*>(scenario)));
    }
}

void CSPTimeRelation::onProcessRemoved(const Process& process)
{
    if(auto scenario = dynamic_cast<const ScenarioModel*>(&process))
    {
        delete(m_subScenarios.take(scenario->id()));
    }
}