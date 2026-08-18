// index.ts
Component({
  data: {

  },
  methods: {
    // 跳转到设备控制主页
    goControlPage() {
      wx.switchTab({
        url: '/pages/main/main'
      })
    }
  }
})