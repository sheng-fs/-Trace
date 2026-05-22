# macOS ICNS 生成说明

这个 `trace.iconset` 文件夹包含了生成 macOS .icns 图标所需的所有尺寸。

## 在 macOS 上转换为 .icns

1. 将整个 `trace.iconset` 文件夹复制到 macOS 电脑上
2. 在终端中运行：
   ```bash
   iconutil -c icns trace.iconset
   ```
3. 这将生成 `trace.icns` 文件

## 或者使用 Python (macOS)

```python
import subprocess
subprocess.run(["iconutil", "-c", "icns", "trace.iconset"])
```

## 包含的尺寸

- icon_16x16.png
- icon_16x16@2x.png
- icon_32x32.png
- icon_32x32@2x.png
- icon_128x128.png
- icon_128x128@2x.png
- icon_256x256.png
- icon_256x256@2x.png
- icon_512x512.png
- icon_512x512@2x.png
