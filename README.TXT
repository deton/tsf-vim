
================================================================================
     tsf-vim version 0.1.0 (2013-12-XXX)

     https://github.com/deton/tsf-vim
     deton@m1.interq.or.jp
================================================================================

  普通のWindowsアプリにおいて、擬似的にViエディタのNormal mode編集操作を
  使えるようにするIMEです。


--------------------------------------------------------------------------------
     インストール
--------------------------------------------------------------------------------


・必須ライブラリ のインストール

  32bit版 Windows では x86 の VC++ パッケージのみを、
  64bit版 Windows では x86 と x64 の VC++ パッケージの両方を
  予めインストールして下さい。

  Microsoft Visual C++ 2010 SP1 再頒布可能パッケージ (x86)
  http://www.microsoft.com/ja-jp/download/details.aspx?id=8328

  Microsoft Visual C++ 2010 SP1 再頒布可能パッケージ (x64)
  http://www.microsoft.com/ja-jp/download/details.aspx?id=13523


・tsf-vim のインストール

  tsf-vim-X.Y.Z.zip を展開し、32bit版 Windows では tsfvim-x86.msi、
  64bit版 Windows では tsfvim-x64.msi を実行して下さい。
  （X, Y, Z はバージョン番号）

・IME設定

  tsf-vimは、半/全キー等が押された時は、他IMEに切り替えます。
  具体的には、以下の2つを送り付けます。
  (1)他IMEに切り替えるためのキーを送り付け。
     送り付けるキーはtsf-vimの設定ダイアログで以下のいずれかを設定。
     ・Alt+Shift
     ・Alt+Shift 2回
     ...
     ・Alt+Shift 9回
     ・Alt+Shift+0
     ・Alt+Shift+1
     ...
     ・Win+Space
     ・Win+Space 2回
     ...
     (なお、Alt+Shift+0等は、Outlook 2010のメール作成ウィンドウだと切り替えが
     行われません)
  (2)切り替え後のIMEをONにするため、漢字キーを送り付け。

  他IMEに切り替えるキーは、Windows 7の場合、以下で設定しておいてください。

  コントロールパネル→「地域と言語」「キーボードと言語」タブ
  →「キーボードの変更」→「テキスト サービスと入力言語」ダイアログ

  「詳細なキー設定」タブ:
  「入力言語のホット キー」の「入力言語を切り替える」が選択された状態で、
  「キー シーケンスの変更」ボタン→「キー シーケンスの変更」ダイアログで、
  「キーボード レイアウトの切り替え」で「左 Alt+Shift」を選択。

   設定例: https://gist.github.com/deton/7393058#ime

--------------------------------------------------------------------------------
     Vi編集操作
