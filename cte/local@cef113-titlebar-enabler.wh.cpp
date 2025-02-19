// ==WindhawkMod==
// @id              cef113-titlebar-enabler
// @name            CEF/Spotify Titlebar Enabler for CEF 113
// @description     Force native frames and title bars for CEF apps
// @version         0.1
// @author          Ingan121
// @github          https://github.com/Ingan121
// @twitter         https://twitter.com/Ingan121
// @homepage        https://www.ingan121.com/
// @include         spotify.exe
// @include         cefclient.exe
// @compilerOptions -lcomctl32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# CEF Titlebar Enabler
* Force native frames and title bars for CEF apps, such as Spotify
* Add other CEF apps on your own
* May not work or look bad in other CEF apps
* Electron apps are NOT supported! Just patch asar to override frame: false to true in BrowserWindow creation
* Tested CEF versions: 102 and 106
* Tested Spotify versions: 1.1.89 and 1.1.97
    * Neither DWM nor non-DWM window controls work properly in these versions
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
// ==/WindhawkModSettings==

/*
CEF_EXPORT cef_window_t* cef_window_create_top_level(
    struct _cef_window_delegate_t* delegate);*/

#include <cstdint>
#include <windhawk_utils.h>

#define CEF_CALLBACK __stdcall
#define cef_window_handle_t HWND
typedef unsigned int uint32;
typedef wchar_t char16;

#pragma region Dirty CEF headers copypasta

// typedef struct _cef_string_utf8_t {
//   char* str;
//   size_t length;
//   void (*dtor)(char* str);
// } cef_string_utf8_t;

// typedef cef_string_utf8_t cef_string_t;
// typedef cef_string_utf8_t* cef_string_userfree_utf8_t;
// typedef cef_string_userfree_utf8_t cef_string_userfree_t;

typedef struct _cef_string_utf16_t {
  char16_t* str;
  size_t length;
  void (*dtor)(char16_t* str);
} cef_string_utf16_t;

typedef cef_string_utf16_t cef_string_t;
typedef cef_string_utf16_t* cef_string_userfree_utf16_t;
typedef cef_string_userfree_utf16_t cef_string_userfree_t;

typedef uint32_t cef_color_t;

///
/// Represents the state of a setting.
///
typedef enum {
  ///
  /// Use the default state for the setting.
  ///
  STATE_DEFAULT = 0,

  ///
  /// Enable or allow the setting.
  ///
  STATE_ENABLED,

  ///
  /// Disable or disallow the setting.
  ///
  STATE_DISABLED,
} cef_state_t;

///
// Show states supported by CefWindowDelegate::GetInitialShowState.
///
typedef enum {
  CEF_SHOW_STATE_NORMAL = 1,
  CEF_SHOW_STATE_MINIMIZED,
  CEF_SHOW_STATE_MAXIMIZED,
  CEF_SHOW_STATE_FULLSCREEN,
} cef_show_state_t;

///
/// Key event types.
///
typedef enum {
  ///
  /// Notification that a key transitioned from "up" to "down".
  ///
  KEYEVENT_RAWKEYDOWN = 0,

  ///
  /// Notification that a key was pressed. This does not necessarily correspond
  /// to a character depending on the key and language. Use KEYEVENT_CHAR for
  /// character input.
  ///
  KEYEVENT_KEYDOWN,

  ///
  /// Notification that a key was released.
  ///
  KEYEVENT_KEYUP,

  ///
  /// Notification that a character was typed. Use this for text input. Key
  /// down events may generate 0, 1, or more than one character event depending
  /// on the key, locale, and operating system.
  ///
  KEYEVENT_CHAR
} cef_key_event_type_t;

///
/// Specifies how a menu will be anchored for non-RTL languages. The opposite
/// position will be used for RTL languages.
///
typedef enum {
  CEF_MENU_ANCHOR_TOPLEFT,
  CEF_MENU_ANCHOR_TOPRIGHT,
  CEF_MENU_ANCHOR_BOTTOMCENTER,
} cef_menu_anchor_position_t;

///
// Specifies where along the main axis the CefBoxLayout child views should be
// laid out.
///
typedef enum {
  ///
  // Child views will be left-aligned.
  ///
  CEF_MAIN_AXIS_ALIGNMENT_START,

  ///
  // Child views will be center-aligned.
  ///
  CEF_MAIN_AXIS_ALIGNMENT_CENTER,

  ///
  // Child views will be right-aligned.
  ///
  CEF_MAIN_AXIS_ALIGNMENT_END,
} cef_main_axis_alignment_t;

///
// Specifies where along the cross axis the CefBoxLayout child views should be
// laid out.
///
typedef enum {
  ///
  // Child views will be stretched to fit.
  ///
  CEF_CROSS_AXIS_ALIGNMENT_STRETCH,

  ///
  // Child views will be left-aligned.
  ///
  CEF_CROSS_AXIS_ALIGNMENT_START,

  ///
  // Child views will be center-aligned.
  ///
  CEF_CROSS_AXIS_ALIGNMENT_CENTER,

  ///
  // Child views will be right-aligned.
  ///
  CEF_CROSS_AXIS_ALIGNMENT_END,
} cef_cross_axis_alignment_t;

///
// Structure representing keyboard event information.
///
typedef struct _cef_key_event_t {
  ///
  // The type of keyboard event.
  ///
  cef_key_event_type_t type;

  ///
  // Bit flags describing any pressed modifier keys. See
  // cef_event_flags_t for values.
  ///
  uint32 modifiers;

  ///
  // The Windows key code for the key event. This value is used by the DOM
  // specification. Sometimes it comes directly from the event (i.e. on
  // Windows) and sometimes it's determined using a mapping function. See
  // WebCore/platform/chromium/KeyboardCodes.h for the list of values.
  ///
  int windows_key_code;

  ///
  // The actual key code genenerated by the platform.
  ///
  int native_key_code;

  ///
  // Indicates whether the event is considered a "system key" event (see
  // http://msdn.microsoft.com/en-us/library/ms646286(VS.85).aspx for details).
  // This value will always be false on non-Windows platforms.
  ///
  int is_system_key;

  ///
  // The character generated by the keystroke.
  ///
  char16 character;

  ///
  // Same as |character| but unmodified by any concurrently-held modifiers
  // (except shift). This is useful for working out shortcut keys.
  ///
  char16 unmodified_character;

  ///
  // True if the focus is currently on an editable field on the page. This is
  // useful for determining if standard key events should be intercepted.
  ///
  int focus_on_editable_field;
} cef_key_event_t;

///
/// Structure representing a point.
///
typedef struct _cef_point_t {
  int x;
  int y;
} cef_point_t;

