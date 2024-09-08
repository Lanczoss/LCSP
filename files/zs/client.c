er_path[1024] = {0};
        strncpy(user_path, t.control_msg, t.path_length);

        //打印输入框
        printf("Cloud %s> ", user_path);
        fflush(stdout);

        //存储标准输入的缓冲区
        char stdin_buf[1024] = {0};
        ssize_t rret = read(STDIN_FILENO, stdin_buf, 1024);
        ERROR_CHECK(rret, -1, "read stdin");

        //将路径名 命令 参数以buf送进splitCommand
        char buf[2048] = {0};
        strcpy(buf, user_path);
        strcat(buf, " ");
        strcat(buf, stdin_buf);

        //拆分、分析字符串
        //将键盘输入的命令拆分成
        //路径名
        //命令序号
        //控制字符数组（命令参数）
        ret = splitCommand(&t, buf);
        ERROR_CHECK(ret, -1, "splitCommand");
        //这里出来的自定义协议有基本的控制信息
        //处理接收的消息
        ret = analysisProtocol(&t, socket_fd);
        if(ret == -1)
        {
            close(socket_fd);
            exit(0);
        }
    }
    close(socket_fd);
}
