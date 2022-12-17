
# MineGame 程式架構

## 架構概觀

踩地雷的架構主要可以分為以下 2 個部分

* ***遊戲邏輯***: 遊戲邏輯由 `MineGame` 這個 C++ class  實現，他實作了踩地雷遊戲的邏輯，包含難度設定、打開地圖、插旗等等邏輯。
* ***使用者介面***: 使用者介面主要由 SDL 實作，利用 SDL 所提供的各種功能來實作包含建立視窗、建立及更新圖像介面等，使用者介面亦會接收滑鼠點擊事件，並依照事件輸入來與 `MineGame` 溝通，最後將 `MineGame` 的結果呈現在畫面上。

以下介紹 ```MineGame``` 這個 C++ class 

## MineGame 的程式碼

```MineGame``` 這個 C++ class 宣告和實作分別在 ***game.h*** 和 ***game.cpp*** 內，以下為 ```MineGame```程式碼解釋

### Member variables

`MineGame` 這個 C++ class 實作了踩地雷遊戲的的邏輯，他的 member variable 可以分為幾類
* 遊戲設定
  * `width` 代表遊戲畫面寬，舉例來說，困難等級的遊戲寬為 30
  * `height` 代表遊戲畫面高，舉例來說，困難等級的遊戲寬為 16
  * `mine_count` 代表遊戲內地雷數量，舉例來說，困難等級的地雷數量為 99
* 遊戲狀態
  * `mine_map` 是一個大小為 `width` x `height` 的 `int` 二維陣列，他記錄了每一格與地雷相關的資訊，`mine_map[y][x]` 代表的是圖面上位於 (x, y) 座標的格子，左上角定義為 (0, 0)。其數值代表的意義如下
    * -2: 遊戲尚未開始，因此尚未放置地雷，這是初始狀態
    * -1: 代表此格為地雷
    * 0: 代表此格沒有地雷，但是相鄰周遭 8 格中有 0 格有地雷
    * 1: 代表此格沒有地雷，但是相鄰周遭 8 格中有 1 格有地雷
    * 2 ~ 8: 依此類推
  * `game_state` 代表遊戲目前進行中的狀態，他有幾種可能
    * GAME_READY: 每當重設遊戲(例如重新開始或調整難易度)後會進入這個狀態，此狀態此時遊戲尚未開始，使用者尚未打開任何一格地圖
    * GAME_RUNNING: 當點擊滑鼠打開任一格地圖後，遊戲會在 `mine_map` 上依照 `mine_count` 數量隨機放置地雷，並更新整張 `mine_map`，這是為了避免使用者第一步就踩到地雷。完成後此時遊戲正式開始，進入 GAME_RUNNING 狀態
    * GAME_WON: 當所有不含地雷的地圖都被打開後，使用者獲勝，遊戲結束並進入 GAME_WON 狀態
    * GAME_LOST: 若遊戲過程中使用者開打含有地雷的地圖，使用者輸了，遊戲結束並進入 GAME_LOST 狀態
  * `grid_state_map` 也是一個大小為 `width` x `height` 的二維陣列，他代表的是遊戲使用者看到的格子狀態，與 `mine_map` 一樣，`grid_state_map[y][x]` 代表的是圖面上位於 (x, y) 座標的格子，其意義如下
    * STATE_COVERED 代表此格尚未被點擊打開
    * STATE_FLAGGED 代表此格尚未被點擊打開，且上面已經放置旗子
    * STATE_MINE_0: 代表此格已被打開，且周遭相鄰的 8 格中有 0 格有地雷
    * STATE_MINE_1: 代表此格已被打開，且周遭相鄰的 8 格中有 1 格有地雷
    * STATE_MINE_2 ~ STATE_MINE_8: 依此類推
    * STATE_MINE_OPEN 代表此格有地雷。當使用者踩到地雷輸了的時候，會掃描所有 STATE_COVERED 的格子，並將內有地雷者更新為這個狀態
    * STATE_FLAGGED_WRONG 代表此格尚未被點擊打開，且上面放置旗子，但實際上該格並無地雷。當使用者踩到地雷輸了的時候，會掃描所有 STATE_FLAGGED 的格子，並將誤標的格子改為這個狀態
    * STATE_MINE_EXPLODE 代表此格已被打開，但 ***其中有地雷***。當使用者輸了的時候，這是使用者最後點擊的那一格。
  * `grid_dirty_map` 也是一個大小為 `width` x `height` 的二維陣列，他代表的是 `grid_state_map` 狀態是否經過更新，亦即每當 `grid_state_map[y][x]` 被更新時，他所對應的 `grid_dirty_map[y][x]` 會同時被設為 `true`
  * `flag_count` 代表遊戲過程中放置旗子的數量
  * `remaining_count` 代表遊戲過程未開格子的數量，當 `remaining_count` 降為 0，使用者獲勝

