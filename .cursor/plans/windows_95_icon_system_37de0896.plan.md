---
name: Windows 95 Icon System
overview: Complete the hybrid icon system by adding 32x32 icons, procedural icon support, and taskbar app button icons. Most core functionality is already implemented.
todos:
  - id: create_32x32_icons
    content: Add 32x32 versions of all 25 existing icons to BuiltinIcons.h
    status: pending
  - id: add_procedural_icon_support
    content: Implement Berry def icon() method support and ui.draw_icon() binding
    status: pending
  - id: taskbar_app_button_icons
    content: Add icon display to taskbar app buttons for open applications
    status: pending
  - id: test_icon_system
    content: Test all icon types (builtin, procedural) in all three locations
    status: completed
isProject: false
---

# Windows 95/3.1 Icon System - Completion Plan

## Implementation Status

### ✅ Already Completed (What we found)

The following components have already been implemented:

1. **IconRenderer.h** - 4-bit indexed color rendering with Win95 palette (16 colors) ✅
2. **BuiltinIcons.h** - 25 Win95-style icons in 16x16 format stored in PROGMEM ✅
3. **BerryScriptInfo** - Extended with iconType and iconValue fields ✅
4. **parseAppMetadata()** - Parses `# icon:` directive (builtin/, procedural, file paths) ✅
5. **BerryApp** - Icon support with drawIcon() method and resolution logic ✅
6. **Window title bars** - Icon drawer support via setIconDrawer() ✅
7. **WindowManager** - Automatically sets up icon drawer when opening apps ✅
8. **ui.icon()** - Berry binding to display built-in icons in UI ✅
9. **Start menu icons** - taskbar.be displays icons for categories and apps ✅
10. **berry apps JSON** - Includes iconType and iconValue in response ✅
11. **Icon directives** - 9/10 Berry apps have `# icon:` directives ✅

Existing icons (16x16 only):

- Files: folder, folder_open, text_file, executable, generic_file
- System: file_manager, log_viewer, info, features, computer, help
- Settings: display, wifi, settings, sensors, rgb_led, paint, wrench, chart
- UI: restart, shutdown, error, warning, sound, power

### ❌ Remaining Work (3 tasks)

1. **32x32 icon versions** - Only 16x16 exists, need 32x32 for start menu (~12.5KB flash)
2. **Procedural icon support** - Berry `def icon()` method not implemented
3. **Taskbar app button icons** - Open apps in taskbar don't show icons yet

## Detailed Implementation Plan

### Task 1: Create 32x32 Icon Versions

**File: `src/FeatureRegistry/Features/UI/BuiltinIcons.h**`

Add 32x32 versions of all 25 existing icons:

- Each 32x32 icon = 512 bytes (32×32 pixels ÷ 2 pixels/byte in 4-bit)
- Total additional flash: ~25 × 512 = 12.5KB PROGMEM

**Changes needed:**

1. Add PROGMEM arrays (e.g., `icon32_folder[]`, `icon32_text_file[]`, etc.)
2. Create IconData descriptors for 32x32 versions
3. Update `getBuiltinIcon()` signature to accept size:

```cpp
 const IconData *getBuiltinIcon(const char *name, int preferredSize = 16)
```

1. Update `drawBuiltinIcon()` to request appropriate size:

```cpp
 inline void drawBuiltinIcon(LGFX_Sprite &canvas, const char *name, int x, int y, int targetSize = 16)
 {
     int sourceSize = (targetSize >= 24) ? 32 : 16;
     const IconData *icon = getBuiltinIcon(name, sourceSize);
     if (!icon) icon = getBuiltinIcon("generic_file", sourceSize);
     drawIndexedIconScaled(canvas, *icon, x, y, targetSize);
 }
```

### Task 2: Procedural Icon Support

**Part A: Berry Method Detection & Calling**

**File: `src/FeatureRegistry/Features/Berry/BerryApp.h**`

Modify `drawIcon()` method (~line 75):

```cpp
void drawIcon(LGFX_Sprite &canvas, int x, int y, int size) override
{
    // Priority 1: Procedural icon if defined
    if (_iconType == "procedural" && checkBerryIconMethod())
    {
        callBerryIconMethod(canvas, x, y, size);
        return;
    }

    // Priority 2: Built-in icon
    if (_iconType == "builtin" && !_iconValue.isEmpty())
    {
        UI::drawBuiltinIcon(canvas, _iconValue.c_str(), x, y, size);
        return;
    }

    // Priority 3: Default based on category
    if (_iconType.isEmpty() && !_startMenu.isEmpty())
    {
        const char *defaultIcon = UI::getDefaultIconForCategory(_startMenu);
        UI::drawBuiltinIcon(canvas, defaultIcon, x, y, size);
        return;
    }

    // Fallback
    UI::drawBuiltinIcon(canvas, "generic_file", x, y, size);
}

private:
bool checkBerryIconMethod();  // Check if Berry class has icon() method
void callBerryIconMethod(LGFX_Sprite &canvas, int x, int y, int size);
```

Implement helper methods to call Berry's `icon()` method using Berry VM API.

**Part B: ui.draw_icon() Binding**

**File: `src/FeatureRegistry/Features/Berry/BerryUIBindings.cpp**`