///
/// Structure representing a rectangle.
///
typedef struct _cef_rect_t {
  int x;
  int y;
  int width;
  int height;
} cef_rect_t;

///
/// Structure representing a size.
///
typedef struct _cef_size_t {
  int width;
  int height;
} cef_size_t;

///
/// Structure representing insets.
///
typedef struct _cef_insets_t {
  int top;
  int left;
  int bottom;
  int right;
} cef_insets_t;

///
/// Docking modes supported by CefWindow::AddOverlay.
///
typedef enum {
  CEF_DOCKING_MODE_TOP_LEFT = 1,
  CEF_DOCKING_MODE_TOP_RIGHT,
  CEF_DOCKING_MODE_BOTTOM_LEFT,
  CEF_DOCKING_MODE_BOTTOM_RIGHT,
  CEF_DOCKING_MODE_CUSTOM,
} cef_docking_mode_t;

///
/// Mouse button types.
///
typedef enum {
  MBT_LEFT = 0,
  MBT_MIDDLE,
  MBT_RIGHT,
} cef_mouse_button_type_t;

// Structure representing a draggable region.
///
typedef struct _cef_draggable_region_t {
  ///
  // Bounds of the region.
  ///
  cef_rect_t bounds;

  ///
  // True (1) this this region is draggable and false (0) otherwise.
  ///
  int draggable;
} cef_draggable_region_t;

///
// All ref-counted framework structures must include this structure first.
///
typedef struct _cef_base_ref_counted_t {
  ///
  // Size of the data structure.
  ///
  size_t size;

  ///
  // Called to increment the reference count for the object. Should be called
  // for every new copy of a pointer to a given object.
  ///
  void(CEF_CALLBACK* add_ref)(struct _cef_base_ref_counted_t* self);

  ///
  // Called to decrement the reference count for the object. If the reference
  // count falls to 0 the object should self-delete. Returns true (1) if the
  // resulting reference count is 0.
  ///
  int(CEF_CALLBACK* release)(struct _cef_base_ref_counted_t* self);

  ///
  // Returns true (1) if the current reference count is 1.
  ///
  int(CEF_CALLBACK* has_one_ref)(struct _cef_base_ref_counted_t* self);

  ///
  // Returns true (1) if the current reference count is at least 1.
  ///
  int(CEF_CALLBACK* has_at_least_one_ref)(struct _cef_base_ref_counted_t* self);
} cef_base_ref_counted_t;

///
/// Implement this structure to handle view events. All size and position values
/// are in density independent pixels (DIP) unless otherwise indicated. The
/// functions of this structure will be called on the browser process UI thread
/// unless otherwise indicated.
///
typedef struct _cef_view_delegate_t {
  ///
  /// Base structure.
  ///
  cef_base_ref_counted_t base;

  ///
  /// Return the preferred size for |view|. The Layout will use this information
  /// to determine the display size.
  ///
  cef_size_t(CEF_CALLBACK* get_preferred_size)(
      struct _cef_view_delegate_t* self,
      struct _cef_view_t* view);

  ///
  /// Return the minimum size for |view|.
  ///
  cef_size_t(CEF_CALLBACK* get_minimum_size)(struct _cef_view_delegate_t* self,
                                             struct _cef_view_t* view);

  ///
  /// Return the maximum size for |view|.
  ///
  cef_size_t(CEF_CALLBACK* get_maximum_size)(struct _cef_view_delegate_t* self,
                                             struct _cef_view_t* view);

  ///
  /// Return the height necessary to display |view| with the provided |width|.
  /// If not specified the result of get_preferred_size().height will be used by
  /// default. Override if |view|'s preferred height depends upon the width (for
  /// example, with Labels).
  ///
  int(CEF_CALLBACK* get_height_for_width)(struct _cef_view_delegate_t* self,
                                          struct _cef_view_t* view,
                                          int width);

  ///
  /// Called when the parent of |view| has changed. If |view| is being added to
  /// |parent| then |added| will be true (1). If |view| is being removed from
  /// |parent| then |added| will be false (0). If |view| is being reparented the
  /// remove notification will be sent before the add notification. Do not
  /// modify the view hierarchy in this callback.
  ///
  void(CEF_CALLBACK* on_parent_view_changed)(struct _cef_view_delegate_t* self,
                                             struct _cef_view_t* view,
                                             int added,
                                             struct _cef_view_t* parent);

  ///
  /// Called when a child of |view| has changed. If |child| is being added to
  /// |view| then |added| will be true (1). If |child| is being removed from
  /// |view| then |added| will be false (0). If |child| is being reparented the
  /// remove notification will be sent to the old parent before the add
  /// notification is sent to the new parent. Do not modify the view hierarchy
  /// in this callback.
  ///
  void(CEF_CALLBACK* on_child_view_changed)(struct _cef_view_delegate_t* self,
                                            struct _cef_view_t* view,
                                            int added,
                                            struct _cef_view_t* child);

  ///
  /// Called when |view| is added or removed from the cef_window_t.
  ///
  void(CEF_CALLBACK* on_window_changed)(struct _cef_view_delegate_t* self,
                                        struct _cef_view_t* view,
                                        int added);

  ///
  /// Called when the layout of |view| has changed.
  ///
  void(CEF_CALLBACK* on_layout_changed)(struct _cef_view_delegate_t* self,
                                        struct _cef_view_t* view,
                                        const cef_rect_t* new_bounds);

  ///
  /// Called when |view| gains focus.
  ///
  void(CEF_CALLBACK* on_focus)(struct _cef_view_delegate_t* self,
                               struct _cef_view_t* view);

  ///
  /// Called when |view| loses focus.
  ///
  void(CEF_CALLBACK* on_blur)(struct _cef_view_delegate_t* self,
                              struct _cef_view_t* view);
} cef_view_delegate_t;

typedef struct _cef_panel_delegate_t {
  ///
  /// Base structure.
  ///
  cef_view_delegate_t base;
} cef_panel_delegate_t;


