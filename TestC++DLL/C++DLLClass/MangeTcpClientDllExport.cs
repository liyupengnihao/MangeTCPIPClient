using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace TestC__DLL.C__DLLClass
{
    //纯C导出是全局的
    public static class MangeTcpClientDllExport
    {
        // DLL 名称（发布时放在 exe 同目录）
        private const string DllPath = "ManageTCPIPClient.dll";

        /// <summary>
        /// 句柄 = IntPtr
        /// </summary>
        public static IntPtr INVALID_HANDLE = IntPtr.Zero;//C++中的void*任意指针(内存地址，C/C++指针,句柄)

        /// <summary>
        /// 创建管理器
        /// </summary>
        /// <returns></returns>
        [DllImport(DllPath, CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr CreateManager();

        /// <summary>
        /// 销毁管理器
        /// </summary>
        /// <param name="manager"></param>
        [DllImport(DllPath, CallingConvention = CallingConvention.StdCall)]
        public static extern void DestroyManager(IntPtr manager);

        /// <summary>
        /// 添加并创建客户端对象
        /// </summary>
        /// <param name="manager"></param>
        /// <param name="clientId"></param>
        /// <returns></returns>
        [DllImport(DllPath, CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern int CreateClient(IntPtr manager, string clientId);

        /// <summary>
        /// 移除并销毁客户端对象
        /// </summary>
        /// <param name="manager"></param>
        /// <param name="clientId"></param>
        /// <returns></returns>
        [DllImport(DllPath, CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern int DestroyClient(IntPtr manager, string clientId);

        /// <summary>
        /// 设置超时
        /// </summary>
        /// <param name="manager"></param>
        /// <param name="clientId"></param>
        /// <param name="timeout"></param>
        /// <returns></returns>
        [DllImport(DllPath, CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern int SetReceiveTimeout(IntPtr manager, string clientId, int timeout);

        /// <summary>
        /// 连接
        /// </summary>
        /// <param name="manager"></param>
        /// <param name="clientId"></param>
        /// <param name="ip"></param>
        /// <param name="port"></param>
        /// <returns></returns>
        [DllImport(DllPath, CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern int ConnectClient(IntPtr manager, string clientId, string ip, int port);

        /// <summary>
        /// 是否连接
        /// </summary>
        /// <param name="manager"></param>
        /// <param name="clientId"></param>
        /// <returns></returns>
        [DllImport(DllPath, CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern int IsClientConnected(IntPtr manager, string clientId);

        /// <summary>
        /// 断开
        /// </summary>
        /// <param name="manager"></param>
        /// <param name="clientId"></param>
        [DllImport(DllPath, CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern void DisconnectClient(IntPtr manager, string clientId);

        /// <summary>
        /// 发送数据
        /// </summary>
        /// <param name="manager"></param>
        /// <param name="clientId"></param>
        /// <param name="data"></param>
        /// <param name="len"></param>
        /// <returns></returns>
        [DllImport(DllPath, CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]//CharSet影响string
        public static extern int SendData(IntPtr manager, string clientId, byte[] data, int len);

        /// <summary>
        /// 接收数据
        /// </summary>
        /// <param name="manager"></param>
        /// <param name="clientId"></param>
        /// <param name="buf"></param>
        /// <param name="bufLen"></param>
        /// <returns></returns>
        [DllImport(DllPath, CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern int RecvData(IntPtr manager, string clientId, byte[] buf, int bufLen);

        /// <summary>
        /// 回调 TCP_RECV_CALLBACK  TCP_RECV_CALLBACK
        /// </summary>
        /// <param name="data"></param>
        /// <param name="len"></param>
        /// <param name="userParam"></param>
        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public delegate void TCP_RECV_CALLBACK(IntPtr data, int len, IntPtr userParam);
    }
}
