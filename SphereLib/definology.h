#pragma once

#include "Stdafx.h"
#include "../EterBase/Singleton.h"

class Definology : public CSingleton<Definology>
{
	public:

		LPDIRECT3DTEXTURE9		vshadow_tex			= nullptr;
		float					vspecular;
		float					vspecular_tree		= 3.5f;
		D3DXVECTOR3				vspecular_light		= { 2.5f, 2.5f, 2.5f };
		float					vpoint_light		= 1.0f;
		float					vshiness			= 15.0f;
		D3DXMATRIX				matLightView;
		D3DXMATRIX				matLightProj;
		D3DXVECTOR3				vsun_pos;
		D3DXVECTOR3				vcurrent_pos;
		std::string				vcurrent_map;
		bool					voutdoor_map		= true;
		bool					vup_sun_pos			= false;
		bool					vdynamic_light		= true;
		bool					vshadows			= true;
		int						vshadows_intensity	= 99;
		float					vshadows_range		= 1.0f;
};

#define vglobal Definology::Instance()




/*
	vglobal.
