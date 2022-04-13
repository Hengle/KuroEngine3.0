#pragma once
//エンジン本体==========================================================
#include<string>
#include"Vec.h"
#include<memory>
#include"Color.h"

#include"D3D12App.h"
#include"WinApp.h"
#include"UsersInput.h"
#include"AudioApp.h"
#include"ImguiApp.h"
#include"GraphicsManager.h"

class Fps;

//シーンの基底クラス
class BaseScene
{
protected:
	BaseScene() {}
	virtual void OnInitialize() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnDraw() = 0;
	virtual void OnImguiDebug() = 0;
	virtual void OnFinalize() = 0;
public:
	virtual ~BaseScene() {};
	void Initialize();
	void Update();
	void Draw();
	void ImguiDebug();
	void Finalize();
};

//シーン遷移（基本は継承して利用するが、１フレ切り替え遷移は継承なしで利用できる）
class SceneTransition
{
protected:
	bool nowTrans = false;
	virtual void OnStart() { }
	virtual bool OnUpdate() { nowTrans = false; return true; }
	virtual void OnDraw() {}
public:
	virtual ~SceneTransition() {}
	void Start() { nowTrans = true; OnStart(); }	//シーン遷移スタート
	bool Update() { return OnUpdate(); }	//シーンを切り替えるタイミングでtrueを返す
	void Draw() { OnDraw(); }
	bool Finish() { return !nowTrans; }	//シーン遷移が完全に終了したか
};

//アプリの初期設定
struct EngineOption
{
	//ウィンドウ関連
	std::string windowName;	//ウィンドウ名
	Vec2<int>windowSize;	//ウィンドウサイズ
	const wchar_t *iconPath = nullptr;	//ウィンドウアイコンファイル名

	//バックバッファ関連
	Color backBuffClearColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
	bool useHDR = false;	//HDRを使うか（HDR対応していない場合使用しない）

	//フレームレート
	int frameRate = 60;
};

//エンジン本体
class KuroEngine : public D3D12AppUser
{
public:
	//シングルトン
	static KuroEngine &Instance()
	{
		static KuroEngine engine;
		return engine;
	}

private:
	KuroEngine() {}

	//初期化されているか
	bool invalid = true;

	//XAudio
	HMODULE xaudioLib;
	//ウィンドウアプリ
	std::unique_ptr<WinApp>winApp;
	//D3D12アプリ
	std::unique_ptr<D3D12App>d3d12App;
	//入力管理
	std::unique_ptr<UsersInput>usersInput;
	//音声関連アプリ
	std::unique_ptr<AudioApp>audioApp;
	//Imguiアプリ
	std::unique_ptr<ImguiApp>imguiApp;

	//グラフィックスマネージャ（レンダリング統括、Zソートなどを行う）
	GraphicsManager gManager;
	std::shared_ptr<ConstantBuffer>parallelMatProjBuff;

	//シーンリスト
	std::vector<BaseScene *>scenes;
	int nowScene;	//現在のシーン番号
	int nextScene = -1;	//次のシーン番号
	SceneTransition *nowSceneTransition;	//現在セットされているシーン遷移

	//FPS固定
	std::shared_ptr<Fps>fps;

	//終了フラグ
	bool end = false;

	void Render()override;
public:
	~KuroEngine();
	void Initialize(const EngineOption &Option);
	void SetSceneList(const std::vector<BaseScene *> &SceneList, const int &AwakeSceneNum);
	void Update();
	void Draw();
	bool End() { return end; }

	//ゲーム終了
	void GameEnd() { end = true; }
	//シーンチェンジ
	void ChangeScene(const int &SceneNum, SceneTransition *SceneTransition)
	{
		nextScene = SceneNum;
		nowSceneTransition = SceneTransition;
		nowSceneTransition->Start();
	}

	//グラフィックスマネージャゲッタ
	GraphicsManager &Graphics() { return gManager; }

	//平行投影行列定数バッファ
	const std::shared_ptr<ConstantBuffer> &GetParallelMatProjBuff()
	{
		return parallelMatProjBuff;
	}

};

