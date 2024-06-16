# [WHU-Spring2024-CompilerProject](https://github.com/numbbbbbplus/WHU-Spring2024-CompilerProject)

## 项目描述

这个项目是武汉大学 2024 年春季编译原理课程的实验项目。它是一个简单的编译器，使用自定义的语言规范解释并执行来自 `test.code` 文件的命令，并使用 `test.input` 文件中的输入值。

## 文件结构

- `main.cpp`: 主代码文件。
- `inputfiles/`
  - `test.code`: 包含需要解释和执行的命令的示例代码文件。
  - `test.input`: 包含输入值的示例输入文件。
  - `CMakeLists.txt`: CMake 构建配置文件。
- `images/`
  - `cmake_result.png`: CMake 编译结果截图。
  - `input_file_location.png`: `test.code` 和 `test.input` 文件位置截图。
  - `VisualStudio_run_result.png`: Visual Studio 运行结果截图。
  - `input_sample.png`: 示例输入文件截图。

## 编译说明

### Windows

使用以下命令编译项目：

```sh
cmake -B build -G "Visual Studio 17 2022"
cmake --build build
```

### Linux

使用以下命令编译项目：

```sh
cmake -B build -G "Unix Makefiles"
cmake --build build
```

### 编译要求
确保你的系统上已安装以下工具：

CMake
Visual Studio 2022 (Windows)
支持 C++17 的编译器

## 运行说明
在编译完成后，请确保 test.code 和 test.input 文件在 build 目录下。然后在 build 目录下运行可执行文件。

![image](https://github.com/numbbbbbplus/WHU-Spring2024-CompilerProject/blob/main/images/input_file_location.png)

## 输入示例
![image](https://github.com/numbbbbbplus/WHU-Spring2024-CompilerProject/blob/main/images/input_sample.png)

## 输出示例
![image](https://github.com/numbbbbbplus/WHU-Spring2024-CompilerProject/blob/main/images/VisualStudio_run_result.png)

## License

[MIT License](LICENSE)
