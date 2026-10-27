# 简单文本编辑器（实验二）

基于 Qt 6 的多窗口文本编辑器，东莞理工学院《Qt 应用程序开发》实验二。
学号：2024414290114　姓名：赖柳燕

## 功能
- 混合式 UI：主窗口布局用 UI 设计器完成，菜单/工具栏 Action 由代码按功能分组创建
- 文件操作：新建 / 打开 / 保存 / 另存为（QFile + QTextStream），标题栏根据修改状态显示 [*]
- 编辑操作：撤销 / 重做 / 剪切 / 复制 / 粘贴 / 全选，Action 使能随编辑状态自动变化
- 查找 / 替换：FindDialog 双模式，支持循环查找、未找到提示、替换与替换全部
- 格式：字体大小、字体颜色、背景色、自动换行开关
- 视图：显示/隐藏工具栏、状态栏；状态栏实时显示"行 x，列 y　字符数 z"
- 行号：QPlainTextEdit 提升为自定义 CodeEditor，左侧行号区 + 当前行高亮，可开关
- 边界处理：未保存关闭弹提示；打开失败弹警告

## 构建
用 Qt Creator 打开 `TextEditor.pro`（Qt 6.x + MinGW），或：

```bash
qmake TextEditor.pro
mingw32-make release
```

## 代码结构
| 文件 | 说明 |
| --- | --- |
| main.cpp | 程序入口 |
| mainwindow.h/.cpp/.ui | 主窗口：菜单、工具栏、全部业务逻辑 |
| codeeditor.h/.cpp | 继承 QPlainTextEdit 的行号编辑器（promote 使用） |
| finddialog.h/.cpp | 查找 / 替换对话框 |
