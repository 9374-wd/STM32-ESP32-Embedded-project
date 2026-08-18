Page({
  data: {
    feedAmount: '',
    waterAmount: ''
  },

  onLoad() {
    this.loadSavedData()
  },

  loadSavedData() {
    let feed = wx.getStorageSync('feedAmount') || 20
    let water = wx.getStorageSync('waterAmount') || 20
    this.setData({
      feedAmount: feed,
      waterAmount: water
    })
  },

  onFeedInput(e) {
    this.setData({
      feedAmount: e.detail.value
    })
  },

  onWaterInput(e) {
    this.setData({
      waterAmount: e.detail.value
    })
  },

  saveSettings() {
    let { feedAmount, waterAmount } = this.data
    
    if (!feedAmount || !waterAmount) {
      wx.showToast({ title: '请填写完整', icon: 'none' })
      return
    }

    wx.setStorageSync('feedAmount', Number(feedAmount))
    wx.setStorageSync('waterAmount', Number(waterAmount))

    wx.showToast({ title: '保存成功' })

    // 发送默认量到硬件
    this.sendCommand({ set_default_feed: Number(feedAmount) })
    this.sendCommand({ set_default_water: Number(waterAmount) })
  },

  // ================================
  // ✅ 测试定时（直接发送）
  // ================================
  testSetFeedTime() {
    this.sendCommand({ set_feed_time: "20:37" })
    wx.showToast({ title: "投喂时间已设置", icon: "success" })
  },

  testSetWaterTime() {
    this.sendCommand({ set_water_time: "20:36" })
    wx.showToast({ title: "加水时间已设置", icon: "success" })
  },

  // ================================
  // ✅ 把 sendCommand 也放到这里
  // 这样所有功能都能正常用！
  // ================================
  sendCommand(cmd) {
    const TOKEN = "version=2018-10-31&res=products%2F63Ge682gyI&et=2092570216&method=sha1&sign=dHsHiuLOKlgyIfMlvBtytwp8zD4%3D";
    const PRODUCT_ID = "63Ge682gyI";
    const DEVICE_NAME = "Cat_Food";
    const TIMEOUT = 15;

    wx.showLoading({ title: "保存中..." });

    wx.request({
      url: `https://iot-api.heclouds.com/datapoint/synccmds?product_id=${PRODUCT_ID}&device_name=${DEVICE_NAME}&timeout=${TIMEOUT}`,
      method: "POST",
      header: {
        "authorization": TOKEN,
        "Content-Type": "application/json",
      },
      data: cmd,
      success: (res) => {
        wx.hideLoading();
        const result = res.data;
        if (result.code === 0) {
          wx.showToast({ title: "保存成功！", icon: "success" });
        } else {
          wx.showToast({ title: "保存失败！", icon: "error" });
        }
      },
      fail: (err) => {
        wx.hideLoading();
        wx.showToast({ title: "网络异常", icon: "error" });
      }
    });
  }
});