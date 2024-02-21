# 此shell是很简易的shell，支持mkdir、cd、over、>、<、>>、&>、2>、2>>、&>>命令
'mkdir' 创建目录 
'cd'    打开目录
'over'  退出shell
'>'     标准输出重定向 'command > filename'
'<'     标准输入重定向 'command < filename'
'>>'    标准输出追加重定向 'command >> filename'
'&>'    同时将标准输出和标准错误输出重定向 'command &> filename'
'2>'    标准错误输出重定向 'command 2> filename'
'2>>'   标准错误输出追加重定向 'command 2>> filename'
'&>>'   同时将标准输出和标准错误输出追加重定向 'command &>> filename'

# 使用
进入项目目录后，直接`docker build -t hshshell .`
然后`docker run -it hshshell`
进入容器后输入`./hshshell`运行shell即可