///
/// A View is a rectangle within the views View hierarchy. It is the base
/// structure for all Views. All size and position values are in density
/// independent pixels (DIP) unless otherwise indicated. Methods must be called
/// on the browser process UI thread unless otherwise indicated.
///
typedef struct _cef_view_t {
  ///
  /// Base structure.
  ///
  cef_base_ref_counted_t base;

  ///
  /// Returns this View as a BrowserView or NULL if this is not a BrowserView.
  ///
  struct _cef_browser_view_t*(CEF_CALLBACK* as_browser_view)(
      struct _cef_view_t* self);

  ///
  /// Returns this View as a Button or NULL if this is not a Button.
  ///
  struct _cef_button_t*(CEF_CALLBACK* as_button)(struct _cef_view_t* self);

  ///
  /// Returns this View as a Panel or NULL if this is not a Panel.
  ///
  struct _cef_panel_t*(CEF_CALLBACK* as_panel)(struct _cef_view_t* self);

  ///
  /// Returns this View as a ScrollView or NULL if this is not a ScrollView.
  ///
  struct _cef_scroll_view_t*(CEF_CALLBACK* as_scroll_view)(
      struct _cef_view_t* self);

  ///
  /// Returns this View as a Textfield or NULL if this is not a Textfield.
  ///
  struct _cef_textfield_t*(CEF_CALLBACK* as_textfield)(
      struct _cef_view_t* self);

  ///
  /// Returns the type of this View as a string. Used primarily for testing
  /// purposes.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t(CEF_CALLBACK* get_type_string)(
      struct _cef_view_t* self);

  ///
  /// Returns a string representation of this View which includes the type and
  /// various type-specific identifying attributes. If |include_children| is
  /// true (1) any child Views will also be included. Used primarily for testing
  /// purposes.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t(CEF_CALLBACK* to_string)(struct _cef_view_t* self,
                                                 int include_children);

  ///
  /// Returns true (1) if this View is valid.
  ///
  int(CEF_CALLBACK* is_valid)(struct _cef_view_t* self);

  ///
  /// Returns true (1) if this View is currently attached to another View. A
  /// View can only be attached to one View at a time.
  ///
  int(CEF_CALLBACK* is_attached)(struct _cef_view_t* self);

  ///
  /// Returns true (1) if this View is the same as |that| View.
  ///
  int(CEF_CALLBACK* is_same)(struct _cef_view_t* self,
                             struct _cef_view_t* that);

  ///
  /// Returns the delegate associated with this View, if any.
  ///
  struct _cef_view_delegate_t*(CEF_CALLBACK* get_delegate)(
      struct _cef_view_t* self);

  ///
  /// Returns the top-level Window hosting this View, if any.
  ///
  struct _cef_window_t*(CEF_CALLBACK* get_window)(struct _cef_view_t* self);

  ///
  /// Returns the ID for this View.
  ///
  int(CEF_CALLBACK* get_id)(struct _cef_view_t* self);

  ///
  /// Sets the ID for this View. ID should be unique within the subtree that you
  /// intend to search for it. 0 is the default ID for views.
  ///
  void(CEF_CALLBACK* set_id)(struct _cef_view_t* self, int id);

  ///
  /// Returns the group id of this View, or -1 if not set.
  ///
  int(CEF_CALLBACK* get_group_id)(struct _cef_view_t* self);

  ///
  /// A group id is used to tag Views which are part of the same logical group.
  /// Focus can be moved between views with the same group using the arrow keys.
  /// The group id is immutable once it's set.
  ///
  void(CEF_CALLBACK* set_group_id)(struct _cef_view_t* self, int group_id);

  ///
  /// Returns the View that contains this View, if any.
  ///
  struct _cef_view_t*(CEF_CALLBACK* get_parent_view)(struct _cef_view_t* self);

  ///
  /// Recursively descends the view tree starting at this View, and returns the
  /// first child that it encounters with the given ID. Returns NULL if no
  /// matching child view is found.
  ///
  struct _cef_view_t*(CEF_CALLBACK* get_view_for_id)(struct _cef_view_t* self,
                                                     int id);

  ///
  /// Sets the bounds (size and position) of this View. |bounds| is in parent
  /// coordinates, or DIP screen coordinates if there is no parent.
  ///
  void(CEF_CALLBACK* set_bounds)(struct _cef_view_t* self,
                                 const cef_rect_t* bounds);

  ///
  /// Returns the bounds (size and position) of this View in parent coordinates,
  /// or DIP screen coordinates if there is no parent.
  ///
  cef_rect_t(CEF_CALLBACK* get_bounds)(struct _cef_view_t* self);

  ///
  /// Returns the bounds (size and position) of this View in DIP screen
  /// coordinates.
  ///
  cef_rect_t(CEF_CALLBACK* get_bounds_in_screen)(struct _cef_view_t* self);

  ///
  /// Sets the size of this View without changing the position. |size| in parent
  /// coordinates, or DIP screen coordinates if there is no parent.
  ///
  void(CEF_CALLBACK* set_size)(struct _cef_view_t* self,
                               const cef_size_t* size);

  ///
  /// Returns the size of this View in parent coordinates, or DIP screen
  /// coordinates if there is no parent.
  ///
  cef_size_t(CEF_CALLBACK* get_size)(struct _cef_view_t* self);

  ///
  /// Sets the position of this View without changing the size. |position| is in
  /// parent coordinates, or DIP screen coordinates if there is no parent.
  ///
  void(CEF_CALLBACK* set_position)(struct _cef_view_t* self,
                                   const cef_point_t* position);

  ///
  /// Returns the position of this View. Position is in parent coordinates, or
  /// DIP screen coordinates if there is no parent.
  ///
  cef_point_t(CEF_CALLBACK* get_position)(struct _cef_view_t* self);

  ///
  /// Sets the insets for this View. |insets| is in parent coordinates, or DIP
  /// screen coordinates if there is no parent.
  ///
  void(CEF_CALLBACK* set_insets)(struct _cef_view_t* self,
                                 const cef_insets_t* insets);

  ///
  /// Returns the insets for this View in parent coordinates, or DIP screen
  /// coordinates if there is no parent.
  ///
  cef_insets_t(CEF_CALLBACK* get_insets)(struct _cef_view_t* self);

  ///
  /// Returns the size this View would like to be if enough space is available.
  /// Size is in parent coordinates, or DIP screen coordinates if there is no
  /// parent.
  ///
  cef_size_t(CEF_CALLBACK* get_preferred_size)(struct _cef_view_t* self);

  ///
  /// Size this View to its preferred size. Size is in parent coordinates, or
  /// DIP screen coordinates if there is no parent.
  ///
  void(CEF_CALLBACK* size_to_preferred_size)(struct _cef_view_t* self);

  ///
  /// Returns the minimum size for this View. Size is in parent coordinates, or
  /// DIP screen coordinates if there is no parent.
  ///
  cef_size_t(CEF_CALLBACK* get_minimum_size)(struct _cef_view_t* self);

  ///
  /// Returns the maximum size for this View. Size is in parent coordinates, or
  /// DIP screen coordinates if there is no parent.
  ///
  cef_size_t(CEF_CALLBACK* get_maximum_size)(struct _cef_view_t* self);

  ///
  /// Returns the height necessary to display this View with the provided width.
  ///
  int(CEF_CALLBACK* get_height_for_width)(struct _cef_view_t* self, int width);

  ///
  /// Indicate that this View and all parent Views require a re-layout. This
  /// ensures the next call to layout() will propagate to this View even if the
  /// bounds of parent Views do not change.
  ///
  void(CEF_CALLBACK* invalidate_layout)(struct _cef_view_t* self);

  ///
  /// Sets whether this View is visible. Windows are hidden by default and other
  /// views are visible by default. This View and any parent views must be set
  /// as visible for this View to be drawn in a Window. If this View is set as
  /// hidden then it and any child views will not be drawn and, if any of those
  /// views currently have focus, then focus will also be cleared. Painting is
  /// scheduled as needed. If this View is a Window then calling this function
  /// is equivalent to calling the Window show() and hide() functions.
  ///
  void(CEF_CALLBACK* set_visible)(struct _cef_view_t* self, int visible);

  ///
  /// Returns whether this View is visible. A view may be visible but still not
  /// drawn in a Window if any parent views are hidden. If this View is a Window
  /// then a return value of true (1) indicates that this Window is currently
  /// visible to the user on-screen. If this View is not a Window then call
  /// is_drawn() to determine whether this View and all parent views are visible
  /// and will be drawn.
  ///
  int(CEF_CALLBACK* is_visible)(struct _cef_view_t* self);

  ///
  /// Returns whether this View is visible and drawn in a Window. A view is
  /// drawn if it and all parent views are visible. If this View is a Window
  /// then calling this function is equivalent to calling is_visible().
  /// Otherwise, to determine if the containing Window is visible to the user
  /// on-screen call is_visible() on the Window.
  ///
  int(CEF_CALLBACK* is_drawn)(struct _cef_view_t* self);

  ///
  /// Set whether this View is enabled. A disabled View does not receive
  /// keyboard or mouse inputs. If |enabled| differs from the current value the
  /// View will be repainted. Also, clears focus if the focused View is
  /// disabled.
  ///
  void(CEF_CALLBACK* set_enabled)(struct _cef_view_t* self, int enabled);

  ///
  /// Returns whether this View is enabled.
  ///
  int(CEF_CALLBACK* is_enabled)(struct _cef_view_t* self);

  ///
  /// Sets whether this View is capable of taking focus. It will clear focus if
  /// the focused View is set to be non-focusable. This is false (0) by default
  /// so that a View used as a container does not get the focus.
  ///
  void(CEF_CALLBACK* set_focusable)(struct _cef_view_t* self, int focusable);

  ///
  /// Returns true (1) if this View is focusable, enabled and drawn.
  ///
  int(CEF_CALLBACK* is_focusable)(struct _cef_view_t* self);

  ///
  /// Return whether this View is focusable when the user requires full keyboard
  /// access, even though it may not be normally focusable.
  ///
  int(CEF_CALLBACK* is_accessibility_focusable)(struct _cef_view_t* self);

  ///
  /// Request keyboard focus. If this View is focusable it will become the
  /// focused View.
  ///
  void(CEF_CALLBACK* request_focus)(struct _cef_view_t* self);

  ///
  /// Sets the background color for this View.
  ///
  void(CEF_CALLBACK* set_background_color)(struct _cef_view_t* self,
                                           cef_color_t color);

  ///
  /// Returns the background color for this View.
  ///
  cef_color_t(CEF_CALLBACK* get_background_color)(struct _cef_view_t* self);

  ///
  /// Convert |point| from this View's coordinate system to DIP screen
  /// coordinates. This View must belong to a Window when calling this function.
  /// Returns true (1) if the conversion is successful or false (0) otherwise.
  /// Use cef_display_t::convert_point_to_pixels() after calling this function
  /// if further conversion to display-specific pixel coordinates is desired.
  ///
  int(CEF_CALLBACK* convert_point_to_screen)(struct _cef_view_t* self,
                                             cef_point_t* point);

  ///
  /// Convert |point| to this View's coordinate system from DIP screen
  /// coordinates. This View must belong to a Window when calling this function.
  /// Returns true (1) if the conversion is successful or false (0) otherwise.
  /// Use cef_display_t::convert_point_from_pixels() before calling this
  /// function if conversion from display-specific pixel coordinates is
  /// necessary.
  ///
  int(CEF_CALLBACK* convert_point_from_screen)(struct _cef_view_t* self,
                                               cef_point_t* point);

  ///
  /// Convert |point| from this View's coordinate system to that of the Window.
  /// This View must belong to a Window when calling this function. Returns true
  /// (1) if the conversion is successful or false (0) otherwise.
  ///
  int(CEF_CALLBACK* convert_point_to_window)(struct _cef_view_t* self,
                                             cef_point_t* point);

  ///
  /// Convert |point| to this View's coordinate system from that of the Window.
  /// This View must belong to a Window when calling this function. Returns true
  /// (1) if the conversion is successful or false (0) otherwise.
  ///
  int(CEF_CALLBACK* convert_point_from_window)(struct _cef_view_t* self,
                                               cef_point_t* point);

  ///
  /// Convert |point| from this View's coordinate system to that of |view|.
  /// |view| needs to be in the same Window but not necessarily the same view
  /// hierarchy. Returns true (1) if the conversion is successful or false (0)
  /// otherwise.
  ///
  int(CEF_CALLBACK* convert_point_to_view)(struct _cef_view_t* self,
                                           struct _cef_view_t* view,
                                           cef_point_t* point);

  ///
  /// Convert |point| to this View's coordinate system from that |view|. |view|
  /// needs to be in the same Window but not necessarily the same view
  /// hierarchy. Returns true (1) if the conversion is successful or false (0)
  /// otherwise.
  ///
  int(CEF_CALLBACK* convert_point_from_view)(struct _cef_view_t* self,
                                             struct _cef_view_t* view,
                                             cef_point_t* point);
} cef_view_t;