### Member functions

配合上述 member variables 的說明，`MineGame` 這個 C++ class 有以下 member functions 可以操作
 * 遊戲設定
   * `SetBeginner`: 設定難度等級為 ***簡單***，即設定 `width` 為 9、`height` 為 9、`mine_count` 為 10
   * `SetIntermediate`: 設定難度等級為 ***中級***，即設定 `width` 為 16、`height` 為 16、`mine_count` 為 40
   * `SetExpert`: 設定難度等級為 ***困難***，即設定 `width` 為 30、`height` 為 16、`mine_count` 為 99
   * `SetCustom`: 依照給定參數設定 `width`、`height`、`mine_count` 
   * `Reset`: 將遊戲重設回 GAME_READY 狀態，亦即 `mine_map`、`grid_state_map` 會分別被重設為 -2 (尚未放置地雷)和 STATE_COVERED (格子未打開) 的狀態
 * 遊戲資訊取得
   * `GetWidth`: 取得 `width`
   * `GetHeight`: 取得 `height`
   * `GetMineCount`: 取得 `mine_count`
   * `GetFlagCount`: 取得 `flag_count`
 * 操作遊戲
   * `TouchFlag(x, y)`: 在地圖上的第 (x, y) 格上放置旗子，地圖的左上角定義為 (0, 0)。若該格子上已經有旗子了，則此一操作會移除旗子。亦即，此操作會將 `grid_state_map[y][x]` 的狀態由 STATE_COVERED 改為 STATE_FLAGGED，或進行相反操作，並更新 `grid_dirty_map[y][x]`
   * `Open(x, y)`: 打開地圖的第 (x, y) 格，地圖的左上角定義為 (0, 0)。若該格底下有地雷，此一操作會將 STATE_COVERED 改為 STATE_MINE_EXPLODE；若沒有地雷，則會將狀態改為 STATE_MINE_[0 ~ 8]。若該格新狀態為 STATE_MINE_0，代表周遭 8 格都無地雷，會繼續打開周遭 8 格，一直遞迴下去直到狀態不再是 STATE_MINE_0。與 `TouchFlag` 一樣，此項操作也會更新 `grid_state_map` 和 `grid_dirty_map`
 * 取得遊戲狀態
   * `GetGameState`: 取得 `game_state`
   * `GetGridState(x, y)`: 取得地圖上第 (x, y) 格的狀態，即回傳 `grid_state_map[y][x]`
   * `GetDirtyGrids`: 操作遊戲後，會有部分格子的狀態被更新，透過此項操作可以獲得所有被更新格子的狀態(即 `grid_dirty_map` 為 `true` 的所有格子)。
   * `ClearDirtyGrids`: 當透過 `GetDirtyGrids` 取得所以格子的新狀態後，呼叫此 function 來重設 `grid_dirty_map` 回 `false`

以下為遊戲狀態與操作之互動關係

![game-state.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/game-state.png)

## 與 MineGame 互動

透過上述介紹，可以了解到與 `MineGame` 互動，基本上是透過 `Open` 和 `TouchFlag` 來告訴 `MineGame` 玩家的操作，待操作完成後呼叫 `GetGameState` 確認結果，並呼叫 `GetDirtyGrids` 與 `ClearDirtyGrids` 獲得每一格的變化，將之反映給玩家。

透過這樣的設計，不管是 GUI 介面或文字介面都可以用同樣的方式來和 `MineGame` 溝通，使用者介面僅需實作以下兩點即可
1. ***將操作翻譯給*** `MineGame`: 即收到操作後，轉呼叫 `Open` 或 `TouchFlag`。以 GUI 介面為例，當滑鼠左鍵點擊時，GUI 介面須將滑鼠點擊位置轉換為每一格的座標 (x, y)，然後呼叫 `Open(x, y)`，藉此告訴 `MineGame` 玩家的操作
2. ***將結果更新至介面***: 當操作完成後，呼叫 `GetGameState`、`GetDirtyGrids`、`ClearDirtyGrids` 並依據各使用者介面特性(例如 GUI 介面就是去繪圖更新視窗內容、文字介面就是更新文字)將操作結果反映給玩家

