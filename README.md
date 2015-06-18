【功能说明】
本Project属于JOS的一个拓展，在JOS lab 5的基础上增加了日志系统，提供简单的日志记录、管理、恢复功能。

【代码说明】
日志系统功能主体位于fs/log.c，为支持日志功能，JOS-Log的其他部分也做了相应改动，比如inc/log.h和fs/fs.h。user/testlog.c是一个测试程序，用于验证日志文件系统的恢复功能。

【演示方式】
demo方式：
JOS-Log拥有全套的JOS文件，在安装有QEMU的Linux环境下可于终端中所在文件夹下运行命令make qemu启动。
在demo前，需将fs/log.c中定义的宏debug的值改为1，以使日志工作时显式地表现出其操作。
修改文件完毕之后，在JOS-Log文件夹下make qemu。JOS系统启动后自动进入shell。此时随意进行文件读写操作，界面中即会出现JSON格式的LOG记录，其类型如下：

	log-record（需要debug值在2及以上）：记录日志。
	log-clear：清除日志，在内容实际写入时出现。
	
利用testlog可以验证从日志恢复功能。
在shell中输入

	$ testlog s
	
此时屏幕上显示test start，然后程序运行到断点进入内核，异常栈显示中断类型为break point。此时关闭JOS（关掉QEMU模拟器界面即可），以使文件内容不会真正写入，只记录在日志块里。
再次make qemu，此时可发现在进入shell前，JOS进行了从日志的恢复工作，显示Log记录log-record。进入shell后，文件内容恢复完成。在shell中输入：

	$ testlog c
	
显示test pass，说明日志文件系统恢复的文件内容和写入内容相同，从日志恢复功能正常。

【成员分工】
见PPT的最后两页

【Github地址】
本Project1的github地址为https://github.com/swordghost/JOS-Log
