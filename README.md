# FPC 指纹传感器 Linux 驱动存档（USB 10a5:9201）

**English summary.** Self-contained archive of an unmaintained community driver for the FPC fingerprint sensor `10a5:9201`, firmware `021.26.2.x` (Redmi Book Pro 14 2022 / RedmiBook). The tested file is `bin/fingerpp3.static` (SHA256 `4e9a3b8d71bb818ceb5b4fa6b16cbfa5b1619afd1eba548cda723396c94e5f8c`). On Ubuntu 26.04, kernel 7.0, GNOME, 2026-10-26, enroll and verify both succeeded. Install with `sudo ./install.sh`, then run `fprintd-enroll -f right-index-finger`. License: AGPL-3.0. Upstream last changed in December 2022. Use at your own risk. After cloning this repository you do not need the upstream remotes to stay online.

## 支持的硬件

小米 Redmi Book Pro 14 2022（RedmiBook）上的 FPC 指纹传感器：

- USB ID：`10a5:9201`
- 固件系列：`021.26.2.x`（实测机器的固件版本是 `021.26.2.031`）

确认传感器在线上：

```bash
lsusb -d 10a5:9201
```

输出里应能看到 FPC Sensor Controller。其它 USB ID，或不是 `021.26.2.x` 的固件，不在这次测试范围内。

## 测试结果

2026-10-26，Ubuntu 26.04，内核 7.0，GNOME：

- `fprintd-enroll` 录入成功
- 指纹校验成功

用的是本仓库 `bin/fingerpp3.static`。systemd 覆盖项包含 `--filter-before-ssim=true --min-score=0.3`。

## 仓库内容

| 路径 | 内容 |
| --- | --- |
| `bin/fingerpp3.static` | 安装脚本使用的二进制。OpenCV 已链进可执行文件，运行时仍需要 `libssl.so.3`、`libcrypto.so.3`、`libusb-1.0.so.0`、`libdbus-1.so.3`。 |
| `bin/fingerpp3.fedora` | 备用构建，依赖系统里的 OpenCV 4.5 和 libevent。 |
| `bin/fingerpp3.ubuntu2210` | 备用构建，依赖 OpenCV 4.6（`*.so.406`）和 libevent。 |
| `install.sh`、`uninstall.sh` | 本仓库的安装和卸载脚本，可重复执行。 |
| `upstream/` | 上游 `install.sh` 和 `fprintd-override.service` 的原样副本。 |
| `src/fingerprint-ocv/` | 上游源码快照。提交号写在 `src/REVISION`。 |
| `SHA256SUMS` | 三个二进制和 `upstream/` 两个文件的校验和。 |
| `LICENSE` | GNU AGPL-3.0 全文。 |

在仓库根目录核对校验和：

```bash
sha256sum -c SHA256SUMS
```

`fingerpp3.fedora` 的 SHA256 以 `804b97d3` 开头，`fingerpp3.ubuntu2210` 以 `f751fe21` 开头。默认安装只用 `fingerpp3.static`。

## 安装

先安装发行版的 `fprintd`，登录和 sudo 还需要对应的 PAM 包。`fingerpp3.static` 另外依赖 OpenSSL、libusb 和 libdbus，发行版桌面系统上通常已经有。

- Debian / Ubuntu（apt）：`sudo apt-get install fprintd libpam-fprintd`。缺库时再装 `libssl3`、`libusb-1.0-0`、`libdbus-1-3`。
- Fedora（dnf）：`sudo dnf install fprintd fprintd-pam`。缺库时再装 `openssl-libs`、`libusb1`、`dbus-libs`。
- Arch Linux（pacman）：`sudo pacman -S fprintd`。缺库时再装 `openssl`、`libusb`、`dbus`。

Ubuntu：

```bash
sudo apt-get install fprintd libpam-fprintd
sudo ./install.sh
```

Fedora：

```bash
sudo dnf install fprintd fprintd-pam
sudo ./install.sh
```

Arch：

```bash
sudo pacman -S fprintd
sudo ./install.sh
```

用 `sudo` 运行 `install.sh`。脚本会：

1. 找不到 `fprintd-list` 时按上面的包名提示并退出。
2. 核对 `bin/fingerpp3.static` 的 SHA256 是否为 `4e9a3b8d71bb818ceb5b4fa6b16cbfa5b1619afd1eba548cda723396c94e5f8c`。
3. 停掉 `fprintd`，把该文件复制为 `/opt/fingerpp/bin/fingerpp`（模式 `0755`）。
4. 写入 `/etc/systemd/system/fprintd.service.d/override.conf`：

