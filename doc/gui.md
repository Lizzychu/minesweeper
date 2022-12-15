
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

上述所有程式碼都放在 ***ui.h*** 和 ***ui.cpp*** 內

## GUI 程式碼

### 程式進入點

程式進入點在 ***main.cpp*** 內。從下方程式碼中可以看到，`main` function 在初始化 SDL 相關資源後，先創建 `MineGam` (即踩地雷遊戲)這個 class。創建完後，更進一步創建 `MineGameWindowUI` (即 GUI 視窗)，並將剛剛創建的 `MineGame` 傳給 GUI 視窗讓兩者可以進行互動。再下一步，令 GUI 透過 `CreateComponents` 建立所有元件，此後便令 GUI 視窗進入 `ProcessEvents` 開始處理使用者輸入。

`ProcessEvents` 內是一個無限迴圈，在視窗程式未關閉之前(即收到 [SDL_QUIT 事件](https://wiki.libsdl.org/SDL2/SDL_EventType))，程式會停在這行。

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