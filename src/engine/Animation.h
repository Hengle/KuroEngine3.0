#pragma once
#include<vector>
struct KeyFrame
{
	int frame;
	float value;
};

struct Animation
{
	int startFrame;
	int endFrame;
	std::vector<KeyFrame>keyFrames;
};