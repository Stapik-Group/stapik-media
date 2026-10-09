#pragma once
#include <gtkmm/applicationwindow.h>

#include "../../core/model/MediaModel.hpp"

class MenuActionHandler
{
public:
    explicit MenuActionHandler(Gtk::ApplicationWindow& window, MediaModel& model);
    ~MenuActionHandler() = default;

    void registerActions();
private:
    Gtk::ApplicationWindow& m_window;
    MediaModel& m_model;

    void onActionConnect() const;
    void onActionQuit() const;
    void onActionSync() const;
};
