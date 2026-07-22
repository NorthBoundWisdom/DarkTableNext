#include <gtest/gtest.h>

#include "ravo/engine/engine.h"

#include "test_support.h"

namespace ravo
{
namespace
{

class RecordingProgressSink final : public ProgressSink
{
public:
    void on_progress(const ProgressEvent &event) override
    {
        events.push_back(event);
    }

    std::vector<ProgressEvent> events;
};

TEST(EngineFacadeTest, ExposesExactlyTheReservedPhaseOneDescriptors)
{
    const auto engine = EngineFacade::create_phase1();

    ASSERT_TRUE(engine) << engine.error().message;
    ASSERT_EQ(engine.value().operations().size(), 7U);
    EXPECT_EQ(engine.value().operations().front().id, "ravo.core.identity");
    EXPECT_EQ(engine.value().operations().back().id, "ravo.output.scale");
}

TEST(EngineFacadeTest, CancelledRequestsNeverReachRendering)
{
    const auto engine = EngineFacade::create_phase1();
    ASSERT_TRUE(engine) << engine.error().message;
    CancellationSource cancellation;
    ASSERT_TRUE(cancellation.cancel("test_cancel"));

    RenderRequest request;
    request.asset = test::valid_recipe().asset;
    request.recipe = test::valid_recipe();
    request.cancellation = cancellation.token();
    const auto rendered = engine.value().render(request);

    ASSERT_FALSE(rendered);
    EXPECT_EQ(rendered.error().code, ErrorCode::kCancelled);
}

TEST(EngineFacadeTest, ValidatedRenderStopsAtTheDeclaredPhaseTwoBoundary)
{
    const auto engine = EngineFacade::create_phase1();
    ASSERT_TRUE(engine) << engine.error().message;
    RecordingProgressSink progress;

    RenderRequest request;
    request.asset = test::valid_recipe().asset;
    request.recipe = test::valid_recipe();
    request.correlation_id = "request-1";
    const auto rendered = engine.value().render(request, &progress);

    ASSERT_FALSE(rendered);
    EXPECT_EQ(rendered.error().code, ErrorCode::kUnsupported);
    ASSERT_EQ(progress.events.size(), 1U);
    EXPECT_EQ(progress.events.front().stage, "validation_complete");
}

} // namespace
} // namespace ravo
