using System;
using System.Net;
using System.IO;
using System.Text;
using System.Threading;
using System.Diagnostics;

public class Program
{
    static string basePath;
    static int port = 80;
    static Process monitorProcess = null;

    static void RunServer()
    {
        HttpListener server = new HttpListener();  // this is the http server
        server.Prefixes.Add("http://localhost:" + port + "/");
        Console.WriteLine("Starting web server at http://localhost:" + port + "/");
        server.Start();   // and start the server

        while (true)
        {
            try
            {
                HttpListenerContext context = server.GetContext();
                HttpListenerResponse response = context.Response;

                // Don't cache anything, ever. http://stackoverflow.com/questions/49547/making-sure-a-web-page-is-not-cached-across-all-browsers
                response.Headers["Cache-Control"] = "no-cache, no-store, must-revalidate";
                response.Headers["Pragma"] = "no-cache";
                response.Headers["Expires"] = "0";

                var path = Uri.UnescapeDataString(context.Request.Url.LocalPath);
                Console.WriteLine("Handling request: " + path);
                if (path == "/")
                    path = "/index.html";

                if (Path.GetExtension(path) == ".gz")
                {
                    response.AddHeader("Content-Encoding", "gzip");
                }

                if (Path.GetExtension(path) == ".br")
                {
                    response.AddHeader("Content-Encoding", "br");
                }

                if (path.EndsWith(".wasm") || path.EndsWith(".wasm.gz") || path.EndsWith(".wasm.br"))
                {
                    response.ContentType = "application/wasm";
                }

                if (path.EndsWith(".js") || path.EndsWith(".js.gz") || path.EndsWith(".js.br"))
                {
                    response.ContentType = "application/javascript";
                }

                var page = basePath + path;
                string msg = null;
                if (!context.Request.IsLocal)
                {
                    Console.WriteLine("Forbidden.");
                    msg = "<HTML><BODY>403 Forbidden.</BODY></HTML>";
                    response.StatusCode = 403;
                }
                else if (!File.Exists(page))
                {
                    Console.WriteLine("Not found.");
                    msg = "<HTML><BODY>404 Not found.</BODY></HTML>";
                    response.StatusCode = 404;
                }
                else
                {
                    FileStream fileStream = File.Open(page, FileMode.Open);
                    BinaryReader reader = new BinaryReader(fileStream);
                    try
                    {
                        // avoid OutputStream.Write with 0 length while using chunked transfer encoding, as the produced dummy '0\r\n' may be treated as response terminator (case 770266)
                        response.ContentLength64 = fileStream.Length;
                        for (byte[] buffer = reader.ReadBytes(4096); buffer.Length > 0; buffer = reader.ReadBytes(4096))
                            response.OutputStream.Write(buffer, 0, buffer.Length);
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine("Caught Exception sending file: " + e);
                    }
                    reader.Close();
                }

                if (msg != null)
                {
                    byte[] buffer = Encoding.UTF8.GetBytes(msg);
                    response.ContentLength64 = buffer.Length;
                    response.OutputStream.Write(buffer, 0, buffer.Length);
                }
                response.Close();
            }
            catch (Exception e)
            {
                Console.WriteLine("Caught Exception handling request: " + e);
            }
        }
    }

    public static int Main(string[] args)
    {
        if (args.Length < 1 || args.Length > 4)
        {
            Console.WriteLine("usage: SimpleWebServer.exe source_directory [port] [pid]");
            return 1;
        }

        basePath = args[0];
        if (args.Length >= 2)
            port = int.Parse(args[1]);

        if (args.Length >= 3)
        {
            var pid = int.Parse(args[2]);
            monitorProcess = Process.GetProcessById(pid);
        }

        Thread serverThread = new Thread(RunServer);
        serverThread.Start();

        while (true)
        {
            if (monitorProcess != null && monitorProcess.HasExited)
            {
                Console.WriteLine("Associated process has died. Exiting server.");
                Environment.Exit(0);
            }
            Thread.Sleep(100);
        }
    }
}
