using CommunityToolkit.Mvvm.ComponentModel;//内部包含ObservableObject类
using CommunityToolkit.Mvvm.Input;//内部包含RelayCommand特性
using System;
using System.Collections.Generic;
using System.Text;
using TestC__DLL.C__DLLClass;//包含MangeTcpClientDllExport类,C++纯C导出的函数封装

namespace TestC__DLL.ViewModels
{
    public partial class TestCVM : ObservableObject
    {
        /// <summary>
        /// 创建管理
        /// </summary>
        [RelayCommand]
        private void CreateManager()
        {
            MangeTcpClientDllExport.INVALID_HANDLE = MangeTcpClientDllExport.CreateManager();
        }
        /// <summary>
        /// 控件是否可用
        /// </summary>
        /// <returns>返回true代表可用，false代表不可用</returns>
        private bool CanCreateManager()
        {
            return true;
        }
        /// <summary>
        /// 销毁管理
        /// </summary>
        [RelayCommand]
        private void DestroyManager()
        {
            MangeTcpClientDllExport.DestroyManager(MangeTcpClientDllExport.INVALID_HANDLE);
        }
        /// <summary>
        /// 添加客户端对象
        /// </summary>
        [RelayCommand]
        private void AddClient()
        {
            MangeTcpClientDllExport.CreateClient(MangeTcpClientDllExport.INVALID_HANDLE, _clientName);
        }
        /// <summary>
        /// 客户端名称
        /// </summary>
        [ObservableProperty]
        private string _clientName = "1";

        /// <summary>
        /// 移除客户端对象
        /// </summary>
        [RelayCommand]
        private void RemoveClient()
        {
            MangeTcpClientDllExport.DestroyClient(MangeTcpClientDllExport.INVALID_HANDLE, _clientName);
        }
        /// <summary>
        /// 服务器IP
        /// </summary>
        [ObservableProperty]
        private string _serverIP = "127.0.0.1";
        /// <summary>
        /// 服务器端口
        /// </summary>
        [ObservableProperty]
        private string _serverPort = "8888";
        /// <summary>
        /// 连接服务器
        /// </summary>
        [RelayCommand]
        private void ConnectServer()
        {
            int ret = MangeTcpClientDllExport.ConnectClient(MangeTcpClientDllExport.INVALID_HANDLE, _clientName, _serverIP, Convert.ToInt32(_serverPort));
            if (ret == -1) LogInfo += $"\r\n连接失败";
        }
        /// <summary>
        /// 断连服务器
        /// </summary>
        [RelayCommand]
        private void DisconnectServer()
        {
            MangeTcpClientDllExport.DisconnectClient(MangeTcpClientDllExport.INVALID_HANDLE, _clientName);
        }
        /// <summary>
        /// 数据内容
        /// </summary>
        [ObservableProperty]
        private string _dataPack = "0";
        /// <summary>
        /// 发送数据
        /// </summary>
        [RelayCommand]
        private void SendData()
        {
            byte[] utf8Bytes = Encoding.UTF8.GetBytes(_dataPack);
            Console.WriteLine($"UTF-8: {BitConverter.ToString(utf8Bytes)}");
            MangeTcpClientDllExport.SendData(MangeTcpClientDllExport.INVALID_HANDLE, _clientName, utf8Bytes, utf8Bytes.Length);
            DateTime now = DateTime.Now;
            LogInfo += $"\r\n{now.ToString("yyyy-MM-dd HH:mm:ss")}\t发送的数据:{_dataPack}";//使用_logInfo不会触发通知UI
        }
        /// <summary>
        /// 单次接收
        /// </summary>
        [RelayCommand]
        private void SingleReceiveData()
        {
            byte[] buf = new byte[4096];
            int bytesCount = MangeTcpClientDllExport.RecvData(MangeTcpClientDllExport.INVALID_HANDLE, _clientName, buf, buf.Length);
            DateTime now = DateTime.Now;
            LogInfo += $"\r\n{now.ToString("yyyy-MM-dd HH:mm:ss")}\t接收的数据:{Encoding.UTF8.GetString(buf,0, bytesCount)}";
        }
        /// <summary>
        /// 打印日志
        /// </summary>
        [ObservableProperty]
        private string _logInfo = "日志信息:";
    }
}
