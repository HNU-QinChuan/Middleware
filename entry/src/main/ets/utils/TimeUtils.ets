export function  getnow(): string {
  let now = new Date();
  const local = now.toLocaleString();        // 依赖系统语言/地区
  // 或按字段手动拼
  const yyyy = now.getFullYear();
  const mm   = String(now.getMonth() + 1).padStart(2, '0');
  const dd   = String(now.getDate()).padStart(2, '0');
  const hh   = String(now.getHours()).padStart(2, '0');
  const mi   = String(now.getMinutes()).padStart(2, '0');
  const ss   = String(now.getSeconds()).padStart(2, '0');
  const custom = `${yyyy}-${mm}-${dd}-${hh}:${mi}:${ss}`;
  return custom;
}