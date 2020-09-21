Qt-标签输入框
===

一个带有标签动画、焦点动画、正确提示、错误警告的单行输入框控件。

## 截图

![截图](picture.gif)

## 使用

1. 把`labedled_edit`文件夹放入Qt工程
2. 这只是一个对`QLineEdit`的包装，通过`setLabelText(QString)`设置标签文字，以及通过`edit()`获取单行编辑框对象，所有修改操作都是针对编辑框了。



## 代码

```C++
#include <LabeledEdit>

// 创建对象
LabeledEdit* le = new LabeledEdit("password", this);

// 显示动画
if (le->edit()->text() == "true")
    le->showCorrect();
else
    le->showWrong();
```



## 注意事项

如果使用错误波浪线`showWrong()`，务必要**注意margin**，不要太小（默认就可以了）。

如果多个输入框一起，可能看起来会比较分散，可以把外部layout的`spacing`设置为0。