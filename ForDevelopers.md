## 開発環境
- MacOSX 12.6 Monterey
- Qt Creator 8.0.1
- Qt 5.15.5 (5.15.5_3)

## 開発環境の構築手順
```
$ brew install qt@5
$ brew install qt-creator
```

## 開発時のTips
### 標準出力と標準エラー出力をファイルにリダイレクトする
```
$ open -F -a AsahikawaProcon-Server.app --stdout stdout.log --stderr stderr.log
```

## 参考資料
https://qiita.com/Kusunoki__/items/e58d00b918cc18cef67a
https://doc.qt.io/qt-6/qmake-variable-reference.html
https://doc.qt.io/qt-6/qstandardpaths.html
