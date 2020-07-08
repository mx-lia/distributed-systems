using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace Server
{
    internal static class TcpServer
    {
        private static TcpListener _listener;
        private const int Limit = 5;
        private static readonly ReaderWriterLockSlim Locker = new ReaderWriterLockSlim();

        public static void Main()
        {
            var localAddress = IPAddress.Parse("192.168.1.106");
            _listener = new TcpListener(localAddress, 2055);
            _listener.Start();
            Console.Write("Waiting for a connection... ");
            for (var i = 0; i < Limit; i++)
            {
                var t = new Thread(Service);
                t.Start();
            }
        }

        private static void Service()
        {
            try
            {
                var bytes = new byte[256];
                while (true)
                {
                    var client = _listener.AcceptTcpClient();
                    Console.WriteLine("Client connected!");
                    var stream = client.GetStream();
                    int i;
                    while ((i = stream.Read(bytes, 0, bytes.Length)) != 0)
                    {
                        var data = Encoding.ASCII.GetString(bytes, 0, i);
                        Console.WriteLine("Received: {0}", data);
                        WriteToFile(data);
                        Thread.Sleep(1000);
                        data = "Data wrote.";
                        var msg = Encoding.ASCII.GetBytes(data);
                        stream.Write(msg, 0, msg.Length);
                        Console.WriteLine("Sent: {0}", data);
                    }
                    client.Close();
                }
            }
            catch (SocketException e)
            {
                Console.WriteLine("SocketException: {0}", e);
            }
        }

        private static void WriteToFile(string message)
        {
            message += "\n";
            Locker.EnterWriteLock();
            using (var writer = new FileStream(@"D:\Study\DistributedSystems\Lab2\Server\TextFile.txt", FileMode.Append))
            {
                var data = new UTF8Encoding(true).GetBytes(message);
                writer.Write(data, 0, message.Length);
            }
            Locker.ExitWriteLock();
        }
    }
}