以下為 [GUI](https://github.com/Lizzychu/minesweeper/blob/master/doc/gui.md) 與遊戲互動關係之示意圖

![game-state.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/gui-game-interaction.png)

## MineGame FAQ
***Q.*** 為什麼要等玩家第一次開啟圖面格子後，才放置地雷?
***A.*** 這是為了保證玩家不會在第一次開啟圖面格子的時候，就踩到地雷輸掉遊戲。這個行為和絕大多數的踩地雷遊戲行為一致。為了實作這個行為，我們特別把 `MineGame` 設計成有 GAME_READY 和 GAME_RUNNING 這兩種狀態。

***Q.*** 怎麼保證玩家不會在第一次開啟圖面格子的時候，就踩到地雷?
***A.*** 放置地雷邏輯實作在 `InitMines` 裡面，他的兩個參數 `skip_x` 和 `skip_y` 代表的就是第一次開啟地圖所點擊的那一格的座標。簡單來說，做法就是將地雷隨機分配在 (`skip_x`, `skip_y`) 之外的所有格子。

詳細演算法如下:
1. 配置一個名為 `map`，大小為 (`width` * `height` - 1) 的陣列。`map` 的大小剛好是 `mine_map` 的大小減 1，減去 1 即是為了排除 (`skip_x`, `skip_y`)
2. 為 `map` 填入 `mine_count` 個 -1 (即地雷)，其餘填 -2 (即初始狀態)。
3. 利用 [random_shuffle](https://cplusplus.com/reference/algorithm/random_shuffle/) 將 `map` 中的 -1 和 -2 隨機打散
4. 若不考慮 `map` 和 `mine_map` 大小差 1，原則上 `map` 和 `mine_map` 這兩個陣列的元素有以下對應關係
   * `map[0]` 對應到 `mine_map[0][0]`
   * `map[1]` 對應到 `mine_map[0][1]`
   * `map[2]` 對應到 `mine_map[0][2]`
   * ...
   * `map[width]` 對應到 `mine_map[1][0]`
   * `map[width + 1]` 對應到 `mine_map[1][1]`
   * `map[width + 2]` 對應到 `mine_map[1][2]`
   * 依此類推，關係式為 
     * `k` = `width` * `i` + `j`
     * `map[k]` 對應到 `mine_map[i][j]`
5. 根據 4. 將 `map` 中的每一個元素，依據下列規格填入 `mine_map`
   * 計算 `skip` = `width` * `skip_y` + `skip_x`，這個 `skip` 就是 (`skip_x`, `skip_y`) 在 `map` 中的位置
   * 針對 `mine_map` 中的每一個元素 `mine_map[i][j]`
     * 計算 `k` = `width` * `i` + `j`
     * 如果 `k` < `skip`，`mine_map[i][j] = map[k]`
     * 如果 `k` == `skip`，略過
     * 如果 `k` > `skip`，`mine_map[i][j] = map[k - 1]`
   * 這個做法恰恰好能保證 (`skip_x`, `skip_y`) 對應到的位置 `skip` 被略過，也是 `map` 大小要比 `mine_map` 減 1 的道理
6. 因為 `mine_map` 中 (`skip_x`, `skip_y`) 不會被填入地雷，這保證玩家不會在第一次開啟圖面格子的時候，就踩到地雷
 
***Q.*** 一次開啟好幾個格子是怎樣實作的?
***A.*** 實作在 `OpenRecursive` 內，`OpenRecursive` 會判斷當下開啟 (x, y) 時，若 `mine_map[y][x]`為 0，代表周遭 8 格都沒有地雷，要遞迴往附近 8 格繼續開，程式碼如下
```C++
    // expand if we click on a 0
    if (this->mine_map[y][x] == 0) {
        int up = y - 1, down = y + 1, left = x - 1, right = x + 1;

        this->OpenRecursive(left, up);
        this->OpenRecursive(left, y);
        this->OpenRecursive(left, down);
        this->OpenRecursive(x, up);
        this->OpenRecursive(x, down);
        this->OpenRecursive(right, up);
        this->OpenRecursive(right, y);
        this->OpenRecursive(right, down);
    }
```

