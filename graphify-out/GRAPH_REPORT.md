# Graph Report - /workspace  (2026-07-25)

## Corpus Check
- Large corpus: 735 files · ~1,060,335 words. Semantic extraction will be expensive (many Claude tokens). Consider running on a subfolder.

## Summary
- 3706 nodes · 7023 edges · 215 communities (202 shown, 13 thin omitted)
- Extraction: 85% EXTRACTED · 15% INFERRED · 0% AMBIGUOUS · INFERRED: 1022 edges (avg confidence: 0.81)
- Token cost: 877,699 input · 0 output

## Community Hubs (Navigation)
- Selection Widget Geometry
- General Settings Tab
- Capture Widget Signals
- Screenshot Saving & Clipboard
- Google Drive OAuth
- Overlay Messages & Spinner
- Flameshot Application Singleton
- Daemon & D-Bus Service
- Google Drive Uploader
- Invert & Pixelate Tools
- Capture Tool Painting
- Upload Widget Base
- Update Notification Widget
- Utility Side Panel
- Arrow Tool
- Capture Request Value Object
- Pin Widget Gestures
- General Config Initializers
- Config Change Signals
- Screen Grabbing Backends
- Capture Tool Object Store
- Color Grab Widget
- Text Tool Configuration
- Side Panel Widget
- Shortcuts Settings Widget
- Path Tool Base
- Config Handler Implementation
- Color Preset Editor
- Command Line Parser
- Capture Tool Button
- Two Point Tool Base
- Circle Counter Tool
- CLI Command Options
- Capture Widget Event Handling
- Filename Editor Tab
- Tray Icon & Launcher
- Visuals Settings Editor
- Text Input Widget
- Toolbar Button Handler
- UI Color Editor
- Magnifier Widget
- App Launcher Widget
- Upload History Store
- Marker Tool
- Notifier Box
- CLI Command Arguments
- Configuration Window
- Capture Context Drawing
- Config Value Handlers
- Notification Widget
- App Launcher Tool
- Pencil Tool
- Pin Tool
- Text Tool Declarations
- Image Label Widget
- Upload History Widget
- Imgur Uploader
- Abstract Logger
- Uploader Backend Manager
- Selection Tool
- Config Docs & Source Groups
- Circle Tool
- Upload Tool Button
- Line Tool
- Rectangle Tool
- Desktop App Metadata
- Toolbar Button Positioning
- Global Shortcut Filter
- Capture Resize & Open-With
- Action Tool Base
- Move Tool
- Upload Confirmation Dialog
- Set Shortcut Dialog
- Button List View
- Accept Tool
- Copy Tool
- Exit Tool
- Redo Tool
- Save Tool
- Size Decrease Tool
- Size Increase Tool
- Undo Tool
- Linux Desktop File Parser
- Capture Button Styling
- Unix Signal Handling
- App Launcher List Population
- Monitor Preview Widget
- Windows Shortcut Parser
- Draggable Widget Helper
- Interface Settings Screenshots
- Annotation Toolbar Demo
- Text Tool Drawing
- Value Handler Types
- Orientable Push Button
- D-Bus Adapter Interface
- Extended Slider Widget
- Terminal Launcher
- Capture Tool State Machine
- Daemon Architecture Docs
- Release 12.0 & Source Groups
- Launcher Item Delegate
- Config Error Resolver
- Active Tool Size & Color
- Color Picker Popup
- Color Palette Definitions
- Side Panel Color Grabbing
- In-Place Annotation Editor Demo
- Wayland Portal Setup
- Strftime Chooser Widget
- Upload History Line Item
- UI Color Editor Logic
- Button List Config Type
- Screen Capture & Logging Concerns
- Text Alignment Screenshot
- Layer Z-Order Demo
- Capture Sidebar Demo
- Imgur Build Gates & 12.1
- Release 0.8 & CLI Refactor
- Upload Dialog Recipients
- System Notifications
- Color Picker Grid Painting
- Hover Event Filter
- RFC Process & Opacity Slider
- Daemon Mode & Project Structure
- Drive Backend Selection Docs
- Colorpicker Editor Screenshot
- Keybinding Cheatsheet Overlay
- Magnifier Shortcut Overlay Demo
- Release & Deployment Pipeline
- Qt Style Override
- Clickable Label
- Color Preset Drag State
- Monitor Preview Mouse Events
- Capture Widget Construction
- Tool Abstraction Docs
- Developer Docs Toolchain
- Shortcuts Settings Screenshot
- Magnify Side Panel Demo
- Application Entry Point
- Tool Factory Registry
- Key Sequence Validation
- Capture Lifecycle Concerns
- Build Stack & Dependencies
- Release 11.0 Daemon Changes
- Hex Color Field Screenshot
- Capture Launcher Screenshot
- Last Region Cache
- Current Screen Resolver
- Text Tool Copy & Editor
- Global Icon Paths
- Strftime Parsing
- Uploader Plugin Gap Docs
- End-User Docs & Drive Option
- Config Permission Hardening
- Region Launcher Screenshot
- Qt Designer UI Authoring
- Dialog Lifetime Crash Fixes
- Debian Packaging Build
- Color Luma Utilities
- Value Handler Declarations
- User Color Config Type
- Selection Size Indicator
- OAuth Client & Sharing Scopes
- Counter Tool Demo
- Invert Tool Demo
- Backend Resolution Mismatch
- Capture Completion Handoff
- Config Handler Declarations
- Color Preset Edit Mode
- Tool Selection Rendering
- Bounded Integer Config
- Capture Widget Mouse Actions
- Action Options Test Script
- Capture Lifecycle Test Script
- Config Handler Docs
- Manual Shell Test Suite
- Monochrome Tray Icon Screenshot
- Pixelate Redaction Demo
- OAuth Implementation Decisions
- Build Matrix Verification
- Portal Request Interface
- Config Error Details Dialog
- Update Info Text
- Path & Icon Lookup
- String Config Handler
- Lower Bounded Integer Config
- Capture Widget Painting
- Windows CLI Wrapper
- Drive Identity Test Script
- Translation Pipeline
- Static Analysis Posture
- Contribution Process
- Counter Pointer Demo
- Drive Folder Caching Risks
- Wayland Detection
- Naming Conventions
- CI & Code Signing
- Text Tool Header
- Pin Widget Construction
- Button Handler Setup
- Capture Tool Population
- Drive Uploader Construction
- Color Grab Construction
- Global Hotkey Support
- Header Include Conventions
- Post Process Script
- Text Alignment Update
- Icon Accessor
- Tool Type Accessor
- Font Weight Update
- Path Option Test Script
- Windows Strftime Bug
- SPDX License Headers
- Donation Policy
- Windows TLS Preflight

## God Nodes (most connected - your core abstractions)
1. `ConfigHandler` - 237 edges
2. `CaptureWidget` - 159 edges
3. `GeneralConf` - 112 edges
4. `QColor` - 87 edges
5. `SelectionWidget` - 75 edges
6. `QPoint` - 71 edges
7. `TextTool` - 61 edges
8. `ImgUploaderBase` - 54 edges
9. `QVariant` - 54 edges
10. `AbstractTwoPointTool` - 51 edges

## Surprising Connections (you probably didn't know these)
- `Upload-to-S3-Bucket Toolbar Action` --semantically_similar_to--> `Google Drive Upload Feature (opt-in build)`  [INFERRED] [semantically similar]
  docs/flameshot-documentation.pdf → docs/google-drive-setup.md
- `Capture Tool Source Group` --implements--> `Pin Menu Close Option`  [INFERRED]
  src/tools/CMakeLists.txt → docs/ReleaseNote_12.1.md
- `Utils Source Group (confighandler, valuehandler, screengrabber)` --implements--> `DesktopFileParser Reads Only .desktop Files`  [INFERRED]
  src/utils/CMakeLists.txt → docs/ReleaseNote_12.1.md
- `flameshot Executable Target` --conceptually_related_to--> `QMake to CMake Buildsystem Migration`  [INFERRED]
  src/CMakeLists.txt → docs/ReleaseNotes_0.8.md
