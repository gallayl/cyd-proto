# app: Sensors
import json

class SensorsApp
  var name
  var light_label

  def init()
    self.name = 'Sensors'
  end

  def setup(content, w, h)
    var row_h = 14
    var y = 4

    var header = ui.label(content, '-- Light Sensor --', 4, y, w - 8, 12)
    ui.set_text_color(header, ui.TEXT_COLOR, ui.WINDOW_BG)
    ui.set_text_size(header, 1)
    ui.set_align(header, ui.LEFT)
    y += row_h

    self.light_label = ui.label(content, 'Value: ...', 4, y, w - 8, 12)
    ui.set_text_color(self.light_label, ui.TEXT_COLOR, ui.WINDOW_BG)
    ui.set_text_size(self.light_label, 1)
    ui.set_align(self.light_label, ui.LEFT)
    y += row_h + 8

    var btn = ui.button(content, 'Refresh', 4, y, 70, 22)
    ui.on_click(btn, / -> self.refresh())

    self.refresh()
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
