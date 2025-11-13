專案介紹

本專案使用到的技術棧包含：
- 渲染層（Rendering）：DirectX 11
- 物理引擎：Jolt Physics
- 圖形化介面（GUI）：ImGui
- 資源管理：自研的資源管理系統
- 網路通訊：基於 ENet 的自製網路模組
- 模型載入：Assimp（多格式模型匯入庫）

專案結構
- base：Gameplay 基礎框架
- gameplay：遊戲邏輯層
- modules：功能模組（如網路、物理、特效等）

Gameplay 框架介紹

Gameplay 框架是整個專案的核心，負責管理遊戲的主要流程與物件生命週期。
整體架構參考 Unity 的設計概念，包含以下核心類別：
- Game：遊戲主類別，負責初始化、主迴圈與整體流程。

GameObject：遊戲物件，代表場景中的一個可運作實體。

Component：元件，掛載在 GameObject 上以提供特定功能。

專案特色

高度模組化設計
例如網路模組、物理模組等都是以 Component 形式實作，擴充與維護都很方便。

支援多人連線
使用 ENet 實現高效、低延遲的網路通訊。

真實物理效果
透過 Jolt Physics 提供物理模擬，並支援網路同步。

內建除錯介面
以 ImGui 建構開發時的除錯視窗，使調試與測試更直覺。

支援多種 3D 模型格式
使用 Assimp 匯入多種格式的 3D 模型。