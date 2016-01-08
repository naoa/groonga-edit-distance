# Groonga edit distance plugin

Groongaで高速に編集距離を求める関数を追加します。

* ``damerau_edit_distance``
* ``edit_distance_bp``
* ``edit_distance_bp_var``

### `damerau_edit_distance``関数

transposition対応のedit_distance

| arg        | description |default|
|:-----------|:------------|:------|
| 1      | 単語1 | NULL |
| 2     | 単語2| NULL |
| 3     | transpositionを含めるか(オプション) | true |

### ```edit_distance_bp```関数

ビットパラレル法により高速に編集距離を算出する。
64文字までの１バイト文字列のみに対応。日本語は未対応。

64文字を超えた場合、自動的にdamerau_edit_distanceに切り替わる。

#### オプション
| arg        | description |default|
|:-----------|:------------|:------|
| 1      | 単語1 | NULL |
| 2     | 単語2| NULL |
| 3     | transpositionを含めるか | true |

### ```edit_distance_bp_var```関数

ビットパラレル法により高速に編集距離を算出する。
日本語、マルチバイト対応版。マルチバイト文字をハッシュテーブルにマッピングする。

#### オプション
| arg        | description |default|
|:-----------|:------------|:------|
| 1      | 単語1 | NULL |
| 2     | 単語2| NULL |
| 3     | transpositionを含めるか | true |
| 4     | 1回のコマンド実行中にハッシュテーブルをキャッシュするか | true |


### 比較

|文字数x文字数| score代入のみ|動的計画法|ビットパラレル固定長| ビットパラレル可変長|
|--- |--------------- |---------------| -------------------- | -------|
| 62x62 | 0.31651 sec | 74.24125 sec | 1.35344 sec | 7.60058 sec|
| 3x62 | 0.31489 sec | 5.20102 sec | 0.92362 sec | 6.25549 sec|
| 62x3 | 0.31565 sec | 5.71125 sec | 0.90643 sec | 2.30969 sec|
| 15x15 | 0.31559 sec | 5.40569 sec | 0.98903 sec | 3.28515 sec|
| 3x3 | 0.31545 sec | 1.18463 sec | 0.80064 sec | 1.53972 sec|

## Install

Install libgroonga-dev / groonga-devel

Build this command.

    % sh autogen.sh
    % ./configure
    % make
    % sudo make install

## Usage

Register `functions/edit_distance`:

    % groonga DB
    > plugin_register functions/edit_distance

Now, you can use custom `edit_distance` function

## Author

Naoya Murakami naoya@createfield.com

## License

LGPL 2.1. See COPYING for details.