--------------------------------------------------------------------------------

  基本的には、KeyVi同様に、送り付けるキーの置換を行います。
    例: 'h'キーが押されたら、VK_LEFTを送り付け。

  f,t,F,T,),(,w,e,b,W,E,B等、アプリに表示されている文字列を取得して動作する
  必要のあるコマンドの場合、TSF(Text Services Framework)を使用して
  文字列取得を行います。
  TSFに対応していないアプリの場合は、文字列取得にIMR_DOCUMENTFEEDを使用します。
  IMR_DOCUMENTFEEDにも対応していないアプリの場合は、動作しません。

  現状、動作を確認しているのは以下のアプリです:
  + Word 2010
  + Outlook 2010
  + (PowerPoint 2010やExcel 2010、Visual Studio 2010/2012ではf等は動きません)
  + WordPad
  + notepad

コマンド 送り付けるキーシーケンス
0       HOME
g$      END (count未対応)
$       改行まで(*)RIGHT (count未対応)
c       CTRL-X,tsf-vim OFF
d       CTRL-X
y       CTRL-C
cc      [count-1]DOWN,END,RIGHT,選択開始,[count-1]UP,CTRL-X,tsf-vim OFF
cj      [count]DOWN,END,RIGHT,選択開始,[count]UP,CTRL-X,tsf-vim OFF
ck      END,RIGHT,選択開始,[count]UP,CTRL-X,tsf-vim OFF
dd      [count-1]DOWN,END,RIGHT,選択開始,[count-1]UP,CTRL-X
        (Wordで選択中に既に行末時ENDを送ると次の行末まで移動するので一度DOWN)
dj      [count]DOWN,END,RIGHT,選択開始,[count]UP,CTRL-X
dk      END,RIGHT,選択開始,[count]UP,CTRL-X
yy      [count-1]DOWN,END,RIGHT,選択開始,[count-1]UP,CTRL-C
yj      [count]DOWN,END,RIGHT,選択開始,[count]UP,CTRL-C
yk      END,RIGHT,選択開始,[count]UP,CTRL-C
C       選択開始,END,CTRL-X,tsf-vim OFF
D       選択開始,END,CTRL-X
f{char} 指定文字まで(*)RIGHT
t{char} 指定文字の前まで(*)RIGHT
F{char} 指定文字まで(*)LEFT
T{char} 指定文字の後まで(*)LEFT
;       最後のftFTを繰り返す
,       最後のftFTを反対方向に繰り返す
r{char} DELETE,{char}
s       選択開始,RIGHT,CTRL-X,tsf-vim OFF
gg      CTRL-HOME
G       CTRL-END,HOME (ただしcount指定時はggと同様)
CTRL-F  PageDown
CTRL-B  PageUp
h       LEFT
l       RIGHT
<Space> RIGHT
j       DOWN
k       UP
+       次行最初の非空白文字まで(*)RIGHT
CTRL-M  次行最初の非空白文字まで(*)RIGHT
i       tsf-vim OFF
I       行の先頭の非空白文字まで(*)RIGHTもしくはLEFT,tsf-vim OFF
a       RIGHT,tsf-vim OFF
A       END,tsf-vim OFF
o       END,RETURN,tsf-vim OFF
O       HOME,RETURN,UP,tsf-vim OFF
p       RIGHT,CTRL-V
P       CTRL-V
u       CTRL-Z
x       選択開始,RIGHT,CTRL-X
X       選択開始,LEFT,CTRL-X
w       次の単語まで(*)RIGHT
W       次の単語まで(*)RIGHT
e       単語末尾まで(*)RIGHT
E       単語末尾まで(*)RIGHT
b       前の単語まで(*)LEFT
B       前の単語まで(*)LEFT
)       文末まで(*)RIGHT
(       文頭まで(*)LEFT
J       文末まで(*)RIGHT,次行の非空白文字まで(*)DELETE
        文末と次行の非空白文字がともに英字の場合は(*)空白挿入: i,SPACE,ESCAPE
        (count未対応)

  c,d,yと移動コマンド(h,f{char},w,$等)の組み合わせ可能。
  一部コマンドはカウント指定も対応。
  (*)はTSF等で文字列取得しているので、未対応アプリでは動作不可。

  半/全キーや漢字キー等が押された時は、他IMEに切り替えます。
  具体的には、Win+SPACE(Windows 8)かAlt+SHIFT(それ以外)を送り付けた後、
  切り替え後のIMEをONにするため、KANJIキーを送り付けます。

  注: 行末移動にVK_END、行頭移動にVK_HOME送り付けを行う場合、
  行が折り返されて表示されていると、表示されている画面端に移動します
  (改行までではなく。)

--------------------------------------------------------------------------------
     設定
--------------------------------------------------------------------------------


・設定ダイアログ

  設定ダイアログは、以下の操作で開くことができます。
    ・言語バーを右クリックし、メニューから「設定」を選択する。
      「テキストサービスと入力言語」ダイアログの「インストールされているサービス」
      からtsf-vimを選択し、プロパティボタンを押す。
    ・言語バーの入力モードアイコンを左クリックし、メニューから「設定」を選択する。
    ・言語バーのヘルプボタンを左クリックし、メニューから「設定」を選択する。
    ・Windows 8 以降の通知領域の入力モードアイコンを右クリックし、メニューから
      「設定」を選択する。

  設定ダイアログで保存した後は、IME OFF → ON で新しい設定が反映されます。
  IME ON/OFF のキーのみ、IME ON → OFF のときにも反映されます。


・設定ファイル

  各種設定は、ディレクトリ %AppData%\tsf-vim\config.xmlに保存されます。

  config.xml ファイルは、UTF-8 の XML フォーマットのテキストファイル
  となっており、テキストエディタで編集可能です。
  変更した後は、IME OFF → ON で新しい設定が反映されます。


・動作

  || *機能*              || *備考*
  ----------------------------------------------------------------------------
  || 他IME切替時送出キー || 他IMEに切り替えるために送り付けるキー


・ホットキー設定

  IMEをON(Normal mode)にするキーや、他IMEに切り替えるキーを指定します。
  仮想キーコード、Alt、Control、Shift の組合せを設定して下さい。
  仮想キーコードの詳細はこちらを参照下さい。 http://msdn.microsoft.com/en-us/library/dd375731
  それぞれ最大で8行です。

  例) Ctrl+[ (0xDB)で、Normal mode。
      Ctrl+, (0xBC)で、他IME切替。

・キー設定デフォルト

  ホットキー設定

  || *機能*      || *キー*                 || *備考*
  -----------------------------------------------------------------------------
  || Normal mode || Esc                    || 0x1B
  || 他IME切替   || Alt+`                  || 0xC0 + Alt
  ||             || 漢字                   || 0x19
  ||             || (Alt+半角/全角  Alt+`) || 0x19
  ||             || 半角/全角(Ctrl+`)      || 0xF3, 0xF4

--------------------------------------------------------------------------------
     その他
--------------------------------------------------------------------------------


・ソースに関して

  ・IMEまわりの処理はCorvusSKKを流用させてもらっています。
    http://code.google.com/p/corvus-skk/
  ・TSFやIMR_DOCUMENTFEEDによる文字列取得関係は、
    Mozcから一部ソースを取り込んでいます。
    http://code.google.com/p/mozc/
  ・Vi編集操作に関しては、Vimやnvi-m17nのソースを参考に、
    処理の流れ等はほぼそのまま使っています。

・制限事項、既知の問題

  ・バグ: 最終行でdd等すると、1つ上の行まで対象になる。
  ・未対応のコマンド多数(text-object, Visual mode等も未対応)
  ・f,t,rでの日本語文字指定未対応(digraph, lmap未対応)
  ・サロゲートペアや結合文字、IVSの考慮は未対応。
  ・ChgKeyでSpaceキーをShiftキーに変更していて、SpaceキーをShiftキーとして
    Aや$等のコマンド入力に使った場合、Shiftキーが押されたままになる。
    一度Shiftキーを押して離せば回復。

・開発環境

  Visual C++ 2010 Express + SP1
  Windows SDK 7.1
  Windows Driver Kit 7.1.0 (Mozcで使用されているATLのため)
  WiX 3.8

・履歴

  ・v0.1.0 (2013-12-XXX)
    ・Insert modeへの切替時に、他IMEに切り替えるのをやめた。
      ESCキーでInsert modeを抜けられるように。
      「半/全」等のキーが押された時に、他IMEに切り替えるようにした。
    ・;,コマンド対応
  ・v0.0.1 (2013-11-10)
    ・最初のリリース
