#include "EntryCommands.hpp"

#include "stapik/locale/LocaleManager.hpp"

#include <tuple>
#include <utility>

AddEntryCommand::AddEntryCommand(MediaModel& model, MediaEntry entry) :
    m_model(model),
    m_entry(std::move(entry))
{}

void AddEntryCommand::execute()
{
    m_model.insertEntry(m_entry);
}

void AddEntryCommand::undo()
{
    std::ignore = m_model.eraseEntry(m_entry.id);
}

std::string AddEntryCommand::description() const
{
    return LocaleManager::instance().translate("command.entry.add");
}

ImportEntriesCommand::ImportEntriesCommand(MediaModel& model, std::vector<MediaEntry> entries) :
    m_model(model),
    m_entries(std::move(entries))
{}

void ImportEntriesCommand::execute()
{
    m_model.insertEntries(m_entries);
}

void ImportEntriesCommand::undo()
{
    std::vector<std::string> ids;
    ids.reserve(m_entries.size());
    for (const auto& entry : m_entries)
        ids.push_back(entry.id);

    m_model.eraseEntries(ids);
}

std::string ImportEntriesCommand::description() const
{
    return LocaleManager::instance().translate("command.entry.import");
}

UpdateEntryCommand::UpdateEntryCommand(MediaModel& model, MediaEntry before, MediaEntry after) :
    m_model(model),
    m_before(std::move(before)),
    m_after(std::move(after))
{}

void UpdateEntryCommand::execute()
{
    std::ignore = m_model.replaceEntry(m_after);
}

void UpdateEntryCommand::undo()
{
    std::ignore = m_model.replaceEntry(m_before);
}

std::string UpdateEntryCommand::description() const
{
    return LocaleManager::instance().translate("command.entry.edit");
}

RemoveEntryCommand::RemoveEntryCommand(MediaModel& model, MediaEntry entry) :
    m_model(model),
    m_entry(std::move(entry))
{}

void RemoveEntryCommand::execute()
{
    m_position = m_model.eraseEntry(m_entry.id);
}

void RemoveEntryCommand::undo()
{
    m_model.insertEntry(m_entry, m_position);
}

std::string RemoveEntryCommand::description() const
{
    return LocaleManager::instance().translate("command.entry.remove");
}
