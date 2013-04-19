﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.IO;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Threading;

using Ragnarok;
using Ragnarok.ObjectModel;
using Ragnarok.Presentation;

namespace Bonako
{
    /// <summary>
    /// ボナンザが止まった理由です。
    /// </summary>
    public enum AbortReason
    {
        Aborted,
        Error,
    }

    /// <summary>
    /// ボナンザ停止時に使われるイベント引数です。
    /// </summary>
    public class BonanzaAbortedEventArgs : EventArgs
    {
        /// <summary>
        /// ボナンザが停止した理由を取得または設定します。
        /// </summary>
        public AbortReason Reason
        {
            get;
            set;
        }

        /// <summary>
        /// コンストラクタ
        /// </summary>
        public BonanzaAbortedEventArgs(AbortReason reason)
        {
            Reason = reason;
        }
    }

    /// <summary>
    /// コマンド受信時に使われるイベント引数です。
    /// </summary>
    public class BonanzaReceivedCommandEventArgs : EventArgs
    {
        /// <summary>
        /// 受信したコマンドを取得または設定します。
        /// </summary>
        public string Command
        {
            get;
            set;
        }

        /// <summary>
        /// コンストラクタ
        /// </summary>
        public BonanzaReceivedCommandEventArgs(string command)
        {
            Command = command;
        }
    }

    /// <summary>
    /// ボナンザ用の例外クラスです。
    /// </summary>
    public class BonanzaException : Exception
    {
        /// <summary>
        /// コンストラクタ
        /// </summary>
        public BonanzaException()
        {
        }

        /// <summary>
        /// コンストラクタ
        /// </summary>
        public BonanzaException(string message)
            : base(message)
        {
        }
    }

    /// <summary>
    /// ボナンザの操作用オブジェクトです。
    /// </summary>
    public class Bonanza : NotifyObject, IDisposable
    {
        /// <summary>
        /// ボナンザの基本使用メモリ(MB)
        /// </summary>
        public const int MemUsedBase = 230;
        /// <summary>
        /// ハッシュサイズの最小値です。(24MB)
        /// </summary>
        public const int HashSizeMinimum = 24;

        private readonly LinkedList<string> writeCommands =
            new LinkedList<string>();
        private volatile bool aborted;
        private volatile bool initialized;
        private Process bonaProcess;
        private Thread readThread;
        private Thread errorReadThread;
        private Thread writeThread;
        private bool disposed = false;

        /// <summary>
        /// コマンド送信時に呼ばれるイベントです。
        /// </summary>
        public event EventHandler<BonanzaReceivedCommandEventArgs> CommandSent;

        /// <summary>
        /// コマンド受信時に呼ばれるイベントです。
        /// </summary>
        public event EventHandler<BonanzaReceivedCommandEventArgs> CommandReceived;

        /// <summary>
        /// エラー受信時に呼ばれるイベントです。
        /// </summary>
        public event EventHandler<BonanzaReceivedCommandEventArgs> ErrorReceived;

        /// <summary>
        /// 初期化が終わった時に呼ばれます。
        /// </summary>
        public event EventHandler MnjInited;

        /// <summary>
        /// ボナンザが終了したときに呼ばれます。
        /// </summary>
        public event EventHandler<BonanzaAbortedEventArgs> Aborted;

        /// <summary>
        /// コンストラクタ
        /// </summary>
        public Bonanza()
        {
            CommandReceived += Bonanza_ReceivedCommand;
            ErrorReceived += Bonanza_ReceivedError;
        }

        /// <summary>
        /// デストラクタ
        /// </summary>
        ~Bonanza()
        {
            Dispose(false);
        }

        /// <summary>
        /// オブジェクトを破棄します。
        /// </summary>
        public void Dispose()
        {
            GC.SuppressFinalize(this);
            Dispose(true);
        }

        /// <summary>
        /// オブジェクトを破棄します。
        /// </summary>
        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                if (disposing)
                {
                    Abort(0);
                }

