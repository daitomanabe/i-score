#include "ConstraintInspectorWidget.hpp"

#include "DialogWidget/AddProcessDialog.hpp"
#include "Widgets/RackWidget.hpp"
#include "Widgets/DurationSectionWidget.hpp"
#include "Widgets/Rack/RackInspectorSection.hpp"

#include "Document/Constraint/ConstraintModel.hpp"
#include "Document/Constraint/ViewModels/Temporal/TemporalConstraintViewModel.hpp"
#include "Document/Constraint/Rack/RackModel.hpp"
#include "Document/Constraint/Rack/Slot/SlotModel.hpp"
#include "Commands/Constraint/AddProcessToConstraint.hpp"
#include "Commands/Constraint/AddLayerInNewSlot.hpp"
#include "Commands/Constraint/AddRackToConstraint.hpp"
#include "Commands/Constraint/RemoveProcessFromConstraint.hpp"
#include "Document/Event/EventModel.hpp"
#include "Commands/Scenario/ShowRackInViewModel.hpp"
#include "Commands/Scenario/HideRackInViewModel.hpp"
#include "ProcessInterface/ProcessModel.hpp"

#include "Inspector/MetadataWidget.hpp"
#include <Inspector/InspectorWidgetList.hpp>
#include "Document/BaseElement/BaseElementModel.hpp"
#include "Process/ScenarioModel.hpp"

#include <core/document/DocumentModel.hpp>
#include <iscore/document/DocumentInterface.hpp>
#include <core/document/Document.hpp>

#include "Inspector/Separator.hpp"
#include "Inspector/SelectionButton.hpp"
#include "ProcessInterface/State/ProcessStateDataInterface.hpp"
#include <QFrame>
#include <QLineEdit>
#include <QLayout>
#include <QLabel>
#include <QFormLayout>
#include <QToolButton>
#include <QPushButton>

using namespace Scenario::Command;
using namespace iscore;
using namespace iscore::IDocument;


ConstraintInspectorWidget::ConstraintInspectorWidget(
        const ConstraintModel& object,
        iscore::Document& doc,
        QWidget* parent) :
    InspectorWidgetBase{object, doc, parent},
    m_currentConstraint{object}
{
    setObjectName("Constraint");

    ////// HEADER
    // metadata
    m_metadata = new MetadataWidget{
            &m_currentConstraint.metadata,
            commandDispatcher(),
            &m_currentConstraint,
            this};
    m_metadata->setType(ConstraintModel::prettyName());

    m_metadata->setupConnections(&m_currentConstraint);

    addHeader(m_metadata);


    ////// BODY
    QPushButton* setAsDisplayedConstraint = new QPushButton {"Full view", this};
    connect(setAsDisplayedConstraint, &QPushButton::clicked,
            [this]()
    {
        auto& base = get<BaseElementModel> (*documentFromObject(m_currentConstraint));

        base.setDisplayedConstraint(&model());
    });

    m_properties.push_back(setAsDisplayedConstraint);

    // Events
    if(auto scenario = qobject_cast<ScenarioModel*>(m_currentConstraint.parent()))
    {
        m_properties.push_back(makeStatesWidget(scenario));
    }

    // Separator
    m_properties.push_back(new Separator {this});

    // Durations
    m_durationSection = new DurationSectionWidget {this};
    m_properties.push_back(m_durationSection);

    // Separator
    m_properties.push_back(new Separator {this});

    // Processes
    m_processSection = new InspectorSectionWidget("Processes", this);
    m_processSection->setObjectName("Processes");

    m_properties.push_back(m_processSection);

    QWidget* addProc = new QWidget(this);
    QHBoxLayout* addProcLayout = new QHBoxLayout;
    addProcLayout->setContentsMargins(0, 0, 0 , 0);
    addProc->setLayout(addProcLayout);
    // Button
    QToolButton* addProcButton = new QToolButton;
    addProcButton->setText("+");
    addProcButton->setObjectName("addAProcess");

    // Text
    auto addProcText = new QLabel("Add Process");
    addProcText->setStyleSheet(QString("text-align : left;"));

    addProcLayout->addWidget(addProcButton);
    addProcLayout->addWidget(addProcText);
    auto addProcess = new AddProcessDialog {this};

    connect(addProcButton,  &QToolButton::pressed,
            addProcess, &AddProcessDialog::launchWindow);

    m_properties.push_back(addProc);

    connect(addProcess, &AddProcessDialog::okPressed,
            this, &ConstraintInspectorWidget::createProcess);

    // Separator
    m_properties.push_back(new Separator {this});

    // Rackes
    m_rackSection = new InspectorSectionWidget {"Rackes", this};
    m_rackSection->setObjectName("Rackes");
    m_rackSection->expand();

    m_rackWidget = new RackWidget {this};

    m_properties.push_back(m_rackSection);
    m_properties.push_back(m_rackWidget);

    // Plugins
    for(auto& plugdata : m_currentConstraint.pluginModelList.list())
    {
        for(iscore::DocumentDelegatePluginModel* plugin : doc.model().pluginModels())
        {
            auto md = plugin->makeElementPluginWidget(plugdata, this);
            if(md)
            {
                m_properties.push_back(md);
                break;
            }
        }
    }

    updateDisplayedValues();

    // Display data
    updateAreaLayout(m_properties);
}