///
/// A View hosting a cef_browser_t instance. Methods must be called on the
/// browser process UI thread unless otherwise indicated.
///
typedef struct _cef_browser_view_t {
  ///
  /// Base structure.
  ///
  cef_view_t base;

  ///
  /// Returns the cef_browser_t hosted by this BrowserView. Will return NULL if
  /// the browser has not yet been created or has already been destroyed.
  ///
  struct _cef_browser_t*(CEF_CALLBACK* get_browser)(
      struct _cef_browser_view_t* self);

  ///
  /// Returns the Chrome toolbar associated with this BrowserView. Only
  /// supported when using the Chrome runtime. The cef_browser_view_delegate_t::
  /// get_chrome_toolbar_type() function must return a value other than
  /// CEF_CTT_NONE and the toolbar will not be available until after this
  /// BrowserView is added to a cef_window_t and
  /// cef_view_delegate_t::on_window_changed() has been called.
  ///
  struct _cef_view_t*(CEF_CALLBACK* get_chrome_toolbar)(
      struct _cef_browser_view_t* self);

  ///
  /// Sets whether accelerators registered with cef_window_t::SetAccelerator are
  /// triggered before or after the event is sent to the cef_browser_t. If
  /// |prefer_accelerators| is true (1) then the matching accelerator will be
  /// triggered immediately and the event will not be sent to the cef_browser_t.
  /// If |prefer_accelerators| is false (0) then the matching accelerator will
  /// only be triggered if the event is not handled by web content or by
  /// cef_keyboard_handler_t. The default value is false (0).
  ///
  void(CEF_CALLBACK* set_prefer_accelerators)(struct _cef_browser_view_t* self,
                                              int prefer_accelerators);
} cef_browser_view_t;

