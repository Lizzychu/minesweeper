
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
   * `window_texture`: 他是一個指向 [SDL_Texture](https://wiki.libsdl.org/SDL2/SDL_Texture) 的 pointer，代表視窗的 texture (可以想像是一張圖，這張圖被更新後要透過 `renderer` 繪製到螢幕上)
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

#### 元件繪圖介面

元件主要透過 `MineGameWindowUI::UpdateWindowTexture` 將自身繪製於視窗上。`UpdateWindowTexture` 實作便是依靠 SDL API 透過 `renderer` (視窗繪圖器) 將傳入的 `texture` (即元件本身的圖層) 繪製於 `window_texture` (即視窗圖層) 上，並將 `window_texture_dirty` 設為 `true`，如此一來下次呼叫 `RefreshWindow` 便會更新視窗內容(繪製於視窗圖層後，尚需呼叫 `RefreshWindow` 才會將視窗圖層更新於螢幕上)。程式碼如下

```C++
int MineGameWindowUI::UpdateWindowTexture(SDL_Texture *texture, const SDL_Rect *rect)
{
    if (this->window_texture != NULL) {
        SDL_SetRenderTarget(this->renderer, this->window_texture);
        SDL_RenderCopy(this->renderer, texture, NULL, rect);
        this->window_texture_dirty = true;
    }

    return 0;
}
```

`MineGameWindowUI::RefreshWindow` 是更新視窗內容的實作，如前所述，該函式內部會先檢查 `window_texture_dirty` 是否為 `true`，以確保在視窗內容沒有改變的狀況下，不需要更新視窗內容於螢幕，以免過度消耗資源，其實作如下

```C++
int MineGameWindowUI::RefreshWindow()
{
    // update texture to window
    if (this->window_texture != NULL && this->window_texture_dirty) {
        std::cout << "RefreshWindow" << std::endl;

        SDL_SetRenderTarget(this->renderer, NULL);
        SDL_RenderCopy(this->renderer, this->window_texture, NULL, NULL);
        SDL_RenderPresent(this->renderer);
        this->window_texture_dirty = false;
    }

    return 0;
}
```

#### 繪製元件的方式

在 GUI 內部，每一個元件都負責更新自身於螢幕特定的繪圖區域，如下圖所示

![gui-drawing-area](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/gui-drawing-area.png)

觀察上圖可以知道，每一個元件有自身繪圖區域，***除了 mine_grid 大小可能因為遊戲難度改變之外，其餘所有元件的大小都是固定的***。除此之外，元件與元件之間尚有間隙，這個***間隙大小也是固定的***，他在程式碼中定義為 `WINDOWS_EDGE_MARGIN`，他的大小是 10 (即 10 個像素)，程式碼如下

```C++
#define WINDOWS_EDGE_MARGIN 10
```

由於視窗大小會隨遊戲難易程度而改變，因此當遊戲改變難易度導致視窗改變大小時，因為元件大小與元件之間的間隙是固定的，我們只要知道元件繪圖時所在的位置，便可以定義上圖中的元件繪圖區域。而元件的位置是動態在視窗改變大小時所呼叫的 `ResizeWindow` 內所決定的，其程式碼如下:

```C++
int MineGameWindowUI::ResizeWindow()
{
    int total_width, total_height;

    // 視窗寬度為 mine_grid 寬度加上 2 個間隙
    // 視窗高度為 mine_grid 高度加上 3 個間隙
    total_width = this->mine_grid->GetWidth() + WINDOWS_EDGE_MARGIN * 2;
    total_height = this->mine_grid->GetHeight() + this->mine_counter->GetHeight() + WINDOWS_EDGE_MARGIN * 3;
    ...

    this->mine_grid->SetLocation(WINDOWS_EDGE_MARGIN, this->mine_counter->GetHeight() + WINDOWS_EDGE_MARGIN * 2);
    this->mine_counter->SetLocation(WINDOWS_EDGE_MARGIN, WINDOWS_EDGE_MARGIN);
    this->time_counter->SetLocation(total_width - WINDOWS_EDGE_MARGIN - this->time_counter->GetWidth(), WINDOWS_EDGE_MARGIN);
    this->face_button->SetLocation(((total_width - this->face_button->GetWidth()) / 2), WINDOWS_EDGE_MARGIN);
    ...
}
```

在了解元件定位與元件繪圖介面後，以下便是 `RedrawWindow` 更新視窗內容的流程
1. 首先設定 `renderer` (繪圖器)目標為 windows_texture (即視窗圖層)
2. 設定 `renderer` 色彩為 (R, G, B) = (0xC0, 0xC0, 0xC0)，即我們看到視窗的淺灰色
3. 呼叫 [SDL_RenderFillRect](https://wiki.libsdl.org/SDL2/SDL_RenderFillRect) 將視窗圖層畫上淺灰色，此時視窗圖層會是一個淺灰色的矩形
4. 依序針對每個元件呼叫其 `Redraw` 介面，要求元件將其繪於視窗圖層上，如此一來上圖繪圖區域中的黑色部分，就會是元件的樣子

程式碼如下

```C++
int MineGameWindowUI::RedrawWindow()
{
    // set background to gray
    SDL_SetRenderTarget(this->renderer, this->window_texture);
    SDL_RenderClear(this->renderer);
    SDL_SetRenderDrawColor(this->renderer, 0xC0, 0xC0, 0xC0, 0);
    SDL_RenderFillRect(this->renderer, NULL);

    // redraw components
    this->mine_grid->Redraw();
    this->time_counter->Redraw();
    this->mine_counter->Redraw();
    this->face_button->Redraw();

    return 0;
}
```

最後以 `FaceButtonUI` 為例，在其 `Redraw` 實作中，便會參考他內部狀態，進而呼叫 `UpdateWindowTexture` 將正確的圖像繪製於視窗圖層中正確的區域，程式碼如下:

```C++
int FaceButtonUI::Redraw()
{
    // pressed
    if (this->current_status == STATUS_FACE_PRESSED) {
        this->window->UpdateWindowTexture(this->face_pressed, this->GetRect());
    }
    // unpressed
    else if (this->current_status == STATUS_FACE_UNPRESSED) {
        this->window->UpdateWindowTexture(this->face_unpressed, this->GetRect());
    }
    // win
    else if (this->current_status == STATUS_FACE_WIN) {
        this->window->UpdateWindowTexture(this->face_win, this->GetRect());
    }
    // lose
    else {
        this->window->UpdateWindowTexture(this->face_lose, this->GetRect());
    }

    return 0;
}
```

### 計數器的實作

計數器(`CounterUI`)如前所述，是 `mine_counter` (計算剩餘地雷數量)和 `timer_counter` (計算遊戲經過時間) 的 GUI 實作。他主要用途就是將一個數字轉為 GUI 顯示在螢幕上。以 `mine_counter` 為例，每當 GUI 與 `MineGame` 互動後，GUI 會透過 `GetFlagCount` 得知玩家在遊戲圖面插了幾隻旗子，剩餘地雷數量便定義為**地雷數量減去旗幟數量**。他的實作方法非常簡單，作法是將數字百位數、十位數、個位數分別取出來，用 `digit1`、`digit2`、`digit3` 來記錄:

```C++
void CounterUI::SetCount(int c)
{
    this->count = c;

    if (c < 0) {
        this->digit1 = 0;
        this->digit2 = 0;
        this->digit3 = 0;
    } else if (c < 10) {
        this->digit1 = 0;
        this->digit2 = 0;
        this->digit3 = c;
    } else if (c < 100) {
        this->digit1 = 0;
        this->digit2 = c / 10;
        this->digit3 = c % 10;
    } else {
        this->digit1 = c / 100;
        this->digit2 = (c % 100) / 10;
        this->digit3 = c % 10;
    }
}
```

當呼叫 `Redraw` (即視窗要求元件將其本身繪製於視窗圖層之繪圖區域) 時，依照前述 `digit1`、`digit2`、`digit3` 來繪製正確的圖案

```C++
int CounterUI::Redraw()
{
    this->window->UpdateWindowTexture(this->background, this->background_rect);
    this->window->UpdateWindowTexture(this->digit[this->digit1], this->digit1_rect);
    this->window->UpdateWindowTexture(this->digit[this->digit2], this->digit2_rect);
    this->window->UpdateWindowTexture(this->digit[this->digit3], this->digit3_rect);

    return 0;
}
```

其中 `digit`陣列存的便是透過呼叫 `LoadTextureFromFile`，從檔案中所讀取到的圖片

```C++
int CounterUI::LoadResources()
{
    this->digit[0] = this->window->LoadTextureFromFile("./images/png/d0.png");
    this->digit[1] = this->window->LoadTextureFromFile("./images/png/d1.png");
    this->digit[2] = this->window->LoadTextureFromFile("./images/png/d2.png");
    this->digit[3] = this->window->LoadTextureFromFile("./images/png/d3.png");
    this->digit[4] = this->window->LoadTextureFromFile("./images/png/d4.png");
    this->digit[5] = this->window->LoadTextureFromFile("./images/png/d5.png");
    this->digit[6] = this->window->LoadTextureFromFile("./images/png/d6.png");
    this->digit[7] = this->window->LoadTextureFromFile("./images/png/d7.png");
    this->digit[8] = this->window->LoadTextureFromFile("./images/png/d8.png");
    this->digit[9] = this->window->LoadTextureFromFile("./images/png/d9.png");
    this->background = this->window->LoadTextureFromFile("./images/png/nums_background.png");

    return 0;
}
```

### 地雷區域的實作

地雷區域以 `MineGridUI` 這個 C++ class 來實現，是整個踩地雷遊戲中最重要的元件，他負責處理滑鼠點擊事件、將事件翻譯並轉送 `MineGame` 與之互動、並將互動結果顯示於 GUI 上。他最主要的進入點在 `MineGridUI::HandleMouseButtonEvent`，其流程如下
1. 如 `FaceButtonUI`，`MineGridUI` 會確認滑鼠點擊位置 (`x`, `y`) 確實位於其繪圖區域內部，僅處理其繪圖區域內部之事件
2. 當滑鼠點擊其繪圖區域內部後，會將滑鼠點擊座標 (`x`, `y`) 轉換為 grid 座標 (`index_x`, `index_y`)，即 `MineGame` 所知道的踩地雷圖面格子的座標
   * 每個踩地雷格子寬和高都定義為 `MINE_GRID_MINE_SIZE`，固定為 24px
   * 若點擊之座標 x 和繪圖區左上角 x1 之相對座標介於 0 ~ 23，就是點擊在第 1 欄；若點擊之座標 x 介於 24 ~ 46，就是點擊在第 2 欄；依此類推
   * 若點擊之座標 y 和繪圖區左上角 y1 之相對座標介於 0 ~ 23，就是點擊在第 1 行；若點擊之座標 x 介於 24 ~ 46，就是點擊在第 2 行；依此類推
   * 由上可知，轉換公式如下
      * `index_x = (event->x - x1) / MINE_GRID_MINE_SIZE`
      * `index_y = (event->y - y1) / MINE_GRID_MINE_SIZE`
3. 在得知 (`index_x`, `index_y`) 即所知道的踩地雷圖面格子的座標後，視滑鼠行為呼叫正確 function 與 `MineGame` 互動
   * 若是點擊滑鼠左鍵後上彈，呼叫 `MineGame::Open` 打開一格
   * 若是點擊滑鼠右鍵後上彈，呼叫 `MineGame::TouchFlag` 在那一格上放置或收回旗幟
4. 呼叫 `RedrawDirtyGrids` 更新 `MineGridUI` 元件之圖層

程式碼如下

```C++
int MineGridUI::HandleMouseButtonEvent(SDL_MouseButtonEvent *event)
{
    int x1, x2, y1, y2;

    x1 = this->GetRect()->x;
    x2 = this->GetRect()->x + this->GetRect()->w;
    y1 = this->GetRect()->y;
    y2 = this->GetRect()->y + this->GetRect()->h;

    // 確保滑鼠點擊位置 (x, y) 位於 (x1, y1) 和 (x2, y2) 之矩形內部
    if (x1 <= event->x && event->x < x2 && y1 <= event->y && event->y < y2) {
        // 將滑鼠點擊位置轉換為 grid 座標
        if (event->x - x1 > MINE_GRID_EDGE_MARGIN && event->y - y1 > MINE_GRID_EDGE_MARGIN) {
            int index_x = (event->x - x1) / MINE_GRID_MINE_SIZE;
            int index_y = (event->y - y1) / MINE_GRID_MINE_SIZE;

            // 視滑鼠行為呼叫 Open 或 TouchFlag
            if (event->button == SDL_BUTTON_LEFT && event->type == SDL_MOUSEBUTTONUP) {
                this->window->GameOpen(index_x, index_y);
            } else if (event->button == SDL_BUTTON_RIGHT && event->type == SDL_MOUSEBUTTONUP) {
                this->window->GameTouchFlag(index_x, index_y);
            }   

            this->RedrawDirtyGrids();
        }
    }
  
    return 0;
}
```

`RedrawDirtyGrids` 顧名思義，就是將 dirty grids (狀態改變的踩地雷圖面格子) 重繪。如前所述，依照[互動示意圖](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/gui-game-interaction.png)，每次 GUI 與 `MineGame` 互動後都要呼叫 `MineGame::GetDirtyGrids` 和 `MineGame::ClearDirtyGrids` 並更新 GUI 以顯示圖面格子最新狀態。這部分就是在 `RedrawDirtyGrids` 內實作的，其流程如下
1. 透過 `MineGameWindoeUI::GameGetDirtyGrids` 取得狀態改變的格子，事實上這個 function 內部就是對 `MineGame` 呼叫上述 `GetDirtyGrids` 和 `ClearDirtyGrids`
2. 取得狀態改變的格子後，針對每一個 `grids[i]` (即踩地雷格子)，更新其所在元件圖層的位置
   * 若 `grids[i]` 在 `MineGame` 之 `x` 座標為 0，其元件圖層之繪圖相對座標亦為 0
   * 若 `grids[i]` 在 `MineGame` 之 `x` 座標為 1，其元件圖層之繪圖相對座標為 24，因為每一格寬高都為 24
   * 若 `grids[i]` 在 `MineGame` 之 `x` 座標為 2，其元件圖層之繪圖相對座標為 48，因為每一格寬高都為 24
   * 其餘 x 座標、y 座標皆以此類推
   * 由上可以得知，若以一矩形 `rect` 代表 `grids[i]` 所在元件圖層的相對位置及其大小，其轉換公式如下，其中 `MINE_GRID_EDGE_MARGIN` 是 `MineGridUI` 元件內部與 grids 留白的間隙，定義為 2px
     * 矩形之相對位置 x 座標: `rect.x = grids[i].x * MINE_GRID_MINE_SIZE + MINE_GRID_EDGE_MARGIN;`
     * 矩形之相對位置 y 座標: `rect.y = grids[i].y * MINE_GRID_MINE_SIZE + MINE_GRID_EDGE_MARGIN;`
     * 矩形之寬度: `rect.w = MINE_GRID_MINE_SIZE;`
     * 矩形之高度: `rect.h = MINE_GRID_MINE_SIZE;`

其程式碼如下

```C++
int MineGridUI::RedrawDirtyGrids()
{
    std::vector<MineGameGrid> grids;

    // 透過 MineGameWindowUI 取得狀態改變的格子
    this->window->GameGetDirtyGrids(grids);

    for (size_t i = 0; i < grids.size(); i++) {
        SDL_Rect rect;

        // 找出每一個格子所在的矩形區域
        rect.x = grids[i].x * MINE_GRID_MINE_SIZE + MINE_GRID_EDGE_MARGIN;
        rect.y = grids[i].y * MINE_GRID_MINE_SIZE + MINE_GRID_EDGE_MARGIN;
        rect.w = MINE_GRID_MINE_SIZE;
        rect.h = MINE_GRID_MINE_SIZE;

        // 依照格子的狀態, 選擇正確的圖示並將之更新於元件圖層上
        ... 
    }

    // 將元件圖層更新於視窗圖層上
    this->Redraw();

    return 0;
}
```