- `Capture Tool Source Group` --implements--> `Pixelate Tool (replaces Blur)`  [INFERRED]
  src/tools/CMakeLists.txt → docs/ReleaseNotes_0.8.md

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **Interactive capture to export flow** — docs_codebase_architecture_main, docs_codebase_architecture_capturerequest, docs_codebase_architecture_flameshot, docs_codebase_architecture_capturewidget, docs_codebase_architecture_screengrabber, docs_codebase_architecture_exportcapture, docs_codebase_architecture_flameshotdaemon [EXTRACTED 1.00]
- **CaptureTool hierarchy and factory instantiation** — docs_codebase_architecture_capturetool, docs_codebase_architecture_abstracttwopointtool, docs_codebase_architecture_abstractpathtool, docs_codebase_architecture_abstractactiontool, docs_codebase_architecture_toolfactory [EXTRACTED 1.00]
- **Hard-coded uploader backend and the pluggable-storage gap** — docs_codebase_architecture_imguploadermanager, docs_codebase_architecture_imguploaderbase, docs_codebase_integrations_imgur, docs_codebase_concerns_hardcoded_uploader_backend, docs_codebase_concerns_pluggable_upload_backends, docs_codebase_structure_add_new_uploader [EXTRACTED 1.00]
- **Conditional Upload Backend Build Gating (ENABLE_IMGUR / ENABLE_GDRIVE)** — src_tools_cmakelists_upload_infrastructure_gate, src_tools_cmakelists_imgur_backend_gate, src_tools_cmakelists_gdrive_backend_gate, src_widgets_cmakelists_upload_widgets_gate [EXTRACTED 1.00]
- **Portal-Based Capture and Its Backend Fallbacks** — docs_usagex11minimalwm_xdg_desktop_portal_capture, docs_usagex11minimalwm_screenshot_backend_gap, docs_usagex11minimalwm_legacy_x11_capture, docs_usagehyprlandswaywlroots_xdg_desktop_portal_wlr, docs_releasenotes_12_0_kde_freedesktop_portal, docs_releasenotes_0_9_xcb_forced_on_wayland [INFERRED 0.85]
- **Flow For Adding A New Config Setting** — docs_dev_src_faq_add_config_setting, docs_dev_src_faq_confighandler, docs_dev_src_faq_valuehandler, docs_dev_src_faq_configwindow, docs_dev_src_faq_config_getter_setter_macro, docs_dev_src_index_confighandler [EXTRACTED 1.00]
- **Text tool property controls that jointly style one annotation** — docs_images_alignment_font_selector, docs_images_alignment_font_style_buttons, docs_images_alignment_text_alignment_control, docs_images_alignment_active_tool_size_slider, docs_images_alignment_active_color_picker, docs_images_alignment_text_annotation_object [INFERRED 0.85]
- **Capture editor chrome surrounding the selection** — docs_images_alignment_selection_overlay, docs_images_alignment_capture_action_toolbar, docs_images_alignment_side_action_toolbar, docs_images_alignment_side_config_panel [INFERRED 0.85]
- **Color Preset Editing Flow: pick on wheel, hex field, ring of stored presets** — docs_images_colorwheel_hsv_color_wheel, docs_images_colorwheel_preset_management, docs_images_colorwheel_color_preset_ring, docs_images_colorwheel_colorpicker_editor [INFERRED 0.85]
- **Interface Tab Configuration Surfaces** — docs_images_colorwheel_ui_color_editor, docs_images_colorwheel_colorpicker_editor, docs_images_colorwheel_opacity_outside_selection, docs_images_colorwheel_button_selection [EXTRACTED 1.00]
- **Interface Tab Appearance Customization Surface** — docs_images_config_interface_ui_color_editor, docs_images_config_interface_colorpicker_editor, docs_images_config_interface_opacity_slider, docs_images_config_interface_button_selection [EXTRACTED 1.00]
- **UI Color Editing Flow (pick target button, then choose hue on wheel)** — docs_images_config_interface_select_target_button, docs_images_config_interface_main_color, docs_images_config_interface_contrast_color, docs_images_config_interface_color_wheel [EXTRACTED 1.00]
- **Shortcuts tab UI: tabbed dialog, description/key table, and the two shortcut families it lists** — docs_images_config_shortcuts_tabbed_settings_layout, docs_images_config_shortcuts_shortcut_table, docs_images_config_shortcuts_paint_tool_shortcuts, docs_images_config_shortcuts_capture_action_shortcuts [EXTRACTED 1.00]
- **Binding-state variants rendered in the Key column: assigned, unassigned, and non-editable mouse binding** — docs_images_config_shortcuts_unassigned_shortcut_slot, docs_images_config_shortcuts_non_editable_mouse_binding, docs_images_config_shortcuts_duplicate_action_multiple_bindings, docs_images_config_shortcuts_shortcut_table [INFERRED 0.85]
- **Region-Select then Annotate Capture Flow Shown in Counter Demo** — docs_images_counter_gif_selection_region, docs_images_counter_gif_dimension_indicator, docs_images_counter_gif_capture_toolbar, docs_images_counter_gif_circle_count_tool [INFERRED 0.85]
- **Capture Session Shortcut Flow (select, annotate, export, exit)** — docs_images_help_screen_mouse_select_area, docs_images_help_screen_mouse_wheel_change_tool_size, docs_images_help_screen_right_click_color_picker, docs_images_help_screen_space_open_side_panel, docs_images_help_screen_ctrl_s_save_to_file, docs_images_help_screen_ctrl_c_copy_to_clipboard, docs_images_help_screen_esc_exit [INFERRED 0.85]
- **Capture Export Destinations Reachable From Selection** — docs_images_help_screen_mouse_select_area, docs_images_help_screen_ctrl_s_save_to_file, docs_images_help_screen_ctrl_c_copy_to_clipboard [INFERRED 0.85]
- **Color selection controls that all converge on the single active color** — docs_images_hex_hex_color_field, docs_images_hex_color_wheel_picker, docs_images_hex_grab_color_button, docs_images_hex_active_color_swatch [INFERRED 0.85]
- **Capture-Time UI Surface: cursor, shortcut overlay, and selection-scoped invert effect** — docs_images_invert_crosshair_cursor, docs_images_invert_shortcut_overlay, docs_images_invert_selection_region, docs_images_invert_inverter_tool [INFERRED 0.75]
- **Annotation Tools Sharing Selected Color and Undo History** — docs_images_large_demo_arrow_tool, docs_images_large_demo_marker_highlighter_tool, docs_images_large_demo_text_tool, docs_images_large_demo_counter_tool, docs_images_large_demo_shape_tools, docs_images_large_demo_pixelate_tool, docs_images_large_demo_color_picker_wheel, docs_images_large_demo_undo_redo_history [INFERRED 0.85]
- **Capture, Annotate, Export Flow** — docs_images_large_demo_capture_selection_region, docs_images_large_demo_annotation_toolbar, docs_images_large_demo_export_actions, docs_images_large_demo_upload_to_cloud_button, docs_images_large_demo_annotate_before_export_flow [INFERRED 0.85]
- **Launcher Capture Configuration Flow** — docs_images_launcher_area_selector, docs_images_launcher_delay_selector, docs_images_launcher_geometry_inputs, docs_images_launcher_take_new_screenshot_button, docs_images_launcher_screenshot_preview_pane [INFERRED 0.85]
- **Layer Management UI (list, reorder buttons, delete, z-order result)** — docs_images_layer_layer_list_panel, docs_images_layer_layer_reorder_buttons, docs_images_layer_delete_layer_button, docs_images_layer_annotation_z_order, docs_images_layer_layer_movement [EXTRACTED 1.00]
- **Capture Overlay Chrome (selection overlay, tool bar, action column, side panel)** — docs_images_layer_selection_overlay, docs_images_layer_capture_tool_bar, docs_images_layer_capture_action_column, docs_images_layer_side_panel [EXTRACTED 1.00]
- **Capture-mode input bindings shown in the magnifier overlay** — docs_images_magnifer_fullscreen_capture_mode, docs_images_magnifer_mouse_select_area, docs_images_magnifer_ctrl_s_save_to_file, docs_images_magnifer_ctrl_c_copy_to_clipboard, docs_images_magnifer_mouse_wheel_tool_size, docs_images_magnifer_right_click_color_picker, docs_images_magnifer_space_side_panel, docs_images_magnifer_esc_exit [EXTRACTED 1.00]
- **Selection-to-output paths (file, clipboard, color pick)** — docs_images_magnifer_mouse_select_area, docs_images_magnifer_ctrl_s_save_to_file, docs_images_magnifer_ctrl_c_copy_to_clipboard, docs_images_magnifer_right_click_color_picker [INFERRED 0.85]
- **Capture Session Flow: select area, adjust tool, pick color, then save or copy** — docs_images_magnify_selection_interaction, docs_images_magnify_tool_size_scroll, docs_images_magnify_grab_color_action, docs_images_magnify_save_and_copy_actions [INFERRED 0.85]
- **Side Panel Color Workflow Widgets** — docs_images_magnify_side_panel, docs_images_magnify_grab_color_action, docs_images_magnify_color_picker [EXTRACTED 1.00]
- **Monochrome tray-icon documentation figure: annotated menu bar showing the app glyph among system status icons** — docs_images_monochrome_monochrome_tray_icon_screenshot, docs_images_monochrome_flame_glyph, docs_images_monochrome_callout_arrow_annotation, docs_images_monochrome_neighboring_status_icons, docs_images_monochrome_macos_menu_bar_integration [INFERRED 0.85]
- **Numbered Counter Callout Feature (badge, drag gesture, leader-line pattern shown in one demo)** — docs_images_number_pointer_counter_pointer_demo, docs_images_number_pointer_incremental_counter_tool, docs_images_number_pointer_drag_to_place_pointer, docs_images_number_pointer_leader_line_callout [INFERRED 0.85]
- **Pixelate Redaction Workflow Shown in the GIF** — docs_images_pixelate_flameshot_capture_overlay, docs_images_pixelate_drag_to_select_region, docs_images_pixelate_pixelate_tool, docs_images_pixelate_screenshot_redaction [INFERRED 0.85]
- **Launcher capture configuration flow: choose area, set delay, refine geometry, then capture** — docs_images_region_launcher_area_selector, docs_images_region_launcher_delay_selector, docs_images_region_launcher_geometry_input, docs_images_region_launcher_take_new_screenshot_action [INFERRED 0.85]
- **Sidebar tool-property controls that mutate the active annotation style** — docs_images_sidebar_text_style_controls, docs_images_sidebar_active_thickness, docs_images_sidebar_active_color, docs_images_sidebar_color_wheel_picker, docs_images_sidebar_grab_color [INFERRED 0.85]
- **Capture-overlay UI surfaces shown together during region annotation** — docs_images_sidebar_selection_overlay, docs_images_sidebar_capture_button_bar, docs_images_sidebar_side_action_buttons, docs_images_sidebar_sidebar_panel [EXTRACTED 1.00]
- **Selection region framed by left, right, and bottom overlay toolbars** — docs_images_small_demo_selection_region, docs_images_small_demo_left_action_toolbar, docs_images_small_demo_right_action_toolbar, docs_images_small_demo_bottom_tool_toolbar [EXTRACTED 1.00]
- **Annotations demonstrated on the captured region** — docs_images_small_demo_pixelate_tool, docs_images_small_demo_arrow_tool, docs_images_small_demo_rectangle_tool, docs_images_small_demo_counter_annotation [EXTRACTED 1.00]
- **Qt Designer panels that together author a .ui form** — docs_images_ui_file_widget_box, docs_images_ui_file_object_inspector, docs_images_ui_file_property_editor, docs_images_ui_file_signal_slot_editor, docs_images_ui_file_launcher_ui_form [INFERRED 0.85]
- **Google Drive Loopback OAuth Authorization Flow** — docs_plans_2026_07_24_001_feat_google_drive_upload_plan_kd6_loopback_flow, docs_plans_2026_07_24_001_feat_google_drive_upload_plan_ktd1_handrolled_oauth, docs_plans_2026_07_24_001_feat_google_drive_upload_plan_ktd2_authorization_mechanics, docs_plans_2026_07_24_001_feat_google_drive_upload_plan_ktd9_single_flight_auth_service, docs_plans_2026_07_24_001_feat_google_drive_upload_plan_gdriveoauth, docs_google_drive_setup_oauth_desktop_client [EXTRACTED 1.00]
- **Drive-Only Build Failure Modes (flag, resolution, routing)** — docs_solutions_developer_experience_building_the_deb_with_gdrive_support_silent_feature_omission, docs_solutions_developer_experience_building_the_deb_with_gdrive_support_stale_cmake_cache_trap, docs_solutions_logic_errors_gdrive_visibility_ui_missing_on_drive_only_builds_backend_resolution_mismatch, docs_solutions_logic_errors_gdrive_visibility_ui_missing_on_drive_only_builds_single_backend_fallback, docs_residual_review_findings_feat_gdrive_integration_r_d_gdrive_only_legacy_imgur_delete [INFERRED 0.85]
- **Capture-Completion Re-Entrancy Fix Mechanism** — docs_plans_2026_07_24_002_refactor_capture_completion_lifecycle_plan_reentrancy_crash, docs_plans_2026_07_24_002_refactor_capture_completion_lifecycle_plan_ktd1_emit_from_closeevent, docs_plans_2026_07_24_002_refactor_capture_completion_lifecycle_plan_ktd2_deferred_export_single_shot, docs_plans_2026_07_24_002_refactor_capture_completion_lifecycle_plan_ktd3_teardown_before_export, docs_plans_2026_07_24_002_refactor_capture_completion_lifecycle_plan_ktd4_guarded_widget_pointer, docs_plans_2026_07_24_002_refactor_capture_completion_lifecycle_plan_ktd6_scoped_dialog_lifetime [EXTRACTED 1.00]

