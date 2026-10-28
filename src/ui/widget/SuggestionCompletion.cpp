#include "SuggestionCompletion.hpp"

#include <gtkmm/entrycompletion.h>
#include <gtkmm/liststore.h>

namespace
{
    class SuggestionColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        SuggestionColumns()
        {
            add(text);
        }

        Gtk::TreeModelColumn<Glib::ustring> text;
    };
}

namespace SuggestionCompletion
{
    void attach(Gtk::Entry& entry, const std::vector<std::string>& suggestions)
    {
        const SuggestionColumns columns;
        const auto store = Gtk::ListStore::create(columns);

        for (const auto& suggestion : suggestions)
            (*store->append())[columns.text] = suggestion;

        const auto completion = Gtk::EntryCompletion::create();
        completion->set_model(store);
        completion->set_text_column(columns.text);
        completion->set_minimum_key_length(1);

        entry.set_completion(completion);
    }
}
