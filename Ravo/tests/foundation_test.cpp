#include <chrono>
#include <string>

#include <gtest/gtest.h>

#include "ravo/foundation/cancellation.h"
#include "ravo/foundation/json.h"

namespace ravo
{
namespace
{

TEST(JsonTest, ParsesUnicodeAndWritesStableObjectOrder)
{
    const auto parsed = parse_json(R"({"z":true,"a":"\uD83D\uDE80"})");

    ASSERT_TRUE(parsed) << parsed.error().message;
    EXPECT_EQ(serialize_json(parsed.value()), R"({"a":"🚀","z":true})");
}

TEST(JsonTest, RejectsDuplicateKeysWithPositionContext)
{
    const auto parsed = parse_json(R"({"id":1,"id":2})");

    ASSERT_FALSE(parsed);
    EXPECT_EQ(parsed.error().code, ErrorCode::kValidation);
    EXPECT_TRUE(parsed.error().context.contains("line"));
    EXPECT_TRUE(parsed.error().context.contains("column"));
}

TEST(CancellationTest, FirstCancellationWinsAndTokensKeepTheReason)
{
    CancellationSource source;
    const auto token = source.token();

    EXPECT_TRUE(source.cancel("user_requested"));
    EXPECT_FALSE(source.cancel("ignored"));
    EXPECT_TRUE(token.is_cancellation_requested());
    EXPECT_EQ(token.reason(), "user_requested");

    const auto checked = token.check();
    ASSERT_FALSE(checked);
    EXPECT_EQ(checked.error().code, ErrorCode::kCancelled);
    EXPECT_EQ(checked.error().context.at("reason"), "user_requested");
}

TEST(CancellationTest, ExpiredDeadlineCancelsWithAStructuredReason)
{
    const auto source = CancellationSource::with_deadline(std::chrono::steady_clock::now() -
                                                          std::chrono::milliseconds{1});
    const auto checked = source.token().check();

    ASSERT_FALSE(checked);
    EXPECT_EQ(checked.error().code, ErrorCode::kCancelled);
    EXPECT_EQ(checked.error().context.at("reason"), "deadline_exceeded");
}

} // namespace
} // namespace ravo
