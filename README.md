# JOS-Log
JOS所有工程文件已上传。
提供一个用户程序testlog用以验证日志文件系统的故障恢复功能。
用法是：
$ testlog s
显示
test start
此时关闭qemu，再重新打开，可见文件系统从日志恢复
此时使用
$ testlog c
显示
test pass
则日志文件系统恢复的文件内容与写入内容相同。
