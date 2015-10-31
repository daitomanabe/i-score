#include "iscore_plugin_cohesion.hpp"
#include "IScoreCohesionControl.hpp"


#include "Commands/CreateCurvesFromAddresses.hpp"
#include "Commands/CreateCurvesFromAddressesInConstraints.hpp"
#include "Commands/CreateStatesFromParametersInEvents.hpp"
#include "Commands/Record.hpp"


iscore_plugin_cohesion::iscore_plugin_cohesion() :
    QObject {}
{
}

iscore::PluginControlInterface* iscore_plugin_cohesion::make_control(iscore::Presenter* pres)
{
    return new IScoreCohesionControl {pres};
}

QStringList iscore_plugin_cohesion::required() const
{
    return {"Scenario"};
}

std::pair<const std::string, CommandGeneratorMap> iscore_plugin_cohesion::make_commands()
{
    std::pair<const std::string, CommandGeneratorMap> cmds;
    boost::mpl::for_each<
            boost::mpl::list<
            CreateCurvesFromAddresses,
            CreateCurvesFromAddressesInConstraints,
            Record,
            SnapshotStatesMacro
            >,
            boost::type<boost::mpl::_>
            >(CommandGeneratorMapInserter2{cmds.second});

    return cmds;
}
