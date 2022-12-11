# Windows環境設定

踩地雷是基於 [SDL](https://www.libsdl.org/) 開發的，而 Windows 版本的踩地雷是透過 [MinGW](https://www.mingw-w64.org/) 編譯，故設定 Windows 編譯環境需要經由以下兩個步驟
1. 安裝 MinGW
2. 安裝 SDL 相關 library

## Windows 上安裝 MinGW 的方法

MinGW 旨在為 Windows 環境下提供 GCC 及其他編譯所需的工具，透過安裝 MinGW，我們便可以在 Windows 上擁有 GCC 及其編譯相關所需的標頭檔 (headers) 及函式庫。然而，MinGW 計畫僅止於提供完整編譯環境，而編譯過程中會需要的建置工具 (如 [Make](https://www.gnu.org/software/make/)) 卻不包含在 MinGW 中，因此我們選擇透過 MSYS ([現為 MSYS2](https://www.msys2.org/)) 來安裝 MinGW。

[MSYS2](https://www.msys2.org/) 旨在為 Windows 環境下，提供 bash shell、GCC 編譯工具、等其他開源軟體的移植，如此一來便可以在 Windows上 擁有一個類似於 Linux 的編譯環境。我們藉由安裝 MSYS2，完成後在 MSYS2 底下繼續安裝 MinGW 以及 Make 來完成編譯環境的建置。

步驟如下:
1. 前往 [MSYS2 首頁](https://www.msys2.org/)下載 MSYS2 安裝檔，下載完成後執行安裝檔
2. 依照說明進行安裝，安裝路徑使用預設值即可

![windows-install-mingw-step-2-1.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-install-mingw-step-2-1.png)

![windows-install-mingw-step-2-2.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-install-mingw-step-2-2.png)

3. 安裝完成後即可啟動 MSYS2

![windows-install-mingw-step-3.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-install-mingw-step-3.png)

4. 在 MSYS2 下鍵入下列指令來安裝 MinGW 和 Make
```
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S make
```

上述步驟都完成後，即完成 MSYS2、MinGW 及 Make 的安裝。以後若要開機 MSYS2 終端機，請選擇 MSYS2 UCRT64，其餘各種終端機的差異請見此連結。

## Windows上安裝 SDL library 的方法

踩地雷相依於 [SDL](https://github.com/libsdl-org/SDL) 及 [SDL_image](https://github.com/libsdl-org/SDL_image) 兩個額外的 library。在 Windows 上安裝 SDL 相當簡單，只要前往 [SDL release](https://github.com/libsdl-org/SDL/releases) 和 SDL_image release 兩處下載，並至解壓縮至踩地雷資料夾即可。

步驟如下:
1. 前往 [SDL release](https://github.com/libsdl-org/SDL/releases) 下載 mingw 版本 release。以 2.26.1 版而言，選擇 ***SDL2-devel-2.26.1-mingw.zip***

![windows-install-sdl-step-1.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-install-sdl-step-1.png)

2. 前往 [SDL_image release](https://github.com/libsdl-org/SDL_image/releases) 下載 mingw 版本 release。以 2.6.2 版而言，選擇 ***SDL2_image-devel-2.6.2-mingw.zip***

![windows-install-sdl-step-2.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-install-sdl-step-2.png)

3. 下載完成後，解壓縮至踩地雷資料夾內即完成

![windows-install-sdl-step-3.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-install-sdl-step-3.png)


## Windows上編譯的方法

踩地雷透過 Makefile 進行編譯，內有三個重要的 target:
1. ***all***: 這是預設的 target，用以編譯出執行檔 mine.exe。然而因為系統缺乏相關 library，若直接執行該執行檔預期會出現錯誤。
2. ***dist***: 是 distribution 的意思，用於建立一個可用於發布的軟體。會產生一個 Minesweeper 資料夾，並將編譯出的執行檔 mine.exe、相關 library、程式所用到的圖片都複製到該資料夾內，執行資料夾內的 mine.exe 便可以啟動踩地雷
3. ***clean***: 清除編譯過程的產物

以下為步驟:
1. 開啟 MSYS2 UCRT64，並用 `cd` 切換至踩地雷資料夾
2. 輸入 `make` 開始編譯

![windows-compile-make.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-compile-make.png)

3. 輸入 `make dist` 開始建立發布軟體，完成後即可見到 Minesweeper 資料夾

![windows-compile-make-dist-1.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-compile-make-dist-1.png)

![windows-compile-make-dist-2.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-compile-make-dist-2.png)

4. 輸入 `make clean` 會清除所有編譯產物

![windows-compile-make-clean.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-compile-make-clean.png)

## Windows上執行的方法

經過 `make dist` 後，會在踩地雷資料夾內產生一個 Minesweeper 的資料夾，該資料夾內有 mine.exe 以及他相依的 library，點擊資料夾內 mine.exe 即可執行。

若要將程式帶至其他電腦執行，只要將新產生的 Minesweeper 資料夾壓縮打包，然後在另外一台電腦上解壓縮後，一樣點選 mine.exe 便可順利執行。

步驟如下:
1. 執行 `make dist` 完成後，會產生 Minesweeper 資料夾

![windows-exec-step-1.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-exec-step-1.png)

2. 進入 Minesweeper 資料夾後，點擊 mine.exe 即可執行

![windows-exec-step-2-1.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-exec-step-2-1.png)

![windows-exec-step-2-2.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/windows-exec-step-2-2.png)


# Mac環境設定

踩地雷是基於 [SDL](https://www.libsdl.org/) 開發的，而 Mac 版本的踩地雷是透過 [clang++](https://clang.llvm.org/) 編譯，故設定 Mac 編譯環境需要經由以下兩個步驟
1. 安裝 clang++ 和 Make
2. 安裝 SDL 相關 library

## Mac 上安裝 clang++ 的方法

clang++ 是 Mac 上預設的 C++ 編譯器，在 Mac 上只要安裝完 Xcode，便會安裝 clang++。同樣的，我們依靠 Make 作為我們建置的工具，安裝 Xcode 亦會一併將 Make給安裝好。

可以在終端機下執行下列指令確認 clang++ 和 Make 是否已經被安裝:
```
clang++ --version
Make --version
```

![mac-install-env-check.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/mac-install-env-check.png)

## Mac 上安裝 SDL library 的方法

踩地雷相依於 [SDL](https://github.com/libsdl-org/SDL) 及 [SDL_image](https://github.com/libsdl-org/SDL_image) 兩個額外的 library，在 Mac 稱之為 [Framework](https://developer.apple.com/library/archive/documentation/MacOSX/Conceptual/OSX_Technology_Overview/SystemFrameworks/SystemFrameworks.html)。因為 Mac 的編譯環境相較於 Windows 限制較多，故須將 SDL 和 SDL_image 安裝至系統指定的路徑 (即 /Library/Frameworks)，以避免編譯過程出錯。

步驟如下:
1. 前往 [SDL release](https://github.com/libsdl-org/SDL/releases) 下載 Mac 版本 release。以 2.26.1 版而言，選擇 ***SDL2-2.26.1.dmg***

![mac-install-sdl-step-1.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/mac-install-sdl-step-1.png)

2. 前往 [SDL_image release](https://github.com/libsdl-org/SDL_image/releases) 下載 Mac 版本 release。以 2.6.2 版而言，選擇 ***SDL2_image-2.6.2.dmg***

![mac-install-sdl-step-2.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/mac-install-sdl-step-2.png)

3. 下載完成後，點擊 dmg 檔，並將其中的 SDL2.framework 拖曳 (即複製) 到 Finder 的 /Library/Framework。SDL2_image 作法亦同。請參考[此篇文章](https://support.apple.com/zh-tw/guide/mac-help/mchlp1236/mac)來開啟 /Library/Framework (即資源庫)。

![mac-install-sdl-step-3.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/mac-install-sdl-step-3.png)

4. 開啟 Finder 前往 /Library/Frameworks 確認 SDL2.framework 和 SDL2_image.framework 存在，即完成安裝

![mac-install-sdl-step-4-1.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/mac-install-sdl-step-4-1.png)

![mac-install-sdl-step-4-2.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/mac-install-sdl-step-4-2.png)


## Mac 上編譯的方法

踩地雷透過 Makefile 進行編譯，內有三個重要的 target:
1. ***all***: 這是預設的 target，用以編譯出執行檔 mine。然而因為系統缺乏相關 library，若直接執行該執行檔會出現錯誤。
2. ***dist***: 是 distribution 的意思，用於建立一個可用於發布的軟體。會產生一個 Mine.app 資料夾，並將編譯出的執行檔 mine、相關 library、程式所用到的圖片都複製到該資料夾內，並根據 Apple 規範設定 [Info.plist](https://developer.apple.com/documentation/bundleresources/information_property_list)。如此一來該資料夾在 Mac OS X 下便會被視為一個 [application bundle](https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFBundles/Introduction/Introduction.html)，只要點擊便會自動執行。
3. ***clean***: 清除編譯過程的產物

以下為步驟:
1. 開啟終端機，並用 `cd` 切換至踩地雷資料夾

![mac-compile-open-terminal.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/mac-compile-open-terminal.png)

2. 輸入 `make` 開始編譯

![mac-compile-make.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/mac-compile-make.png)

3. 輸入 `make dist` 開始建立 application bundle

![mac-compile-make-dist.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/mac-compile-make-dist.png)

4. 輸入 `make clean` 清除所有編譯產物

![mac-compile-make-clean.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/mac-compile-make-clean.png)

## Mac 上執行的方法

經過 `make dist` 後，會在踩地雷資料夾內產生一個 Mine.app 的資料夾。然而，因為我們已經遵循 Apple Application Bundle 規範設定 Info.plist，因此這個資料夾在 Finder 上會顯示成一個可執行的應用程式。目前設定的 icon 是老師圖像，雙擊 icon 即可執行。

步驟如下:
1. 執行 `make dist` 完成後，會產生 Mine.app，在 Finder 顯示為一個有老師圖像的應用程式。

![mac-exec-step-1.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/mac-exec-step-1.png)

2. 點擊即可執行

![mac-exec-step-2.png](https://github.com/Lizzychu/minesweeper/blob/master/doc/images/mac-exec-step-2.png)





