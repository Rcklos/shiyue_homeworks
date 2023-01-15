## 作业2

## Git操作

### 基本的五步操作

```bash
# 初始化仓库
git init

# 关联远程仓库
git remote add origin <仓库连接>

# 添加文件缓存
git add README.md

# 提交缓存
git commit -m "初始化仓库"

# 推送提交
git push origin master
```

### 常用的查看指令

```bash
# 查看仓库状态
git status

# 查看Git配置
git config --global --list

# 查看仓库
git remote -l

# 查看分支
git branch -l

# 查看提交日志
git log

# 直观查看提交情况
git log --graph --abbrev-commit --pretty=oneline

# 查看文件修改情况
git show <文件>

# 查看提交的文件修改情况
git diff <文件>
```

### 常用的协作命令

```bash
# 下载远程分支到本地分支
git fetch origin matser:dummy

# 拉取远程分支并尝试合并
git pull origin master

# 开辟新的分支
git chckout -b <新分支名称>

# 切换分支
git switch <需要切换的分支>

# 处理冲突(merge形式)
git merge 目标分支 源分支

# 处理冲突(rebase形式)
git rebase <需要作为基的分支>

# 回滚版本
git reset
```
