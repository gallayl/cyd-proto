# app: Info
import json

class InfoApp
  var name

  def init()
    self.name = 'Info'
  end

  def setup(content, w, h)
    var scroll = ui.scrollable(content)
    var row_h = 14
    var y = 4

    var info_str = action('info')
    var info = json.load(info_str)

    self.add_row(scroll, y, w, '-- ESP --')
    y += row_h

    if info != nil
      if info.find('esp') != nil
        var esp = info['esp']
        self.add_row(scroll, y, w, 'SDK: ' + str(esp['sdkVersion']))
        y += row_h
        self.add_row(scroll, y, w, 'CPU: ' + str(esp['cpuFreqMhz']) + ' MHz')
        y += row_h
        self.add_row(scroll, y, w, 'Heap: ' + str(esp['freeHeap']) + ' B')
        y += row_h
        self.add_row(scroll, y, w, 'Sketch: ' + str(esp['freeSkSpace']) + ' B')
        y += row_h
      end

      y += 4
      self.add_row(scroll, y, w, '-- Flash --')
      y += row_h

      if info.find('flash') != nil
        var fl = info['flash']
        self.add_row(scroll, y, w, 'Size: ' + str(fl['size']) + ' B')
        y += row_h
        self.add_row(scroll, y, w, 'Speed: ' + str(fl['speed'] / 1000000) + ' MHz')
        y += row_h
      end

      y += 4
      self.add_row(scroll, y, w, '-- Filesystem --')
      y += row_h

      if info.find('fs') != nil
        var fs = info['fs']
        self.add_row(scroll, y, w, 'Total: ' + str(fs['totalBytes']) + ' B')
        y += row_h
        self.add_row(scroll, y, w, 'Used: ' + str(fs['usedBytes']) + ' B')
        y += row_h
      end
    end

    y += 4
    self.add_row(scroll, y, w, '-- WiFi --')
    y += row_h

    var wifi_str = action('wifi info')
    var wifi = json.load(wifi_str)
    if wifi != nil
      if wifi.find('sta') != nil
        self.add_row(scroll, y, w, 'IP: ' + str(wifi['sta']['ipAddress']))
        y += row_h
        self.add_row(scroll, y, w, 'MAC: ' + str(wifi['sta']['macAddress']))
        y += row_h
        self.add_row(scroll, y, w, 'SSID: ' + str(wifi['sta']['ssid']))
        y += row_h
      end
      if wifi.find('wifiRssiDb') != nil
        self.add_row(scroll, y, w, 'RSSI: ' + str(wifi['wifiRssiDb']) + ' dBm')
        y += row_h
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

return InfoApp