const ConstraintModel& ConstraintInspectorWidget::model() const
{
    return m_currentConstraint;
}

void ConstraintInspectorWidget::updateDisplayedValues()
{
    // Cleanup the widgets
    for(auto& process : m_processesSectionWidgets)
    {
        m_processSection->removeContent(process);
    }

    m_processesSectionWidgets.clear();

    for(auto& rack_pair : m_rackesSectionWidgets)
    {
        m_rackSection->removeContent(rack_pair.second);
    }

    m_rackesSectionWidgets.clear();

    // Cleanup the connections
    for(auto& connection : m_connections)
    {
        QObject::disconnect(connection);
    }

    m_connections.clear();

    // Constraint interface
    m_connections.push_back(
                con(model(),	&ConstraintModel::processCreated,
                        this,		&ConstraintInspectorWidget::on_processCreated));
    m_connections.push_back(
                con(model(),	&ConstraintModel::processRemoved,
                        this,		&ConstraintInspectorWidget::on_processRemoved));
    m_connections.push_back(
                con(model(),	&ConstraintModel::rackCreated,
                        this,		&ConstraintInspectorWidget::on_rackCreated));
    m_connections.push_back(
                con(model(),	&ConstraintModel::rackRemoved,
                        this,		&ConstraintInspectorWidget::on_rackRemoved));

    m_connections.push_back(
                con(model(), &ConstraintModel::viewModelCreated,
                        this,    &ConstraintInspectorWidget::on_constraintViewModelCreated));
    m_connections.push_back(
                con(model(), &ConstraintModel::viewModelRemoved,
                        this,    &ConstraintInspectorWidget::on_constraintViewModelRemoved));

    // Processes
    for(const auto& process : model().processes())
    {
        displaySharedProcess(process);
    }

    // Rack
    for(const auto& rack : model().racks())
    {
        setupRack(rack);
    }
}

void ConstraintInspectorWidget::createProcess(QString processName)
{
    auto cmd = new AddProcessToConstraint
    {
               iscore::IDocument::unsafe_path(model()),
               processName
};
    emit commandDispatcher()->submitCommand(cmd);
}

void ConstraintInspectorWidget::createRack()
{
    auto cmd = new AddRackToConstraint(
                   iscore::IDocument::unsafe_path(model()));
    emit commandDispatcher()->submitCommand(cmd);
}

void ConstraintInspectorWidget::createLayerInNewSlot(QString processName)
{
    // TODO this will bite us when the name does not contain the id anymore.
    // We will have to stock the id's somewhere.
    auto cmd = new AddLayerInNewSlot(
                   iscore::IDocument::unsafe_path(model()),
                   id_type<Process>(processName.toInt()));

    emit commandDispatcher()->submitCommand(cmd);
}

void ConstraintInspectorWidget::activeRackChanged(QString rack, ConstraintViewModel* vm)
{
    // TODO mettre à jour l'inspecteur si la rack affichée change (i.e. via une commande réseau).
    if (m_rackWidget == nullptr)
        return;

    if(rack == m_rackWidget->hiddenText)
    {
        if(vm->isRackShown())
        {
            auto cmd = new HideRackInViewModel(vm);
            emit commandDispatcher()->submitCommand(cmd);
        }
    }
    else
    {
        bool ok {};
        auto id = id_type<RackModel> (rack.toInt(&ok));

        if(ok)
        {
            auto cmd = new ShowRackInViewModel(vm, id);
            emit commandDispatcher()->submitCommand(cmd);
        }
    }
}

