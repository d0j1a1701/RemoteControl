# RemoteControl / xShell

一个简单的程序让你可以在其他电脑上远程运行命令提示符(cmd.exe)。

## 使用说明

1. 在被控电脑上运行`Server.exe`。
2. 在控制电脑上运行`Client.exe`并输入被控端`IP`。
3. 自动连接。

## 编译选项

- 无论是`Server`还是`Client`都需要添加编译选项`-lwsock32`(链接Windows Socket库)。
- 如果想静默运行`Server`端可以在编译选项中添加`-mwindows`，这样可以隐藏控制台。