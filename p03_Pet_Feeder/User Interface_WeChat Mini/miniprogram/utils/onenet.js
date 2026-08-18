// utils/onenet.js
import CryptoJS from 'crypto-js';

// 你的 OneNet 产品信息（从平台复制）
const PRODUCT_ID = "63Ge682gyI";
const ACCESS_KEY = "gdOnp78RN7mVAzHzPX89++VzcfO3+xN5v79X+ZexzdE=";

/**
 * 生成 OneNet 产品级 Token（和你 Python 脚本完全等价）
 * @returns {string} 可用的 Authorization Token
 */
export function getOneNetToken() {
  const version = "2018-10-31";
  const res = `products/${PRODUCT_ID}`;
  // 有效期24小时（秒级时间戳）
  const et = Math.floor(Date.now() / 1000) + 24 * 3600;
  const method = "sha1";

  // 1. Base64 解码 AccessKey
  const key = CryptoJS.enc.Base64.parse(ACCESS_KEY);

  // 2. 拼接签名字符串（和 Python 里的 org 完全一样）
  const org = `${et}\n${method}\n${res}\n${version}`;

  // 3. HMAC-SHA1 签名
  const signBytes = CryptoJS.HmacSHA1(org, key);
  const sign = CryptoJS.enc.Base64.stringify(signBytes);

  // 4. URL 编码（对应 Python 的 quote）
  const encodedRes = encodeURIComponent(res);
  const encodedSign = encodeURIComponent(sign);

  // 5. 拼接最终 Token 字符串
  const token = `version=${version}&res=${encodedRes}&et=${et}&method=${method}&sign=${encodedSign}`;
  return token;
}