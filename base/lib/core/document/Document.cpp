#include <core/document/Document.hpp>
#include <core/document/DocumentModel.hpp>
#include <core/document/DocumentView.hpp>
#include <core/document/DocumentPresenter.hpp>

#include <iscore/plugins/panel/PanelFactory.hpp>
#include <iscore/plugins/panel/PanelPresenter.hpp>
#include <iscore/plugins/panel/PanelModel.hpp>

#include <iscore/plugins/documentdelegate/DocumentDelegateFactoryInterface.hpp>
#include <iscore/plugins/documentdelegate/DocumentDelegatePresenterInterface.hpp>

#include <core/application/OpenDocumentsFile.hpp>
#include <QLayout>
#include <QStandardPaths>
#include <QSettings>

using namespace iscore;
Document::Document(DocumentDelegateFactoryInterface* factory,
                   QWidget* parentview,
                   QObject* parent) :
    NamedObject {"Document", parent},
    m_objectLocker{this},
    m_backupMgr{new DocumentBackupManager{*this}}
{
    /// Construction of the document model

    // Note : we have to separate allocation
    // because the model delegates init might call IDocument::path()
    // which requires the pointer to m_model to be intialized.
    std::allocator<DocumentModel> allocator;
    m_model = allocator.allocate(1);
    allocator.construct(m_model, factory, this);
    m_view = new DocumentView{factory, this, parentview};
    m_presenter = new DocumentPresenter{factory,
            m_model,
            m_view,
            this};

    init();
}

void Document::init()
{
    con(m_selectionStack, &SelectionStack::currentSelectionChanged,
            this, [&] (const Selection& s)
            {
                for(auto& panel : m_model->panels())
                {
                    panel->setNewSelection(s);
                }
                m_model->setNewSelection(s);
            });

}

void Document::setBackupMgr(DocumentBackupManager* backupMgr)
{
    m_backupMgr = backupMgr;
}

Document::~Document()
{
    // We need a custom destructor because
    // for the sake of simplicity, we want the presenter
    // to be deleted before the model.
    // (Else we would have to fine-grain the deletion of the selection stack).

    delete m_presenter;
    delete m_view;
}

void Document::setupNewPanel(PanelFactory* factory)
{
    m_model->addPanel(factory->makeModel(m_model));
}

void Document::bindPanelPresenter(PanelPresenter* pres)
{
    using namespace std;
    auto localmodel = std::find_if(begin(model().panels()),
                                   end(model().panels()),
                                   [&] (PanelModel* model)
    {
        return model->panelId() == pres->panelId();
    });
    ISCORE_ASSERT(localmodel != end(model().panels()));

    pres->setModel(*localmodel);
}
