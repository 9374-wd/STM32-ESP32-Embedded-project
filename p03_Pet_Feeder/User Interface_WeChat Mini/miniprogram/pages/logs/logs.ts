import { formatTime } from '../../utils/util'

Component({
  data: {
    logs: [] as string[],
    showLogModal: false
  },

  lifetimes: {
    attached() {
      this.loadLogs()
    }
  },

  methods: {
    loadLogs() {
      let logs = wx.getStorageSync<string[]>('logs') || []
      logs = logs.filter(item => typeof item === 'string' && !/^\d+$/.test(item))
      this.setData({ logs })
    },

    // ======================
    // 只给主页面调用
    // ======================
    logFeed() {
      let logs = wx.getStorageSync<string[]>('logs') || []
      const time = formatTime(new Date())
      logs.unshift(`✅ 投食 → ${time}`)
      if (logs.length > 30) logs = logs.slice(0, 30)
      wx.setStorageSync('logs', logs)
      this.loadLogs()
    },

    logWater() {
      let logs = wx.getStorageSync<string[]>('logs') || []
      const time = formatTime(new Date())
      logs.unshift(`💧 加水 → ${time}`)
      if (logs.length > 30) logs = logs.slice(0, 30)
      wx.setStorageSync('logs', logs)
      this.loadLogs()
    },

    goLogs() {
      this.loadLogs()
      this.setData({ showLogModal: true })
    },

    closeLogModal() {
      this.setData({ showLogModal: false })
    },

    clearLogs() {
      wx.showModal({
        title: '提示',
        content: '确定要清空所有日志吗？',
        success: (res) => {
          if (res.confirm) {
            wx.removeStorageSync('logs')
            this.setData({ logs: [] })
            wx.showToast({ title: '已清空', icon: 'success' })
          }
        }
      })
    },

    bindDevice() {
      wx.showToast({ title: '设备绑定', icon: 'none' })
    },

    goSetting() {
      wx.switchTab({ url: '/pages/notice/notice' })
    },

    logout() {
      wx.showModal({
        title: '提示',
        content: '确定退出登录？',
        success: (res) => {
          if (res.confirm) {
            wx.clearStorageSync()
            wx.showToast({ title: '退出成功' })
          }
        }
      })
    }
  }
})