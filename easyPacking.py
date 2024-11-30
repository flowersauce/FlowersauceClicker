import os
import shutil
import subprocess
import zipfile

# 配置区
qt_bin_path = r"D:\APP\Work\Qt\6.7.2\mingw_64\bin"  # Qt bin 文件夹路径
mingw_bin_path = r"D:\APP\Work\Qt\6.7.2\mingw_64\bin"  # MinGW bin 文件夹路径
build_dir = r"build\rel"  # 构建目录路径
output_dir = r"output\FlowersauceClicker"  # 打包输出目录路径
exe_name = "FlowersauceClicker.exe"  # 应用程序名称
runtime_dlls = ["libstdc++-6.dll", "libgcc_s_seh-1.dll", "libwinpthread-1.dll"]  # 运行时库


def main():
    # 检查路径是否有效
    exe_path = os.path.join(build_dir, exe_name)
    if not os.path.exists(exe_path):
        print(f"错误: 找不到可执行文件: {exe_path}")
        return

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # 清理输出目录
    for filename in os.listdir(output_dir):
        file_path = os.path.join(output_dir, filename)
        if os.path.isfile(file_path) or os.path.islink(file_path):
            os.unlink(file_path)
        elif os.path.isdir(file_path):
            shutil.rmtree(file_path)

    # 将可执行文件复制到输出目录
    shutil.copy(exe_path, output_dir)
    print(f"已复制可执行文件到输出目录: {output_dir}")

    # 自动复制运行时库文件
    for dll in runtime_dlls:
        dll_path = os.path.join(mingw_bin_path, dll)
        if os.path.exists(dll_path):
            shutil.copy(dll_path, output_dir)
            print(f"已复制运行时库文件: {dll}")
        else:
            print(f"错误: 找不到运行时库文件: {dll_path}")

    # 调用 windeployqt 工具
    windeployqt_path = os.path.join(qt_bin_path, "windeployqt.exe")
    if not os.path.exists(windeployqt_path):
        print(f"错误: 找不到 windeployqt 工具: {windeployqt_path}")
        return

    cmd = [windeployqt_path, os.path.join(output_dir, exe_name)]
    print(f"运行命令: {' '.join(cmd)}")
    subprocess.run(cmd, check=True)
    print("依赖库已部署完成")

    # 压缩 output\FlowersauceClicker 到 output\FlowersauceClicker.zip
    zip_file_path = os.path.join("output", "FlowersauceClicker.zip")
    with zipfile.ZipFile(zip_file_path, "w", zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(output_dir):
            for file in files:
                file_path = os.path.join(root, file)
                arcname = os.path.relpath(file_path, os.path.dirname(output_dir))
                zipf.write(file_path, arcname)

    print("打包完成！")


if __name__ == "__main__":
    main()
