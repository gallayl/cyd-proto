# app: Sensors
# startMenu: /Programs/Sensors
import json

class SensorsApp
  var name
  var light_label
  var auto_refresh

  def init()
    self.name = 'Sensors'
    self.auto_refresh = false
  end

  def setup(content, w, h)
    var y = 4

    # Light sensor group box
    var light_gb = ui.groupbox(content, 'Light Sensor', 2, y, w - 4, 60)

    self.light_label = ui.label(light_gb, 'Value: ...', 4, 14, w - 20, 14)
    ui.set_text_color(self.light_label, ui.TEXT_COLOR, ui.WINDOW_BG)
    ui.set_text_size(self.light_label, 1)
    ui.set_align(self.light_label, ui.LEFT)

    var btn = ui.button(light_gb, 'Refresh', 4, 32, 70, 18)
    ui.on_click(btn, / -> self.refresh())

    # auto-refresh checkbox
    var auto_cb = ui.checkbox(light_gb, 'Auto', 80, 32, 80, 18)
    ui.on_change(auto_cb, def (checked)
      self.auto_refresh = checked
    end)

    y += 66

    self.refresh()
    ui.timer(2000, def ()
      if self.auto_refresh
        self.refresh()
      end
    end)
  end

  def refresh()
    var result_str = action('getLightSensorValue')
    var result = json.load(result_str)
    if result != nil && result.find('value') != nil
      ui.set_text(self.light_label, 'Value: ' + str(result['value']))
    end
  end

  def teardown()
  end
end

return SensorsApp
