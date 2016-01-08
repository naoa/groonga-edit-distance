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
| 62x62 | 0.31589 sec | 74.16554 sec | 1.38189 sec | 6.19149 sec|
| 3x62 | 0.31312 sec | 5.26055 sec | 0.91457 sec | 5.87688 sec|
| 62x3 | 0.31560 sec | 5.73797 sec | 0.93845 sec | 1.81229 sec|
| 15x15 | 0.31624 sec | 5.40047 sec | 0.93917 sec | 2.79704 sec|
| 3x3 | 0.31591 sec | 1.19619 sec | 0.79177 sec | 1.37829 sec|

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
