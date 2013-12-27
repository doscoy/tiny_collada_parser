tiny_collada_parser
===================

非常にシンプルなCOLLADAのパーサー（インポータ）です。

COLLADAファイルに定義された頂点情報、法線、UV、マテリアルを抜き出します。



利用方法
---------------------
tiny_collada_parser.cppとtiny_collada_parser.hppの２つを利用したい環境に追加して下さい。



ライセンス
---------------------
zlibライセンスです。

詳細は付属の[LICENSE](https://github.com/doscoy/tiny_collada_parser/blob/master/LICENSE)を参照してください。

サンプル
---------------------
ROOT/samples以下にサンプルコードが入っています。
* sample01 ---  .daeファイルから抜き出した情報を表示します。
* sample02 --- .daeファイルから抜き出した頂点情報をGLUTを使って描画します。



