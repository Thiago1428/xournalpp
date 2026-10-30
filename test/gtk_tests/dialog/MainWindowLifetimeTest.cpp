#include <array>

#include "control/Control.h"
#include "control/XournalMain.h"
#include "gui/GladeSearchpath.h"
#include "gui/MainWindow.h"

#include "GtkTest.h"
#include "config-test.h"

namespace {
class MainWindowLifetimeTest: public GtkTest {
    void runTest(GtkApplication* application) override {
        XournalMain::initLocalisation();
        GladeSearchpath resources;
        resources.addSearchDirectory(GET_UI_FOLDER);
        resources.addSearchDirectory(GET_RESOURCE_TEMPLATE_FOLDER);
        auto* settings = gtk_settings_get_default();
        ASSERT_NE(settings, nullptr);
        for (int iteration = 0; iteration < 2; ++iteration) {
            SCOPED_TRACE(iteration);
            std::array<gulong, 2> handlers{};
            {
                Control control(G_APPLICATION(application), &resources);
                MainWindow window(&resources, &control, application);
                control.initWindow(&window, true);
                window.populate(&resources);
                control.enableAutosave(false);
                size_t i = 0;
                for (const char* detail: {"gtk-theme-name", "gtk-application-prefer-dark-theme"}) {
                    handlers[i] = g_signal_handler_find(
                            settings, static_cast<GSignalMatchType>(G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_DETAIL), 0,
                            g_quark_from_string(detail), nullptr, nullptr, &window);
                    EXPECT_NE(handlers[i++], 0);
                }
            }
            for (auto handler: handlers) {
                EXPECT_FALSE(g_signal_handler_is_connected(settings, handler))
                        << "Destroyed windows must not receive GtkSettings theme notifications";
            }
            for (int pending = 0; pending < 100 && g_main_context_iteration(nullptr, false); ++pending) {}
        }
    }
};
}  // namespace

TEST_F(MainWindowLifetimeTest, disconnectsThemeCallbacksWhenWindowIsDestroyed) {}
