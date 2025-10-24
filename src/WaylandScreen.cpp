#include "WaylandScreen.hpp"

#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <cstdint>
#include <qcontainerfwd.h>
#include <qsize.h>
#include <qstringliteral.h>
#include <qtclasshelpermacros.h>
#include <qglobalstatic.h>
#include <vector>
#include <wayland-client.h>

class CWaylandScreen : public QtWayland::wl_output {
  public:
    explicit CWaylandScreen(struct ::wl_registry* registry, uint32_t id, uint32_t version);
    ~CWaylandScreen() override;
    Q_DISABLE_COPY_MOVE(CWaylandScreen);
    SWaylandScreenInfo info;

  protected:
    void output_mode(uint32_t flags, int32_t width, int32_t height, int32_t refresh) override;
    void output_name(const QString& name) override;
};

struct RegistryData {
    std::vector<CWaylandScreen*> screens;
    struct ::wl_registry* registry;
};

static void registry_global(void* data, struct ::wl_registry* registry, uint32_t id, const char* interface, uint32_t version) {
    auto* regData = static_cast<RegistryData*>(data);
    
    if (strcmp(interface, "wl_output") == 0) {
        regData->screens.emplace_back(new CWaylandScreen(registry, id, version));
    }
}

static void registry_global_remove(void* data, struct ::wl_registry* registry, uint32_t id) {
    // Not needed for our use case
}

static const struct wl_registry_listener registry_listener = {
    registry_global,
    registry_global_remove
};

std::vector<SWaylandScreenInfo> SWaylandScreenInfo::enumerateScreens() {
    auto* nativeApp = QGuiApplication::platformNativeInterface();
    if (!nativeApp)
        return {};
    
    auto* display = static_cast<struct ::wl_display*>(
        nativeApp->nativeResourceForIntegration("wl_display"));
    
    if (!display)
        return {};
    
    RegistryData regData;
    regData.registry = wl_display_get_registry(display);
    if (!regData.registry)
        return {};
    
    wl_registry_add_listener(regData.registry, &registry_listener, &regData);
    wl_display_roundtrip(display);
    
    std::vector<SWaylandScreenInfo> info;
    for (auto* screen : regData.screens) {
        info.push_back(screen->info);
        delete screen;
    }
    
    wl_registry_destroy(regData.registry);
    
    return info;
}

CWaylandScreen::CWaylandScreen(struct ::wl_registry* registry, uint32_t id, uint32_t version) 
    : QtWayland::wl_output(registry, id, qMin(version, 4u)) {
}

CWaylandScreen::~CWaylandScreen() {
    release();
}

void CWaylandScreen::output_mode(uint32_t flags, int width, int height, int refresh) {
    info.pixelSize = QSize(width, height);
}

void CWaylandScreen::output_name(const QString& name) {
    info.name = name;
}