///
/// Settings used when initializing a CefBoxLayout.
///
typedef struct _cef_box_layout_settings_t {
  ///
  /// If true (1) the layout will be horizontal, otherwise the layout will be
  /// vertical.
  ///
  int horizontal;

  ///
  /// Adds additional horizontal space between the child view area and the host
  /// view border.
  ///
  int inside_border_horizontal_spacing;

  ///
  /// Adds additional vertical space between the child view area and the host
  /// view border.
  ///
  int inside_border_vertical_spacing;

  ///
  /// Adds additional space around the child view area.
  ///
  cef_insets_t inside_border_insets;

  ///
  /// Adds additional space between child views.
  ///
  int between_child_spacing;

  ///
  /// Specifies where along the main axis the child views should be laid out.
  ///
  cef_main_axis_alignment_t main_axis_alignment;

  ///
  /// Specifies where along the cross axis the child views should be laid out.
  ///
  cef_cross_axis_alignment_t cross_axis_alignment;

  ///
  /// Minimum cross axis size.
  ///
  int minimum_cross_axis_size;

  ///
  /// Default flex for views when none is specified via CefBoxLayout methods.
  /// Using the preferred size as the basis, free space along the main axis is
  /// distributed to views in the ratio of their flex weights. Similarly, if the
  /// views will overflow the parent, space is subtracted in these ratios. A
  /// flex of 0 means this view is not resized. Flex values must not be
  /// negative.
  ///
  int default_flex;
} cef_box_layout_settings_t;

///
/// A Panel is a container in the views hierarchy that can contain other Views
/// as children. Methods must be called on the browser process UI thread unless
/// otherwise indicated.
///
typedef struct _cef_panel_t {
  ///
  /// Base structure.
  ///
  cef_view_t base;

  ///
  /// Returns this Panel as a Window or NULL if this is not a Window.
  ///
  struct _cef_window_t*(CEF_CALLBACK* as_window)(struct _cef_panel_t* self);

  ///
  /// Set this Panel's Layout to FillLayout and return the FillLayout object.
  ///
  struct _cef_fill_layout_t*(CEF_CALLBACK* set_to_fill_layout)(
      struct _cef_panel_t* self);

  ///
  /// Set this Panel's Layout to BoxLayout and return the BoxLayout object.
  ///
  struct _cef_box_layout_t*(CEF_CALLBACK* set_to_box_layout)(
      struct _cef_panel_t* self,
      const cef_box_layout_settings_t* settings);

  ///
  /// Get the Layout.
  ///
  struct _cef_layout_t*(CEF_CALLBACK* get_layout)(struct _cef_panel_t* self);

  ///
  /// Lay out the child Views (set their bounds based on sizing heuristics
  /// specific to the current Layout).
  ///
  void(CEF_CALLBACK* layout)(struct _cef_panel_t* self);

  ///
  /// Add a child View.
  ///
  void(CEF_CALLBACK* add_child_view)(struct _cef_panel_t* self,
                                     struct _cef_view_t* view);

  ///
  /// Add a child View at the specified |index|. If |index| matches the result
  /// of GetChildCount() then the View will be added at the end.
  ///
  void(CEF_CALLBACK* add_child_view_at)(struct _cef_panel_t* self,
                                        struct _cef_view_t* view,
                                        int index);

  ///
  /// Move the child View to the specified |index|. A negative value for |index|
  /// will move the View to the end.
  ///
  void(CEF_CALLBACK* reorder_child_view)(struct _cef_panel_t* self,
                                         struct _cef_view_t* view,
                                         int index);

  ///
  /// Remove a child View. The View can then be added to another Panel.
  ///
  void(CEF_CALLBACK* remove_child_view)(struct _cef_panel_t* self,
                                        struct _cef_view_t* view);

  ///
  /// Remove all child Views. The removed Views will be deleted if the client
  /// holds no references to them.
  ///
  void(CEF_CALLBACK* remove_all_child_views)(struct _cef_panel_t* self);

  ///
  /// Returns the number of child Views.
  ///
  size_t(CEF_CALLBACK* get_child_view_count)(struct _cef_panel_t* self);

  ///
  /// Returns the child View at the specified |index|.
  ///
  struct _cef_view_t*(
      CEF_CALLBACK* get_child_view_at)(struct _cef_panel_t* self, int index);
} cef_panel_t;

