# 自动同步与双语每日构建

本规范定义本 fork（`Su-Qingmu/cs2`）相对于上游 `tiansongyu/cs2_cheat` 的核心增量：**每日自动同步上游、构建 Windows EXE、发布中英双语 release**。

流水线实现在 `.github/workflows/build.yaml`，并由 `.github/workflows/update-files.yml`（偏移更新）在偏移提交后调度触发。

## 1. 工作流身份

| 项 | 值 |
|---|---|
| Workflow 文件 | `.github/workflows/build.yaml` |
| 名称 | `Build CS2 ESP (SDL2 + ImGui, bilingual)` |
| 权限 | `contents: write`（推送 `main`、创建/移动 tag、创建 release） |
| 并发组 | `build-bilingual-${{ github.repository }}`，`cancel-in-progress: false` |

> **重要**：上游 `tiansongyu/cs2_cheat` 的 `main` 分支同样存在一个 `build.yaml`。同步上游时若该文件发生冲突，必须保留 fork 的版本（`git checkout --ours -- .github/workflows/build.yaml`），否则会丢失双语流水线。`sync` job 已经内置该冲突处理。

## 2. 触发条件

| 触发 | 触发方式 | 说明 |
|---|---|---|
| `schedule` | 每日 16:00 UTC（北京时间 00:00） | 上游无变动且未强制构建时跳过构建与发布 |
| `push` 到 `main` | 仅命中路径过滤（源码、测试、`i18n/`、`scripts/` 等） | 纯偏移文件变更（`external-cheat-base/generated/**`）不触发 |
| `workflow_dispatch` | 手动运行，可指定 `force_build` / `source_sha` | `source_sha` 非空时直接构建该提交且不改动 `main` |

### `force_build` 与 `source_sha`

- `force_build`（boolean，默认 `false`）：跳过「上游无变动 → 跳过构建」判定，强制构建并发布。
- `source_sha`（string，默认 `''`）：指定要构建的确切提交。`update-files.yml` 在提交新的偏移文件后会用这个入参调度本工作流。

## 3. 作业图

```
                       ┌───────────────┐
        触发 ─────────▶│    sync job   │  (ubuntu)  合并上游、决定是否构建、计算 tag
                       └───────┬───────┘
                               │ should_build=true
              ┌────────────────┴─────────────────┐
              ▼                                  ▼
      ┌───────────────────┐          ┌────────────────────────┐
      │   unit-tests job  │          │ public-relay-tests job │  (ubuntu)
      │      (ubuntu)     │          │   Go 测试 / 镜像构建    │
      └─────────┬─────────┘          └────────────────────────┘
                │ web-radar-static 构件
        ┌───────┴────────┐
        ▼                ▼
┌────────────────┐ ┌────────────────┐
│  build-en job  │ │  build-zh job  │  (windows-2022)
│   cs2-en.exe   │ │   cs2-cn.exe   │
└────────┬───────┘ └────────┬───────┘
         └────────┬─────────┘
                  ▼
          ┌────────────────┐
          │  release job   │  (ubuntu)  创建/移动 tag、发布 cs2-bilingual.zip
          └────────────────┘
```

### 3.1 sync（ubuntu-latest）

**输出**（作业级 `outputs`，这是 `release` job 能拿到 tag 的唯一可靠途径）：

- `should_build`：`true` / `false`
- `source_sha`：本次要构建的提交 SHA
- `tag`：形如 `26.11.13` 的日期标签

**步骤**：

1. `actions/checkout@v5` 检出 fork `main`（`fetch-depth: 0`）
2. 计算 release tag：`y=${ts:2:2}`、`m=$(date -u +%-m)`、`d=$(date -u +%-d)` → `YY.M.D`
3. 若 `source_sha` 非空：直接以该提交为 `source_sha`、`should_build=true`，**不合并上游、不改动 `main`**
4. 否则：`git checkout -B main origin/main` → 拉取 `upstream/main` → 若上游有新提交则 `git merge --no-edit --no-ff -m "Sync upstream: ${sha}"` 并 `git push origin HEAD:main`
   - 合并冲突时：`.github/workflows/build.yaml` 一律取 fork 版本；其余冲突未解决则直接失败
5. `should_build` 判定：`force_build == true` **或** 上游确有新提交 **或** 触发事件是 `push` → `true`；否则 `false`（跳过构建与发布）

### 3.2 unit-tests / public-relay-tests（ubuntu-latest）

沿用上游的质量门禁：

- C++ 回归测试（viewport / runtime timing / web radar model）
- 中英文案翻译回归测试（`python3 tests/translate_menu_tests.py`）
- 内嵌 CivetWeb 服务的冒烟测试
- Web Radar 前端 `npm ci` → `npm test` → `npm build` → 校验产物，并上传 `web-radar-static` 构件
- `radar-relay` 的 `go test -race` / `go vet` / `govulncheck`，以及 Relay 与 Caddy 镜像构建、部署配置校验

### 3.3 build-en / build-zh（windows-2022）

两个作业都检出同一个 `source_sha`，下载同一个 `web-radar-static` 构件，然后 `msbuild external-cheat-base.sln /p:Configuration=Release /p:Platform=x64`。

