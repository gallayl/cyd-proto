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
    var y = 4

    # Heap info group box
    var heap_gb = ui.groupbox(scroll, 'Memory', 2, y, w - 4, 34)
    var info_str = action('info')
    var info = json.load(info_str)
    if info != nil && info.find('esp') != nil
      var esp = info['esp']
      var free = esp['freeHeap']
      var total = 327680
      var used = total - free
      var text = str(used / 1024) + 'KB / ' + str(total / 1024) + 'KB'
      var lbl = ui.label(heap_gb, text, 4, 14, w - 20, 12)
      ui.set_text_color(lbl, ui.TEXT_COLOR, ui.WINDOW_BG)
      ui.set_text_size(lbl, 1)
      ui.set_align(lbl, ui.LEFT)
    end
    y += 42

    # Features group box
    var feat_gb = ui.groupbox(scroll, 'Features', 2, y, w - 4, 0)
    var fy = 14

    var feat_str = action('features')
    var features = json.load(feat_str)
    if features != nil
      for i : 0 .. size(features) - 1
        var f = features[i]
        var fname = str(f['name'])
        var state = str(f['state'])
        var is_running = (state == 'RUNNING')

        # checkbox to show running state
        var cb = ui.checkbox(feat_gb, fname, 2, fy, w - 60, 16)
        ui.set_checked(cb, is_running)

        var can_start = (state == 'PENDING' || state == 'STOPPED' || state == 'ERROR')
        var can_stop = is_running

        if can_start || can_stop
          var btn_label = can_stop ? 'Stop' : 'Start'
          var btn = ui.button(feat_gb, btn_label, w - 56, fy, 42, 16)

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
        end

        fy += 20
      end
    end

    # resize group box to fit its content
    var feat_gb_h = fy + 18
    var gb_bounds = ui.bounds(feat_gb)
    ui.set_bounds(feat_gb, gb_bounds[0], gb_bounds[1], gb_bounds[2], feat_gb_h)

    y += feat_gb_h + 6
    ui.set_content_height(scroll, y)
  end

  def teardown()
  end
end

return FeaturesApp