## Communities (215 total, 13 thin omitted)

### Community 0 - "Selection Widget Geometry"
Cohesion: 0.05
Nodes (86): ConfigErrorDetails::ConfigErrorDetails(), QWidget, isVisible, initShortcuts, selectAll, QKeySequence, QCursor, QEvent (+78 more)

### Community 1 - "General Settings Tab"
Cohesion: 0.04
Nodes (63): QScrollArea, QString, GeneralConf, changeSavePath, chooseFolder, m_abortNotifications, m_allowMultipleGuiInstances, m_antialiasingPinZoom (+55 more)

### Community 2 - "Capture Widget Signals"
Cohesion: 0.03
Nodes (64): quint64, CaptureWidget, captureCancelled, captureCompleted, colorChanged, m_activeButton, m_activeTool, m_activeToolIsMoved (+56 more)

### Community 3 - "Screenshot Saving & Clipboard"
Cohesion: 0.05
Nodes (48): QMetaType, QMimeData, QByteArray, saveScreenshotToFilesystem, saveToFile, DesktopInfo, DESKTOP_SESSION, GDMSESSION (+40 more)

### Community 4 - "Google Drive OAuth"
Cohesion: 0.08
Nodes (55): QDateTime, quint16, QByteArray, QObject, QString, GDriveOAuth, abortFlow, accessToken (+47 more)

### Community 5 - "Overlay Messages & Spinner"
Cohesion: 0.06
Nodes (49): OverlayMessage, QStack, QPaintEvent, QList, QPair, QRect, QString, QWidget (+41 more)

### Community 6 - "Flameshot Application Singleton"
Cohesion: 0.07
Nodes (49): CaptureLauncher, ConfigWindow, CaptureWidget, function, Origin, QPixmap, QRect, QWidget (+41 more)

### Community 7 - "Daemon & D-Bus Service"
Cohesion: 0.07
Nodes (48): getVersion, CaptureWidget, CaptureWidget, QByteArray, QNetworkReply, QPixmap, QRect, QString (+40 more)

### Community 8 - "Google Drive Uploader"
Cohesion: 0.09
Nodes (48): NotificationWidget, QNetworkReply, QString, QStringList, QUrl, GDriveUploader, applyNextRecipient, applySharing (+40 more)

### Community 9 - "Invert & Pixelate Tools"
Cohesion: 0.07
Nodes (45): CaptureTool, QIcon, QObject, QPainter, QPixmap, QRect, QString, Type (+37 more)

### Community 10 - "Capture Tool Painting"
Cohesion: 0.05
Nodes (32): CaptureTool, boundingRect, closeOnButtonPressed, copy, description, drawEnd, drawMove, drawStart (+24 more)

### Community 11 - "Upload Widget Base"
Cohesion: 0.06
Nodes (44): LoadSpinner, QPixmap, QUrl, QWidget, Q_OBJECT, QPixmap, QString, QWidget (+36 more)

### Community 12 - "Update Notification Widget"
Cohesion: 0.07
Nodes (40): QWheelEvent, CaptureLauncher::onCaptureFailed(), QKeyEvent, QString, QWidget, generateKernelString(), QT_BEGIN_NAMESPACE, QT_END_NAMESPACE (+32 more)

### Community 13 - "Utility Side Panel"
Cohesion: 0.07
Nodes (42): CaptureWidget, CaptureWidget, QWidget, Q_OBJECT, QPointer, QWidget, QListWidget, QPropertyAnimation (+34 more)

### Community 14 - "Arrow Tool"
Cohesion: 0.07
Nodes (39): ArrowStyle, QLine, QLineF, copyParams, QPair, ArrowTool, ArrowTool::ArrowTool(), boundingRect (+31 more)

### Community 15 - "Capture Request Value Object"
Cohesion: 0.08
Nodes (38): eTask, CaptureRequest, addPinTask, addSaveTask, addTask, captureMode, data, delay (+30 more)

### Community 16 - "Pin Widget Gestures"
Cohesion: 0.08
Nodes (38): QEvent, QKeyEvent, Q_OBJECT, QPixmap, qreal, QWidget, PinWidget, closePin (+30 more)

### Community 17 - "General Config Initializers"
Cohesion: 0.09
Nodes (34): QWidget, GeneralConf::GeneralConf(), initAllowMultipleGuiInstances, initAntialiasingPinZoom, initAutoCloseIdleDaemon, initAutostart, initConfigButtons, initCopyAndCloseAfterUpload (+26 more)

### Community 18 - "Config Change Signals"
Cohesion: 0.05
Nodes (39): allowMultipleGuiInstancesChanged, autoCloseIdleDaemonChanged, autostartChanged, GeneralConf::captureActiveMonitorChanged(), GeneralConf::checkForUpdatesChanged(), historyConfirmationToDelete, importConfiguration, GeneralConf::initGDriveSettings() (+31 more)

### Community 19 - "Screen Grabbing Backends"
Cohesion: 0.11
Nodes (36): QPixmap, QEvent, QObject, QPixmap, QRect, QScreen, QWidget, Q_OBJECT (+28 more)

### Community 20 - "Capture Tool Object Store"
Cohesion: 0.08
Nodes (34): QUndoCommand, CaptureToolObjects, append, at, captureToolObjects, clear, find, findWithRadius (+26 more)

### Community 21 - "Color Grab Widget"
Cohesion: 0.09
Nodes (34): QRect, ColorGrabWidget, color, colorGrabbed, colorUpdated, cursorPos, eventFilter, finalize (+26 more)

### Community 22 - "Text Tool Configuration"
Cohesion: 0.08
Nodes (33): AlignmentFlag, QString, QWidget, Q_OBJECT, QWidget, QComboBox, QPushButton, QVBoxLayout (+25 more)

### Community 23 - "Side Panel Widget"
Cohesion: 0.07
Nodes (34): ColorGrabWidget, ColorWheel, Q_OBJECT, QPixmap, QWidget, QCheckBox, QColorPickingEventFilter, QLabel (+26 more)

### Community 24 - "Shortcuts Settings Widget"
Cohesion: 0.09
Nodes (25): QStringList, QString, QWidget, Q_OBJECT, QList, QStringList, QWidget, QCheckBox (+17 more)

### Community 25 - "Path Tool Base"
Cohesion: 0.09
Nodes (29): AbstractPathTool, AbstractPathTool::AbstractPathTool(), addPoint, boundingRect, closeOnButtonPressed, copyParams, drawEnd, drawMove (+21 more)

### Community 26 - "Config Handler Implementation"
Cohesion: 0.16
Nodes (32): QSet, updateComponents, assertKeyRecognized, baseName, checkAndHandleError, checkForErrors, checkSemantics, checkShortcutConflicts (+24 more)

### Community 27 - "Color Preset Editor"
Cohesion: 0.08
Nodes (31): ColorPickerEditMode, ColorPickerEditor, addPreset, ColorPickerEditor::ColorPickerEditor(), deletePreset, m_addPresetButton, m_addPresetLabel, m_color (+23 more)

### Community 28 - "Command Line Parser"
Cohesion: 0.14
Nodes (30): const_iterator, CommandLineParser, AddArgument, addHelpOption, AddOption, AddOptions, addVersionOption, findParent (+22 more)

### Community 29 - "Capture Tool Button"
Cohesion: 0.09
Nodes (30): CaptureToolButton, animatedShow, CaptureToolButton::CaptureToolButton(), description, getPriorityByButton, icon, initButton, iterableButtonTypes (+22 more)

### Community 30 - "Two Point Tool Base"
Cohesion: 0.10
Nodes (29): AbstractTwoPointTool, AbstractTwoPointTool::AbstractTwoPointTool(), adjustedVector, boundingRect, closeOnButtonPressed, drawEnd, drawMove, drawMoveWithAdjustment (+21 more)

### Community 31 - "Circle Counter Tool"
Cohesion: 0.11
Nodes (28): CircleCountTool, boundingRect, CircleCountTool::CircleCountTool(), copy, description, handleMouseWheelEvent, icon, info (+20 more)

### Community 32 - "CLI Command Options"
Cohesion: 0.14
Nodes (29): QList, optionsToString(), CommandOption, addChecker, checkValue, CommandOption::CommandOption(), dashedNames, description (+21 more)

### Community 33 - "Capture Widget Event Handling"
Cohesion: 0.13
Nodes (28): QCloseEvent, Request, closeEvent, deleteToolWidgetOrClose, drawToolsData, emitCaptureOutcome, flushPendingOutcome, handleToolSignal (+20 more)

### Community 34 - "Filename Editor Tab"
Cohesion: 0.10
Nodes (27): QString, QWidget, FileNameEditor, addToNameEditor, FileNameEditor::FileNameEditor(), initLayout, initWidgets, m_clearButton (+19 more)

### Community 35 - "Tray Icon & Launcher"
Cohesion: 0.12
Nodes (25): QMenu, QSystemTrayIcon, instance, CaptureLauncher::connectCaptureSlots(), CaptureLauncher::disconnectCaptureSlots(), CaptureLauncher::updateCountdown(), QObject, Q_OBJECT (+17 more)

### Community 36 - "Visuals Settings Editor"
Cohesion: 0.11
Nodes (25): QComboBox, ButtonListView, ColorPickerEditor, QWidget, ExtendedSlider, Q_OBJECT, QWidget, QVBoxLayout (+17 more)

### Community 37 - "Text Input Widget"
Cohesion: 0.10
Nodes (23): QTextEdit, paintEvent, AlignmentFlag, QFont, QShowEvent, QWidget, Q_OBJECT, QSize (+15 more)

### Community 38 - "Toolbar Button Handler"
Cohesion: 0.08
Nodes (25): QVector, ButtonHandler, buttonsAreInside, hideSectionUnderMouse, init, m_allSidesBlocked, m_blockedBotton, m_blockedLeft (+17 more)

### Community 39 - "UI Color Editor"
Cohesion: 0.10
Nodes (25): CaptureToolButton, ClickableLabel, ColorWheel, Q_OBJECT, QWidget, Type, QHBoxLayout, QLabel (+17 more)

