#pragma once
#include "Scene.h"

class GamingScene : public Scene {
public:
	void Init() override;
	void Update(float dt) override;
	void Uninit() override;

private:
	void ShowModeSelectionDialog();
};
