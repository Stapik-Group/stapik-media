#include "MediaEntrySerializer.hpp"

#include "stapik/domain/IdGenerator.hpp"

#include <stdexcept>

namespace
{
    std::string categoryToString(const MediaCategory category)
    {
        switch (category)
        {
            using enum MediaCategory;
            case Movie: return "movie";
            case Series: return "series";
            case Cartoon: return "cartoon";
            case Book: return "book";
            case Album: return "album";
            case Game: return "game";
        }
        throw std::invalid_argument("Unknown MediaCategory");
    }

    MediaCategory categoryFromString(const std::string &str)
    {
        using enum MediaCategory;
        if (str == "movie") return Movie;
        if (str == "series") return Series;
        if (str == "cartoon") return Cartoon;
        if (str == "book") return Book;
        if (str == "album") return Album;
        if (str == "game") return Game;
        throw std::invalid_argument("Unknown category string: " + str);
    }

    std::string precisionToString(const DatePrecision precision)
    {
        switch (precision)
        {
            using enum DatePrecision;
            case Day: return "day";
            case Month: return "month";
            case Year: return "year";
        }
        throw std::invalid_argument("Unknown DatePrecision");
    }

    DatePrecision precisionFromString(const std::string &str)
    {
        using enum DatePrecision;
        if (str == "day") return Day;
        if (str == "month") return Month;
        if (str == "year") return Year;
        throw std::invalid_argument("Unknown precision string: " + str);
    }

    nlohmann::json detailsToJson(const MediaDetails &details)
    {
        return std::visit([]<typename T>(const T &d) -> nlohmann::json
        {
            if constexpr (std::is_same_v<T, ScreenDetails>)
                return {{"director", d.director}, {"genre", d.genre}};
            else if constexpr (std::is_same_v<T, BookDetails>)
                return {{"author", d.author}, {"genre", d.genre}, {"isAudiobook", d.isAudiobook}};
            else if constexpr (std::is_same_v<T, AlbumDetails>)
                return {{"performer", d.performer}, {"publisher", d.publisher}, {"genre", d.genre}};
            else if constexpr (std::is_same_v<T, GameDetails>)
                return {{"studio", d.studio}, {"publisher", d.publisher}, {"platform", d.platform}};
            return {};
        }, details);
    }

    MediaDetails detailsFromJson(const MediaCategory category, const nlohmann::json &j)
    {
        switch (category)
        {
            using enum MediaCategory;
            case Movie:
            case Series:
            case Cartoon:
                return ScreenDetails{
                    j.at("director").get<std::string>(),
                    j.at("genre").get<std::string>()
                };
            case Book:
                return BookDetails{
                    j.at("author").get<std::string>(),
                    j.at("genre").get<std::string>(),
                    j.at("isAudiobook").get<bool>()
                };
            case Album:
                return AlbumDetails{
                    j.at("performer").get<std::string>(),
                    j.at("publisher").get<std::string>(),
                    j.at("genre").get<std::string>()
                };
            case Game:
                return GameDetails{
                    j.at("studio").get<std::string>(),
                    j.at("publisher").get<std::string>(),
                    j.at("platform").get<std::string>()
                };
        }
        throw std::invalid_argument("Unknown MediaCategory in detailsFromJson");
    }
}

nlohmann::json MediaEntrySerializer::toJson(const MediaEntry &entry)
{
    return {
        {"id", entry.id},
        {"category", categoryToString(entry.category)},
        {"title", entry.title},
        {
            "releaseDate", {
                {"date", entry.releaseDate.toIsoDateString()},
                {"precision", precisionToString(entry.releaseDate.precision)}
            }
        },
        {"consumed", {{"month", entry.consumed.month()}, {"year", entry.consumed.year()}}},
        {"details", detailsToJson(entry.details)}
    };
}

MediaEntry MediaEntrySerializer::fromJson(const nlohmann::json &json)
{
    const auto category = categoryFromString(json.at("category").get<std::string>());
    const auto &rd = json.at("releaseDate");
    const auto precision = precisionFromString(rd.at("precision").get<std::string>());

    auto id = json.value("id", std::string{});
    if (id.empty())
        id = stapik::domain::generateId();

    return MediaEntry{
        category,
        json.at("title").get<std::string>(),
        ReleaseDate::fromIsoDateString(rd.at("date").get<std::string>(), precision),
        stapik::domain::YearMonth(
            json.at("consumed").at("year").get<int>(),
            json.at("consumed").at("month").get<int>()
        ),
        detailsFromJson(category, json.at("details")),
        std::move(id)
    };
}