### Community 40 - "Magnifier Widget"
Cohesion: 0.10
Nodes (24): QPainterPath, QPainter, QPixmap, QWidget, Q_OBJECT, QPixmap, QWidget, MagnifierWidget (+16 more)

### Community 41 - "App Launcher Widget"
Cohesion: 0.10
Nodes (24): QTemporaryFile, AppLauncherWidget, m_appsMap, m_filterList, m_keepOpen, m_keepOpenCheckbox, m_layout, m_lineEdit (+16 more)

### Community 42 - "Upload History Store"
Cohesion: 0.13
Nodes (21): QList, path, showPostUploadDialog, QPixmap, QString, QList, QString, History (+13 more)

### Community 43 - "Marker Tool"
Cohesion: 0.12
Nodes (22): color, CaptureTool, QIcon, QObject, QPainter, QPixmap, QRect, QString (+14 more)

### Community 44 - "Notifier Box"
Cohesion: 0.11
Nodes (22): hide, show, QHideEvent, QString, QWidget, Q_OBJECT, QString, QWidget (+14 more)

### Community 45 - "CLI Command Arguments"
Cohesion: 0.13
Nodes (17): CommandArgument, CommandArgument::CommandArgument(), description, isRoot, m_description, m_name, name, operator== (+9 more)

### Community 46 - "Configuration Window"
Cohesion: 0.11
Nodes (21): ConfigWindow, keyPressEvent, m_filenameEditor, m_filenameEditorTab, m_generalConfig, m_generalConfigTab, m_shortcuts, m_shortcutsTab (+13 more)

### Community 47 - "Capture Context Drawing"
Cohesion: 0.10
Nodes (20): drawStart, CaptureContext, circleCount, fullscreen, mousePos, origScreenshot, request, savePath (+12 more)

### Community 48 - "Config Value Handlers"
Cohesion: 0.14
Nodes (18): expected, expected, QString, ExistingDir, check, expected, fallback, FilenamePattern (+10 more)

### Community 49 - "Notification Widget"
Cohesion: 0.12
Nodes (21): QString, QWidget, Q_OBJECT, QWidget, NotificationWidget, animatedHide, animatedShow, m_content (+13 more)

### Community 50 - "App Launcher Tool"
Cohesion: 0.13
Nodes (20): AppLauncher, AppLauncher::AppLauncher(), capture, closeOnButtonPressed, copy, description, icon, name (+12 more)

### Community 51 - "Pencil Tool"
Cohesion: 0.14
Nodes (20): CaptureTool, QIcon, QObject, QPainter, QPixmap, QString, Type, Q_OBJECT (+12 more)

### Community 52 - "Pin Tool"
Cohesion: 0.13
Nodes (20): CaptureTool, QIcon, QObject, QString, Type, Q_OBJECT, QPixmap, QRect (+12 more)

### Community 53 - "Text Tool Declarations"
Cohesion: 0.09
Nodes (22): AlignmentFlag, CaptureTool, Q_OBJECT, QFont, QPointer, QRect, QString, TextTool (+14 more)

### Community 54 - "Image Label Widget"
Cohesion: 0.14
Nodes (19): Q_DECL_OVERRIDE, setPixmap, QGraphicsDropShadowEffect, QPixmap, Q_OBJECT, QLabel, QPixmap, ImageLabel (+11 more)

### Community 55 - "Upload History Widget"
Cohesion: 0.15
Nodes (19): QFileInfo, clearHistoryLayout(), HistoryFileName, QPixmap, QString, QWidget, Q_OBJECT, QT_BEGIN_NAMESPACE (+11 more)

### Community 56 - "Imgur Uploader"
Cohesion: 0.13
Nodes (19): QNetworkRequest, QNetworkReply, QPixmap, QString, QWidget, Q_OBJECT, ImgurUploader, deleteImage (+11 more)

### Community 57 - "Abstract Logger"
Cohesion: 0.14
Nodes (19): QTextStream, AbstractLogger, AbstractLogger::AbstractLogger(), error, info, m_defaultChannel, m_enableMessageHeader, m_notificationPath (+11 more)

### Community 58 - "Uploader Backend Manager"
Cohesion: 0.13
Nodes (19): QObject, QPixmap, QString, QStringList, QWidget, Q_OBJECT, QObject, QString (+11 more)

### Community 59 - "Selection Tool"
Cohesion: 0.14
Nodes (19): CaptureTool, QIcon, QObject, QPainter, QPixmap, QString, Type, Q_OBJECT (+11 more)

### Community 60 - "Config Docs & Source Groups"
Cohesion: 0.15
Nodes (20): How To Add A New Config Setting, CONFIG_GETTER_SETTER Macro, ConfigHandler (config getters/setters), ConfigWindow (configuration GUI tabs), Setting Groups: General and Shortcuts, ValueHandler (validation/conversion), ConfigHandler UI-Decoupled Configuration, Check For New Release Feature (+12 more)

### Community 61 - "Circle Tool"
Cohesion: 0.14
Nodes (18): CircleTool, CircleTool::CircleTool(), copy, description, icon, name, pressed, process (+10 more)

### Community 62 - "Upload Tool Button"
Cohesion: 0.15
Nodes (18): CaptureTool, QIcon, QObject, QString, Type, Q_OBJECT, QPixmap, ImgUploaderTool (+10 more)

### Community 63 - "Line Tool"
Cohesion: 0.14
Nodes (18): CaptureTool, QIcon, QObject, QPainter, QPixmap, QString, Type, Q_OBJECT (+10 more)

### Community 64 - "Rectangle Tool"
Cohesion: 0.14
Nodes (18): CaptureTool, QIcon, QObject, QPainter, QPixmap, QString, Type, Q_OBJECT (+10 more)

### Community 65 - "Desktop App Metadata"
Cohesion: 0.13
Nodes (15): DesktopAppData, categories, description, exec, icon, name, showInTerminal, QIcon (+7 more)

### Community 66 - "Toolbar Button Positioning"
Cohesion: 0.26
Nodes (19): adjustHorizontalCenter, calculateShift, contains, ensureSelectionMinimumSize, expandSelection, horizontalPoints, intersectWithAreas, moveButtonsToPoints (+11 more)

### Community 67 - "Global Shortcut Filter"
Cohesion: 0.12
Nodes (16): QAbstractNativeEventFilter, qintptr, QObject, QByteArray, QObject, GlobalShortcutFilter, getNativeModifier, GlobalShortcutFilter::GlobalShortcutFilter() (+8 more)

### Community 68 - "Capture Resize & Open-With"
Cohesion: 0.11
Nodes (16): QMessageBox, QUndoStack, QPixmap, showOpenWithMenu(), resizeEvent, resizeEvent, ColorPicker, HoverEventFilter (+8 more)

### Community 69 - "Action Tool Base"
Cohesion: 0.13
Nodes (17): AbstractActionTool, AbstractActionTool::AbstractActionTool(), boundingRect, drawEnd, drawMove, drawStart, isSelectable, isValid (+9 more)

### Community 70 - "Move Tool"
Cohesion: 0.16
Nodes (17): CaptureTool, QIcon, QObject, QString, Type, Q_OBJECT, MoveTool, closeOnButtonPressed (+9 more)

### Community 71 - "Upload Confirmation Dialog"
Cohesion: 0.14
Nodes (18): Q_OBJECT, QDialog, ImgUploadDialog, buttonBox, layout, m_driveActive, m_recipients, m_recipientsLabel (+10 more)

### Community 72 - "Set Shortcut Dialog"
Cohesion: 0.18
Nodes (16): QLayout, QKeyEvent, Q_OBJECT, QDialog, QString, QVBoxLayout, SetShortcutDialog, accept (+8 more)

### Community 73 - "Button List View"
Cohesion: 0.18
Nodes (16): QListWidgetItem, ButtonListView, ButtonListView::ButtonListView(), initButtonList, m_buttonTypeByName, m_listButtons, reverseItemCheck, selectAll (+8 more)

### Community 74 - "Accept Tool"
Cohesion: 0.17
Nodes (16): AcceptTool, AcceptTool::AcceptTool(), closeOnButtonPressed, copy, description, icon, name, pressed (+8 more)

### Community 75 - "Copy Tool"
Cohesion: 0.17
Nodes (16): CopyTool, closeOnButtonPressed, copy, CopyTool::CopyTool(), description, icon, name, pressed (+8 more)

### Community 76 - "Exit Tool"
Cohesion: 0.17
Nodes (16): CaptureTool, QIcon, QObject, QString, Type, ExitTool, closeOnButtonPressed, copy (+8 more)

### Community 77 - "Redo Tool"
Cohesion: 0.17
Nodes (16): CaptureTool, QIcon, QObject, QString, Type, Q_OBJECT, RedoTool, closeOnButtonPressed (+8 more)

### Community 78 - "Save Tool"
Cohesion: 0.17
Nodes (16): CaptureTool, QIcon, QObject, QString, Type, Q_OBJECT, SaveTool, closeOnButtonPressed (+8 more)

### Community 79 - "Size Decrease Tool"
Cohesion: 0.17
Nodes (16): CaptureTool, QIcon, QObject, QString, Type, Q_OBJECT, SizeDecreaseTool, closeOnButtonPressed (+8 more)

### Community 80 - "Size Increase Tool"
Cohesion: 0.17
Nodes (16): CaptureTool, QIcon, QObject, QString, Type, Q_OBJECT, SizeIncreaseTool, closeOnButtonPressed (+8 more)

### Community 81 - "Undo Tool"
Cohesion: 0.17
Nodes (16): CaptureTool, QIcon, QObject, QString, Type, Q_OBJECT, UndoTool, closeOnButtonPressed (+8 more)

### Community 82 - "Linux Desktop File Parser"
Cohesion: 0.14
Nodes (16): QDir, QMap, QString, QStringList, QVector, DesktopFileParser, getAppsByCategory, m_appList (+8 more)

### Community 83 - "Capture Button Styling"
Cohesion: 0.18
Nodes (14): QPushButton, Flameshot::Flameshot(), CaptureButton, CaptureButton::CaptureButton(), globalStyleSheet, init, m_mainColor, public (+6 more)

### Community 84 - "Unix Signal Handling"
Cohesion: 0.18
Nodes (14): QSocketNotifier, QObject, Q_OBJECT, QObject, SignalDaemon, handleSigInt, handleSigTerm, intSignalHandler (+6 more)

### Community 85 - "App Launcher List Population"
Cohesion: 0.18
Nodes (16): addAppsToListWidget, AppLauncherWidget::AppLauncherWidget(), checkboxClicked, configureListView, initAppMap, initListWidget, keyPressEvent, launch (+8 more)

