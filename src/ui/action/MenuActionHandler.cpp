#include "MenuActionHandler.hpp"

#include "stapik/app/AppContext.hpp"
#include "stapik/locale/LocaleManager.hpp"
#include "stapik/storage/CloudStorageConfigStorage.hpp"
#include "stapik/ui/dialog/ConfirmDialog.hpp"
#include "stapik/ui/dialog/ConnectDialog.hpp"
#include "stapik/ui/dialog/DialogUtils.hpp"
#include "stapik/ui/dialog/RestoreBackupDialog.hpp"

#include <format>
#include <string>

namespace
{
    std::string connectedSecondaryText()
    {
        const auto& loc = LocaleManager::instance();
        const auto& applicationName = stapik::app::AppContext::instance().info().internalName;
        auto text = loc.translate("cloud.connected.secondary");

        if (CloudStorageConfigStorage::secretsKeptInConfigFile(applicationName).any())
            text = std::format("{}\n\n{}", text, loc.translate("cloud.plainFileSecrets"));

        return text;
    }
}

MenuActionHandler::MenuActionHandler(Gtk::ApplicationWindow& window, MediaModel& model):
    m_window(window),
    m_model(model) {}

MenuActionHandler::~MenuActionHandler()
{
    m_outcomeConnection.disconnect();
    m_statusConnection.disconnect();
}

void MenuActionHandler::registerActions()
{
    const auto connectAction = m_window.add_action("connect", sigc::mem_fun(*this, &MenuActionHandler::onActionConnect));
    m_disconnectAction = m_window.add_action("disconnect", sigc::mem_fun(*this, &MenuActionHandler::onActionDisconnect));
    m_window.add_action("quit", sigc::mem_fun(*this, &MenuActionHandler::onActionQuit));
    const auto syncAction = m_window.add_action("sync", sigc::mem_fun(*this, &MenuActionHandler::onActionSync));
    const auto restoreBackupAction = m_window.add_action("restoreBackup", sigc::mem_fun(*this, &MenuActionHandler::onActionRestoreBackup));

    connectAction->set_enabled(m_model.isWritable());
    syncAction->set_enabled(m_model.isWritable());
    restoreBackupAction->set_enabled(m_model.isWritable());

    m_disconnectAction->set_enabled(m_model.hasCloudConnection());
    m_statusConnection = m_model.signalSyncStatusChanged().connect([this](const stapik::sync::SyncStatus) { m_disconnectAction->set_enabled(m_model.hasCloudConnection()); });
    m_outcomeConnection = m_model.signalCloudOutcome().connect(sigc::mem_fun(*this, &MenuActionHandler::showConnectFeedback));
}

void MenuActionHandler::onActionConnect()
{
    showConnectDialog(m_window, m_model.cloudConfig(), [this](const CloudStorageConfig& config) { handleConfirmedConfig(config); });
}

void MenuActionHandler::onActionDisconnect()
{
    const auto& loc = LocaleManager::instance();

    showConfirmDialog(
        m_window,
        ConfirmDialogOptions{
            .title = loc.translate("dialog.disconnect.title"),
            .message = loc.translate("dialog.disconnect.message"),
            .confirmLabel = loc.translate("dialog.disconnect.button"),
            .destructive = true
        },
        [this] { m_model.disconnectCloud(); });
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

void MenuActionHandler::handleConfirmedConfig(const CloudStorageConfig& config)
{
    if (!m_model.connectCloud(config))
    {
        const auto& loc = LocaleManager::instance();
        showMessageDialog(m_window, loc.translate("cloud.failed.header"), loc.translate("sync.status.error"));
        return;
    }

    m_connectFeedbackPending = true;
}

void MenuActionHandler::showConnectFeedback(const MediaModel::CloudOutcome& outcome)
{
    if (!m_connectFeedbackPending)
        return;

    m_connectFeedbackPending = false;

    const auto& loc = LocaleManager::instance();
    using stapik::sync::SyncState;

    switch (outcome.state)
    {
        case SyncState::Synchronized:
        case SyncState::ServerWon:
            showMessageDialog(m_window, loc.translate("cloud.connected"), connectedSecondaryText());
            break;
        case SyncState::OfflineKeptLocal:
        case SyncState::RemoteUnreadable:
        case SyncState::AuthRejected:
        case SyncState::DecryptionFailed:
        case SyncState::DocumentTooLarge:
            showMessageDialog(m_window, loc.translate("cloud.failed.header"), outcome.message);
            break;
        case SyncState::LostRaceAcceptedServer:
        case SyncState::LostRaceKeptLocal:
            break;
    }
}
