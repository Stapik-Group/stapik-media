#include "ui/util/ConsumptionText.hpp"

#include <gtest/gtest.h>

TEST(ConsumptionTextTest, ScreenCategoriesShareTheViewingWording)
{
    EXPECT_EQ(ConsumptionText::key("list.entry.consumed", MediaCategory::Movie), "list.entry.consumed.watched");
    EXPECT_EQ(ConsumptionText::key("list.entry.consumed", MediaCategory::Series), "list.entry.consumed.watched");
    EXPECT_EQ(ConsumptionText::key("list.entry.consumed", MediaCategory::Cartoon), "list.entry.consumed.watched");
}

TEST(ConsumptionTextTest, EveryOtherCategoryHasItsOwnWording)
{
    EXPECT_EQ(ConsumptionText::key("dialog.entry.consumedDate.label", MediaCategory::Book), "dialog.entry.consumedDate.label.read");
    EXPECT_EQ(ConsumptionText::key("dialog.entry.consumedDate.label", MediaCategory::Album), "dialog.entry.consumedDate.label.listened");
    EXPECT_EQ(ConsumptionText::key("dialog.entry.consumedDate.label", MediaCategory::Game), "dialog.entry.consumedDate.label.completed");
}
