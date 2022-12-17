
# GUI 程式架構

## 架構概觀

踩地雷的架構主要可以分為以下 2 個部分

* ***遊戲邏輯***: 遊戲邏輯由 `MineGame` 這個 C++ class  實現，他實作了踩地雷遊戲的邏輯，包含難度設定、打開地圖、插旗等等邏輯。
* ***使用者介面***: 使用者介面主要由 SDL 實作，利用 SDL 所提供的各種功能來實作包含建立視窗、建立及更新圖像介面等，使用者介面亦會接收滑鼠點擊事件，並依照事件輸入來與 `MineGame` 溝通，最後將 `MineGame` 的結果呈現在畫面上。

使用者介面(GUI) 是由 SDL 所實作，我們主要用它建立視窗、收集使用者輸入、產生事件、以及更新視窗內容等等。在架構上我們將 GUI 分為幾個部分
* ***視窗***
  * 視窗底層靠 SDL 的 [SDL_Window](https://wiki.libsdl.org/SDL2/SDL_Window) 所實現，他代表的是螢幕上一個矩形的可視區域
  * 在我們程式內，我們實作 `MineGameWindowUI` 這個 C++ class 來封裝 SDL_Window
  * 視窗需要負責接收並正確處理 [SDL_Event](https://wiki.libsdl.org/SDL2/SDL_Event)，SDL_Event 代表的是各式各樣的輸入，如關閉程式、滑鼠移動、滑鼠點擊等等操作事件
* ***元件***
  * 元件是在視窗可視區域內一個可以操作或可視的物件，例如按鈕、倒數計時器、踩地雷可點擊的格狀圖面區域等等
  * 我們實作以下元件來實現踩地雷遊戲的 GUI
    * 重設按鈕: 即位於視窗中央上方的笑臉按鈕，他可以重設踩地雷遊戲或顯示遊戲勝敗，以 `FaceButtonUI` 這個 C++ class 來實現
    * 計數器: 即位於視窗左上角及右上角的七段顯示計數器，左上角的代表剩餘地雷數量，右上角的代表經過時間，他們都是以 `CounterUI` 這個 C++ class 來實現
    * 踩地雷可點擊的格狀區域: 佔視窗最大部分面積的遊戲區域，他以 `MineGridUI` 這個 C++ class 來實現
* ***其他***
  * 其餘是相對與 GUI 關係較小的元件
  * `MineGameTimer`: 這是一個基於 [SDL Timer](https://wiki.libsdl.org/SDL2/SDL_AddTimer)的計時器，我們用這個 C++ class 來封裝 SDL Timer，並定時觸發 SDL Event 讓 ```MineGameWindow``` (即主視窗) 來處理
  * `SplashScreen`: 這是 splash screen (過場動畫) 的實現，我們用這個 C++ class 來實現過場動畫，讓玩家獲勝時能獲得老師的微笑

上述所有程式碼都放在 ***ui.h*** 和 ***ui.cpp*** 內，GUI 與 `MineGame` 互動示意如下

![gui-game-interaction.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/gui-game-interaction.png)

## GUI 程式碼

### 程式進入點

程式進入點在 ***main.cpp*** 內。從下方程式碼中可以看到，`main` function 在初始化 SDL 相關資源後，先創建 `MineGam` (即踩地雷遊戲)這個 class。創建完後，更進一步創建 `MineGameWindowUI` (即 GUI 視窗)，並將剛剛創建的 `MineGame` 傳給 GUI 視窗讓兩者可以進行互動。再下一步，令 GUI 透過 `CreateComponents` 建立所有元件，此後便令 GUI 視窗進入 `ProcessEvents` 開始處理使用者輸入。

`ProcessEvents` 內是一個無限迴圈，在視窗程式未關閉之前(即收到 [SDL_QUIT 事件](https://wiki.libsdl.org/SDL2/SDL_EventType))，程式會停在這行。***上圖顯示 GUI 與 `MineGame` 之間的互動主要都是發生在這個 function 內***。

一旦收到 SDL_QUIT 事件，程式會跳出 `ProcessEvents` 內的無限迴圈，進而執行 `DestroyComponents` 釋放所有元件及元件資源。接著 `MineGameWindowUI` 會被 delete，視窗因而被消滅，之後 `MineGame` 被 delete，釋放 `MineGame` 配置的空間與資源，程式最後呼叫 [SDL_Quit](https://wiki.libsdl.org/SDL2/SDL_Quit) 釋放 SDL 配置的資源，至此程式結束，`main` return 0。

程式碼如下

```C++
int main(int argc, char** argv)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        std::cerr << "SDL_Init failed with error: " << SDL_GetError() << std::endl;
        return -1;
    }

    MineGameWindowUI *ui;
    MineGame *game;

    // create game
    game = new MineGame();
    game->SetBeginner();

    // create UI, interact with game through UI
    ui = new MineGameWindowUI(game);
    ui->CreateComponents();
    ui->ProcessEvents();
    ui->DestroyComponents();

    delete ui;
    delete game;

    // Quit SDL subsystems
    SDL_Quit();

    return 0;
}
```
### 視窗

視窗代表的是螢幕上一個矩形的可視區域(如下圖)，我們的 GUI 程式便是透過 SDL library 繪製視窗，透過視窗與玩家互動

![gui-window.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/gui-window.png)

#### Member variables
視窗由 `MineGameWindowUI` 這個 C++ class 實現，他的 member variable 可以分為幾類
 * `MineGame`相關
   * `game`: 他是一個指向 `MineGame` 的 pointer，讓視窗可以和 `MineGame` 互動
 * 視窗與繪圖相關: 如前所述，視窗要負責 GUI 的繪製，因此 `MineGameWindowUI` 內部保留以下 member variables 實作相關功能
   * `window`: 他是一個指向 SDL_Window 的 pointer，代表視窗本身
   * `renderer`: 他是一個指向 [SDL_Renderer](https://wiki.libsdl.org/SDL2/SDL_Renderer) 的 pointer，代表視窗的渲染器 (可以想像是 GPU，負責繪製視窗圖面的裝置)
   * `window_texture`: 他是一個指向 [SDL_Texture](https://wiki.libsdl.org/SDL2/SDL_Texture) 的 pointer，代表視窗的 texture (可以想像是一張圖，這張圖被更新後要透過 `renderer` 繪製到視窗上`renderer` 繪製到視窗上)
   * `window_texture_dirty`: 代表 `window_texture` 是否有被更新，只有當 `window_texture` 被更新後，我們才進行繪圖，這可以避免過度更新視窗內容以增進效率、節省運算資源
 * 元件相關
   * `mine_grid`: 他是一個指向 `MineGridUI` 的 pointer，代表踩地雷可點擊的格狀區域
   * `face_button`: 他是一個指向 `FaceButtonUI` 的 pointer，代表視窗中央上方的笑臉按鈕
   * `mine_counter`: 他是一個指向 `CounterUI` 的 pointer，代表視窗左上角的剩餘地雷數量計數器
   * `time_counter`: 他也是一個指向 `CounterUI` 的 pointer，代表視窗右上角的經過時間計數器
   * `winning_splash`: 他是一個指向 `SplashScreen` 的 pointer，代表遊戲勝利後的過場畫面
   * `menu`: 他是一個指向 `MineGameMenu` 的 pointer，代表視窗的功能表
 * 計時器
   * `splash_timer`: 一個指向 `MineGameTimer` 的 pointer，用於過場畫面(即 `winning_splash`)的計時
   * `count_down_timer`: 一個指向 `MineGameTimer` 的 pointer，用於遊戲經過時間的計時

以下為視窗各元件之示意圖

![gui-components.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/gui-components.png)

#### Member functions

`MineGameWindowUI` 最重要的程式碼進入點在於 `ProcessEvents` 這個 function。

`ProcessEvents` 內部為一個無窮迴圈，不停處理來自使用者的各種事件，由於 SDL 已經在各平台(如 Windows、Mac OS X)上幫我們封裝並統一行為，我們這邊便可以透過相同的程式碼來進行跨平台開發。在收到 SDL Event 後，若他是一個 SDL_QUIT 事件，我們便跳出迴圈準備結束程式；若是其他事件，我們進入 `DispatchEvent` 來進行處理。

最後，處理完事件後，我們呼叫 `RefreshWindow` 來更新視窗區域。程式碼如下

```C++
int MineGameWindowUI::ProcessEvents()
{
    while (true) {
        SDL_Event e; 

        if (SDL_WaitEvent(&e) <= 0) {
            std::cerr << "SDL_WaitEvent failed with error: " << SDL_GetError() << std::endl;
            continue;
        }

        if (e.type == SDL_QUIT) {
            std::cout << "got quit event" << std::endl;
            break;
        }

        this->DispatchEvent(&e);
        this->RefreshWindow();
    }

    return 0;
}
```

`DispatchEvent` 是 `MineGameWindowUI` 處理各種事件的核心，簡述邏輯如下
 * `DispatchEvent` 內會先透過 `GetGameState` 來取得處理事件前的 `MineGame` 狀態
 * 之後會分辨 SDL Event 的種類來傳送給正確的元件處理，例如
   * 滑鼠移動 ([SDL_MouseMotionEvent](https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent)) 與滑鼠點擊 ([SDL_MouseButtonEvent](https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent))事件會傳送給 `face_button` 和 `mine_grid` 來處理，這兩個元件會再透過 `game` pointer 與 `MineGame` 互動
   * 其餘事件都是由 SDL User Event 觸發，包含
     * `count_down_timer` 觸發的 Timer 事件會導致 `time_counter` (遊戲時間經過計數器) 增加
     * `splash_timer` 觸發的 Timer 事件會導致 `winning_splash` (遊戲勝利過場畫面) 重繪
     * 視窗功能表觸發的事件(選擇遊戲難易程度)會進而透過 `game` 改變遊戲難度，進而觸發視窗重繪
 * 處理完事件後，特別是上述的滑鼠相關事件，會再透過 `GetGameState`確認事件前後遊戲狀態是否有改變
   * 如果遊戲從 GAME_READY 變為 GAME_RUNNING，會觸發 `count_down_timer` 開始計時
   * 如果遊戲從 GAME_RUNNING 變為 GAME_WON 或 GAME_LOST，會觸發繪製勝利過場畫面(贏了)或改變 `face_button` 的外觀(輸了)，並停止 `count_down_timer` 計時

除上述主要進入點之外，`MineGameWindowUI`亦提供一些 function 讓元件可以與之互動
 * `LoadTextureFromFile`: 讀取一個 png 檔案，並將之轉為 texture，可用於後續視窗繪圖
 * `UpdateWindowTexture`: 讓各元件可以將它內部保有的 texture 繪製於視窗上

#### 處理使用者輸入事件

如前段所述，視窗主要是透過 [SDL_WaitEvent](https://wiki.libsdl.org/SDL2/SDL_WaitEvent) 獲得相對應的 SDL_Event，並分析該事件的類型，將之總送給需要負責的元件來處理。以下就滑鼠點擊事件([SDL_MouseButtonEvent](https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent))流程進行說明

1. 在視窗中(即 `MineGameWindowUI::DispatchEvent`)，會初步針對事件類型進行分類，並將之轉送給相對應的元件，程式碼如下

```C++
int MineGameWindowUI::DispatchEvent(SDL_Event *base_event)
{
    ...
    // mouse click
    else if (base_event->type == SDL_MOUSEBUTTONUP || base_event->type == SDL_MOUSEBUTTONDOWN) {
        SDL_MouseButtonEvent *e = (SDL_MouseButtonEvent*)base_event;

        this->face_button->HandleMouseButtonEvent(e);
        this->mine_grid->HandleMouseButtonEvent(e);
    }
    ...
}
```

2. 在元件內部，會透過該事件所需攜帶的參數，判斷滑鼠點擊的位置，***如果點擊在元件內部，則元件必須要處理這個事件***。以 `FaceButtonUI` (微笑按鈕) 而言，他的處理邏輯如下

```C++
int FaceButtonUI::HandleMouseButtonEvent(SDL_MouseButtonEvent *event)
{
    int x1, x2, y1, y2;

    // (x1, y1) 為左上角, (x2, y2) 為右下角
    x1 = this->GetRect()->x;
    x2 = this->GetRect()->x + this->GetRect()->w;
    y1 = this->GetRect()->y;
    y2 = this->GetRect()->y + this->GetRect()->h;

    // 如果事件點擊位置 (x, y) 落在 (x1, y1) 到 (x2, y2) 圍成的矩形內，則處理該事件
    // 若左鍵下壓，設按鈕為下壓狀態 (pressed)；若左鍵上彈，設按鈕為回復狀態 (unpressed)
    if (x1 <= event->x && event->x < x2 && y1 <= event->y && event->y < y2) {
        if (event->button == SDL_BUTTON_LEFT) {
            if (event->type == SDL_MOUSEBUTTONDOWN) {
                this->SetStatus(FaceButtonUI::STATUS_FACE_PRESSED);
            } else if (event->type == SDL_MOUSEBUTTONUP) {
                this->SetStatus(FaceButtonUI::STATUS_FACE_UNPRESSED);
                this->window->GameReset();
            }
       }
    }

    return 0;
}
```

以下為示意圖，若滑鼠點擊位置 (x, y) 落在 (x1, y1) 到 (x2, y2) 圍成的矩形內，則視滑鼠是左鍵下壓或上彈事件，設定按鈕狀態

![gui-face-button-event.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/gui-face-button-event.png)

若是左鍵下壓 (SDL_MOUSEBUTTONDOWN)，設定為 pressed 狀態如下圖

![face_pressed.png](https://github.com/Lizzychu/minesweeper/blob/master/images/png/face_pressed.png)

若是左鍵上彈 (SDL_MOUSEBUTTONUP)，設定為 unpressed 狀態如下圖，並同時通知 `MineGame` 做 `Reset`

![face_unpressed.png](https://github.com/Lizzychu/minesweeper/blob/master/images/png/face_unpressed.png)