///
/// A Window is a top-level Window/widget in the Views hierarchy. By default it
/// will have a non-client area with title bar, icon and buttons that supports
/// moving and resizing. All size and position values are in density independent
/// pixels (DIP) unless otherwise indicated. Methods must be called on the
/// browser process UI thread unless otherwise indicated.
///
typedef struct _cef_window_t {
  ///
  /// Base structure.
  ///
  cef_panel_t base;

  ///
  /// Show the Window.
  ///
  void(CEF_CALLBACK* show)(struct _cef_window_t* self);

  ///
  /// Hide the Window.
  ///
  void(CEF_CALLBACK* hide)(struct _cef_window_t* self);

  ///
  /// Sizes the Window to |size| and centers it in the current display.
  ///
  void(CEF_CALLBACK* center_window)(struct _cef_window_t* self,
                                    const cef_size_t* size);

  ///
  /// Close the Window.
  ///
  void(CEF_CALLBACK* close)(struct _cef_window_t* self);

  ///
  /// Returns true (1) if the Window has been closed.
  ///
  int(CEF_CALLBACK* is_closed)(struct _cef_window_t* self);

  ///
  /// Activate the Window, assuming it already exists and is visible.
  ///
  void(CEF_CALLBACK* activate)(struct _cef_window_t* self);

  ///
  /// Deactivate the Window, making the next Window in the Z order the active
  /// Window.
  ///
  void(CEF_CALLBACK* deactivate)(struct _cef_window_t* self);

  ///
  /// Returns whether the Window is the currently active Window.
  ///
  int(CEF_CALLBACK* is_active)(struct _cef_window_t* self);

  ///
  /// Bring this Window to the top of other Windows in the Windowing system.
  ///
  void(CEF_CALLBACK* bring_to_top)(struct _cef_window_t* self);

  ///
  /// Set the Window to be on top of other Windows in the Windowing system.
  ///
  void(CEF_CALLBACK* set_always_on_top)(struct _cef_window_t* self, int on_top);

  ///
  /// Returns whether the Window has been set to be on top of other Windows in
  /// the Windowing system.
  ///
  int(CEF_CALLBACK* is_always_on_top)(struct _cef_window_t* self);

  ///
  /// Maximize the Window.
  ///
  void(CEF_CALLBACK* maximize)(struct _cef_window_t* self);

  ///
  /// Minimize the Window.
  ///
  void(CEF_CALLBACK* minimize)(struct _cef_window_t* self);

  ///
  /// Restore the Window.
  ///
  void(CEF_CALLBACK* restore)(struct _cef_window_t* self);

  ///
  /// Set fullscreen Window state.
  ///
  void(CEF_CALLBACK* set_fullscreen)(struct _cef_window_t* self,
                                     int fullscreen);

  ///
  /// Returns true (1) if the Window is maximized.
  ///
  int(CEF_CALLBACK* is_maximized)(struct _cef_window_t* self);

  ///
  /// Returns true (1) if the Window is minimized.
  ///
  int(CEF_CALLBACK* is_minimized)(struct _cef_window_t* self);

  ///
  /// Returns true (1) if the Window is fullscreen.
  ///
  int(CEF_CALLBACK* is_fullscreen)(struct _cef_window_t* self);

  ///
  /// Set the Window title.
  ///
  void(CEF_CALLBACK* set_title)(struct _cef_window_t* self,
                                const cef_string_t* title);

  ///
  /// Get the Window title.
  ///
  // The resulting string must be freed by calling cef_string_userfree_free().
  cef_string_userfree_t(CEF_CALLBACK* get_title)(struct _cef_window_t* self);

  ///
  /// Set the Window icon. This should be a 16x16 icon suitable for use in the
  /// Windows's title bar.
  ///
  void(CEF_CALLBACK* set_window_icon)(struct _cef_window_t* self,
                                      struct _cef_image_t* image);

  ///
  /// Get the Window icon.
  ///
  struct _cef_image_t*(CEF_CALLBACK* get_window_icon)(
      struct _cef_window_t* self);

  ///
  /// Set the Window App icon. This should be a larger icon for use in the host
  /// environment app switching UI. On Windows, this is the ICON_BIG used in
  /// Alt-Tab list and Windows taskbar. The Window icon will be used by default
  /// if no Window App icon is specified.
  ///
  void(CEF_CALLBACK* set_window_app_icon)(struct _cef_window_t* self,
                                          struct _cef_image_t* image);

  ///
  /// Get the Window App icon.
  ///
  struct _cef_image_t*(CEF_CALLBACK* get_window_app_icon)(
      struct _cef_window_t* self);

  ///
  /// Add a View that will be overlayed on the Window contents with absolute
  /// positioning and high z-order. Positioning is controlled by |docking_mode|
  /// as described below. The returned cef_overlay_controller_t object is used
  /// to control the overlay. Overlays are hidden by default.
  ///
  /// With CEF_DOCKING_MODE_CUSTOM:
  ///   1. The overlay is initially hidden, sized to |view|'s preferred size,
  ///      and positioned in the top-left corner.
  ///   2. Optionally change the overlay position and/or size by calling
  ///      CefOverlayController methods.
  ///   3. Call CefOverlayController::SetVisible(true) to show the overlay.
  ///   4. The overlay will be automatically re-sized if |view|'s layout
  ///      changes. Optionally change the overlay position and/or size when
  ///      OnLayoutChanged is called on the Window's delegate to indicate a
  ///      change in Window bounds.
  ///
  /// With other docking modes:
  ///   1. The overlay is initially hidden, sized to |view|'s preferred size,
  ///      and positioned based on |docking_mode|.
  ///   2. Call CefOverlayController::SetVisible(true) to show the overlay.
  ///   3. The overlay will be automatically re-sized if |view|'s layout changes
  ///      and re-positioned as appropriate when the Window resizes.
  ///
  /// Overlays created by this function will receive a higher z-order then any
  /// child Views added previously. It is therefore recommended to call this
  /// function last after all other child Views have been added so that the
  /// overlay displays as the top-most child of the Window.
  ///
  struct _cef_overlay_controller_t*(CEF_CALLBACK* add_overlay_view)(
      struct _cef_window_t* self,
      struct _cef_view_t* view,
      cef_docking_mode_t docking_mode);

  ///
  /// Show a menu with contents |menu_model|. |screen_point| specifies the menu
  /// position in screen coordinates. |anchor_position| specifies how the menu
  /// will be anchored relative to |screen_point|.
  ///
  void(CEF_CALLBACK* show_menu)(struct _cef_window_t* self,
                                struct _cef_menu_model_t* menu_model,
                                const cef_point_t* screen_point,
                                cef_menu_anchor_position_t anchor_position);

  ///
  /// Cancel the menu that is currently showing, if any.
  ///
  void(CEF_CALLBACK* cancel_menu)(struct _cef_window_t* self);

  ///
  /// Returns the Display that most closely intersects the bounds of this
  /// Window. May return NULL if this Window is not currently displayed.
  ///
  struct _cef_display_t*(CEF_CALLBACK* get_display)(struct _cef_window_t* self);

  ///
  /// Returns the bounds (size and position) of this Window's client area.
  /// Position is in screen coordinates.
  ///
  cef_rect_t(CEF_CALLBACK* get_client_area_bounds_in_screen)(
      struct _cef_window_t* self);

  ///
  /// Set the regions where mouse events will be intercepted by this Window to
  /// support drag operations. Call this function with an NULL vector to clear
  /// the draggable regions. The draggable region bounds should be in window
  /// coordinates.
  ///
  void(CEF_CALLBACK* set_draggable_regions)(
      struct _cef_window_t* self,
      size_t regionsCount,
      cef_draggable_region_t const* regions);

  ///
  /// Retrieve the platform window handle for this Window.
  ///
  cef_window_handle_t(CEF_CALLBACK* get_window_handle)(
      struct _cef_window_t* self);

  ///
  /// Simulate a key press. |key_code| is the VKEY_* value from Chromium's
  /// ui/events/keycodes/keyboard_codes.h header (VK_* values on Windows).
  /// |event_flags| is some combination of EVENTFLAG_SHIFT_DOWN,
  /// EVENTFLAG_CONTROL_DOWN and/or EVENTFLAG_ALT_DOWN. This function is exposed
  /// primarily for testing purposes.
  ///
  void(CEF_CALLBACK* send_key_press)(struct _cef_window_t* self,
                                     int key_code,
                                     uint32 event_flags);

  ///
  /// Simulate a mouse move. The mouse cursor will be moved to the specified
  /// (screen_x, screen_y) position. This function is exposed primarily for
  /// testing purposes.
  ///
  void(CEF_CALLBACK* send_mouse_move)(struct _cef_window_t* self,
                                      int screen_x,
                                      int screen_y);

  ///
  /// Simulate mouse down and/or mouse up events. |button| is the mouse button
  /// type. If |mouse_down| is true (1) a mouse down event will be sent. If
  /// |mouse_up| is true (1) a mouse up event will be sent. If both are true (1)
  /// a mouse down event will be sent followed by a mouse up event (equivalent
  /// to clicking the mouse button). The events will be sent using the current
  /// cursor position so make sure to call send_mouse_move() first to position
  /// the mouse. This function is exposed primarily for testing purposes.
  ///
  void(CEF_CALLBACK* send_mouse_events)(struct _cef_window_t* self,
                                        cef_mouse_button_type_t button,
                                        int mouse_down,
                                        int mouse_up);

  ///
  /// Set the keyboard accelerator for the specified |command_id|. |key_code|
  /// can be any virtual key or character value.
  /// cef_window_delegate_t::OnAccelerator will be called if the keyboard
  /// combination is triggered while this window has focus.
  ///
  void(CEF_CALLBACK* set_accelerator)(struct _cef_window_t* self,
                                      int command_id,
                                      int key_code,
                                      int shift_pressed,
                                      int ctrl_pressed,
                                      int alt_pressed);

  ///
  /// Remove the keyboard accelerator for the specified |command_id|.
  ///
  void(CEF_CALLBACK* remove_accelerator)(struct _cef_window_t* self,
                                         int command_id);

  ///
  /// Remove all keyboard accelerators.
  ///
  void(CEF_CALLBACK* remove_all_accelerators)(struct _cef_window_t* self);
} cef_window_t;


