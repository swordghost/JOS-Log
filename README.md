# JOS-Log
JOS所有工程文件已上传。\n
提供一个用户程序testlog用以验证日志文件系统的故障恢复功能。\n
用法是：\n
$ testlog s\n
显示\n
test start\n
此时关闭qemu，再重新打开，可见文件系统从日志恢复\n
此时使用\n
$ testlog c\n
显示\n
test pass\n
则日志文件系统恢复的文件内容与写入内容相同。\n
