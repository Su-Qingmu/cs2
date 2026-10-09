# Auto-Sync & Daily EXE Build Workflow

本规范定义本 fork(`Su-Qingmu/cs2`)相对于上游 `tiansongyu/cs2_cheat` 的核心增量:**每日自动同步上游、构建 Windows EXE、发布日期形 release**。该流水线实现在 `.github/workflows/sync-upstream-build.yml`,并由本文档给出完整设计约束,便于任何人(或 AI)按规范触发、调试、扩展。

## 1. 工作流身份

| 项 | 值 |
|---|---|
| Workflow 文件 | `.github/workflows/sync-upstream-build.yml` |
| 名称 | `Sync Upstream and Publish Daily EXE` |
| 权限 | `contents: write`(创建/移动 tag + 创建 release) |
| 并发组 | `sync-upstream-${{ github.repository }}`,`cancel-in-progress: false`(同一时间只允许一个实例,但不互相取消) |

## 2. 触发条件

| 触发 | 触发方式 | 是否受 `force_build` 影响 |
|---|---|---|
| `schedule` | 每日 16:00 UTC(北京时间 00:00) | 否 |
| `push` 到 `main` | 任何 push 到 fork 的 main 分支 | 否 |
| `workflow_dispatch` | 手动运行(可指定 `force_build`) | **是** |

### `force_build` 行为

- 类型:boolean,默认 `false`
- 路径:仅在 `workflow_dispatch` 触发时可被设置
- 行为:跳过"上游无变动 → 跳过 build"判定,强制 build + release;若 upstream 同时有新提交,会先 merge 再 build

## 3. 三阶段流水线

```
        ┌──────────────┐    should_build=true     ┌──────────────┐
 触发 → │   sync job   │ ───────────────────────→ │  build job   │ ─┐
        │  (ubuntu)    │                          │ (windows)    │  │
        └──────────────┘                          └──────────────┘  │
                                                                    ↓
        ┌──────────────┐                                         ┌──────────────┐
        │ release job  │ ←────────────────────────────────────── │  zip artifact│
        │  (ubuntu)    │                                         └──────────────┘
        └──────────────┘
```

### 3.1 sync(ubuntu-latest)

**输入**: fork main HEAD + upstream `tiansongyu/cs2_cheat@main`

**步骤**:

1. `actions/checkout@v5` 检出 fork main(`fetch-depth: 0`,保留所有历史)
2. 配置 git author = `github-actions[bot] <41898282+github-actions[bot]@users.noreply.github.com>`
3. 添加 `upstream` remote 指向 `https://github.com/tiansongyu/cs2_cheat.git`
4. `git fetch --no-tags upstream main` + `git fetch origin main`
5. `git checkout -B main origin/main`(用 fork main 重建本地 main)
6. 计算 `upstream_sha = git rev-parse upstream/main`
7. **判定**:
   - 若 `FORCE_BUILD=true`(dispatch + force_build):**不跳过**,继续 merge;若 upstream 也有新提交则先 merge,否则直接在当前 HEAD build
   - 否则若 `git merge-base --is-ancestor upstream/main HEAD` 为真:upstream 已是 fork HEAD 的祖先 → 输出 `should_build=false`,**结束**
   - 否则: `git merge --no-edit --no-ff -m "Sync upstream: ${upstream_sha}" upstream/main` + `git push origin HEAD:main` → 输出 `should_build=true`

**输出**:
- `should_build`: `true` / `false`
- `source_sha`: 同步完成后的 fork HEAD SHA(无论是否 merge,都是 fork main 当时的 HEAD)

### 3.2 build(windows-2022,仅 `should_build=true`)

**步骤**:

1. **Compute release tag** — 计算今天 UTC 日期,转成 `YY.M.D` 格式:
   - `ts = date -u +%Y-%m-%d`
   - `y = ${ts:2:2}`(年,取后两位)
   - `m = date -u +%-m`(月,无前导零)
   - `d = date -u +%-d`(日,无前导零)
   - 输出 `tag=${y}.${m}.${d}`,例 `26.10.8`