///
/// Implement this structure to handle window events. The functions of this
/// structure will be called on the browser process UI thread unless otherwise
/// indicated.
///
typedef struct _cef_window_delegate_t {
  ///
  /// Base structure.
  ///
  cef_panel_delegate_t base;

  ///
  /// Called when |window| is created.
  ///
  void(CEF_CALLBACK* on_window_created)(struct _cef_window_delegate_t* self,
                                        struct _cef_window_t* window);

  ///
  /// Called when |window| is closing.
  ///
  void(CEF_CALLBACK* on_window_closing)(struct _cef_window_delegate_t* self,
                                        struct _cef_window_t* window);

  ///
  /// Called when |window| is destroyed. Release all references to |window| and
  /// do not attempt to execute any functions on |window| after this callback
  /// returns.
  ///
  void(CEF_CALLBACK* on_window_destroyed)(struct _cef_window_delegate_t* self,
                                          struct _cef_window_t* window);

  ///
  /// Called when |window| is activated or deactivated.
  ///
  void(CEF_CALLBACK* on_window_activation_changed)(
      struct _cef_window_delegate_t* self,
      struct _cef_window_t* window,
      int active);

  ///
  /// Called when |window| bounds have changed. |new_bounds| will be in DIP
  /// screen coordinates.
  ///
  void(CEF_CALLBACK* on_window_bounds_changed)(
      struct _cef_window_delegate_t* self,
      struct _cef_window_t* window,
      const cef_rect_t* new_bounds);

  ///
  /// Return the parent for |window| or NULL if the |window| does not have a
  /// parent. Windows with parents will not get a taskbar button. Set |is_menu|
  /// to true (1) if |window| will be displayed as a menu, in which case it will
  /// not be clipped to the parent window bounds. Set |can_activate_menu| to
  /// false (0) if |is_menu| is true (1) and |window| should not be activated
  /// (given keyboard focus) when displayed.
  ///
  struct _cef_window_t*(CEF_CALLBACK* get_parent_window)(
      struct _cef_window_delegate_t* self,
      struct _cef_window_t* window,
      int* is_menu,
      int* can_activate_menu);

  ///
  /// Return the initial bounds for |window| in density independent pixel (DIP)
  /// coordinates. If this function returns an NULL CefRect then
  /// get_preferred_size() will be called to retrieve the size, and the window
  /// will be placed on the screen with origin (0,0). This function can be used
  /// in combination with cef_view_t::get_bounds_in_screen() to restore the
  /// previous window bounds.
  ///
  cef_rect_t(CEF_CALLBACK* get_initial_bounds)(
      struct _cef_window_delegate_t* self,
      struct _cef_window_t* window);

  ///
  /// Return the initial show state for |window|.
  ///
  cef_show_state_t(CEF_CALLBACK* get_initial_show_state)(
      struct _cef_window_delegate_t* self,
      struct _cef_window_t* window);

  ///
  /// Return true (1) if |window| should be created without a frame or title
  /// bar. The window will be resizable if can_resize() returns true (1). Use
  /// cef_window_t::set_draggable_regions() to specify draggable regions.
  ///
  int(CEF_CALLBACK* is_frameless)(struct _cef_window_delegate_t* self,
                                  struct _cef_window_t* window);

  ///
  /// Return true (1) if |window| should be created with standard window buttons
  /// like close, minimize and zoom. This function is only supported on macOS.
  ///
  int(CEF_CALLBACK* with_standard_window_buttons)(
      struct _cef_window_delegate_t* self,
      struct _cef_window_t* window);

  ///
  /// Return whether the titlebar height should be overridden, and sets the
  /// height of the titlebar in |titlebar_height|. On macOS, it can also be used
  /// to adjust the vertical position of the traffic light buttons in frameless
  /// windows. The buttons will be positioned halfway down the titlebar at a
  /// height of |titlebar_height| / 2.
  ///
  int(CEF_CALLBACK* get_titlebar_height)(struct _cef_window_delegate_t* self,
                                         struct _cef_window_t* window,
                                         float* titlebar_height);

  ///
  /// Return true (1) if |window| can be resized.
  ///
  int(CEF_CALLBACK* can_resize)(struct _cef_window_delegate_t* self,
                                struct _cef_window_t* window);

  ///
  /// Return true (1) if |window| can be maximized.
  ///
  int(CEF_CALLBACK* can_maximize)(struct _cef_window_delegate_t* self,
                                  struct _cef_window_t* window);

  ///
  /// Return true (1) if |window| can be minimized.
  ///
  int(CEF_CALLBACK* can_minimize)(struct _cef_window_delegate_t* self,
                                  struct _cef_window_t* window);

  ///
  /// Return true (1) if |window| can be closed. This will be called for user-
  /// initiated window close actions and when cef_window_t::close() is called.
  ///
  int(CEF_CALLBACK* can_close)(struct _cef_window_delegate_t* self,
                               struct _cef_window_t* window);

  ///
  /// Called when a keyboard accelerator registered with
  /// cef_window_t::SetAccelerator is triggered. Return true (1) if the
  /// accelerator was handled or false (0) otherwise.
  ///
  int(CEF_CALLBACK* on_accelerator)(struct _cef_window_delegate_t* self,
                                    struct _cef_window_t* window,
                                    int command_id);

  ///
  /// Called after all other controls in the window have had a chance to handle
  /// the event. |event| contains information about the keyboard event. Return
  /// true (1) if the keyboard event was handled or false (0) otherwise.
  ///
  int(CEF_CALLBACK* on_key_event)(struct _cef_window_delegate_t* self,
                                  struct _cef_window_t* window,
                                  const cef_key_event_t* event);

  ///
  /// Called when the |window| is transitioning to or from fullscreen mode. The
  /// transition occurs in two stages, with |is_competed| set to false (0) when
  /// the transition starts and true (1) when the transition completes. This
  /// function is only supported on macOS.
  ///
  void(CEF_CALLBACK* on_window_fullscreen_transition)(
      struct _cef_window_delegate_t* self,
      struct _cef_window_t* window,
      int is_completed);
} cef_window_delegate_t;

