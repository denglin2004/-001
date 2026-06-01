# Git & GitHub 使用指南（嵌入式开发实用版)

---

## 一、核心概念速查

### 1.1 Git 是什么？

Git 是一个**本地版本管理工具**，它记录你每次修改了什么文件、改了什么内容。
即使没有网络，Git 也能在你电脑上完整工作。

### 1.2 GitHub 是什么？

GitHub 是一个**云端代码托管平台**，把你本地 Git 仓库的内容同步到网上，
方便多人协作、代码备份、版本发布。

### 1.3 关键概念

```
┌─────────────────────────────────────────────────────────────┐
│                        你的电脑（本地）                        │
│                                                             │
│   工作区(Working)    暂存区(Stage)    本地仓库(Repository)    │
│   ┌──────────┐      ┌──────────┐      ┌──────────────┐     │
│   │ 你正在    │ git  │ 准备提交  │ git  │ 已提交的版本   │     │
│   │ 编辑的文件 │ add  │ 的文件   │commit│ 历史记录      │     │
│   └──────────┘  ──► └──────────┘  ──► └──────────────┘     │
│       .c .h         暂存区快照         commit1→commit2→...   │
│                                                             │
└─────────────────────────────────┬───────────────────────────┘
                                  │ git push (上传)
                                  ▼
                    ┌──────────────────────────┐
                    │     GitHub（云端）         │
                    │                          │
                    │   origin/master           │
                    │   origin/main             │
                    │   origin/feature          │
                    │                          │
                    └──────────────────────────┘
                                  │ git pull (下载)
                                  ▼
                        其他协作者的电脑
```

| 术语 | 含义 | 类比 |
|------|------|------|
| **仓库(Repository)** | 项目的完整历史记录 | 项目的"档案室" |
| **提交(Commit)** | 一次保存的快照 | 拍一张照片存档 |
| **分支(Branch)** | 独立的开发线路 | 一条平行的时间线 |
| **主分支(main/master)** | 最终发布版本所在的分支 | 正式版 |
| **远程(Remote)** | 云端仓库（通常是GitHub） | 网盘 |
| **origin** | 你关联的远程仓库的名字 | "我的网盘" |
| **push** | 本地 → 云端 | 上传 |
| **pull** | 云端 → 本地 | 下载 |
| **merge** | 把两个分支合并到一起 | 拼接两条时间线 |
| **冲突(Conflict)** | 两个分支改了同一处代码 | 需要人工决定保留哪个 |

---

## 二、你刚才遇到的问题复盘

### 问题1：push 成功了，但 GitHub 上看不到变化

```
原因：你的代码在 master 分支，但 GitHub 默认显示 main 分支

你的仓库有 3 个分支：
  main     ← GitHub 默认打开这个（旧代码）
  master   ← 你一直在这里开发（最新代码）
  feature  ← 学习用的，跟项目无关

解决：看 GitHub 页面左上角的分支下拉框，切换到 master 就能看到最新代码
永久解决：Settings → Default branch → 改为 master
```

### 问题2：main 和 master 有什么区别？

```
Git 早期默认主分支叫 master，后来 GitHub 改为叫 main。
两者功能完全一样，只是名字不同。

你的仓库两个都有，导致分散了：
  - main 创建得早，后来没更新
  - master 是你实际开发用的

建议：只保留一个主分支（master），main 可以删掉或忽略
```

### 问题3：merge 时出现冲突

```
原因：main 和 master 都修改了 Readme.txt 的同一行（项目负责人名字）
Git 不知道保留哪个，需要你手动选择。

解决步骤：
  1. 打开冲突文件，找到 <<<<<<< HEAD 和 >>>>>>> master 之间的内容
  2. 删掉冲突标记，保留你想要的内容
  3. git add <文件> → git commit

冲突标记长这样：
  <<<<<<< HEAD          ← main 分支的内容
  项目负责人：邓林
  =======
  项目负责人：杨涛       ← master 分支的内容
  >>>>>>> master

你想要哪个就保留哪个，删掉其他标记即可。
```

---

## 三、日常开发最常用命令

### 3.1 每次开发的基本流程

```bash
# 第1步：改代码（在 MounRiver Studio 里正常开发）

# 第2步：查看改了哪些文件
git status

# 第3步：查看具体改了什么
git diff

# 第4步：把修改的文件加入暂存区
git add APP/core_control.c APP/core_control.h    # 指定文件
# 或者
git add APP/                                       # 整个目录

# 第5步：提交到本地仓库
git commit -m "简短描述你改了什么"

# 第6步：推送到 GitHub
git push origin master
```

### 3.2 查看历史

```bash
# 查看提交记录（简略）
git log --oneline

# 查看最近 5 次提交
git log --oneline -5

# 查看某次提交改了什么
git show <提交ID>

# 查看两个分支的差异
git diff master..main
```

### 3.3 分支操作

