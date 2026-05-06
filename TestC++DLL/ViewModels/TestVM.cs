using System;
using System.Collections.Generic;
using System.Text;

using CommunityToolkit.Mvvm.ComponentModel;//内部包含ObservableObject类
using CommunityToolkit.Mvvm.Input;//内部包含RelayCommand特性

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

        }
        /// <summary>
        /// 断连服务器
        /// </summary>
        [RelayCommand]
        private void DisconnectServer()
        {

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

        }
        /// <summary>
        /// 单次接收
        /// </summary>
        [RelayCommand]
        private void SingleReceiveData()
        {

        }
        /// <summary>
        /// 打印日志
        /// </summary>
        [ObservableProperty]
        private string _logInfo = "日志信息:";
    }
}
