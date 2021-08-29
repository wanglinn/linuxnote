# 1 vscode remote sshd

# 2 vscode remote sshd 免密码登录

## 2.1 在windows 下生成密钥
利用cmd 生成密钥
ssh-keygen -t rsa -C "xxx"   其中-t代表密钥类型为rsa类型， -C 为注释
生成的公钥和私钥保存在 c:/user/admin/.ssh/ 下
id_rsa为私钥
id_rsa.pub为公钥

## 2.2 在远端配置authorized_keys
在要连接的用户下的~/.ssh 文件夹下 编辑文件 authorized_keys， 添加 id_rsa.pub 中的内容。

## 2.3 在windows下配置config
在windows下 编辑 C:\Users\admin.ssh\config ， 添加内容 IdentityFile "C:\Users\admin\.ssh\id_rsa"  然后保存即可。
