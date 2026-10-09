#pragma once
#include <gtkmm/applicationwindow.h>

#include "../../core/model/MediaModel.hpp"

#include "stapik/cloud/CloudStorageConfig.hpp"


class MenuActionHandler
{
public:
    explicit MenuActionHandler(Gtk::ApplicationWindow& window, MediaModel& model);
    ~MenuActionHandler() = default;

    void registerActions();
private:
    static constexpr auto MEDIA_FILENAME = "media.json";

    Gtk::ApplicationWindow& m_window;
    MediaModel& m_model;

    void onActionConnect() const;
    void onActionQuit() const;
    void onActionSync() const;

    void handleConnectResult(const CloudStorageConfig& config) const;
    void applyCloudConfig(const CloudStorageConfig& config) const;
};
