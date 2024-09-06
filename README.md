# baidu_loud_storage_project

#### 介绍
百度网盘项目

#### 软件架构
软件架构说明


#### 安装教程

1.  git clone
2.  家目录创建files文件夹
3.  xxxx

#### 自定义协议说明

1.  int command 是命令的枚举类型
2.  int msg_length 是本次发送携带的控制信息长度，请记住服务端要进行拼接
3.  int file_length 是假设本次要发送内容(send_flag == 1)时候，要发送的内容长度
4.  int parameter_num 是本次控制信息的字符数组有多少段字符串(多少段控制信息)
5.  bool send_flag 判断本次是否需要发送文件内容
6.  char control_msg 发送的控制信息，标准格式：路径名，命令，命令所携带的参数

#### 参与贡献


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
