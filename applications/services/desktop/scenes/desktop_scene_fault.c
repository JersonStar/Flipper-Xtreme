#include <furi_hal.h>

#include "../desktop_i.h"
#include "../../../settings/desktop_settings/desktop_settings_app.h"

#define DesktopFaultEventExit 0x00FF00FF

void desktop_scene_fault_callback(void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopFaultEventExit);
}

void desktop_scene_fault_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;

    DesktopSettings* settings = malloc(sizeof(DesktopSettings));
    DESKTOP_SETTINGS_LOAD(settings);

    Popup* popup = desktop->hw_mismatch_popup;
    popup_set_context(popup, desktop);
    if(settings->sfw_mode) {
        popup_set_header(
            popup,
            "Flipper crashed\n but has been rebooted",
            60,
            14 + STATUS_BAR_Y_SHIFT,
            AlignCenter,
            AlignCenter);
    } else {
        popup_set_header(
            popup,
            "Slut passed out\n but is now back",
            60,
            14 + STATUS_BAR_Y_SHIFT,
            AlignCenter,
            AlignCenter);
    }

    char* message = (char*)furi_hal_rtc_get_fault_data();
    popup_set_text(popup, message, 60, 37 + STATUS_BAR_Y_SHIFT, AlignCenter, AlignCenter);
    popup_set_callback(popup, desktop_scene_fault_callback);
    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdHwMismatch);
    free(settings);
}

bool desktop_scene_fault_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopFaultEventExit:
            scene_manager_previous_scene(desktop->scene_manager);
            consumed = true;
            break;
        default:
            break;
        }
    }

    return consumed;
}

void desktop_scene_fault_on_exit(void* context) {
    UNUSED(context);
    furi_hal_rtc_set_fault_data(0);
}
