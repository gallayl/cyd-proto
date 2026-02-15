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

  def init()
    self.app_buttons = []
    self.prev_apps_json = ''
  end

  def setup(content, w, h)
    self.content = content
    self.w = w
    self.h = h

    # Start button (left side)
    self.start_btn = ui.button(content, 'Start', 2, 2, 50, h - 4)
    ui.on_click(self.start_btn, /-> action('wm start_menu'))

    # Keyboard toggle button (right side)
    self.kb_btn = ui.button(content, 'Kb', w - 24, 2, 22, h - 4)
    ui.on_click(self.kb_btn, /-> action('wm keyboard'))

    ui.timer(300, /-> self.refresh())
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

    var app_x = 56
    var kb_x = self.w - 26
    var avail = kb_x - app_x - 2
    var count = #apps
    if count == 0 return end

    var btn_w = avail / count
    if btn_w > 80 btn_w = 80 end

    for i : 0 .. count - 1
      var app = apps[i]
      var bx = app_x + i * btn_w
      var btn = ui.button(self.content, app['name'], bx, 2, btn_w - 2, self.h - 4)
      # focused apps get pressed-in look (swapped border colors)
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
