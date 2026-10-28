#pragma once

#include "../model/MediaEntry.hpp"
#include "../model/MediaModel.hpp"

#include "stapik/command/ICommand.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

class AddEntryCommand final : public stapik::command::ICommand
{
public:
    AddEntryCommand(MediaModel& model, MediaEntry entry);

    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;

private:
    MediaModel& m_model;
    MediaEntry m_entry;
};

class ImportEntriesCommand final : public stapik::command::ICommand
{
public:
    ImportEntriesCommand(MediaModel& model, std::vector<MediaEntry> entries);

    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;

private:
    MediaModel& m_model;
    std::vector<MediaEntry> m_entries;
};

class UpdateEntryCommand final : public stapik::command::ICommand
{
public:
    UpdateEntryCommand(MediaModel& model, MediaEntry before, MediaEntry after);

    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;

private:
    MediaModel& m_model;
    MediaEntry m_before;
    MediaEntry m_after;
};

class RemoveEntryCommand final : public stapik::command::ICommand
{
public:
    RemoveEntryCommand(MediaModel& model, MediaEntry entry);

    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;

private:
    MediaModel& m_model;
    MediaEntry m_entry;
    std::optional<std::size_t> m_position;
};