### Community 86 - "Monitor Preview Widget"
Cohesion: 0.13
Nodes (15): enterEvent, Q_OBJECT, QWidget, MonitorPreview, enterEvent, m_contrastColor, m_monitorIndex, m_textLabel (+7 more)

### Community 87 - "Windows Shortcut Parser"
Cohesion: 0.19
Nodes (15): QDir, QMap, QString, QStringList, QVector, QStringList, QVector, WinLnkFileParser (+7 more)

### Community 88 - "Draggable Widget Helper"
Cohesion: 0.14
Nodes (15): QEvent, QObject, QWidget, DraggableWidgetMaker, DraggableWidgetMaker::DraggableWidgetMaker(), eventFilter, m_isDragging, m_isPressing (+7 more)

### Community 89 - "Interface Settings Screenshots"
Cohesion: 0.15
Nodes (16): Button Selection Checklist, Capture Tool Set (Pencil, Line, Arrow, Rectangular Selection, Rectangle), HSV Color Wheel with Triangle Picker, Colorpicker Editor Sub-tab, Configuration Dialog, Contrast Color Setting, Dark Theme Visual Styling, Interface Tab (+8 more)

### Community 90 - "Annotation Toolbar Demo"
Cohesion: 0.16
Nodes (16): Annotate-Then-Export Capture Flow, Floating Circular-Button Annotation Toolbar, Arrow Annotation Tool, Capture Selection Region with Corner/Edge Handles, Radial Color Picker Wheel, Numbered Step Counter Tool, Export Actions (Copy to Clipboard, Save to File, Upload to Cloud), Captured Subject: GNU 'A Quick Guide to GPLv3' Page (+8 more)

### Community 91 - "Text Tool Drawing"
Cohesion: 0.12
Nodes (15): closeOnButtonPressed, drawEnd, drawMove, isChanged, isSelectable, isValid, move, onColorChanged (+7 more)

### Community 92 - "Value Handler Types"
Cohesion: 0.20
Nodes (16): Color, check, fallback, m_def, process, representation, QVariant, Region (+8 more)

### Community 93 - "Orientable Push Button"
Cohesion: 0.16
Nodes (15): Orientation, QIcon, QSize, QString, QWidget, Orientation, Q_OBJECT, OrientablePushButton (+7 more)

### Community 94 - "D-Bus Adapter Interface"
Cohesion: 0.21
Nodes (13): QDBusAbstractAdaptor, instance, QByteArray, QObject, QString, FlameshotDBusAdapter, attachPin, attachScreenshotToClipboard (+5 more)

### Community 95 - "Extended Slider Widget"
Cohesion: 0.19
Nodes (13): QWidget, ExtendedSlider, ExtendedSlider::ExtendedSlider(), fireTimer, m_timer, mappedValue, modificationsEnded, public (+5 more)

### Community 96 - "Terminal Launcher"
Cohesion: 0.17
Nodes (13): QObject, QStringList, Q_OBJECT, QObject, QString, TerminalApp, arg, name (+5 more)

### Community 97 - "Capture Tool State Machine"
Cohesion: 0.27
Nodes (15): activeButtonToolType, commitCurrentTool, handleButtonLeftClick, handleButtonRightClick, mouseReleaseEvent, pushObjectsStateToUndoStack, pushToolToStack, releaseActiveTool (+7 more)

### Community 98 - "Daemon Architecture Docs"
Cohesion: 0.16
Nodes (14): Config hot reload via QFileSystemWatcher, Daemon vs transient-process split, FlameshotDaemon resident singleton, FlameshotDBusAdapter org.flameshot.Flameshot interface, main() entry point (bootstrap + CLI parse), Security: D-Bus session-bus surface, QClipboard / KSystemClipboard Wayland clipboard, GitHub Releases API update check (+6 more)

### Community 99 - "Release 12.0 & Source Groups"
Cohesion: 0.20
Nodes (14): FLAMESHOT_DEBUG_CAPTURE CMake Variable, Sidebar Toggle Button, Color Picker Zoom / Magnify, --region Flag (xrandr WxH+x+y Syntax), Last Region Cache, Layer Movement (Up/Down), Selection Magnifier, Migration to .ui XML / Qt Designer Widgets (+6 more)

### Community 100 - "Launcher Item Delegate"
Cohesion: 0.20
Nodes (12): QStyledItemDelegate, QStyleOptionViewItem, QModelIndex, QObject, QPainter, QSize, Q_OBJECT, LauncherItemDelegate (+4 more)

### Community 101 - "Config Error Resolver"
Cohesion: 0.25
Nodes (12): ConfigResolver, ConfigResolver::ConfigResolver(), layout, populate, resetLayout, QWidget, QDialog, QGridLayout (+4 more)

### Community 102 - "Active Tool Size & Color"
Cohesion: 0.22
Nodes (14): activeButtonTool, cancel, childEnter, childLeave, keyPressEvent, keyReleaseEvent, onToolSizeChanged, setDrawColor (+6 more)

### Community 103 - "Color Picker Popup"
Cohesion: 0.19
Nodes (12): ColorPicker, ColorPicker::ColorPicker(), colorSelected, hideEvent, mouseMoveEvent, public, setNewColor, showEvent (+4 more)

### Community 104 - "Color Palette Definitions"
Cohesion: 0.14
Nodes (14): ColorPickerWidget, defaultLargeColorPalette, defaultSmallColorPalette, m_colorAreaList, m_colorAreaSize, m_colorList, m_lastIndex, m_selectedIndex (+6 more)

### Community 105 - "Side Panel Color Grabbing"
Cohesion: 0.20
Nodes (13): QEvent, QHideEvent, QObject, eventFilter, finalizeGrab, hideEvent, onColorChanged, onColorGrabAborted (+5 more)

### Community 106 - "In-Place Annotation Editor Demo"
Cohesion: 0.18
Nodes (13): Small Demo Screenshot (capture editor in action), Arrow Annotation, Bottom Drawing Tool Toolbar (pencil, line, arrow, rect outline, filled rect, ellipse, marker, text), In-Place Capture Annotation UI, Numbered Counter Annotation (badge "1"), Left Action Toolbar (save, cancel, upload, open-with, pin), Pixelate / Redaction Annotation, High-Contrast Accent Styling of Overlay Controls (+5 more)

### Community 107 - "Wayland Portal Setup"
Cohesion: 0.15
Nodes (13): KDE Uses Freedesktop Portal, Suggest Setting XDG_CURRENT_DESKTOP When DE Undetected, Compositor Floating/Borderless Window Rules For Flameshot, xdpw Duplicate Screenshot Token Bug, Hyprland Support, Portal Non-Response Hang Troubleshooting, River Must Spoof XDG_CURRENT_DESKTOP=sway, XDG_CURRENT_DESKTOP Environment Setup (+5 more)

### Community 108 - "Strftime Chooser Widget"
Cohesion: 0.15
Nodes (11): QGridLayout, QWidget, Q_OBJECT, QMap, QString, QWidget, StrftimeChooserWidget, m_buttonData (+3 more)

### Community 109 - "Upload History Line Item"
Cohesion: 0.17
Nodes (11): QWidget, HistoryFileName, QPixmap, QString, QWidget, QT_BEGIN_NAMESPACE, QT_END_NAMESPACE, HistoryFileName (+3 more)

### Community 110 - "UI Color Editor Logic"
Cohesion: 0.23
Nodes (12): CaptureToolButton, QString, QWidget, changeInputColor, changeLastButton, initButtons, initColorWheel, initHexColorInput (+4 more)

### Community 111 - "Button List Config Type"
Cohesion: 0.29
Nodes (12): BList, ButtonList, check, fallback, fromIntList, normalizeButtons, process, representation (+4 more)

### Community 112 - "Screen Capture & Logging Concerns"
Cohesion: 0.18
Nodes (12): AbstractLogger multi-target logging, No-exceptions error handling strategy, ScreenGrabber platform capture, Fragile: cross-platform screen grabbing, Wayland portal capture security posture, Defensive inline validation over exceptions, Prefer AbstractLogger over raw qWarning/qDebug, org.freedesktop.Notifications (+4 more)

### Community 113 - "Text Alignment Screenshot"
Cohesion: 0.26
Nodes (12): Active Color Swatch, Grab Color and Color Wheel, Active Tool Size Slider, Circular Capture Action Toolbar (pin, upload, close, save, copy), Font Family Selector (.AppleSystemUIFont), Font Style Buttons (strikethrough, underline, bold, italic), Numbered Red Callout Documentation Convention, Text Alignment Feature Screenshot (Flameshot capture editor), Capture Selection Overlay with Resize Handles (+4 more)

### Community 114 - "Layer Z-Order Demo"
Cohesion: 0.24
Nodes (12): Annotation Z-Order Rendering, Capture Action Column (pin, open app, upload, cancel, save), Capture Tool Button Bar (circular purple tool buttons), Delete Layer Button (trash icon), Layer List Panel (annotation stack listing), Layer Movement Up/Down, Layer Movement Demo Animation (layer.gif), Layer Reorder Buttons (up arrow / down arrow) (+4 more)

### Community 115 - "Capture Sidebar Demo"
Cohesion: 0.24
Nodes (12): Active Color Swatch, Active Thickness Slider, Capture Tool Button Bar (shape, marker, text, pixelate, counter, size, move, undo, redo, copy, save, close), Color Wheel Picker, Contextual Tool Property Disclosure, Grab Color (eyedropper) Action, Selection Region Overlay with Handles, Side Action Buttons (pin, open-with-app, upload) (+4 more)

### Community 116 - "Imgur Build Gates & 12.1"
Cohesion: 0.18
Nodes (12): DesktopFileParser Reads Only .desktop Files, Imgur Application Client ID Rename, Launch On Start Disabled By Default, Pin Menu Close Option, Flameshot v12.1.0 Release, Imgur Upload History, Upload Confirmation Dialog, User-Supplied Imgur API Key (+4 more)

### Community 117 - "Release 0.8 & CLI Refactor"
Cohesion: 0.20
Nodes (12): Circle Counter Tool, Generic Package Confinement Limits (Snap/Flatpak/AppImage), Basic Launcher Panel, Pixelate Tool (replaces Blur), QMake to CMake Buildsystem Migration, Flameshot 0.8 Release, --accept-on-select Flag, Complete CLI Refactor (+4 more)

### Community 118 - "Upload Dialog Recipients"
Cohesion: 0.24
Nodes (11): QString, SetShortcutDialog::SetShortcutDialog(), QT_END_NAMESPACE, QDialog(), QString, QStringList, ImgUploadDialog::ImgUploadDialog(), onAccept (+3 more)

### Community 119 - "System Notifications"
Cohesion: 0.20
Nodes (10): QObject, QString, Q_OBJECT, QObject, QDBusInterface, SystemNotification, m_interface, public (+2 more)

