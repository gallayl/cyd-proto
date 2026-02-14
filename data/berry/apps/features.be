# app: Features
import json

class FeaturesApp
  var name
  var content
  var app_w
  var app_h

  def init()
    self.name = 'Features'
  end

  def setup(content, w, h)
    self.content = content
    self.app_w = w
    self.app_h = h
    self.build_ui()
    ui.timer(2000, / -> ui.mark_dirty())
  end

  def build_ui()
    ui.clear(self.content)

    var w = self.app_w
    var scroll = ui.scrollable(self.content)

    var row_h = 14
    var btn_h = 18
    var y = 4

    self.add_row(scroll, y, w, 'Heap:')
    y += row_h

    # Heap info from system
    var info_str = action('info')
    var info = json.load(info_str)
    if info != nil && info.find('esp') != nil
      var esp = info['esp']
      var free = esp['freeHeap']
      var total = 327680
      var used = total - free
      var text = str(used / 1024) + 'KB / ' + str(total / 1024) + 'KB'
      self.add_row(scroll, y, w, text)
      y += row_h + 6
    else
      y += 6
    end

    self.add_row(scroll, y, w, '-- Registered Features --')
    y += row_h

    var feat_str = action('features')
    var features = json.load(feat_str)
    if features != nil
      for i : 0 .. size(features) - 1
        var f = features[i]
        var fname = str(f['name'])
        var state = str(f['state'])
        var line = fname + ': ' + state

        self.add_row(scroll, y, w - 50, line)

        var can_start = (state == 'PENDING' || state == 'STOPPED' || state == 'ERROR')
        var can_stop = (state == 'RUNNING')

        if can_start || can_stop
          var btn_w = 42
          var btn_x = w - btn_w - 4
          var btn_label = can_stop ? 'Stop' : 'Start'
          var btn = ui.button(scroll, btn_label, btn_x, y, btn_w, btn_h)

          if can_stop
            var n = fname
            ui.on_click(btn, def ()
              action('features stop ' + n)
              self.build_ui()
              ui.mark_dirty()
            end)
          else
            var n = fname
            ui.on_click(btn, def ()
              action('features start ' + n)
              self.build_ui()
              ui.mark_dirty()
            end)
          end
          y += btn_h + 2
        else
          y += row_h
        end
      end
    end

    ui.set_content_height(scroll, y)
  end

  def add_row(scroll, y, w, text)
    var lbl = ui.label(scroll, text, 4, y, w - 8, 12)
    ui.set_text_color(lbl, ui.TEXT_COLOR, ui.WINDOW_BG)
    ui.set_text_size(lbl, 1)
    ui.set_align(lbl, ui.LEFT)
  end

  def teardown()
  end
end

return FeaturesApp