#pragma endregion

_cef_window_t* mainWindow;

int WINAPI is_frameless_hook(struct _cef_window_delegate_t* self, struct _cef_window_t* window) {
    Wh_Log(L"is_frameless_hook");
    return 0;
}

LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    if (uMsg == WM_NCHITTEST || uMsg == WM_NCLBUTTONDOWN || uMsg == WM_NCPAINT || uMsg == WM_NCCREATE || uMsg == WM_NCCALCSIZE) {
        // Unhook Spotify's custom window control hover handling
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

typedef _cef_window_t* (*cef_window_create_top_level_t)(cef_window_delegate_t* delegate);
cef_window_create_top_level_t cef_window_create_top_level_original;
_cef_window_t* cef_window_create_top_level_hook(cef_window_delegate_t* delegate) {
    Wh_Log(L"cef_window_create_top_level_hook");

    Wh_Log(L"is_frameless offset: %#x", (char *)&(delegate->is_frameless) - (char *)delegate);
    delegate->is_frameless = is_frameless_hook;
    mainWindow = cef_window_create_top_level_original(delegate);
    Wh_Log(L"get_window_handle offset: %#x", (char *)&(mainWindow->get_window_handle) - (char *)mainWindow);
    SetWindowSubclass(mainWindow->get_window_handle(mainWindow), SubclassProc, 0, 0);
    return NULL;
}

int cnt = 0;

typedef void (*set_background_color_t)(struct _cef_view_t* self, cef_color_t color);
set_background_color_t CEF_CALLBACK set_background_color_original;
void CEF_CALLBACK set_background_color_hook(struct _cef_view_t* self, cef_color_t color) {
    Wh_Log(L"set_background_color_hook: %#x", color);
    // 0x87000000: normal, 0x3fffffff: hover, 0x33ffffff: active, 0xffc42b1c: close button hover, 0xff941320: close button active
    set_background_color_original(self, color);
    return;
}

typedef void (*add_child_view_t)(struct _cef_panel_t* self, struct _cef_view_t* view);
add_child_view_t CEF_CALLBACK add_child_view_original;
void CEF_CALLBACK add_child_view_hook(struct _cef_panel_t* self, struct _cef_view_t* view) {
    Wh_Log(L"add_child_view_hook: %d", cnt++);
    // 0: Minimize, 1: Maximize, 2: Close, 3: Menu (removing this also prevents alt key from working)
    //if (cnt == 4) {
        add_child_view_original(self, view);set_background_color_original = view->set_background_color;
        Wh_Log(L"set_background_color offset: %#x", (char *)&(view->set_background_color) - (char *)view);
        view->set_background_color = set_background_color_hook;
    //}
    return;
}

typedef _cef_panel_t* (*cef_panel_create_t)(_cef_panel_delegate_t* delegate);
cef_panel_create_t cef_panel_create_original;
_cef_panel_t* cef_panel_create_hook(_cef_panel_delegate_t* delegate) {
    Wh_Log(L"cef_panel_create_hook");
    _cef_panel_t* panel = cef_panel_create_original(delegate);
    add_child_view_original = panel->add_child_view;
    Wh_Log(L"add_child_view offset: %#x", (char *)&(panel->add_child_view) - (char *)panel);
    panel->add_child_view = add_child_view_hook;
    return panel;
}

typedef int (*cef_version_info_t)(int entry);

// The mod is being initialized, load settings, hook functions, and do other
// initialization stuff if required.
BOOL Wh_ModInit() {
    Wh_Log(L"Init");

    HMODULE cefModule = LoadLibrary(L"libcef.dll");
    cef_window_create_top_level_t cef_window_create_top_level =
        (cef_window_create_top_level_t)GetProcAddress(cefModule,
                                                "cef_window_create_top_level");
    cef_panel_create_t cef_panel_create =
        (cef_panel_create_t)GetProcAddress(cefModule, "cef_panel_create");
    cef_version_info_t cef_version_info =
        (cef_version_info_t)GetProcAddress(cefModule, "cef_version_info");

    Wh_Log(L"CEF v%d.%d.%d.%d (Chromium v%d.%d.%d.%d) Loaded",
        cef_version_info(0),
        cef_version_info(1),
        cef_version_info(2),
        cef_version_info(3),
        cef_version_info(4),
        cef_version_info(5),
        cef_version_info(6),
        cef_version_info(7)
    );

    Wh_SetFunctionHook((void*)cef_window_create_top_level,
                       (void*)cef_window_create_top_level_hook,
                       (void**)&cef_window_create_top_level_original);
    Wh_SetFunctionHook((void*)cef_panel_create, (void*)cef_panel_create_hook,
                      (void**)&cef_panel_create_original);
    return TRUE;
}

// The mod is being unloaded, free all allocated resources.
void Wh_ModUninit() {
    Wh_Log(L"Uninit");
}

// The mod setting were changed, reload them.
void Wh_ModSettingsChanged() {
    Wh_Log(L"SettingsChanged");
}