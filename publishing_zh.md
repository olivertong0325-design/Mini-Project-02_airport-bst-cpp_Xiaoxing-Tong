# 手动发布到 GitHub：网页上传步骤

仓库由你自己创建并发布。以下步骤使用浏览器即可完成，无需安装 Git。

## 1. 下载并解压

解压交付的 `Mini_Project_02_Xiaoxing_Tong.zip`。打开解压后的 `airport-bst-cpp` 文件夹，
应能直接看到 `README.md`、`src`、`data`、`docs` 等内容。

## 2. 创建公开仓库

登录你的 GitHub 账号，打开：

https://github.com/new?name=airport-bst-cpp&visibility=public

确认以下设置：

- **Owner**：你的账号。
- **Repository name**：`airport-bst-cpp`。
- **Description**，可填写：`Airport lookup service using a binary search tree in C++17`。
- **Visibility**：`Public`。
- 保持空仓库：不要让 GitHub 自动生成 README、`.gitignore` 或 License 文件，
  本项目已经附有 README、`.gitignore` 和数据许可证说明。

点击 **Create repository**。

## 3. 打开文件上传页

在新建的空仓库页面，点击 **uploading an existing file**。
如果已经显示文件列表，则使用 **Add file → Upload files**。

## 4. 上传项目内容

从解压后的项目文件夹中，选择并拖入以下文件和文件夹：

```text
src/
data/
docs/
tests/
scripts/
README.md
DATA_LICENSE.md
CMakeLists.txt
.gitignore
```

**选择文件夹里面的这些内容直接上传**，使 `README.md`、`src/`、`data/` 等位于
仓库根目录。这样访问仓库首页就能直接看到项目介绍。

`bin/` 是方便你在 Windows 本机运行的已编译程序，可以保留在电脑上，不必上传。
压缩包本身也不作为源码上传。网页上传不会应用 `.gitignore` 的过滤规则，因此
请手动排除 `bin/`。本交付文件的数量和大小都适合网页直接上传。

## 5. 提交上传

等待文件上传完成，填写提交说明，例如：

```text
Implement airport BST lookup service and experiments
```

按页面提示完成提交。对新建的个人空仓库，直接把初始文件提交到默认分支即可。
如果页面引导你创建分支和 Pull Request，则在完成后合并，让文件出现在默认分支。

## 6. 检查发布结果

确认仓库首页显示 **Public**，并且：

- 首页能够直接显示英文 README。
- `src/` 中包含 `.cpp` 和 `.h` 源文件。
- `data/` 中包含提供的两个原始数据文件。
- `docs/pseudocode.md` 能正常打开。
- `docs/demo.gif` 能播放，或点击原始文件后能播放。
- README 中的 Task 6 实测表格、测试说明、OpenFlights.org 数据署名都能看到。

提交给老师的链接为：

```text
https://github.com/你的GitHub用户名/airport-bst-cpp
```

这是仓库首页地址，不是 `settings` 设置页面或单个文件的地址。

## 已建立仓库时

如果你已经创建了同名空仓库，直接从第 3 步开始。如果仓库已经有一份 GitHub
自动生成的 README，可在上传时用交付的完整 `README.md` 替换该文件。

## 官方操作说明

- 创建仓库：https://docs.github.com/en/repositories/creating-and-managing-repositories/creating-a-new-repository
- 上传文件与文件夹：https://docs.github.com/en/repositories/working-with-files/managing-files/adding-a-file-to-a-repository

交付项目中的英文 `publishing.md` 另附 Git 命令行方式，可按需要选用。
