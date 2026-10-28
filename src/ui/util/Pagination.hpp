#pragma once

#include <cstddef>

namespace Pagination
{
    struct PageSlice
    {
        std::size_t page;
        std::size_t totalPages;
        std::size_t first;
        std::size_t last;
    };

    [[nodiscard]] PageSlice sliceFor(std::size_t itemCount, std::size_t requestedPage, std::size_t pageSize);
}
