using System;
using System.Net;
using System.Net.Sockets;

namespace Client1
{
	internal static class TcpClient
	{
		public static void Main()
		{
			try
			{
				var i = 0;
				Console.WriteLine("Waiting for connection");
				var client = new System.Net.Sockets.TcpClient();
				client.Connect("DESKTOP-O75SA5J", 2055);
				Console.WriteLine("Connection established");
				var stream = client.GetStream();
				while (i < 3)
				{
					string message = "client";
					var data = System.Text.Encoding.ASCII.GetBytes(message);
					stream.Write(data, 0, data.Length);
					Console.WriteLine($"Sent: {message} at {DateTime.Now}");
					data = new byte[256];
					var bytes = stream.Read(data, 0, data.Length);
					var responseData = System.Text.Encoding.ASCII.GetString(data, 0, bytes);
					Console.WriteLine("Received: {0}", responseData);
					i++;
				}

				stream.Close();
				client.Close();
			}
			catch (ArgumentNullException e)
			{
				Console.WriteLine("ArgumentNullException: {0}", e);
			}
			catch (SocketException e)
			{
				Console.WriteLine("SocketException: {0}", e);
			}

			Console.WriteLine("\n Press Enter to continue...");
			Console.ReadLine();
		}
	}
}