                this.disposed = true;
            }
        }

        /// <summary>
        /// プロパティの変更を通知します。
        /// </summary>
        public override void NotifyPropertyChanged(PropertyChangedEventArgs e)
        {
            base.NotifyPropertyChanged(e);

            WPFUtil.InvalidateCommand();
        }

        /// <summary>
        /// 並列化サーバーに接続したかどうかを取得または設定します。
        /// </summary>
        public bool IsConnected
        {
            get { return GetValue<bool>("IsConnected"); }
            set { SetValue("IsConnected", value); }
        }

        /// <summary>
        /// mnjinitコマンドが完了したかどうかを取得または設定します。
        /// </summary>
        public bool? IsMnjInited
        {
            get { return GetValue<bool?>("IsMnjInited"); }
            set { SetValue("IsMnjInited", value); }
        }

        /// <summary>
        /// ボナンザを停止します。
        /// </summary>
        public void Abort(AbortReason reason, int millis = 500)
        {
            using (LazyLock())
            {
                this.aborted = true;

                // 初期化されていなければ、そのまま帰ります。
                if (!this.initialized)
                {
                    return;
                }

                WriteCommand("quit");

                // ボナンザプロセスを終了します。
                var process = this.bonaProcess;
                if (process != null)
                {
                    if (!process.WaitForExit(millis))
                    {
                        process.Kill();
                    }

                    this.bonaProcess = null;
                }

                if (this.writeThread != null)
                {
                    this.writeThread.Join(0);
                    this.writeThread = null;
                }

                if (this.readThread != null)
                {
                    this.readThread.Join(0);
                    this.readThread = null;
                }

                if (this.errorReadThread != null)
                {
                    this.errorReadThread.Join(0);
                    this.errorReadThread = null;
                }

                this.initialized = false;
                IsConnected = false;
            }

            Aborted.SafeRaiseEvent(this, new BonanzaAbortedEventArgs(reason));
        }
        
        /// <summary>
        /// ボナンザを起動します。
        /// </summary>
        public void Initialize()
        {
            try
            {
                using (LazyLock())
                {
                    if (this.initialized)
                    {
                        throw new InvalidOperationException(
                            "ボナンザはすでに既に初期化されています。");
                    }
                    if (this.aborted)
                    {
                        throw new InvalidOperationException(
                            "ボナンザはすでに既に終了しています。");
                    }

                    // まずプロセスを起動します。
                    Start();

                    // 
                    if (this.aborted)
                    {
                        throw new BonanzaException();
                    }

                    this.initialized = true;
                }
            }
            catch (Exception ex)
            {
                WPFUtil.UIProcess(() =>
                    DialogUtil.ShowError(ex,
                        "ボナンザの初期化に失敗しました。"));

                Abort(AbortReason.Error);
            }
        }

        /// <summary>
        /// ボナンザは実行ファイルと同じパスにある必要があります。
        /// </summary>
        private static string GetBonasterPath()
        {
            var asm = Assembly.GetEntryAssembly();
            var dirname = asm.Location;
            var exename = "bonaster.exe";

            // 一つ上の階層までなら許すことにします。
            for (var i = 0; i < 1; ++i)
            {
                dirname = Path.GetDirectoryName(dirname);

                var path = Path.Combine(dirname, exename);
                if (File.Exists(path))
                {
                    return path;
                }
            }

            return null;
        }

        /// <summary>
        /// ボナンザプロセスを起動します。
        /// </summary>
        private void Start()
        {
            var filepath = GetBonasterPath();
            if (string.IsNullOrEmpty(filepath))
            {
                throw new FileNotFoundException(
                    string.Format("{0}が見つかりません。", filepath));
            }

            var startInfo = new ProcessStartInfo
            {
                FileName = filepath,
                WorkingDirectory = Path.GetDirectoryName(filepath),
                Arguments = "csa_shogi",
                CreateNoWindow = true,
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardInput = true,
                RedirectStandardError = true,
            };

            var process = new Process
            {
                StartInfo = startInfo,
            };

            if (!process.Start())
            {
                throw new InvalidOperationException(
                    "ボナンザの起動に失敗しました。");
            }

            process.Exited += (sender, e) =>
                Abort(AbortReason.Aborted);

            ReadStart(process.StandardOutput);
            ErrorReadStart(process.StandardError);
            WriteStart(process.StandardInput);

            this.bonaProcess = process;
        }

        #region コールバック
        /// <summary>
        /// mnjprepareの結果を識別する正規表現です。
        /// </summary>
        private static readonly Regex MnjPrepareRegex = new Regex(
            @"^info mnjprepare\s*([\w]+)\s*$",
            RegexOptions.IgnoreCase);

        void Bonanza_ReceivedCommand(object sender, BonanzaReceivedCommandEventArgs e)
        {
            if (IsMnjInited == null)
            {
                var m = MnjPrepareRegex.Match(e.Command);
                if (m.Success)
                {
                    var inited = (m.Groups[1].Value == "ok");

                    IsMnjInited = inited;
                    if (inited)
                    {
                        MnjInited.SafeRaiseEvent(this, EventArgs.Empty);
                    }
                }
            }
        }

        void Bonanza_ReceivedError(object sender, BonanzaReceivedCommandEventArgs e)
        {
            var error = e.Command;

            if (error.StartsWith("ERROR: "))
            {
                /*WpfUtil.UIProcess(() =>
                    DialogUtil.ShowError(
                        string.Format(
                            "ボナンザにエラーが発生しました。{0}{0}" +
                            "理由：{1}",
                            Environment.NewLine,
                            error.Substring(7))));*/

                Abort(AbortReason.Error);
            }
            else if (error.StartsWith("WARNING: "))
            {
                /*var message = error.Substring(9);

                if (message == "shut down connection")
                {
                    WpfUtil.UIProcess(() =>
                        DialogUtil.ShowError(
                            string.Format(
                                "ボナ子がサーバーから切断されました。{0}{0}" +
                                "ボナ子を再起動してください。",
                                Environment.NewLine)));
                }*/
                Abort(AbortReason.Error);
            }
        }
        #endregion

        #region mnj
        /// <summary>
        /// mnjprepare処理を開始します。
        /// </summary>
        public void BeginPrepareMnj()
        {
            var command = string.Format(
                "mnjprepare 15 {0}",
                MathEx.RandInt());

            WriteCommand(command);
        }

        /// <summary>
        /// 並列化サーバーに接続します。
        /// </summary>
        public void Connect(string serverAddress, int serverPort, int dfpnPort,
                            string name, int threadNum, int hashSize)
        {
            using (LazyLock())
            {
                if (!this.initialized)
                {
                    throw new InvalidOperationException(
                        "ボナンザが起動していません。");
                }

                if (IsMnjInited != true)
                {
                    throw new InvalidOperationException(
                        "mnjが初期化されていません。");
                }

                if (IsConnected)
                {
                    throw new InvalidOperationException(
                        "既に並列化サーバーに接続しています。");
                }
                
                if (string.IsNullOrEmpty(name) ||
                    !name.All(_ => char.IsLetterOrDigit(_) || _ == '_'))
                {
                    throw new ArgumentException(
                        "名前はアルファベット＋数字の組み合わせでお願いします。");
                }

                WriteCommand(string.Format("tlp num {0}", threadNum));
                WriteCommand(string.Format("hash {0}", hashSize));

                var command = string.Format(
                    "dfpn_client {0} {1}",
                    serverAddress,
                    dfpnPort);
                WriteCommand(command);

                // 並列化サーバーへの接続コマンドを発行します。
                command = string.Format(
                    "mnj {0} {1} {2} {3}",
                    serverAddress,
                    serverPort,
                    name,
                    threadNum);
                WriteCommand(command);

                IsConnected = true;
            }
        }
        #endregion

        #region dfpn
        /// <summary>
        /// DFPNサーバーに接続します。
        /// </summary>
        public void ConnectToDfpn(string serverAddress, int serverPort, string name,
                                  int threadNum, int hashSize)
        {
            using (LazyLock())
            {
                if (!this.initialized)
                {
                    throw new InvalidOperationException(
                        "ボナンザが起動していません。");
                }

                if (IsConnected)
                {
                    throw new InvalidOperationException(
                        "既に並列化サーバーに接続しています。");
                }

                if (string.IsNullOrEmpty(name) ||
                    !name.All(_ => char.IsLetterOrDigit(_) || _ == '_'))
                {
                    throw new ArgumentException(
                        "名前はアルファベット＋数字の組み合わせでお願いします。");
                }

                WriteCommand(string.Format("tlp num {0}", threadNum));
                WriteCommand(string.Format("hash {0}", hashSize));

                // DFPNサーバーへの接続コマンドを発行します。
                var command = string.Format(
                    "dfpn connect {0} {1} {2}",
                    serverAddress,
                    serverPort,
                    name);
                WriteCommand(command);

                IsConnected = true;
            }
        }
        #endregion

        #region ボナンザ読み込み
        /// <summary>
        /// ボナンザからの読み込みを開始します。
        /// </summary>
        private void ReadStart(StreamReader reader)
        {
            this.readThread = new Thread(ReadMain)
            {
                IsBackground = true,
                Name = "Bonanza Read Thread",
                Priority = ThreadPriority.AboveNormal,
            };

            this.readThread.Start(reader);
        }

        /// <summary>
        /// ボナンザからの読み込みを行います。
        /// </summary>
        private void ReadMain(object arg)
        {
            var reader = (StreamReader)arg;

            while (!this.aborted)
            {
                var line = reader.ReadLine();
                if (line == null)
                {
                    break;
                }

                CommandReceived.SafeRaiseEvent(
                    this, new BonanzaReceivedCommandEventArgs(line));
            }
        }
        #endregion

        #region ボナンザエラー読み込み
        /// <summary>
        /// ボナンザからのエラー読み込みを開始します。
        /// </summary>
        private void ErrorReadStart(StreamReader reader)
        {
            this.errorReadThread = new Thread(ErrorReadMain)
            {
                IsBackground = true,
                Name = "Bonanza ErrorRead Thread",
                Priority = ThreadPriority.AboveNormal,
            };

            this.errorReadThread.Start(reader);
        }

        /// <summary>
        /// ボナンザからのエラー読み込みを行います。
        /// </summary>
        private void ErrorReadMain(object arg)
        {
            var reader = (StreamReader)arg;

            while (!this.aborted)
            {
                var line = reader.ReadLine();
                if (line == null)
                {
                    break;
                }

                ErrorReceived.SafeRaiseEvent(
                    this, new BonanzaReceivedCommandEventArgs(line));
            }
        }
        #endregion

        #region ボナンザ書き込み
        /// <summary>
        /// ボナンザへの書き込みコマンドを設定します。
        /// </summary>
        public void WriteCommand(string command)
        {
            if (string.IsNullOrEmpty(command))
            {
                return;
            }

            command.Trim();

            lock (this.writeCommands)
            {
                this.writeCommands.AddLast(command);

                Monitor.PulseAll(this.writeCommands);
            }
        }

        /// <summary>
        /// ボナンザへの書き込みコマンドを一つ取得します。
        /// </summary>
        private string GetWriteCommand()
        {
            lock (this.writeCommands)
            {
                while (!this.aborted && !this.writeCommands.Any())
                {
                    Monitor.Wait(this.writeCommands, 500);
                }

                if (this.aborted && !this.writeCommands.Any())
                {
                    return null;
                }

                // 書き込みコマンドを一つ取り出します。
                var command = this.writeCommands.FirstOrDefault();
                this.writeCommands.RemoveFirst();
                return command;
            }
        }

        /// <summary>
        /// ボナンザへの書き込みを開始します。
        /// </summary>
        private void WriteStart(StreamWriter writer)
        {
            this.writeThread = new Thread(WriteMain)
            {
                IsBackground = true,
                Name = "Bonanza WriteCommand Thread",
                Priority = ThreadPriority.AboveNormal,
            };

            this.writeThread.Start(writer);
        }

        /// <summary>
        /// ボナンザへの書き込みを行います。
        /// </summary>
        private void WriteMain(object arg)
        {
            var writer = (StreamWriter)arg;

            while (!this.aborted)
            {
                var command = GetWriteCommand();
                if (command == null)
                {
                    continue;
                }

                writer.WriteLine(command + "\n");
                writer.Flush();

                CommandSent.SafeRaiseEvent(this,
                    new BonanzaReceivedCommandEventArgs(command));
            }
        }
        #endregion
    }
}
