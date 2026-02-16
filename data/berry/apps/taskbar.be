# app: Taskbar
import json

class TaskbarApp
  var content
  var w
  var h
  var start_btn
  var kb_btn
  var app_buttons
  var prev_apps_json

  # start menu state
  var categories       # ordered list of category names
  var cat_apps         # map: category name -> list of {label, path}
  var menu_popup       # main menu popup handle
  var sub_popup        # submenu popup handle
  var menu_visible
  var active_cat       # currently shown submenu category index (-1 = none)

  def init()
    self.app_buttons = []
    self.prev_apps_json = ''
    self.categories = []
    self.cat_apps = {}
    self.menu_popup = nil
    self.sub_popup = nil
    self.menu_visible = false
    self.active_cat = -1
  end

  def setup(content, w, h)
    self.content = content
    self.w = w
    self.h = h

    # discover apps
    self.discover_apps()
    log('taskbar: discovered ' + str(self.categories.size()) + ' categories')

    # build start menu popups
    self.build_menu()
    log('taskbar: menu_popup=' + str(self.menu_popup) + ' sub_popup=' + str(self.sub_popup))

    # Start button (left side)
    self.start_btn = ui.button(content, 'Start', 2, 2, 50, h - 4)
    var me = self
    ui.on_click(self.start_btn, def () me.toggle_menu() end)

    # Keyboard toggle button (right side)
    self.kb_btn = ui.button(content, 'Kb', w - 24, 2, 22, h - 4)
    ui.on_click(self.kb_btn, def () action('wm keyboard') end)

    ui.timer(300, def () me.refresh() end)
  end

  def discover_apps()
    var result = action('berry apps')
    var data = json.load(result)
    if data == nil || data.find('apps') == nil return end

    var cat_order = []
    var cat_map = {}

    for app : data['apps']
      var category = app['category']
      var label = app['label']
      var path = app['path']
      if category == '' || label == '' continue end

      if cat_map.find(category) == nil
        cat_map[category] = []
        cat_order.push(category)
      end
      var entry = {}
      entry['label'] = label
      entry['path'] = path
      cat_map[category].push(entry)
    end

    self.categories = cat_order
    self.cat_apps = cat_map
  end

  def build_menu()
    var num_cats = self.categories.size()
    if num_cats == 0 return end

    var item_h = ui.MENU_ITEM_HEIGHT
    var sep_h = ui.MENU_SEPARATOR_HEIGHT
    var menu_w = ui.MENU_WIDTH
    var pad = 4

    # main menu height: categories + separator + Restart + padding
    var menu_h = num_cats * item_h + sep_h + item_h + pad
    var menu_x = 2
    var menu_y = ui.TASKBAR_Y - menu_h

    self.menu_popup = ui.popup(menu_x, menu_y, menu_w, menu_h)

    var me = self
    var btn_y = 2
    for i : 0 .. num_cats - 1
      var cat = self.categories[i]
      var btn = ui.button(self.menu_popup, cat, 2, btn_y, menu_w - 4, item_h)
      self.style_menu_item(btn)
      # filled arrow on right side
      var arrow = ui.label(self.menu_popup, '\x10', menu_w - 16, btn_y, 12, item_h)
      ui.set_text_color(arrow, ui.TEXT_COLOR, ui.MENU_BG)
      var idx = i
      ui.on_click(btn, def () me.show_submenu(idx) end)
      btn_y += item_h
    end

    # skip separator gap
    btn_y += sep_h

    # Restart button
    var restart_btn = ui.button(self.menu_popup, 'Restart', 2, btn_y, menu_w - 4, item_h)
    self.style_menu_item(restart_btn)
    ui.on_click(restart_btn, def () action('restart') end)

    # create submenu popup (reused for all categories)
    var max_items = 0
    for cat : self.categories
      var n = self.cat_apps[cat].size()
      if n > max_items max_items = n end
    end
    var sub_w = ui.SUB_MENU_WIDTH
    var sub_h = max_items * item_h + pad
    var sub_x = menu_x + menu_w - 1
    var sub_y = menu_y

    self.sub_popup = ui.popup(sub_x, sub_y, sub_w, sub_h)
  end

  def style_menu_item(btn)
    ui.set_bg_color(btn, ui.MENU_BG)
    ui.set_border_colors(btn, ui.MENU_BG, ui.MENU_BG)
    ui.set_text_color(btn, ui.TEXT_COLOR, ui.MENU_BG)
    ui.set_align(btn, ui.LEFT)
  end

  def toggle_menu()
    log('toggle_menu: popup=' + str(self.menu_popup) + ' visible=' + str(self.menu_visible))
    if self.menu_visible
      self.hide_menu()
    else
      self.show_menu()
    end
  end

  def show_menu()
    if self.menu_popup == nil return end
    ui.show_popup(self.menu_popup)
    self.menu_visible = true
    self.active_cat = -1
    ui.mark_dirty()
  end

  def hide_menu()
    if self.menu_popup != nil
      ui.hide_popup(self.menu_popup)
    end
    if self.sub_popup != nil
      ui.hide_popup(self.sub_popup)
    end
    self.menu_visible = false
    self.active_cat = -1
    ui.mark_dirty()
  end

  def show_submenu(cat_idx)
    if cat_idx == self.active_cat return end
    self.active_cat = cat_idx

    if self.sub_popup == nil return end

    var cat = self.categories[cat_idx]
    var apps = self.cat_apps[cat]

    # clear old submenu content
    ui.clear(self.sub_popup)

    var item_h = ui.MENU_ITEM_HEIGHT
    var pad = 4
    var sub_w = ui.SUB_MENU_WIDTH
    var sub_h = apps.size() * item_h + pad

    # reposition submenu: align with the category item
    var menu_y = ui.TASKBAR_Y - (self.categories.size() * item_h + ui.MENU_SEPARATOR_HEIGHT + item_h + pad)
    var sub_y = menu_y + cat_idx * item_h
    if sub_y + sub_h > ui.TASKBAR_Y
      sub_y = ui.TASKBAR_Y - sub_h
    end
    if sub_y < 0
      sub_y = 0
    end
    ui.set_bounds(self.sub_popup, 2 + ui.MENU_WIDTH - 1, sub_y, sub_w, sub_h)

    var me = self
    var btn_y = 2
    for app : apps
      var btn = ui.button(self.sub_popup, app['label'], 2, btn_y, sub_w - 4, item_h)
      self.style_menu_item(btn)
      var path = app['path']
      ui.on_click(btn, def ()
        me.hide_menu()
        action('berry run ' + path)
      end)
      btn_y += item_h
    end

    ui.show_popup(self.sub_popup)
    ui.mark_dirty()
  end

  def refresh()
    var result = action('wm list')
    if result == self.prev_apps_json return end
    self.prev_apps_json = result
    var data = json.load(result)
    if data != nil && data.find('apps') != nil
      self.rebuild(data['apps'])
    end
  end

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

    for i : 0 .. count - 1
      var app = windowed[i]
      var bx = app_x + i * btn_w
      var btn = ui.button(self.content, app['name'], bx, 2, btn_w - 2, self.h - 4)
      if app['focused']
        ui.set_border_colors(btn, ui.BUTTON_SHADOW, ui.BUTTON_HIGHLIGHT)
      end
      var name = app['name']
      ui.on_click(btn, def () action('wm focus ' + name) end)
      self.app_buttons.push(btn)
    end
    ui.mark_dirty()
  end

  def teardown()
  end
end

return TaskbarApp
