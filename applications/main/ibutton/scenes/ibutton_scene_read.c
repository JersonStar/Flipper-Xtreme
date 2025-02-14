#include "../ibutton_i.h"
#include <dolphin/dolphin.h>
#include "../../../settings/desktop_settings/desktop_settings_app.h"

static void ibutton_scene_read_callback(void* context) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventWorkerRead);
}

void ibutton_scene_read_on_enter(void* context) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;
    iButtonKey* key = ibutton->key;
    iButtonWorker* worker = ibutton->key_worker;
    DesktopSettings* settings = malloc(sizeof(DesktopSettings));
    DESKTOP_SETTINGS_LOAD(settings);

    popup_set_header(popup, "iButton", 95, 26, AlignCenter, AlignBottom);
    popup_set_text(popup, "Waiting\nfor key ...", 95, 30, AlignCenter, AlignTop);
    if(settings->sfw_mode) {
        popup_set_icon(popup, 0, 5, &I_DolphinWait_61x59_sfw);
    } else {
        popup_set_icon(popup, 0, 5, &I_DolphinWait_61x59);
    }

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewPopup);
    furi_string_set(ibutton->file_path, IBUTTON_APP_FOLDER);

    ibutton_worker_read_set_callback(worker, ibutton_scene_read_callback, ibutton);
    ibutton_worker_read_start(worker, key);

    ibutton_notification_message(ibutton, iButtonNotificationMessageReadStart);
    free(settings);
}

bool ibutton_scene_read_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    SceneManager* scene_manager = ibutton->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == iButtonCustomEventWorkerRead) {
            bool success = false;
            iButtonKey* key = ibutton->key;

            if(ibutton_key_get_type(key) == iButtonKeyDS1990) {
                if(!ibutton_key_dallas_crc_is_valid(key)) {
                    scene_manager_next_scene(scene_manager, iButtonSceneReadCRCError);
                } else if(!ibutton_key_dallas_is_1990_key(key)) {
                    scene_manager_next_scene(scene_manager, iButtonSceneReadNotKeyError);
                } else {
                    success = true;
                }
            } else {
                success = true;
            }

            if(success) {
                ibutton_notification_message(ibutton, iButtonNotificationMessageSuccess);
                scene_manager_next_scene(scene_manager, iButtonSceneReadSuccess);
                DOLPHIN_DEED(DolphinDeedIbuttonReadSuccess);
            }
        }
    }

    return consumed;
}

void ibutton_scene_read_on_exit(void* context) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;
    ibutton_worker_stop(ibutton->key_worker);
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);

    ibutton_notification_message(ibutton, iButtonNotificationMessageBlinkStop);
}
