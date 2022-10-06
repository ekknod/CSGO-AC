#define _CRT_SECURE_NO_WARNINGS

#include "engine.h"
#include "../utils.h"
#include <math.h>
#include <stdio.h>

namespace hooks
{
	DWORD viewangle_tid = 0;

	//
	// viewangles .data location is CSGO-AC.dll
	// it could be encrypted and decrypted in GetViewAngles/SetViewAngles routine if wanted to.
	// 
	static vec3 viewangles{0, -89.0f, 0};
	static void __fastcall GetViewAngles(void *, void *, vec3 &va);
	static void __fastcall SetViewAngles(void *, void *, vec3 &va);
}

static float AngleNormalize( float angle )
{
	angle = fmodf(angle, 360.0f);
	if (angle > 180) 
	{
    		angle -= 360;
	}
	if (angle < -180)
	{
    		angle += 360;
	}
	return angle;
}

static void __fastcall hooks::GetViewAngles(void *, void *, vec3 &va)
{
	if (viewangle_tid == 0)
	{
		viewangle_tid = utils::get_current_thread_id();
	}
	
	if (viewangle_tid != utils::get_current_thread_id())
	{
		va = vec3{};
		return;
	}
	va = viewangles;
}

static void __fastcall hooks::SetViewAngles(void *, void *, vec3 &va)
{
	if (viewangle_tid == 0)
	{
		viewangle_tid = utils::get_current_thread_id();
	}

	if (viewangle_tid != utils::get_current_thread_id())
	{
		return;
	}

	viewangles.x = AngleNormalize(va.x);
	viewangles.y = AngleNormalize(va.y);
	viewangles.z = AngleNormalize(va.z);
}

BOOL engine::InstallHooks(void)
{
	printf("[engine::Install]\n");

	PVOID factory = utils::get_interface_factory("engine.dll");
	if (factory == 0)
		return 0;

	PVOID IEngineClient = utils::get_interface(factory, "VEngineClient014");
	if (IEngineClient == 0)
		return 0;

	printf("[engine::IEngineClient]: %p\n", IEngineClient);

	utils::Hook(
		utils::get_interface_function(IEngineClient, 18), // GetViewAngles,
		(PVOID)hooks::GetViewAngles
	);
	
	utils::Hook(
		utils::get_interface_function(IEngineClient, 19), // SetViewAngles,
		(PVOID)hooks::SetViewAngles
	);

	printf("[engine::Install] complete\n");

	return 1;
}
