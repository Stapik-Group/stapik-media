#include "MenuActionHandler.hpp"

#include "stapik/locale/LocaleManager.hpp"
#include "stapik/ui/dialog/ConnectDialog.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"
#include "stapik/ui/dialog/RestoreBackupDialog.hpp"

MenuActionHandler::MenuActionHandler(Gtk::ApplicationWindow& window, MediaModel& model):
    m_window(window),
    m_model(model) {}

void MenuActionHandler::registerActions()
{
    m_window.add_action("connect", sigc::mem_fun(*this, &MenuActionHandler::onActionConnect));
    m_window.add_action("quit", sigc::mem_fun(*this, &MenuActionHandler::onActionQuit));
    m_window.add_action("sync", sigc::mem_fun(*this, &MenuActionHandler::onActionSync));
    m_window.add_action("restoreBackup", sigc::mem_fun(*this, &MenuActionHandler::onActionRestoreBackup));
}

void MenuActionHandler::onActionConnect() const
{
    showConnectDialog(m_window, m_model.cloudConfig(), [this](const CloudStorageConfig& config)
    {
        if (m_model.connectCloud(config))
            return;

        const auto& loc = LocaleManager::instance();
        showMessageDialog(m_window, loc.translate("cloud.failed.header"), loc.translate("sync.status.error"));
    });
}

void MenuActionHandler::onActionQuit() const
{
    m_window.close();
}

void MenuActionHandler::onActionSync() const
{
    m_model.syncNow();
}

void MenuActionHandler::onActionRestoreBackup() const
{
    showRestoreBackupDialog(m_window, m_model.listBackups(), [this](const stapik::document::BackupInfo& backup) { handleChosenBackup(backup); });
}

void MenuActionHandler::handleChosenBackup(const stapik::document::BackupInfo& backup) const
{
    if (m_model.restoreBackup(backup))
        return;

    const auto& loc = LocaleManager::instance();
    showMessageDialog(m_window, loc.translate("dialog.backup.restoreFailed.title"), loc.translate("dialog.backup.restoreFailed.text"));
}
