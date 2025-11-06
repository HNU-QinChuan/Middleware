#pragma once
constexpr int width=100;//x
constexpr int height=100;//y
constexpr double resolution=0.2;

constexpr float actual_half_width=resolution*width/2;
constexpr float actual_half_height=resolution*height/2;
constexpr float actual_width=resolution*width;
constexpr float actual_height=resolution*width;


constexpr int expansion_size=1/resolution;