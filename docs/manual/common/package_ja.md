`full` パッケージには次のビルドが含まれています。

- Windows (x86_64)
- Linux (x86_64)
- macOS (Universal 2)

Linux ビルドは Ubuntu 20.04 でビルドしています。もし Ubuntu 20.04 以外のディストリビューションを使っているときは、プラグインが読み込まれないなどの不具合が起こることがあります。この場合は[ビルド手順](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md)に沿ってソースコードからビルドしてください。

macOS で使用するときは `macOS` パッケージのダウンロードを推奨します。また Apple Developer Program に加入していないので、インストール時に Gatekeeper を迂回するように設定する必要があります。詳細は <a href="#macos">インストール -> macOS</a> の節を参照してください。
