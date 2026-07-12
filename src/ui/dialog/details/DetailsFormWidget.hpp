#pragma once

#include "../../../core/model/MediaEntry.hpp"
#include <gtkmm/box.h>

class DetailsFormWidget : public Gtk::Box
{
public:
    DetailsFormWidget() = default;
    ~DetailsFormWidget() override = default;

    virtual void loadFrom(const MediaDetails& details) = 0;
    [[nodiscard]] virtual MediaDetails toDetails() const = 0;
};