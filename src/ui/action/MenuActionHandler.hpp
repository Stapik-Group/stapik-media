#pragma once

#include "../../core/model/MediaModel.hpp"

#include "stapik/document/BackupInfo.hpp"

#include <giomm/simpleaction.h>
#include <gtkmm/applicationwindow.h>
#include <sigc++/connection.h>

class MenuActionHandler
{
public:
    explicit MenuActionHandler(Gtk::ApplicationWindow& window, MediaModel& model);
    ~MenuActionHandler();

    MenuActionHandler(const MenuActionHandler&) = delete;
    MenuActionHandler& operator=(const MenuActionHandler&) = delete;

    void registerActions();
private:
    Gtk::ApplicationWindow& m_window;
    MediaModel& m_model;
    Glib::RefPtr<Gio::SimpleAction> m_disconnectAction;
    sigc::connection m_outcomeConnection;
    sigc::connection m_statusConnection;
    bool m_connectFeedbackPending = false;

    void onActionConnect();
    void onActionDisconnect();
    void onActionQuit() const;
    void onActionSync() const;
    void onActionRestoreBackup() const;
    void handleChosenBackup(const stapik::document::BackupInfo& backup) const;
    void handleConfirmedConfig(const CloudStorageConfig& config);
    void showConnectFeedback(const MediaModel::CloudOutcome& outcome);
};
