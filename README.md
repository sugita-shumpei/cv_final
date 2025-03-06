# Light Field 実装

## 概要

このプロジェクトは、Computer Visionの最終課題として作成したLight Fieldの実装です。

## 機能

- **test0**: 四重線形補間 (Exercise 14.8-2)
- **test1**: リフォーカシング (Exercise 14.8-3)

## ビルド手順

1. `build` ディレクトリを作成し、移動:
   ```sh
   mkdir build && cd build
   ```
2. CMakeを実行してプロジェクトを設定:
   ```sh
   cmake ../
   ```

## データのインストール

必要なデータをインストールするには、以下のコマンドを実行:

```sh
python install_data.py
```