2. `actions/checkout@v5` 检出 `ref: ${{ needs.sync.outputs.source_sha }}`
3. `actions/setup-node@v4` 装 Node.js 22,缓存 `web-radar/package-lock.json`
4. `web-radar/`:`npm ci && npm test && npm run build && node scripts/validate-bundle.mjs dist`
5. `microsoft/setup-msbuild@v3`
6. `msbuild external-cheat-base.sln /p:Configuration=Release /p:Platform=x64`
7. `actions/upload-artifact@v4`:`cs2-exe-latest` 目录(原始 x64/Release 内容)
8. `Compress-Archive` 打 zip
9. `actions/upload-artifact@v4`:`latest-release-package`(zip,供 release job 消费)

### 3.3 release(ubuntu-latest,仅 `should_build=true`)

**步骤**:

1. `actions/download-artifact@v4` 把 `latest-release-package` 拉下来到 `release-package/`
2. **Point release tag at synchronized source** — 探测 `refs/tags/<tag>`,已存在则 PATCH,否则 POST 创建。关键:
   ```bash
   ref="repos/${GITHUB_REPOSITORY}/git/ref/tags/${RELEASE_TAG}"
   status="$(gh api "$ref" -i 2>/dev/null | head -n 1 | awk '{print $2}' || true)"
   status="${status:-000}"
   if [[ "$status" == "200" ]]; then
     gh api --method PATCH "$ref" -f sha="${SOURCE_SHA}" -F force=true >/dev/null
   else
     gh api --method POST "repos/${GITHUB_REPOSITORY}/git/refs" \
       -f ref="refs/tags/${RELEASE_TAG}" -f sha="${SOURCE_SHA}" >/dev/null
   fi
   ```
   `|| true` 和默认值 `000` 是为了让 `set -e + pipefail` 在 gh api 4xx 时不中断。
3. **Publish release** — `softprops/action-gh-release@v2`:
   - `tag_name: ${{ needs.build.outputs.tag }}`
   - `name: Build <tag>`
   - `body`: 包含 source SHA + tag
   - `files: release-package/cs2-exe-latest.zip`
   - `overwrite_files: true`(同日再次触发会覆盖)
   - `make_latest: true`(在 UI 上标为 Latest)

## 4. Tag 与 Release 命名

| 触发日期(UTC) | Tag | Release name |
|---|---|---|
| 2026-10-08 | `26.10.8` | `Build 26.10.8` |
| 2026-12-31 | `26.12.31` | `Build 26.12.31` |
| 2027-01-01 | `27.1.1` | `Build 27.1.1` |

- 年份取后两位(`2026` → `26`,`2125` → `25`)
- 月、日不补前导零
- 同日再次触发:`tag` 相同 → 走 PATCH 路径 + `overwrite_files: true` → 同一 release 的 zip 被覆盖
- 跨日触发:新 tag,与历史 release 并存

## 5. 失败模式与处理

