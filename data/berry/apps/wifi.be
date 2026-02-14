# app: WiFi
import json

class WifiApp
  var name
  var scroll
  var status_lbl
  var ip_lbl
  var ssid_lbl
  var rssi_lbl
  var scan_start_y
  var scan_labels
  var app_w

  def init()
    self.name = 'WiFi'
    self.scan_labels = []
  end

  def setup(content, w, h)
    self.app_w = w
    var scroll = ui.scrollable(content)
    self.scroll = scroll
    var row_h = 14
    var y = 4

    # Connection group box
    var conn_gb = ui.groupbox(scroll, 'Connection', 2, y, w - 4, 72)
    var cy = 14

    self.status_lbl = self.add_row_label(conn_gb, cy, w - 16, 'Status: ...')
    cy += row_h
    self.ip_lbl = self.add_row_label(conn_gb, cy, w - 16, 'IP: ...')
    cy += row_h
    self.ssid_lbl = self.add_row_label(conn_gb, cy, w - 16, 'SSID: ...')
    cy += row_h
    self.rssi_lbl = self.add_row_label(conn_gb, cy, w - 16, 'RSSI: ...')

    y += 78

    # Scan group box
    var scan_gb = ui.groupbox(scroll, 'Networks', 2, y, w - 4, 28)

    var btn = ui.button(scan_gb, 'Scan', 2, 14, 60, 18)
    ui.on_click(btn, / -> self.do_scan())

    y += 34
    self.scan_start_y = y
    ui.set_content_height(scroll, y)

    self.refresh_info()
    ui.timer(5000, / -> self.refresh_info())
  end

  def refresh_info()
    var wifi_str = action('wifi info')
    var info = json.load(wifi_str)
    if info == nil return end

    if info.find('sta') != nil
      var sta = info['sta']
      ui.set_text(self.ssid_lbl, 'SSID: ' + str(sta['ssid']))
      ui.set_text(self.ip_lbl, 'IP: ' + str(sta['ipAddress']))
    end

    if info.find('wifiStrength') != nil
      ui.set_text(self.status_lbl, 'Status: ' + str(info['wifiStrength']))
    end

    if info.find('wifiRssiDb') != nil
      ui.set_text(self.rssi_lbl, 'RSSI: ' + str(info['wifiRssiDb']) + ' dBm')
    end
  end

  def do_scan()
    var scan_str = action('wifi list')
    var networks = json.load(scan_str)

    # remove old scan labels
    for lbl : self.scan_labels
      ui.remove_child(self.scroll, lbl)
    end
    self.scan_labels = []

    var y = self.scan_start_y
    var w = self.app_w
    var row_h = 14

    if networks == nil || size(networks) == 0
      self.scan_labels.push(self.add_row_label(self.scroll, y, w, 'No networks found'))
      y += row_h
    else
      var count = size(networks)
      if count > 10 count = 10 end
      for i : 0 .. count - 1
        var n = networks[i]
        var line = str(n['ssid']) + ' (' + str(n['rssi']) + ' dBm)'
        self.scan_labels.push(self.add_row_label(self.scroll, y, w, line))
        y += row_h
      end
    end

    ui.set_content_height(self.scroll, y)
  end

  def add_row_label(parent, y, w, text)
    var lbl = ui.label(parent, text, 4, y, w - 8, 12)
    ui.set_text_color(lbl, ui.TEXT_COLOR, ui.WINDOW_BG)
    ui.set_text_size(lbl, 1)
    ui.set_align(lbl, ui.LEFT)
    return lbl
  end

  def teardown()
  end
end

return WifiApp
