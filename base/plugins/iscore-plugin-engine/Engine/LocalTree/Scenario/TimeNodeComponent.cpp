#include "TimeNodeComponent.hpp"
#include "MetadataParameters.hpp"
#include <Scenario/Document/TimeNode/Trigger/TriggerModel.hpp>
#include <State/Value.hpp>
#include <ossia/editor/state/state_element.hpp>

namespace Engine
{
namespace LocalTree
{
TimeNode::TimeNode(
        ossia::net::node_base& parent,
        const Id<iscore::Component>& id,
        Scenario::TimeNodeModel& timeNode,
        DocumentPlugin& doc,
        QObject* parent_comp):
    CommonComponent{parent, timeNode.metadata(), doc, id, "StateComponent", parent_comp}
{
    m_properties.push_back(
    add_setProperty<::State::impulse_t>(node(), "trigger",
                                       [&] (auto) {
        timeNode.trigger()->triggeredByGui();
    }));
}
}
}