### Community 120 - "Color Picker Grid Painting"
Cohesion: 0.21
Nodes (10): onDisplayGridChanged, onGridSizeChanged, ColorPickerWidget::ColorPickerWidget(), initColorPicker, paintEvent, repaint, updateSelection, updateWidget (+2 more)

### Community 121 - "Hover Event Filter"
Cohesion: 0.21
Nodes (10): QEvent, QObject, Q_OBJECT, QObject, HoverEventFilter, eventFilter, HoverEventFilter::HoverEventFilter(), hoverIn (+2 more)

### Community 122 - "RFC Process & Opacity Slider"
Cohesion: 0.20
Nodes (11): RFC template (docs/0000-template.md), RFC: Add an opacity slider to Tool Settings, Anti-goal: preserve text readability under marker, Microsoft Snip & Sketch highlighter comparison, Tool Settings panel (slider under Active thickness), Consensus-building before submitting an RFC, Final Comment Period (FCP) with disposition, RFC life-cycle and "active" status (+3 more)

### Community 123 - "Daemon Mode & Project Structure"
Cohesion: 0.20
Nodes (11): Root vs src CMakeLists Separation Of Concerns, Daemon As Host For Persistent Clipboard Content, Daemon Mode, Flameshot Singleton High-Level API, FlameshotDaemon, Flameshot Project Structure, Pointer-To-Member Signal/Slot Convention, Close After Capture Option (+3 more)

### Community 124 - "Drive Backend Selection Docs"
Cohesion: 0.18
Nodes (11): Copy URL After Upload Setting, Flameshot Settings (General, Interface, Information tabs), Open Launcher (capture mode and delay), GDriveUploader Backend, Backend-Selectable ImgUploaderManager (uploadStorage key), KD2 Drive as Settings-Selectable Target Alongside Imgur, KTD10 Async Per-Request Reply Connections, KTD4 Resumable Upload Protocol (not multipart) (+3 more)

### Community 125 - "Colorpicker Editor Screenshot"
Cohesion: 0.29
Nodes (11): Button Selection Tool Checklist, Radial Color Preset Ring, Colorpicker Editor Sub-tab, Flameshot Configuration Dialog Screenshot (Colorpicker Editor), Dark Theme Configuration UI, Capture Drawing Tools (Pencil, Line, Arrow, Rectangle, Circle, Marker, Text, Circle Counter, Pixelate, Invert, Selection Size Indicator), HSV Color Wheel with Triangle Value/Saturation Selector, Interface Configuration Tab (+3 more)

### Community 126 - "Keybinding Cheatsheet Overlay"
Cohesion: 0.29
Nodes (11): In-Capture Annotation Tooling, Ctrl+C: Copy Selection To Clipboard, Ctrl+S: Save Screenshot To A File, Dimmed Frozen Desktop Backdrop, Esc: Exit Capture Mode, Screenshot Tool Help Overlay Screen, Centered Keybinding Cheatsheet Panel, Mouse: Select Screenshot Area (+3 more)

### Community 127 - "Magnifier Shortcut Overlay Demo"
Cohesion: 0.24
Nodes (11): Ctrl+C: Copy selection to clipboard, Ctrl+S: Save screenshot to a file, Discoverable Hotkeys via Centered Cheat-Sheet, Esc: Exit capture mode, Fullscreen Screen-Capture Selection Mode, Magnifier Tool Demo (animated GIF), Mouse: Select screenshot area, Mouse Wheel: Change tool size (+3 more)

### Community 128 - "Release & Deployment Pipeline"
Cohesion: 0.24
Nodes (11): Translation Additions, Binary SHA256 Verification Step, Release Checklist, Version Bump Locations (CMake, deb, rpm, metainfo), PREDEFINED_COLOR_PALETTE_LARGE Definition, flameshot Executable Target, FLAMESHOT_GIT_HASH Compile Definition, macdeployqt Deployment and Code Signing (+3 more)

### Community 129 - "Qt Style Override"
Cohesion: 0.20
Nodes (9): QProxyStyle, QStyleHintReturn, QStyleOption, QWidget, Q_OBJECT, StyleOverride, public, StyleOverride::styleHint() (+1 more)

### Community 130 - "Clickable Label"
Cohesion: 0.22
Nodes (9): ClickableLabel, ClickableLabel::ClickableLabel(), clicked, mousePressEvent, public, QString, QWidget, Q_OBJECT (+1 more)

### Community 131 - "Color Preset Drag State"
Cohesion: 0.18
Nodes (11): ColorPickerEditMode, colorSelected, m_config, m_draggedPresetInitialPos, m_isDragging, m_isPressing, m_mouseMovePos, m_mousePressPos (+3 more)

### Community 132 - "Monitor Preview Mouse Events"
Cohesion: 0.22
Nodes (10): mouseMoveEvent, mousePressEvent, QEvent, QPixmap, QScreen, QWidget, leaveEvent, MonitorPreview::MonitorPreview() (+2 more)

### Community 133 - "Capture Widget Construction"
Cohesion: 0.25
Nodes (11): CaptureWidget::CaptureWidget(), initButtons, initContext, initHelpMessage, initPanel, initQuitPrompt, makeChild, newShortcut (+3 more)

### Community 134 - "Tool Abstraction Docs"
Cohesion: 0.22
Nodes (10): AbstractActionTool non-drawing action base, AbstractPathTool freehand base, AbstractTwoPointTool shape-tool base, CaptureTool abstract base, ToolFactory enum-to-class tool registry, How to add a new annotation tool, CaptureTool::Type enum append-only ordering rule, Marker tool non-customisable opacity problem (+2 more)

### Community 135 - "Developer Docs Toolchain"
Cohesion: 0.22
Nodes (10): capturetool.h Doxygen Exclusion Workaround, MkDoxy API Generation Plugin, Flameshot Developer Docs MkDocs Site Config, deploy-dev-docs GitHub Workflow, dev-docs-staging Branch, Documentation Toolchain (MkDocs + Doxygen + MkDoxy), Generated HTML Post-Processing, Reference-Style Links Convention (+2 more)

### Community 136 - "Shortcuts Settings Screenshot"
Cohesion: 0.27
Nodes (10): Configuration Dialog Screenshot: Shortcuts Tab, Capture Action Shortcuts (move selection Ctrl+m, undo Ctrl+z, redo Ctrl+Shift+z, copy Ctrl+c, save Ctrl+s, quit Ctrl+q, upload, open with Ctrl+o, pin), Flameshot Configuration Dialog, Same Action Listed Once Per Binding (Copy selection to clipboard appears twice), Non-Editable Mouse Binding (greyed 'Left Double-click' for copy selection), Paint Tool Shortcuts (pencil, line d, arrow a, selection s, rectangle r, circle, marker m, text t, pixelate b, inverter i), Two-Column Description/Key Shortcut Table, Shortcuts Configuration Tab (+2 more)

