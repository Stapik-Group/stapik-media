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
    m_model.insertEntry(m_model.entries().size(), m_entry);
}

void AddEntryCommand::undo()
{
    std::ignore = m_model.eraseEntry(m_entry.id);
}

std::string AddEntryCommand::description() const
{
    return LocaleManager::instance().translate("command.entry.add");
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

RemoveEntryCommand::RemoveEntryCommand(MediaModel& model, MediaEntry entry, const std::size_t position) :
    m_model(model),
    m_entry(std::move(entry)),
    m_position(position)
{}

void RemoveEntryCommand::execute()
{
    std::ignore = m_model.eraseEntry(m_entry.id);
}

void RemoveEntryCommand::undo()
{
    m_model.insertEntry(m_position, m_entry);
}

std::string RemoveEntryCommand::description() const
{
    return LocaleManager::instance().translate("command.entry.remove");
}
