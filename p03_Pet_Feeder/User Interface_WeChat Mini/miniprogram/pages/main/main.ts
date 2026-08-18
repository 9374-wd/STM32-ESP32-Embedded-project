// 获取今天星期几（0=周日，1=周一，2=周二，3=周三，4=周四，5=周五，6=周六）
function getTodayWeek() {
  return new Date().getDay();
}

Page({
  // 唯一用来判断：云端日志的唯一时间戳
  lastUniqueKey: "", // 加这个！
  data: {
    list: [],// 温湿度、食物重量展示列表

    timerFeedStatus: false,// 投食定时开关
    timerWaterStatus: false,// 加水定时开关

    showTimeModal: false,// 时间选择弹窗显示/隐藏
    currentModalType: '',// 当前弹窗类型（feed/water）
    selectedTime: '00:00', // 弹窗选中时间

    feedTime: wx.getStorageSync('feedTime') || "00:00",
    waterTime: wx.getStorageSync('waterTime') || "00:00",
  // 星期选择列表
    weekList: [
      { name: '一', checked: false },
      { name: '二', checked: false },
      { name: '三', checked: false },
      { name: '四', checked: false },
      { name: '五', checked: false },
      { name: '六', checked: false },
      { name: '日', checked: false },
    ],

    feedWeek: wx.getStorageSync('feedWeek') || [],
    waterWeek: wx.getStorageSync('waterWeek') || [],
  },

  lastFeedMinute: -1,// 上次投食时间（分钟数），防止重复触发
  lastWaterMinute: -1,
//生命周期函数,页面加载时执行
  onLoad() {
    this.getOneNetData();// 立即获取一次数据
    setInterval(() => this.getOneNetData(), 3000);//3秒定时器轮询访问
    this.startAllTimer();// 启动所有定时任务
  },
// 页面卸载时执行
  onUnload() {
    if (this.feedTimer) clearInterval(this.feedTimer);
    if (this.waterTimer) clearInterval(this.waterTimer);//页面关闭时清除定时器，防止内存泄漏 / 后台重复执行
  },
//点击星期，切换选中 / 取消
  toggleWeek(e) {
    const index = e.currentTarget.dataset.index;
    let weekList = this.data.weekList;
    weekList[index].checked = !weekList[index].checked;//点击后状态 取反
    this.setData({ weekList });//页面刷新勾选 / 取消了星期。
  },
// 自动勾选已保存的星期
  resetWeekCheck(weekArr) {
    let weekList = this.data.weekList.map((item, index) => {
      item.checked = weekArr.includes(index + 1) || (index === 6 && weekArr.includes(0));
      return item;
    });
    this.setData({ weekList });
  },

  getOneNetData() {
    const TOKEN = "version=2018-10-31&res=products%2F63Ge682gyI&et=2092570216&method=sha1&sign=dHsHiuLOKlgyIfMlvBtytwp8zD4%3D";
    wx.request({
      url: "https://iot-api.heclouds.com/datapoint/history-datapoints",
      method: "GET",
      header: { authorization: TOKEN, version: "2018-10-31" },
      data: {
        product_id: "63Ge682gyI",
        device_name: "Cat_Food",
        datastream_ids: "temp,humi,food_weight,log",
        limit: 1,
        order: "desc"
      },
      success: (res) => {
        if (res.data.code === 0) {
          const data = res.data.data.datastreams;
          let showList = [];
          const temp = data.find(i => i.id === "temp");
          const humi = data.find(i => i.id === "humi");
          const food = data.find(i => i.id === "food_weight");
          const logData = data.find(i => i.id === "log");
  
          if (temp) showList.push({ name: "温度", value: temp.datapoints[0].value + " ℃" });
          if (humi) showList.push({ name: "湿度", value: humi.datapoints[0].value + " %" });
          if (food) showList.push({ name: "食物重量", value: food.datapoints[0].value + " g" });
          this.setData({ list: showList.length ? showList : [{ name: "提示", value: "未上报" }] });
  
          // ======================
        // ✅ 真正永不重复的核心
        // ======================
        if (logData && logData.datapoints && logData.datapoints.length > 0) {
          const point = logData.datapoints[0];
          const val = point.value;
          
          // 生成唯一KEY：用 云端时间 + 动作类型
          const uniqueKey = point.time + "_" + val;

          // 如果已经记录过 → 直接跳过
          if (this.lastUniqueKey === uniqueKey) {
            return;
          }
          // 保存唯一KEY
          this.lastUniqueKey = uniqueKey;

          // 写入日志
          const { formatTime } = require('../../utils/util');
          let logs = wx.getStorageSync('logs') || [];
          const timeStr = formatTime(new Date());
          const realType = val.split('_')[0]; 
          const logStr = realType === "feed"
            ? `✅ 投食 → ${timeStr}`
            : `💧 加水 → ${timeStr}`;  

          logs.unshift(logStr);
          if (logs.length > 30) logs = logs.slice(0, 30);
          wx.setStorageSync('logs', logs);
          console.log("✅ 写入日志：", logStr);
        }
        }
      },
      fail: () => this.setData({ list: [{ name: "温度", value: "网络异常" }] })
    })
  },

  sendCommand(cmd, successCallback) {
    const TOKEN = "version=2018-10-31&res=products%2F63Ge682gyI&et=2092570216&method=sha1&sign=dHsHiuLOKlgyIfMlvBtytwp8zD4%3D";
    wx.request({
      url: "https://iot-api.heclouds.com/datapoint/synccmds?product_id=63Ge682gyI&device_name=Cat_Food&timeout=15",
      method: "POST",
      header: { authorization: TOKEN, "Content-Type": "application/json" },
      data: cmd,
      success: () => {
        wx.showToast({ title: "执行成功", icon: "success" });
        successCallback();
      },
      fail: () => {
        wx.showToast({ title: "执行失败", icon: "error" });
      },
      complete: () => wx.hideLoading()
    })
  },

  sendFeedCommand() {
    wx.showLoading({ title: "执行中..." });
    this.sendCommand({ feed: wx.getStorageSync('feedAmount') || 20 }, () => { });
  },

  sendWaterCommand() {
    wx.showLoading({ title: "执行中..." });
    this.sendCommand({ water: wx.getStorageSync('waterAmount') || 20 }, () => { });
  },

  toggleTimerFeed() {
    let st = !this.data.timerFeedStatus;
    this.setData({ timerFeedStatus: st });
    const today = getTodayWeek();
    const inWeek = this.data.feedWeek.includes(today);
    if (st) {
      wx.showToast({ title: inWeek ? "今日投食" : "非今日投食", icon: "success" });
    } else {
      wx.showToast({ title: "定时已关闭", icon: "none" });
    }
    this.startAllTimer();
  },

  toggleTimerWater() {
    let st = !this.data.timerWaterStatus;
    this.setData({ timerWaterStatus: st });
    const today = getTodayWeek();
    const inWeek = this.data.waterWeek.includes(today);
    if (st) {
      wx.showToast({ title: inWeek ? "今日加水" : "非今日加水", icon: "success" });
    } else {
      wx.showToast({ title: "定时已关闭", icon: "none" });
    }
    this.startAllTimer();
  },

  showFeedTimePicker() {
    this.resetWeekCheck(this.data.feedWeek);
    this.setData({ showTimeModal: true, currentModalType: 'feed', selectedTime: this.data.feedTime });
  },
  showWaterTimePicker() {
    this.resetWeekCheck(this.data.waterWeek);
    this.setData({ showTimeModal: true, currentModalType: 'water', selectedTime: this.data.waterTime });
  },
  closeTimePicker() {
    this.setData({ showTimeModal: false });
  },
  onTimeChange(e) {
    this.setData({ selectedTime: e.detail.value });
  },

  confirmTime() {
    const { currentModalType, selectedTime, weekList } = this.data;
    const weekArr = weekList.map((item, index) =>
      item.checked ? (index === 6 ? 0 : index + 1) : null
    ).filter(i => i !== null);

    if (currentModalType === 'feed') {
      wx.setStorageSync('feedTime', selectedTime);
      wx.setStorageSync('feedWeek', weekArr);
      this.setData({ feedTime: selectedTime, feedWeek: weekArr });
    } else {
      wx.setStorageSync('waterTime', selectedTime);
      wx.setStorageSync('waterWeek', weekArr);
      this.setData({ waterTime: selectedTime, waterWeek: weekArr });
    }
    wx.showToast({ title: '保存成功', icon: 'success' });
    this.closeTimePicker();
    this.lastFeedMinute = -1;
    this.lastWaterMinute = -1;
    this.startAllTimer();
  },

  startAllTimer() {
    if (this.feedTimer) clearInterval(this.feedTimer);
    if (this.waterTimer) clearInterval(this.waterTimer);

    this.feedTimer = setInterval(() => {
      const { timerFeedStatus, feedTime, feedWeek } = this.data;
      if (!timerFeedStatus || !feedTime) return;

      const now = new Date();
      const current = `${now.getHours().toString().padStart(2, 0)}:${now.getMinutes().toString().padStart(2, 0)}`;
      const today = getTodayWeek();
      const nowMin = now.getHours() * 60 + now.getMinutes();

      if (current === feedTime && feedWeek.includes(today) && nowMin !== this.lastFeedMinute) {
        this.lastFeedMinute = nowMin;
        this.sendFeedCommand();
      }
    }, 1000);

    this.waterTimer = setInterval(() => {
      const { timerWaterStatus, waterTime, waterWeek } = this.data;
      if (!timerWaterStatus || !waterTime) return;

      const now = new Date();
      const current = `${now.getHours().toString().padStart(2, 0)}:${now.getMinutes().toString().padStart(2, 0)}`;
      const today = getTodayWeek();
      const nowMin = now.getHours() * 60 + now.getMinutes();

      if (current === waterTime && waterWeek.includes(today) && nowMin !== this.lastWaterMinute) {
        this.lastWaterMinute = nowMin;
        this.sendWaterCommand();
      }
    }, 1000);
  },

  onPullDownRefresh() {
    this.getOneNetData();
  }
})