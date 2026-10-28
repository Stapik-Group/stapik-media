#pragma once

#include <gtkmm/entry.h>

#include <string>
#include <vector>

namespace SuggestionCompletion
{
    void attach(Gtk::Entry& entry, const std::vector<std::string>& suggestions);
}
