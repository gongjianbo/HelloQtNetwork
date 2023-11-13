网络控制协议
控制信息通过UTF8编码的JSON传递，图像数据通过二进制传递
帧结构：帧头4+帧长4+JSON数据长度4+JSON内容N+二进制数据长度4+二进制数据内容N+校验4+帧尾4
4Byte head：0xFF 0xFF 0x00 0x00 
4Byte frame length：帧长，uint32小端，含头尾等，即整个帧的字节长度
4Byte control length：control 数据的字节长度，uint32小端
NByte control：控制信息采用UTF8编码的Json字符串
4Byte data length：data 数据的字节长度，uint32小端，命令消息固定0
NByte data：放图像等二进制数据
4Byte check：校验位，暂未使用全0
4Byte tail：0x00 0x00 0xFF 0xFF