```bash
# 查看所有分支（* 号表示当前分支）
git branch -a

# 创建新分支
git branch feature-new-sensor

# 切换到新分支
git checkout feature-new-sensor

# 创建并切换（上面两条命令的简写）
git checkout -b feature-new-sensor

# 切回 master
git checkout master

# 合并某分支到当前分支
git merge feature-new-sensor

# 删除本地分支
git branch -d feature-new-sensor

# 删除远程分支
git push origin --delete feature-new-sensor
```

### 3.4 同步远程

```bash
# 查看远程仓库信息
git remote -v

# 从远程拉取最新代码（不自动合并）
git fetch origin

# 从远程拉取并合并到当前分支
git pull origin master

# 推送本地分支到远程
git push origin master
```

### 3.5 撤销操作

```bash
# 撤销某个文件的修改（未 add 之前）
git restore APP/core_control.c

# 从暂存区移除（已 add 但未 commit）
git restore --staged APP/core_control.c

# 修改最近一次 commit 的说明
git commit --amend -m "新的提交说明"

# 注意：以下命令会丢失数据，谨慎使用！
# git reset --hard HEAD    ← 丢弃所有未提交的修改
# git push --force          ← 覆盖远程，可能影响队友
```

---

## 四、分支管理策略（适合你的项目)

### 4.1 推荐的分支模型

```
master     ──●──●──●──●──●──●──●──  (稳定版本，随时可以编译运行)
              \     /     \     /
feature-xxx   ●──●──       ●──●──   (新功能开发分支)
              │                  │
              │  开发完成 → merge │
              └──────────────────┘

工作流程：
1. 从 master 创建功能分支：git checkout -b feature-xxx
2. 在功能分支上开发、提交
3. 开发完成后合并回 master：git checkout master && git merge feature-xxx
4. 推送到 GitHub：git push origin master
```

### 4.2 你的项目实际建议

```
你目前是单人开发，最简单的策略：

master      ← 只有这个分支就够了
  │
  ├── 日常开发直接在 master 上提交
  ├── 每次提交前确保代码能编译通过
  └── 定期 push 到 GitHub 做备份

不需要 main 和 feature 分支。
如果 main 确实没用，可以在 GitHub 上删除：
  Settings → Branches → Default branch 改为 master → 删除 main
```

---

## 五、提交规范建议

### 5.1 commit message 写法

```
格式：<类型>: <简短描述>

类型：
  feat     新功能
  fix      修复bug
  refactor 重构（不改变功能）
  docs     文档更新
  style    代码格式调整
  test     测试相关
  chore    构建/工具相关

好的例子：
  feat: 添加ESP32云端S路径任务触发支持
  fix: 修复CAN接收数据覆盖问题
  refactor: 统一S任务参数读取来源
  docs: 更新CAN通信矩阵说明

不好的例子：
  update          ← 太模糊，不知道改了什么
  修改代码        ← 没说改了什么
  asdfgh          ← 乱写的
```

### 5.2 什么时候该提交？

```
✅ 该提交的时候：
  - 完成一个功能点
  - 修复了一个bug
  - 代码能正常编译
  - 准备尝试一个可能出问题的改动（先存档）

❌ 不该提交的时候：
  - 代码编译报错
  - 只改了一半
  - 调试用的临时代码没删
```

---

## 六、常用场景速查表

| 场景 | 命令 |
|------|------|
| 我改了代码，想保存 | `git add . && git commit -m "描述"` |
| 我想把代码传到GitHub | `git push origin master` |
| 我想看看改了什么 | `git status` + `git diff` |
| 我想看提交历史 | `git log --oneline -10` |
| 我想撤销某个文件的修改 | `git restore <文件>` |
| 我想创建新功能分支 | `git checkout -b feature-xxx` |
| 我想合并分支 | `git checkout master && git merge feature-xxx` |
| 我想从GitHub拉取最新代码 | `git pull origin master` |
| 我想看远程有哪些分支 | `git branch -a` |
| 我想知道当前在哪个分支 | `git branch` (带*号的是当前) |
| 我想删除远程分支 | `git push origin --delete 分支名` |

---

## 七、你当前仓库的状态总结

```
远程仓库：https://github.com/denglin2004/-001.git

分支状况：
  master   ← 你一直在用，最新提交 89a4271（S路径多源触发）
  main     ← 刚合并了master，内容已同步（755326b）
  feature  ← 学习分支，跟项目无关

建议操作：
  1. 以后只在 master 上开发
  2. GitHub Settings → Default branch 改为 master
  3. main 分支可以删除或忽略
  4. feature 分支如果没用也可以删除
```

---

## 八、遇到问题怎么办？

```bash
# 1. 先看状态
git status

# 2. 看日志
git log --oneline -5

# 3. 看远程状态
git remote -v
git branch -a

# 4. 如果合并有冲突，解决后：
git add <冲突文件>
git commit

# 5. 如果想放弃合并回到合并前：
git merge --abort

# 6. 如果想放弃所有本地修改回到上次提交：
git restore .          ← 恢复所有文件
```

记住两个最重要的原则：
1. **commit 前先 `git status` 看清楚**
2. **不确定的操作先 `git branch` 确认自己在哪个分支**