```ini
[Service]
ExecStart=
ExecStart=/opt/fingerpp/bin/fingerpp --bus=system --filter-before-ssim=true --min-score=0.3
```

5. 执行 `systemctl daemon-reload`，再重启 `fprintd`。
6. 轮询 `fprintd-list $SUDO_USER`，直到输出里出现 `/net/reactivated/Fprint/Device/`。最多等约 45 秒。

设备出现后，用刚才那个普通用户执行（不要给这条命令加 sudo，否则会录到 root 上）：

```bash
fprintd-enroll -f right-index-finger
```

再执行一次 `sudo ./install.sh` 只会覆盖同一路径并重新轮询，可以重复运行。

## 录入

`fprintd-enroll` 没有进度条。按提示把手指放到传感器上，再抬起来，反复几次，直到终端打印 `enroll-completed`。中间若像停住了，继续抬起、按下，不要马上中断。

录入后可以检查：

```bash
fprintd-list "$USER"
fprintd-verify
```

## 登录和 sudo

指纹录进 fprintd 之后，还要让 PAM 在登录和 `sudo` 时调用它。

- Ubuntu / Debian：`sudo pam-auth-update`，启用 Fingerprint authentication。
- Fedora：`sudo authselect enable-feature with-fingerprint`。

Arch 没有对应的一条开关。在要用指纹的 PAM 配置里，把 `pam_fprintd.so` 放进靠前的 `auth` 段，常见文件是 `/etc/pam.d/system-local-login` 和 `/etc/pam.d/sudo`：

```
auth    sufficient    pam_fprintd.so
```

改 PAM 时留一个已经拿到 root 的终端，确认新配置可用再关掉它。

## 已知问题

- 覆盖项里如果没有 `--filter-before-ssim=true --min-score=0.3`，录入进行到一半会触发 OpenCV `Mat` 的 `rowRange` 断言，进程崩溃并留下 core dump。`install.sh` 写入的 override 已经带上这两个参数。
- `fprintd` 刚重启的几秒内，`fprintd-list` 可能报 `GetDefaultDevice doesn't exist`。等几秒再运行同一条命令。安装脚本会自己重试大约 45 秒；超时后用 `fprintd-list` 手动再试即可。

## 卸载

```bash
sudo ./uninstall.sh
```

脚本删掉 `/etc/systemd/system/fprintd.service.d/override.conf` 和 `/opt/fingerpp`。drop-in 目录如果因此变空，一并删掉。然后 `daemon-reload` 并重启 `fprintd`，回到发行版自带的守护进程。再运行一次同样是安全的。

已经录入的指纹仍在 `/var/lib/fprint`，卸载不会删除它们。

## 上游、许可和风险

这是社区驱动，上游自 2022-12 起没有再更新，使用风险自负。二进制和源码都放在本仓库里，是为了以后在 Fedora、Ubuntu 或 Arch 上重装时，不依赖上游仓库继续在线。

- 驱动源码：<https://github.com/vrolife/fingerprint-ocv>，AGPL-3.0。快照提交 `7691977ca8d28050f95a42454880b572ce7f9d5e`（2022-12-04，`Remove openssl libusb dbus from vcpkg.json`），目录是 `src/fingerprint-ocv/`。该提交锁定并已一并放入快照的子模块：
  - `asyncdbus` `62c2c87092c7674fcd3ec98573a2a436bfff9d83`（AGPL-3.0）
  - `asyncusb` `d9e29478037123cfff8f009d7112250c4b2fd3df`（AGPL-3.0）
  - `jinx` `154366c25a0b8c3f531c37efbfffa4934a138523`（此提交的 LICENSE 为 AGPL-3.0）
- 预编译文件和 `upstream/` 里的脚本、服务片段来自 <https://github.com/vrolife/modern_laptop> 的 `drivers/fingerprint/`，提交 `cc4db790f91c7f5ac76e9160ddc9a754ff2ca5e9`。
- 作者：pom（<https://github.com/vrolife>）。

构建用的 vcpkg 没有放进仓库。上述源码提交把它固定在 `163fe7bd3d67c41200617caaa245b5ba2ba854e6`（<https://github.com/microsoft/vcpkg>，MIT，2022-12-02）。`fingerpp3.static` 静态链接了 OpenCV（Apache-2.0）；这份 OpenCV 源码由 vcpkg 在编译时下载，不在快照中。重装已经测过的二进制不需要重新编译。

顶层 `LICENSE` 与 `src/fingerprint-ocv/LICENSE` 都是 GNU Affero General Public License v3 原文，上游许可证文件保持原样。
