#include "EntrySuggestions.hpp"

#include "stapik/text/TextFolding.hpp"
#include "stapik/text/Trim.hpp"

#include <algorithm>
#include <map>
#include <type_traits>
#include <variant>

namespace
{
    enum class Family { Screen, Book, Album, Game };

    Family familyOf(const MediaCategory category)
    {
        switch (category)
        {
            using enum MediaCategory;
            case Movie:
            case Series:
            case Cartoon: return Family::Screen;
            case Book: return Family::Book;
            case Album: return Family::Album;
            case Game: return Family::Game;
        }
        return Family::Screen;
    }

    class Tally
    {
    public:
        void add(const std::string& value)
        {
            const auto trimmed = stapik::text::trim(value);
            if (trimmed.empty())
                return;

            ++m_spellings[stapik::text::foldCase(trimmed)][trimmed];
        }

        [[nodiscard]] std::vector<std::string> ranked() const
        {
            struct Candidate
            {
                std::string key;
                std::string spelling;
                std::size_t total = 0;
            };

            std::vector<Candidate> candidates;
            candidates.reserve(m_spellings.size());

            for (const auto& [key, spellings] : m_spellings)
            {
                Candidate candidate{ .key = key, .spelling = {}, .total = 0 };
                std::size_t best = 0;

                for (const auto& [spelling, count] : spellings)
                {
                    candidate.total += count;
                    if (count > best)
                    {
                        best = count;
                        candidate.spelling = spelling;
                    }
                }

                candidates.push_back(std::move(candidate));
            }

            std::ranges::sort(candidates, [](const Candidate& left, const Candidate& right)
            {
                return left.total != right.total ? left.total > right.total : left.key < right.key;
            });

            std::vector<std::string> result;
            result.reserve(candidates.size());
            for (auto& candidate : candidates)
                result.push_back(std::move(candidate.spelling));

            return result;
        }

    private:
        std::map<std::string, std::map<std::string, std::size_t>> m_spellings;
    };
}

namespace EntrySuggestions
{
    Lists collect(const std::vector<MediaEntry>& entries, const MediaCategory category)
    {
        const auto family = familyOf(category);

        Tally creators;
        Tally publishers;
        Tally genres;
        Tally platforms;

        for (const auto& entry : entries)
        {
            if (familyOf(entry.category) != family)
                continue;

            std::visit([&]<typename T>(const T& details)
            {
                if constexpr (std::is_same_v<T, ScreenDetails>)
                {
                    creators.add(details.director);
                    genres.add(details.genre);
                }
                else if constexpr (std::is_same_v<T, BookDetails>)
                {
                    creators.add(details.author);
                    genres.add(details.genre);
                }
                else if constexpr (std::is_same_v<T, AlbumDetails>)
                {
                    creators.add(details.performer);
                    publishers.add(details.publisher);
                    genres.add(details.genre);
                }
                else
                {
                    creators.add(details.studio);
                    publishers.add(details.publisher);
                    platforms.add(details.platform);
                }
            }, entry.details);
        }

        return Lists{
            .creators = creators.ranked(),
            .publishers = publishers.ranked(),
            .genres = genres.ranked(),
            .platforms = platforms.ranked()
        };
    }
}
