#include <winsock2.h>
#include <ws2tcpip.h>
#include "application_boundary.h"
#include "heap_boundary.h"
#include <new>
#include <map>
#include <string>
extern ApplicationBoundaryPilot* g_applicationBoundaryPilot;
extern int g_factoryZeroInitializeBoundaryPilot;
typedef char ObservedFactoryStorageIsFour[(sizeof(ApplicationBoundaryPilot) == 4) ? 1 : -1];
// Observed export name; cdecl is a chosen zero-argument model, not uniquely proven.
extern "C" __declspec(dllexport) ApplicationBoundaryPilot* __cdecl GetApplication()
{
    if (g_applicationBoundaryPilot == NULL) {
        InitializeHeapManagerPilot();
        ApplicationBoundaryPilot* application;
        AllocatePrivateHeapMemoryPilot(1, sizeof(ApplicationBoundaryPilot),
            reinterpret_cast<void**>(&application), g_factoryZeroInitializeBoundaryPilot);
        if (application != NULL) {
            application = new (application) ApplicationBoundaryPilot;
            g_applicationBoundaryPilot = application;
            return application;
        }
        g_applicationBoundaryPilot = NULL;
    }
    return g_applicationBoundaryPilot;
}
// Real outer-method definitions; their unresolved implementation dependencies
// remain explicit. The compiler emits the four-slot table from virtual ordering.
#include "lifecycle.cpp"
#include "window_handle.cpp"
#include "path_checks.cpp"
#include "path_resolver.cpp"
#include "path_copy.cpp"
#include "path_suffix.cpp"
#include "initialization_error.cpp"
#include "disk_space_dispatch.cpp"
#include "disk_space_fallback.cpp"
#include "disk_space_query.cpp"
#include "startup_window.cpp"
#include "startup_window_stop.cpp"
#include "startup_window_thread.cpp"
#include "startup_window_procedure.cpp"
#include "path_append.cpp"
#include "checked_arithmetic.cpp"
#include "construction_records.cpp"
#include "pointer_owner_updates.cpp"
#include "heap_release.cpp"
#include "numbered_names.cpp"
#include "startup_image.cpp"
#include "main_window_classes.cpp"
#include "main_keyboard_hook.cpp"
#include "monitor_selection.cpp"
#include "window_geometry.cpp"
#include "geometry_error.cpp"
#include "language_module.cpp"
#include "file_enumeration.cpp"
#include "font_resources.cpp"
#include "suffix_registry.cpp"
#include "file_enumeration_open_next.cpp"
#include "alternate_enumeration.cpp"
#include "cab_owner.cpp"
#include "cab_stream.cpp"
#include "cab_image.cpp"
#include "cab_error.cpp"
#include "cab_extraction.cpp"
#include "file_stream_cache.cpp"
#include "file_stream_open.cpp"
#include "file_stream_consumers.cpp"
#include "stream_parser.cpp"
#include "stream_parser_queries.cpp"
#include "stream_parser_writes.cpp"
#include "shutdown_body.cpp"
#include "cab_stream_producers.cpp"
#include "cab_stream_methods.cpp"
#include "cab_buffer_cache.cpp"
#include "cab_object_pools.cpp"
#include "cab_slot_state.cpp"
#include "cab_pool_lock.cpp"
#include "cab_stream_tables.cpp"
#include "window_clip_regions.cpp"
#include "panel_pixel_transfer.cpp"
#include "window_controller_mode.cpp"
#include "dialog_registry.cpp"
#include "resource_strings.cpp"
#include "panel_surface_lifecycle.cpp"
#include "main_window_sizing.cpp"
#include "main_window_close.cpp"
#include "main_window_timer_work.cpp"
#include "window_input_cursor.cpp"
#include "window_input_dispatch.cpp"
#include "main_window_startup_tail.cpp"
#include "main_window_modal.cpp"
#include "main_window_graphics_transition.cpp"
#include "main_window_product_text.cpp"
#include "main_window_command.cpp"
#include "main_window_controller_lifecycle.cpp"
#include "panel_bitmap_draw.cpp"
#include "panel_message_dispatch.cpp"
#include "panel_menu_host.cpp"
#include "panel_window_record.cpp"
#include "panel_menu.cpp"
#include "main_window_profile.cpp"
#include "main_window_create.cpp"
#include "main_window_service_shutdown.cpp"
#include "service_window_shutdown.cpp"
#include "service_client_registry.cpp"
#include "service_client_transport.cpp"
#include "service_client_completion.cpp"
#include "service_client_buffers.cpp"
#include "service_packet_dispatch.cpp"
#include "service_packet_open.cpp"
#include "service_packet_send.cpp"
#include "service_transport_lifecycle.cpp"
#include "service_client_connection.cpp"
#include "service_transport_tree.cpp"
#include "service_client_construction.cpp"
#include "service_transport_listeners.cpp"
#include "service_application_registration.cpp"
#include "service_listener_configuration.cpp"
#include "addon_startup.cpp"
#include "addon_trust_provider.cpp"
#include "addon_trust.cpp"
#include "display_settings.cpp"
#include "panel_window_geometry.cpp"
#include "panel_float_window.cpp"
#include "panel_surface_refresh.cpp"
#include "panel_child_refresh.cpp"
#include "panel_shadow_drag.cpp"
#include "message_loop.cpp"
