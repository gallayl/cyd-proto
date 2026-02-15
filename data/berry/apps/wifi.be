# app: WiFi
import json

class WifiApp
  var name
  var scroll
  var status_lbl
  var ip_lbl
  var ssid_lbl
  var rssi_lbl
  var scan_gb
  var scan_gb_rel_y
  var scan_gb_abs_x
  var scan_gb_abs_y
  var scan_gb_abs_w
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
    self.scan_gb_rel_y = y
    self.scan_gb = ui.groupbox(scroll, 'Networks', 2, y, w - 4, 40)
    var bi = 0
    for v : ui.bounds(self.scan_gb)
      if bi == 0 self.scan_gb_abs_x = v
      elif bi == 1 self.scan_gb_abs_y = v
      elif bi == 2 self.scan_gb_abs_w = v
      end
      bi += 1
    end

    var btn = ui.button(self.scan_gb, 'Scan', 2, 14, 60, 18)
    ui.on_click(btn, / -> self.do_scan())

    y += 46
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

    # remove old scan labels from the groupbox
    for lbl : self.scan_labels
      ui.remove_child(self.scan_gb, lbl)
    end
    self.scan_labels = []

    var w = self.app_w
    var row_h = 14
    var ry = 36

    if networks == nil || size(networks) == 0
      self.scan_labels.push(self.add_row_label(self.scan_gb, ry, w - 16, 'No networks found'))
      ry += row_h
    else
      var count = 0
      for n : networks
        if count >= 10 break end
        var line = str(n['ssid']) + ' (' + str(n['rssi']) + ' dBm)'
        self.scan_labels.push(self.add_row_label(self.scan_gb, ry, w - 16, line))
        ry += row_h
        count += 1
      end
    end

    # resize the groupbox to fit its content
    var new_h = ry + 18
    ui.set_bounds(self.scan_gb, self.scan_gb_abs_x, self.scan_gb_abs_y, self.scan_gb_abs_w, new_h)

    ui.set_content_height(self.scroll, self.scan_gb_rel_y + new_h + 4)
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