### Community 137 - "Magnify Side Panel Demo"
Cohesion: 0.31
Nodes (10): Screenshot Capture Overlay UI, Color Picker Wheel with Hex Readout (#5d6c78), Discoverability via On-Canvas Shortcut Hints, Grab Color Action (eyedropper), Magnifier Tool Demo (magnify.gif), Save to File (Ctrl+S) and Copy to Clipboard (Ctrl+C) Exit Paths, Mouse Selection of Screenshot Area, In-Capture Keyboard Shortcut Cheatsheet Overlay (+2 more)

### Community 138 - "Application Entry Point"
Cohesion: 0.47
Nodes (9): QSharedMemory, QTranslator, configureApp(), configureTranslation(), guiMutexLock(), main(), reinitializeAsQApplication(), requestCaptureAndWait() (+1 more)

### Community 139 - "Tool Factory Registry"
Cohesion: 0.24
Nodes (8): CaptureTool, QObject, Type, Q_OBJECT, QObject, ToolFactory, CreateTool, public

### Community 140 - "Key Sequence Validation"
Cohesion: 0.24
Nodes (10): QKeySequence, QKeySequence, KeySequence, check, expected, fallback, KeySequence::KeySequence(), m_fallback (+2 more)

### Community 141 - "Capture Lifecycle Concerns"
Cohesion: 0.28
Nodes (9): CaptureContext per-capture transient state, CaptureRequest / ExportTask value object, CaptureWidget full-screen editor, exportCapture task-bitmask dispatch, Flameshot application singleton, Single-threaded Qt event loop constraint, Perf: full capture-object redraw on every update, Fragile: global singletons / shared mutable state (+1 more)

### Community 142 - "Build Stack & Dependencies"
Cohesion: 0.22
Nodes (9): Dependency risk: Qt6-default with Qt5 legacy path, Qt-specific idioms (Q_OBJECT, tr(), typed connects), ENABLE_IMGUR build gate, ccache compiler cache, CMake >= 3.22 build system + CPack, Notable CMake feature options, C++20 language requirement, Qt 6 widgets/GUI framework (+1 more)

### Community 143 - "Release 11.0 Daemon Changes"
Cohesion: 0.25
Nodes (9): Location-Transparent Daemon Calls via D-Bus, Single-Action (One-Off) Mode, Unresolved Fractional Scaling Issue, DBus No Longer Required For CLI, One-Off Mode (Optional Systray Daemon), Qt5 Deprecation Cleanup For Qt6, Flameshot v11.0 Beta, UNIX Qt DBus Dependency and Service Files (+1 more)

### Community 144 - "Hex Color Field Screenshot"
Cohesion: 0.36
Nodes (9): Active Color Swatch Preview, Active Tool Size Slider, HSV Color Wheel Picker with Saturation/Value Triangle, Empty State Panel (<Empty> placeholder area with delete button), Grab Color Eyedropper Button, Hex Color Text Field (#383838), Multi-Modal Color Entry (wheel, eyedropper, and exact hex text all set one active color), Annotated Side Panel Screenshot Highlighting Hex Color Field (+1 more)

### Community 145 - "Capture Launcher Screenshot"
Cohesion: 0.31
Nodes (9): Area Selector (Rectangular Region), Capture Launcher Window (Flameshot), Capture Mode Panel, Delay Selector (No Delay), WxH+x+y Geometry Inputs, Preview-Plus-Controls Launcher Layout, Pre-Capture Configuration Concept, Screenshot Preview Pane (+1 more)

### Community 146 - "Last Region Cache"
Cohesion: 0.31
Nodes (7): QRect, QString, getCachePath(), getLastRegion(), QRect, QString, setLastRegion()

### Community 147 - "Current Screen Resolver"
Cohesion: 0.39
Nodes (6): QScreen, QGuiAppCurrentScreen, currentScreen, m_currentScreen, screenAt, QScreen

### Community 148 - "Text Tool Copy & Editor"
Cohesion: 0.22
Nodes (9): CaptureTool, QObject, QWidget, closeEditor, configurationWidget, copy, copyParams, TextTool::TextTool() (+1 more)

### Community 149 - "Global Icon Paths"
Cohesion: 0.31
Nodes (6): QString, GlobalValues::iconPath(), GlobalValues::iconPathPNG(), GlobalValues::trayIconPath(), GlobalValues::versionInfo(), QString

### Community 150 - "Strftime Parsing"
Cohesion: 0.53
Nodes (7): create_specifier_list(), format_time_string(), match_specifiers(), replace_all(), split(), string, vector

### Community 151 - "Uploader Plugin Gap Docs"
Cohesion: 0.36
Nodes (8): ImgUploaderBase backend-agnostic upload widget, ImgUploaderManager backend selector, Hard-coded single uploader backend (Imgur), Missing feature: pluggable upload backends, Imgur image hosting integration, Local upload history storage (imgur.{deletehash}.{name}), OpenSSL (Windows HTTPS uploads), How to add a new uploader backend

### Community 152 - "End-User Docs & Drive Option"
Cohesion: 0.25
Nodes (8): How to Make a Screenshot (select area, Enter, Space, Esc), Flameshot Screenshot Tool (end-user overview), Paint Tool Options (pencil, arrow, marker, text, blur), Upload-to-S3-Bucket Toolbar Action, ENABLE_GDRIVE Build Option, "Flameshot screenshots" Fixed Folder, Google Drive Upload Feature (opt-in build), U8 Admin Setup Guide and Packaging Check

### Community 153 - "Config Permission Hardening"
Cohesion: 0.25
Nodes (8): Disconnect Action (clear + revoke credentials), Least-Privilege drive.file Scope, Refresh Token Is the Value to Protect, R-E Silent Server-Side Revocation Failure on Disconnect, ConfigHandler::flush() (m_settings.sync()), QSettings Deferred Write Defeats chmod Hardening, reassertConfigPermissions (write-then-chmod order), Rejected Alternative: process-wide umask(0077)

### Community 154 - "Region Launcher Screenshot"
Cohesion: 0.32
Nodes (8): Area Selector (Rectangular Region), Capture Launcher Window (Rectangular Region mode), Capture Mode Settings Panel, Delay Selector (No Delay), WxH+x+y Geometry Input Fields, Rectangular Region mode reveals explicit geometry fields, Multi-Monitor Screen Preview Thumbnail, Take New Screenshot Action Button

### Community 155 - "Qt Designer UI Authoring"
Cohesion: 0.36
Nodes (8): launcher.ui Form (QWidget root with QHBoxLayout), Lowering the GUI Contribution Barrier for Non-C++ Developers, Object Inspector (Object/Class Tree), Property Editor (objectName and QWidget properties), Qt Designer Screenshot Editing launcher.ui, Signal/Slot Editor (Sender, Signal, Receiver, Slot), Qt Designer .ui XML Widget Authoring Workflow, Widget Box Palette (Layouts, Spacers, Buttons, Item Views)

### Community 156 - "Dialog Lifetime Crash Fixes"
Cohesion: 0.25
Nodes (8): KD4 Default Visibility = Anyone in Org With Link, KTD6 Org Domain Derived From Authenticated Account, KTD7 Sharing UI Inside Existing Upload Confirmation Dialog, KTD2 Defer Export With Zero-Delay Single-Shot, KTD4 Guarded m_captureWindow Pointer in Deferred Handler, KTD6 Block-Scoped ImgUploadDialog Without WA_DeleteOnClose, Destructor Re-Entrancy Crash on Drive Upload Confirmation, uploadWithoutConfirmation Red-Herring Hypothesis

### Community 157 - "Debian Packaging Build"
Cohesion: 0.29
Nodes (8): Flameshot .deb Build Procedure, Packaging Metadata Lives in packaging/debian, Not Root debian/, Parent-Directory Write Trap at dpkg-deb Step, debian/rules Quirks: .git for syncqt, FETCHCONTENT_FULLY_DISCONNECTED=OFF, Assemble .deb Directly From Staged debian/flameshot Tree, -DENABLE_GDRIVE=ON in packaging/debian/rules, Silent Feature Omission From Default-OFF Build Flag, Stale CMakeCache Trap Makes a "Fixed" Build Lie

### Community 158 - "Color Luma Utilities"
Cohesion: 0.39
Nodes (6): QColor, ColorUtils::colorIsDark(), ColorUtils::contrastColor(), qreal, getColorLuma(), Color::Color()

### Community 159 - "Value Handler Declarations"
Cohesion: 0.25
Nodes (7): QKeySequence, CaptureTool, Bool, check, expected, fallback, m_def

### Community 160 - "User Color Config Type"
Cohesion: 0.25
Nodes (8): UserColors, check, expected, fallback, m_max, m_min, process, representation

### Community 161 - "Selection Size Indicator"
Cohesion: 0.32
Nodes (8): changeEvent, extendedRect, extendedSelection, initSelection, showxywh, updateSizeIndicator, QEvent, QRect

### Community 162 - "OAuth Client & Sharing Scopes"
Cohesion: 0.29
Nodes (7): Capability-URL Leakage Risk, Internal Workspace Consent Screen, Org-Registered Desktop-App OAuth Client, Scope Upgrade Re-Consent on Existing Grants, Four Sharing Visibility Levels, openid/email Sign-In Scopes for Org Domain, KD1 Bring-Your-Own Org OAuth Client

### Community 163 - "Counter Tool Demo"
Cohesion: 0.43
Nodes (7): Auto-Incrementing Numbered Badge Placement, Floating Capture Toolbar, Circle Count (Counter) Annotation Tool, Counter Tool Animated Demo (Flameshot GitHub page annotated), Live Selection Dimension Readout (890x454), Screenshot Selection Region with Drag Handles, Edge-Docked Side Panel Toggle Button

### Community 164 - "Invert Tool Demo"
Cohesion: 0.43
Nodes (7): Capture Mode Shortcut Bindings (Mouse, Ctrl+S, Ctrl+C, Wheel, Right Click, Space, Esc), Crosshair Capture Cursor, Invert Tool Demo Animation (invert.gif), Flameshot Screenshot Application, Color Inversion Tool, Selection Region Applied Effect Scope, In-Capture Keyboard Shortcut Help Overlay

### Community 165 - "Backend Resolution Mismatch"
Cohesion: 0.33
Nodes (6): KTD8 Drive-Safe History Packing and Per-Type Links, U3 Drive-Safe History Packing Unit, R-D Drive-Only Build Misroutes Legacy Imgur History Deletes, Backend-Resolution Mismatch Hides Drive Visibility UI, Raw Config vs Resolved State Divergence (defect class), Single-Backend-Compiled-In Fallback Branch

### Community 166 - "Capture Completion Handoff"
Cohesion: 0.29
Nodes (7): Capture-Completion Signal Carrying Pixmap, Geometry, CaptureRequest, Flameshot Owns Post-Capture Lifecycle, GNOME/Wayland Clipboard-Hold Workaround, KTD1 Emit Completion From closeEvent, KTD3 Schedule Teardown Before Running Export, KTD5 No "Capture Aborted" for Widget Destroyed Without Close, KTD7 gui() Checks Construction-Time Grab Failure

### Community 167 - "Config Handler Declarations"
Cohesion: 0.29
Nodes (6): QSettings, exportFileConfiguration, AbstractLogger, QFileSystemWatcher, QTextStream, ValueHandler

### Community 168 - "Color Preset Edit Mode"
Cohesion: 0.29
Nodes (5): ColorPickerEditMode::ColorPickerEditMode(), eventFilter, QEvent, QObject, QWidget

### Community 169 - "Tool Selection Rendering"
Cohesion: 0.33
Nodes (6): QPainter, QPixmap, QRect, boundingRect, drawObjectSelection, process

### Community 170 - "Bounded Integer Config"
Cohesion: 0.29
Nodes (7): BoundedInt, check, expected, fallback, m_def, m_max, m_min

### Community 171 - "Capture Widget Mouse Actions"
Cohesion: 0.43
Nodes (7): activeToolObject, deleteCurrentTool, drawObjectSelection, mousePressEvent, selectToolItemAtPos, showColorPicker, QPointer

### Community 172 - "Action Options Test Script"
Cohesion: 0.52
Nodes (6): cmd(), display_img(), flameshot(), notify(), action_options.sh script, wait_for_key()

### Community 173 - "Capture Lifecycle Test Script"
Cohesion: 0.57
Nodes (5): cmd(), expect(), expect_on(), capture_lifecycle.sh script, wait_for_key()

### Community 174 - "Config Handler Docs"
Cohesion: 0.40
Nodes (6): ConfigHandler QSettings singleton, ValueHandler typed config validation, Security: external process launching via QProcess arg vectors, Hard-coded default Imgur Client-ID, QSettings INI runtime config storage, How to add a new config option

### Community 175 - "Manual Shell Test Suite"
Cohesion: 0.33
Nodes (6): Fragile: 168 platform #ifdef occurrences, tests/action_options.sh, tests/capture_lifecycle.sh, Manual interactive shell test scripts, tests/path_option.sh, Platform-gated manual test cases rule

### Community 176 - "Monochrome Tray Icon Screenshot"
Cohesion: 0.53
Nodes (6): Yellow Callout Arrow Annotation, Flame Application Glyph, macOS Menu Bar / System Tray Integration, Monochrome Tray Icon Option, Monochrome Tray Icon Screenshot (macOS menu bar), Neighboring System Status Icons (battery, wifi, search, control center)

### Community 177 - "Pixelate Redaction Demo"
Cohesion: 0.40
Nodes (6): Drag-to-Select Region Interaction, Flameshot Capture Overlay, GitHub Pull Request List of flameshot-org/flameshot (Demo Subject), Pixelate Tool Demo Animation, Pixelate Annotation Tool, Screenshot Redaction of Sensitive Regions

### Community 178 - "OAuth Implementation Decisions"
Cohesion: 0.33
Nodes (6): GDriveOAuth (process-wide auth service), KD6 Loopback-Redirect OAuth With System Browser, KTD1 Hand-Rolled OAuth on QTcpServer + QNetworkAccessManager, KTD2 PKCE S256 + State + Forced Consent Mechanics, KTD9 Single-Flight, Bounded, Cancelable Shared Auth Service, R-B Loopback Listener Parses One readyRead Chunk

### Community 179 - "Build Matrix Verification"
Cohesion: 0.33
Nodes (6): KTD3 ENABLE_GDRIVE + Backend-Neutral ENABLE_UPLOADER Gate, U1 Introduce ENABLE_GDRIVE and Backend-Neutral Gating, Verification Contract: Build Matrix + Manual Acceptance (no C++ test framework), tests/capture_lifecycle.sh Manual Walkthrough, No Automated Test Coverage (plan-level accepted condition), ENABLE_GDRIVE Implies ENABLE_UPLOADER (wider surface)

### Community 180 - "Portal Request Interface"
Cohesion: 0.33
Nodes (4): QDBusAbstractInterface, QDBusPendingReply, OrgFreedesktopPortalRequestInterface, Response

### Community 181 - "Config Error Details Dialog"
Cohesion: 0.40
Nodes (4): ConfigErrorDetails, QDialog, QT_BEGIN_NAMESPACE, namespace()

### Community 182 - "Update Info Text"
Cohesion: 0.40
Nodes (6): QString, description, info, name, updateFamily, updateText

### Community 183 - "Path & Icon Lookup"
Cohesion: 0.40
Nodes (5): QString, QStringList, PathInfo::blackIconPath(), PathInfo::translationsPaths(), PathInfo::whiteIconPath()

### Community 184 - "String Config Handler"
Cohesion: 0.33
Nodes (6): QString, String, check, expected, fallback, m_def

### Community 185 - "Lower Bounded Integer Config"
Cohesion: 0.33
Nodes (6): LowerBoundedInt, check, expected, fallback, m_def, m_min

### Community 186 - "Capture Widget Painting"
Cohesion: 0.47
Nodes (6): drawErrorMessage, drawInactiveRegion, paintEvent, CaptureWidget::showAppUpdateNotification(), QPainter, QString

### Community 187 - "Windows CLI Wrapper"
Cohesion: 0.60
Nodes (5): CallFlameshot(), joinArgs(), wmain(), wchar_t, wstring

### Community 188 - "Drive Identity Test Script"
Cohesion: 0.73
Nodes (5): expect(), expect_with(), gdrive_account_identity.sh script, show_identity(), wait_for_key()

### Community 189 - "Translation Pipeline"
Cohesion: 0.40
Nodes (5): Qt Linguist .ts files are not TypeScript, Bug: UI language change requires restart, Weblate translation pipeline, Qt Linguist .ts translation sources, data/ resources directory

### Community 190 - "Static Analysis Posture"
Cohesion: 0.40
Nodes (5): Static analysis posture as test substitute, Near-total absence of automated tests, clang-tidy all-checks with WarningsAsErrors, No automated unit-test suite, Vacuously passing ctest step

### Community 191 - "Contribution Process"
Cohesion: 0.40
Nodes (5): clang-format Code Formatting Requirement, Code Contribution Process (fork, format, branch, PR), Issue Reporting Guidelines, RFC Process for Larger Changes, Translation Contributions

### Community 192 - "Counter Pointer Demo"
Cohesion: 0.60
Nodes (5): Numbered Counter With Pointer Demo Screenshot, Drag-To-Place Pointer Tail Gesture (drag while placing a marker to grow a leader line toward the feature being labelled, so a numbered badge can sit in clear space instead of covering the subject), Incremental Counter Marker Tool, Leader-Line Callout Annotation Pattern, Screenshot Annotation Toolset

### Community 193 - "Drive Folder Caching Risks"
Cohesion: 0.40
Nodes (5): KD3 Single Folder Confined by drive.file, KD5 Refresh Token in Plaintext Config, KTD5 Cached Folder ID With 404 Re-Discovery, R-A Concurrent First-Run Duplicate Folder Race, R-C Stale Folder Cache Heals Only on 404

### Community 195 - "Naming Conventions"
Cohesion: 0.50
Nodes (4): Anonymous-namespace internal helpers, m_ prefix for member variables, Naming patterns (lowercase files, PascalCase classes), File name mirrors primary class name

### Community 196 - "CI & Code Signing"
Cohesion: 0.67
Nodes (4): clang-format Mozilla style (pinned v11), GitHub Actions + AppVeyor CI pipeline, SignPath Windows code-signing webhook, CI correctness gates (compile + format lint)

### Community 197 - "Text Tool Header"
Cohesion: 0.50
Nodes (3): QPointer, TextConfig, TextWidget

### Community 198 - "Pin Widget Construction"
Cohesion: 0.50
Nodes (4): QPixmap, QRect, QWidget, PinWidget::PinWidget()

### Community 199 - "Button Handler Setup"
Cohesion: 0.67
Nodes (4): ButtonHandler::ButtonHandler(), setButtons, CaptureToolButton, QObject

### Community 200 - "Capture Tool Population"
Cohesion: 0.50
Nodes (4): CaptureTool, QList, QPointer, fillCaptureTools

### Community 201 - "Drive Uploader Construction"
Cohesion: 0.67
Nodes (3): QPixmap, QWidget, GDriveUploader::GDriveUploader()

### Community 202 - "Color Grab Construction"
Cohesion: 0.67
Nodes (3): ColorGrabWidget::ColorGrabWidget(), QPixmap, QWidget

## Ambiguous Edges - Review These
- `Circle Count (Counter) Annotation Tool` → `Edge-Docked Side Panel Toggle Button`  [AMBIGUOUS]
  docs/images/counter.gif · relation: conceptually_related_to
- `Invert Tool Demo Animation (invert.gif)` → `Crosshair Capture Cursor`  [AMBIGUOUS]
  docs/images/invert.gif · relation: references

## Knowledge Gaps
- **878 isolated node(s):** `post-process.sh script`, `m_name`, `m_description`, `m_withHelp`, `m_withVersion` (+873 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **13 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **What is the exact relationship between `Circle Count (Counter) Annotation Tool` and `Edge-Docked Side Panel Toggle Button`?**
  _Edge tagged AMBIGUOUS (relation: conceptually_related_to) - confidence is low._
- **What is the exact relationship between `Invert Tool Demo Animation (invert.gif)` and `Crosshair Capture Cursor`?**
  _Edge tagged AMBIGUOUS (relation: references) - confidence is low._
- **Why does `ConfigHandler` connect `Config Change Signals` to `Selection Widget Geometry`, `General Settings Tab`, `Capture Widget Signals`, `Color Preset Drag State`, `Google Drive OAuth`, `Screenshot Saving & Clipboard`, `Flameshot Application Singleton`, `Daemon & D-Bus Service`, `Google Drive Uploader`, `Invert & Pixelate Tools`, `Application Entry Point`, `Upload Widget Base`, `Monitor Preview Mouse Events`, `Capture Widget Construction`, `Arrow Tool`, `Capture Request Value Object`, `Pin Widget Gestures`, `General Config Initializers`, `Update Notification Widget`, `Screen Grabbing Backends`, `Text Tool Copy & Editor`, `Text Tool Configuration`, `Shortcuts Settings Widget`, `Config Handler Implementation`, `Color Preset Editor`, `Overlay Messages & Spinner`, `Capture Tool Button`, `User Color Config Type`, `Filename Editor Tab`, `Tray Icon & Launcher`, `Visuals Settings Editor`, `Text Input Widget`, `Config Handler Declarations`, `Color Preset Edit Mode`, `Upload History Store`, `Notifier Box`, `Configuration Window`, `Config Value Handlers`, `Update Info Text`, `Imgur Uploader`, `Uploader Backend Manager`, `Capture Widget Painting`, `Capture Resize & Open-With`, `Pin Widget Construction`, `Button List View`, `Drive Uploader Construction`, `Capture Button Styling`, `App Launcher List Population`, `Text Tool Drawing`, `Capture Tool State Machine`, `Config Error Resolver`, `Active Tool Size & Color`, `Color Picker Popup`, `Upload History Line Item`, `UI Color Editor Logic`, `Upload Dialog Recipients`, `System Notifications`, `Color Picker Grid Painting`?**
  _High betweenness centrality (0.275) - this node is a cross-community bridge._
- **Why does `QColor` connect `Color Luma Utilities` to `Selection Widget Geometry`, `Capture Widget Signals`, `Capture Widget Construction`, `Overlay Messages & Spinner`, `Invert & Pixelate Tools`, `Update Notification Widget`, `Utility Side Panel`, `Arrow Tool`, `Pin Widget Gestures`, `Color Grab Widget`, `Side Panel Widget`, `Path Tool Base`, `Color Preset Editor`, `Capture Tool Button`, `Two Point Tool Base`, `Circle Counter Tool`, `Value Handler Declarations`, `User Color Config Type`, `Text Input Widget`, `UI Color Editor`, `Magnifier Widget`, `Marker Tool`, `Notifier Box`, `Capture Context Drawing`, `Config Value Handlers`, `App Launcher Tool`, `Pencil Tool`, `Pin Tool`, `Text Tool Declarations`, `Image Label Widget`, `Capture Widget Painting`, `Selection Tool`, `Circle Tool`, `Upload Tool Button`, `Line Tool`, `Rectangle Tool`, `Toolbar Button Positioning`, `Move Tool`, `Capture Tool Population`, `Accept Tool`, `Copy Tool`, `Exit Tool`, `Redo Tool`, `Save Tool`, `Size Decrease Tool`, `Size Increase Tool`, `Icon Accessor`, `Undo Tool`, `Capture Button Styling`, `Monitor Preview Widget`, `Text Tool Drawing`, `Value Handler Types`, `Active Tool Size & Color`, `Color Palette Definitions`, `Side Panel Color Grabbing`, `UI Color Editor Logic`, `Color Picker Grid Painting`?**
  _High betweenness centrality (0.144) - this node is a cross-community bridge._
- **Why does `CaptureWidget` connect `Capture Widget Signals` to `Selection Widget Geometry`, `Capture Widget Construction`, `Config Change Signals`, `Capture Tool Object Store`, `Color Luma Utilities`, `Selection Size Indicator`, `Capture Widget Event Handling`, `Toolbar Button Handler`, `Magnifier Widget`, `Capture Widget Mouse Actions`, `Capture Context Drawing`, `Imgur Uploader`, `Capture Widget Painting`, `Toolbar Button Positioning`, `Capture Resize & Open-With`, `Button List View`, `Capture Tool State Machine`, `Active Tool Size & Color`, `Color Picker Grid Painting`?**
  _High betweenness centrality (0.070) - this node is a cross-community bridge._
- **Are the 28 inferred relationships involving `ConfigHandler` (e.g. with `updateComponents` and `appendShortcut`) actually correct?**
  _`ConfigHandler` has 28 INFERRED edges - model-reasoned connections that need verification._
- **Are the 11 inferred relationships involving `QColor` (e.g. with `process` and `enterEvent`) actually correct?**
  _`QColor` has 11 INFERRED edges - model-reasoned connections that need verification._