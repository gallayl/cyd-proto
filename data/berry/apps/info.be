# app: Info
import json

class InfoApp
  var name

  def init()
    self.name = 'Info'
  end

  def setup(content, w, h)
    # use tabs to organize info sections
    var tab_ctrl = ui.tabs(content, 0, 0, w, h)

    var esp_tab = ui.tabs_add(tab_ctrl, 'ESP')
    var flash_tab = ui.tabs_add(tab_ctrl, 'Flash')
    var fs_tab = ui.tabs_add(tab_ctrl, 'FS')
    var wifi_tab = ui.tabs_add(tab_ctrl, 'WiFi')

    var info_str = action('info')
    var info = json.load(info_str)

    # ESP tab
    var y = 4
    var row_h = 14
    if info != nil && info.find('esp') != nil
      var esp = info['esp']
      self.add_row(esp_tab, y, w - 8, 'SDK: ' + str(esp['sdkVersion']))
      y += row_h
      self.add_row(esp_tab, y, w - 8, 'CPU: ' + str(esp['cpuFreqMhz']) + ' MHz')
      y += row_h
      self.add_row(esp_tab, y, w - 8, 'Free Heap: ' + str(esp['freeHeap']) + ' B')
      y += row_h
      self.add_row(esp_tab, y, w - 8, 'Free Sketch: ' + str(esp['freeSkSpace']) + ' B')
      y += row_h
    else
      self.add_row(esp_tab, y, w - 8, 'No ESP info available')
    end

    # Flash tab
    y = 4
    if info != nil && info.find('flash') != nil
      var fl = info['flash']
      self.add_row(flash_tab, y, w - 8, 'Size: ' + str(fl['size']) + ' B')
      y += row_h
      self.add_row(flash_tab, y, w - 8, 'Speed: ' + str(fl['speed'] / 1000000) + ' MHz')
      y += row_h
    else
      self.add_row(flash_tab, y, w - 8, 'No flash info available')
    end

    # Filesystem tab
    y = 4
    if info != nil && info.find('fs') != nil
      var fs = info['fs']
      self.add_row(fs_tab, y, w - 8, 'Total: ' + str(fs['totalBytes']) + ' B')
      y += row_h
      self.add_row(fs_tab, y, w - 8, 'Used: ' + str(fs['usedBytes']) + ' B')
      y += row_h
      var free = fs['totalBytes'] - fs['usedBytes']
      self.add_row(fs_tab, y, w - 8, 'Free: ' + str(free) + ' B')
      y += row_h
    else
      self.add_row(fs_tab, y, w - 8, 'No filesystem info')
    end

    # WiFi tab
    y = 4
    var wifi_str = action('wifi info')
    var wifi = json.load(wifi_str)
    if wifi != nil
      if wifi.find('sta') != nil
        self.add_row(wifi_tab, y, w - 8, 'IP: ' + str(wifi['sta']['ipAddress']))
        y += row_h
        self.add_row(wifi_tab, y, w - 8, 'MAC: ' + str(wifi['sta']['macAddress']))
        y += row_h
        self.add_row(wifi_tab, y, w - 8, 'SSID: ' + str(wifi['sta']['ssid']))
        y += row_h
      end
      if wifi.find('wifiRssiDb') != nil
        self.add_row(wifi_tab, y, w - 8, 'RSSI: ' + str(wifi['wifiRssiDb']) + ' dBm')
        y += row_h
      end
    else
      self.add_row(wifi_tab, y, w - 8, 'No WiFi info available')
    end
  end

  def add_row(parent, y, w, text)
    var lbl = ui.label(parent, text, 4, y, w, 12)
    ui.set_text_color(lbl, ui.TEXT_COLOR, ui.WINDOW_BG)
    ui.set_text_size(lbl, 1)
    ui.set_align(lbl, ui.LEFT)
  end

  def teardown()
  end
end

return InfoApp