区别只有一步：`build-zh` 在编译前执行文案翻译：

```bash
python scripts/translate_menu.py \
  --map i18n/zh_map.json \
  --source external-cheat-base/src/features/menu.hpp \
  --in-place \
  --strict
```

编译产物分别重命名为 `cs2-en.exe` 与 `cs2-cn.exe`，并各自上传构件。

### 3.4 release（ubuntu-latest）

1. 下载 `en-build` 与 `zh-build` 两个构件，合并为 `cs2-bilingual.zip`（内含 `cs2-en.exe`、`cs2-cn.exe`、`SDL2.dll`、`CIVETWEB-LICENSE.md` 与 `web-radar/dist/`）
2. **创建/移动 tag**：
   - `GET repos/{repo}/git/ref/tags/{tag}` 命中 → `PATCH` + `force=true` 把已有 tag 移到本次构建的提交
   - 未命中 → `POST repos/{repo}/git/refs` 新建 `refs/tags/{tag}`
3. `softprops/action-gh-release@v2` 发布 release，`overwrite_files: true`、`make_latest: true`

> 早期版本把 tag 的计算放在 `build` 作业内，却没有声明作业级 `outputs`，导致 `needs.build.outputs.tag` 恒为空，tag 与 release 全部失败。现在 tag 在 `sync` 作业计算并通过作业级 `outputs` 传递。

## 4. 中英双语机制

### 4.1 为什么不再是「第二个分支 + 补丁」

旧方案把 `menu.hpp` 的中文版维护在 `zh-main` 分支，并用 `.github/i18n/zh.patch` 在每次同步后重新套用。该方案有两个硬伤：

- 上游只要改动 `menu.hpp`，补丁就会失效——同步 job 只会打印一行 `WARN`，中文构建**静默退回英文界面**；
- 补丁把 C++ 标识符也改成了中文（如 `webRadar暂停WhenUnfocused`），可读性与可维护性都很差。

### 4.2 现在的做法

| 文件 | 作用 |
|---|---|
| `i18n/zh_map.json` | 英文文案 → 中文文案的对照表（约 220 条），按需增量维护 |
| `scripts/translate_menu.py` | 依据对照表改写 `menu.hpp` 中的**窄字符串字面量** |
| `tests/translate_menu_tests.py` | 翻译器的回归测试 |

翻译器的硬性保证：

- 只改写窄字符串字面量；宽字符串（`L"..."`）永不改动——持久化配置键因此保持原样；
- 以 `##` 开头的 ImGui ID 永不改动；
- 注释与预处理指令永不改动；
- 中英文的 `printf` 转换说明符必须完全一致，否则该条文案保持英文并告警；
- 上游新增的文案暂时没有译文时保持英文，不会导致构建失败。

因此上游可以自由改动 `menu.hpp`：英文构建始终拿到最新界面，中文构建最多是新增文案暂时显示为英文。

## 5. 与偏移更新工作流的契约

`update-files.yml` 每小时检查 `cs2-dumper`，在提交新的偏移文件之后通过 `workflow_dispatch` 调度本工作流：

```bash
gh workflow run build.yaml --repo <repo> --ref main \
  -f force_build=true -f source_sha="<sha>"
```

**该调度有两个硬性前提**：

1. `build.yaml` 必须存在于默认分支（`main`）；
2. `build.yaml` 必须声明 `workflow_dispatch` 触发器，并接受 `force_build` 与 `source_sha` 两个入参。

任一条不满足，GitHub 会返回：

```
HTTP 422: Workflow does not have 'workflow_dispatch' trigger
```

这正是 fork 早期把 `build.yaml` 换成 `sync-upstream-build.yml` 之后，偏移更新工作流几乎每次都失败的原因——被调度的文件在默认分支上已经不存在。

`trigger-build` job 现在会在调度前做前置检查：先确认 `build.yaml` 在 `main` 上存在，再确认其中包含 `workflow_dispatch`，任一不满足时直接用 `::error::` 指出具体原因；真正的调度还会重试三次。

## 6. 排查清单

| 现象 | 排查方向 |
|---|---|
| `HTTP 422: Workflow does not have 'workflow_dispatch' trigger` | 确认 `build.yaml` 已在 `main` 上，且包含 `workflow_dispatch` 与 `force_build` / `source_sha` 入参 |
| 中文界面显示英文 | 运行 `python scripts/translate_menu.py --map i18n/zh_map.json --source external-cheat-base/src/features/menu.hpp --report --list-unmapped`，把缺失文案补进 `i18n/zh_map.json` |
| 中文界面出现英文片段 | 检查 `i18n/zh_map.json` 中该条目的 `printf` 说明符是否与英文一致（脚本会拒绝并告警） |
| Release 为空 / tag 未创建 | 检查 `sync` 作业的 `tag` 与 `source_sha` 输出是否为空；`release` 作业在两者为空时会直接失败并给出提示 |
| 构建被跳过 | `sync` 判定 `should_build=false`：上游无变动且未设置 `force_build`。手动运行时勾选 `force_build` |
| `msbuild` 报缺少 `web-radar/dist/index.html` | `unit-tests` 作业没有成功上传 `web-radar-static` 构件，先看该作业的日志 |
