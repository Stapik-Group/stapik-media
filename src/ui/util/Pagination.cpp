#include "Pagination.hpp"

#include <algorithm>

namespace Pagination
{
    PageSlice sliceFor(const std::size_t itemCount, const std::size_t requestedPage, const std::size_t pageSize)
    {
        const auto size = std::max<std::size_t>(pageSize, 1);
        const auto totalPages = std::max<std::size_t>((itemCount + size - 1) / size, 1);
        const auto page = std::clamp<std::size_t>(requestedPage, 1, totalPages);
        const auto first = (page - 1) * size;
        const auto last = std::min(first + size, itemCount);

        return PageSlice{ .page = page, .totalPages = totalPages, .first = first, .last = last };
    }
}