Add new binding after existing ui_icon (~line 700):

```cpp
// ui.draw_icon(canvas_handle, icon_name, x, y, size)
// Draws icon directly to canvas sprite (for procedural icons)
static int ui_draw_icon(bvm *vm)
{
    auto *app = berryCurrentApp();
    if (!app || be_top(vm) < 5)
        be_return_nil(vm);

    int canvasH = be_toint(vm, 1);
    const char *iconName = be_tostring(vm, 2);
    int x = be_toint(vm, 3);
    int y = be_toint(vm, 4);
    int size = be_toint(vm, 5);

    // Get canvas from handle
    LGFX_Sprite *canvas = getCanvasFromHandle(app, canvasH);
    if (canvas)
    {
        UI::drawBuiltinIcon(*canvas, iconName, x, y, size);
    }

    be_return_nil(vm);
}
```

Register it (~line 1714):

```cpp
reg("draw_icon", ui_draw_icon);
```

**Berry usage example:**

```berry
# icon: procedural

class CustomApp
  def icon(canvas, x, y, size)
    # Draw folder as base
    ui.draw_icon(canvas, 'folder', x, y, size)
    # Add custom overlays here using canvas drawing methods
  end
end
```

### Task 3: Taskbar App Button Icons

**Part A: Expose Icon Data in wm list**

**File: May need to modify WindowManager or create action**

The `wm list` action currently returns open apps but likely doesn't include icon info.

Options:

1. Add icon metadata to wm list JSON
2. Have taskbar query each app's iconType/iconValue from the Berry apps list

**Part B: Update taskbar.be**

**File: `data/berry/apps/taskbar.be**`

Modify `rebuild()` method (starting at line 278):

```berry
def rebuild(apps)
  # remove old app buttons
  for btn : self.app_buttons
    ui.remove_child(self.content, btn)
  end
  self.app_buttons = []

  # filter to windowed apps only
  var windowed = []
  for app : apps
    if app.find('windowed') == nil || app['windowed']
      windowed.push(app)
    end
  end

  var app_x = 56
  var kb_x = self.w - 26
  var avail = kb_x - app_x - 2
  var count = windowed.size()
  if count == 0 return end

  var btn_w = avail / count
  if btn_w > 80 btn_w = 80 end

  var icon_sz = 12  # Small icon for taskbar buttons
  var icon_pad = 2

  for i : 0 .. count - 1
    var app = windowed[i]
    var bx = app_x + i * btn_w

    # Get icon name for this app (need to match with apps list)
    var icon_name = self.get_app_icon(app['name'])
    if icon_name != nil && icon_name != ''
      var icon_y = 2 + (self.h - 4 - icon_sz) / 2
      ui.icon(self.content, icon_name, bx + icon_pad, icon_y, icon_sz)
    end

    # Button with text (offset if icon exists)
    var text_offset = icon_name != nil && icon_name != '' ? icon_sz + icon_pad * 2 : icon_pad
    var btn_x = bx + text_offset
    var btn_w_adj = btn_w - text_offset - 2

    var btn = ui.button(self.content, app['name'], btn_x, 2, btn_w_adj, self.h - 4)
    if app['focused']
      ui.set_border_colors(btn, ui.BUTTON_SHADOW, ui.BUTTON_HIGHLIGHT)
    end
    var name = app['name']
    ui.on_click(btn, def () action('wm focus ' + name) end)
    self.app_buttons.push(btn)
  end
  ui.mark_dirty()
end

def get_app_icon(app_name)
  # Match app name with discovered apps to get icon
  for cat : self.categories
    for app : self.cat_apps[cat]
      if app['label'] == app_name
        return self.get_icon_name(app)
      end
    end
  end
  return 'generic_file'  # Fallback
end
```

## Testing Plan

1. **32x32 Icons Test:**

- Open start menu
- Verify all app icons display at 32x32 in submenu
- Check that icons are sharp, not pixelated from scaling

1. **Procedural Icons Test:**

- Create test Berry app with `# icon: procedural`
- Implement `def icon()` method using `ui.draw_icon()`
- Verify icon appears in:
  - Window title bar (16x16)
  - Start menu (32x32)
  - Taskbar button (12x12)

1. **Taskbar Button Icons Test:**

- Open multiple apps (File Manager, Paint, Settings)
- Verify each taskbar button shows correct app icon
- Check icon positioning and text offset
- Verify focused app highlighting still works

1. **Memory Verification:**

- Check flash usage after adding 32x32 icons
- Should be ~12.5KB additional
- Verify no RAM impact (all icons in PROGMEM)

## Summary

**Flash Memory Impact:**

- Existing: ~3KB (25 × 128 bytes for 16x16 icons)
- Adding: ~12.5KB (25 × 512 bytes for 32x32 icons)
- Total: ~15.5KB in PROGMEM (flash, not RAM)

**Code Complexity:**

- Low: Most infrastructure already exists
- Primary work is creating 32x32 icon pixel data
- Berry procedural support requires VM method calling
- Taskbar integration is mostly Berry script changes

**User Benefits:**

- Higher quality icons in start menu
- Consistent icon display across all UI locations
- Extensibility via procedural icons
- Better visual app identification in taskbar
