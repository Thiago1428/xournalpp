#include <memory>

#include "control/ClipboardHandler.h"

#include "GtkTest.h"

namespace {
class Listener: public ClipboardListener {
public:
    bool active = true;
    unsigned lateCallbacks = 0;
    void clipboardCutCopyEnabled(bool) override {}
    void clipboardPasteEnabled(bool) override { lateCallbacks += active ? 0U : 1U; }
    void clipboardPasteText(std::string) override { lateCallbacks += active ? 0U : 1U; }
    void clipboardPasteImage(GdkPixbuf*) override { lateCallbacks += active ? 0U : 1U; }
    void clipboardPasteXournal(ObjectInputStream&) override { lateCallbacks += active ? 0U : 1U; }
    void deleteSelection() override {}
};

class ClipboardLifetimeTest: public GtkTest {
    void runTest(GtkApplication* application) override {
        auto* window = gtk_application_window_new(application);
        Listener listener;
        for (int i = 0; i < 16; ++i) {
            listener.active = true;
            auto handler = std::make_unique<ClipboardHandler>(&listener, window);
            if (i % 2 != 0) {
                // Cover both initial target queries and paste dispatch from refreshed targets.
                EXPECT_TRUE(handler->paste());
            }
            handler.reset();
            listener.active = false;
            // Requests inspect available target types; clipboard contents are not changed.
            for (int pass = 0; pass < 10; ++pass) {
                for (int pending = 0; pending < 100 && g_main_context_iteration(nullptr, false); ++pending) {}
                g_usleep(1000);
            }
            EXPECT_EQ(listener.lateCallbacks, 0);
        }
    }
};
}  // namespace

TEST_F(ClipboardLifetimeTest, ignoresPendingResultsAfterHandlerDestruction) {}