void ConstraintInspectorWidget::displaySharedProcess(const Process& process)
{
    InspectorSectionWidget* newProc = new InspectorSectionWidget(process.processName());

    // Process
    auto processWidget = InspectorWidgetList::makeInspectorWidget(
                process.processName(), process, newProc);
    newProc->addContent(processWidget);

    // Start & end state
    QWidget* stateWidget = new QWidget;
    QFormLayout* stateLayout = new QFormLayout;
    stateLayout->setSpacing(0);
    stateLayout->setContentsMargins(0, 0, 0, 0);
    stateWidget->setLayout(stateLayout);

    if(auto start = process.startState())
    {
        auto startWidg = InspectorWidgetList::makeInspectorWidget(
                    start->stateName(), *start, newProc);
        stateLayout->addRow(tr("Start state"), startWidg);
    }

    if(auto end = process.endState())
    {
        auto endWidg = InspectorWidgetList::makeInspectorWidget(
                    end->stateName(), *end, newProc);
        stateLayout->addRow(tr("End state"), endWidg);
    }
    newProc->addContent(stateWidget);

    // Delete button
    auto deleteButton = new QPushButton{tr("Delete")};
    connect(deleteButton, &QPushButton::pressed, this, [=,id=process.id()] ()
    {
        auto cmd = new RemoveProcessFromConstraint{iscore::IDocument::unsafe_path(model()), id};
        emit commandDispatcher()->submitCommand(cmd);
    });
    newProc->addContent(deleteButton);

    // Global setup
    m_processesSectionWidgets.push_back(newProc);
    m_processSection->addContent(newProc);

    connect(processWidget,   SIGNAL(createViewInNewSlot(QString)),
            this,   SLOT(createLayerInNewSlot(QString)));
}

void ConstraintInspectorWidget::setupRack(const RackModel& rack)
{
    // Display the widget
    RackInspectorSection* newRack = new RackInspectorSection {
            QString{"Rack.%1"} .arg(*rack.id().val()),
            rack,
            this};

    m_rackesSectionWidgets[rack.id()] = newRack;
    m_rackSection->addContent(newRack);
}

QWidget* ConstraintInspectorWidget::makeStatesWidget(ScenarioModel* scenar)
{
    QWidget* eventWid = new QWidget{this};
    QFormLayout* eventLay = new QFormLayout {eventWid};
    eventLay->setVerticalSpacing(0);

    if(auto sst = m_currentConstraint.startState())
    {
        auto btn = SelectionButton::make(
                    &scenar->state(sst),
                    selectionDispatcher(),
                    this);
        eventLay->addRow(tr("Start state"), btn);
    }

    if(auto est = m_currentConstraint.endState())
    {
        auto btn = SelectionButton::make(
                    &scenar->state(est),
                    selectionDispatcher(),
                    this);
        eventLay->addRow(tr("End state"), btn);
    }

    return eventWid;
}

void ConstraintInspectorWidget::on_processCreated(
        QString processName,
        id_type<Process> processId)
{
    updateDisplayedValues();
}

void ConstraintInspectorWidget::on_processRemoved(id_type<Process> processId)
{
    updateDisplayedValues();
}


void ConstraintInspectorWidget::on_rackCreated(id_type<RackModel> rackId)
{
    setupRack(model().rack(rackId));
    m_rackWidget->viewModelsChanged();
}

void ConstraintInspectorWidget::on_rackRemoved(id_type<RackModel> rackId)
{
    auto ptr = m_rackesSectionWidgets[rackId];
    m_rackesSectionWidgets.erase(rackId);

    if(ptr)
    {
        ptr->deleteLater();
    }

    m_rackWidget->viewModelsChanged();
}

void ConstraintInspectorWidget::on_constraintViewModelCreated(id_type<ConstraintViewModel> cvmId)
{
    m_rackWidget->viewModelsChanged();
}

void ConstraintInspectorWidget::on_constraintViewModelRemoved(id_type<ConstraintViewModel> cvmId)
{
    m_rackWidget->viewModelsChanged();
}