| 症状 | 原因 | 处理 |
|---|---|---|
| sync 步骤卡在"Checkout fork main" | 仓库 token 权限不足 | 检查 `permissions.contents: write`;在 fork Settings → Actions → Workflow permissions 设为 "Read and write" |
| `Point release tag` 步骤报 404 | 同上(GITHUB_TOKEN 在 r13 runner 偶发读不到 `git/ref/tags/<tag>`) | 已用 `\|\| true` + 默认值兜底;若仍失败,改用 `gh release create` 直接创建 release(无需预先建 tag) |
| build 步骤 `msbuild` 失败 | 偏移文件与 cs2-dumper 不一致 / `update-files.yml` 跑挂了 | 先看 `Update CS2 Offset Files` 最近一次 run;失败需先修偏移再重跑本工作流 |
| `node scripts/validate-bundle.mjs dist` 失败 | web-radar 构建产物缺关键文件 | 看 `web-radar` 测试日志;本地 `npm run build && node scripts/validate-bundle.mjs dist` 复现 |
| release 已存在但 PATCH 失败 | 罕见,tag 指向不存在的 commit | 删除该 tag 后重跑,或带 `force_build=true` 触发 |
| 调度时间漂移 | GitHub 调度不保证精确到分钟 | 看 [Actions 列表](https://github.com/Su-Qingmu/cs2/actions/workflows/sync-upstream-build.yml) 实际执行时间 |

## 6. 调试与手动复现

### 6.1 一次性跑

[Actions 页面](https://github.com/Su-Qingmu/cs2/actions/workflows/sync-upstream-build.yml) → **Run workflow** → 默认 → 等待约 6–10 分钟。

### 6.2 强制跑(忽略"无变动跳过")

同上 → 勾选 **Run workflow** 面板里的 `force_build = true`。

### 6.3 在 PR 分支上验证

若改了 `.github/workflows/sync-upstream-build.yml`,推到一个分支(如 `test/auto-sync`)并选择 `workflow_dispatch` 从该分支触发 —— sync job 仍会读 fork `main`,但 release job 用的是该分支的 workflow 定义。

### 6.4 本地近似

```bash
# 1) 拉上游
git remote add upstream https://github.com/tiansongyu/cs2_cheat.git
git fetch --no-tags upstream main
git checkout -B main origin/main
if ! git merge-base --is-ancestor upstream/main HEAD; then
  git merge --no-edit --no-ff -m "Sync upstream: $(git rev-parse upstream/main)" upstream/main
fi

# 2) 本地构建
cd web-radar && npm ci && npm test && npm run build && cd ..
msbuild external-cheat-base.sln /p:Configuration=Release /p:Platform=x64
# PowerShell: Compress-Archive -Path x64/Release/* -DestinationPath cs2-exe-latest.zip
```

## 7. 安全约束

- `permissions: contents: write` 是必需的(scope 比 default 的 read 多 `write`),但 workflow 仍只写 fork 自己的 tag 和 release,不写 upstream
- 资产文件是 `cs2-exe-latest.zip`,与 build.yaml 的 `Release-Binaries.zip` 同源;`make_latest: true` 在 GitHub UI 上把当日 release 标为 Latest
- 不引入新 token;完全使用 GitHub 默认 `GITHUB_TOKEN`,无第三方 secrets
- `web-radar/dist/maps/NOTICE.txt` 和 `SOURCE.json` 必须在每次发布包里(MSBuild post-build + validate-bundle 强制保证)

## 8. 与上游 / 其他 workflow 的关系

| Workflow | 触发 | 角色 | 是否保留 |
|---|---|---|---|
| `sync-upstream-build.yml` | schedule + push + dispatch | **核心**:抓上游 + build + release | **保留**(本文档) |
| `update-files.yml` | schedule(每小时) | 从 cs2-dumper 拉新偏移;若变更触发 build | **保留**(本仓库需要) |
| `build.yaml`(上游原版) | push + dispatch | 上游的 CI:跑测试 + 打 v2.0.x tag + 发 release | **可选**:本 fork 的 main 上其存在仅为兼容上游,实际工作被 `sync-upstream-build.yml` 覆盖;若要清理可考虑删除 |
| `build-exe.yml` | push tag `v*` + dispatch | 打 v* tag 的 release(本地手动 `v1.0.x` 用) | **可选**:与新的日期形 tag 命名重复;若不再用 `v*` tag 触发 release,可考虑删除 |

> 决策记录:本文档定稿时(2026-10-08),fork 仍保留 `build.yaml` 和 `build-exe.yml` 以便对照;若后续确认只用日期形 tag,可在单独 PR 中删除并把任何外部引用迁移过来。
