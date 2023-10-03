
#include "pch.h"
#include "AssetManager.h"

void AssetManager::InitializeResources()
{
	if (bIsInitialized)
		return;

	bIsInitialized = true;

	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Main Characters//Pink Man//Idle (32x32).png";
		std::size_t FPS = 6; // 10 // 60
		std::size_t SpriteFPS = 1;
		sSpriteSheet::SharedPtr SpriteSheet = sSpriteSheet::Create("IDLE");
		SpriteSheet->FPS = FPS;
		SpriteSheet->AddSprite(sSprite::Create("Idle_1", TextureLoc, FBounds2D(FVector2(5, 6), FVector2(27, 32))), SpriteFPS, 0);
		SpriteSheet->AddSprite(sSprite::Create("Idle_2", TextureLoc, FBounds2D(FVector2(37, 6), FVector2(59, 32))), SpriteFPS, 1);
		SpriteSheet->AddSprite(sSprite::Create("Idle_3", TextureLoc, FBounds2D(FVector2(69, 6), FVector2(91, 32))), SpriteFPS, 2);
		SpriteSheet->AddSprite(sSprite::Create("Idle_4", TextureLoc, FBounds2D(FVector2(101, 5), FVector2(123, 32))), SpriteFPS, 3);
		SpriteSheet->AddSprite(sSprite::Create("Idle_5", TextureLoc, FBounds2D(FVector2(133, 5), FVector2(155, 32))), SpriteFPS, 4);
		SpriteSheet->AddSprite(sSprite::Create("Idle_6", TextureLoc, FBounds2D(FVector2(165, 4), FVector2(187, 32))), SpriteFPS, 5);
		SpriteSheet->AddSprite(sSprite::Create("Idle_7", TextureLoc, FBounds2D(FVector2(197, 4), FVector2(219, 32))), SpriteFPS, 6);
		SpriteSheet->AddSprite(sSprite::Create("Idle_8", TextureLoc, FBounds2D(FVector2(229, 4), FVector2(251, 32))), SpriteFPS, 7);
		SpriteSheet->AddSprite(sSprite::Create("Idle_9", TextureLoc, FBounds2D(FVector2(261, 4), FVector2(283, 32))), SpriteFPS, 8);
		SpriteSheet->AddSprite(sSprite::Create("Idle_10", TextureLoc, FBounds2D(FVector2(293, 5), FVector2(315, 32))), SpriteFPS, 9);
		SpriteSheet->AddSprite(sSprite::Create("Idle_11", TextureLoc, FBounds2D(FVector2(325, 5), FVector2(347, 32))), SpriteFPS, 10);

		StoreSpriteSheet(SpriteSheet);
	}

	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Main Characters//Pink Man//Run (32x32).png";
		std::size_t FPS = 6; // 10 // 60
		sSpriteSheet::SharedPtr Running = sSpriteSheet::Create("Running");
		Running->FPS = FPS;
		Running->AddSprite(sSprite::Create("Running_1", TextureLoc, FBounds2D(FVector2(5, 3), FVector2(27, 32))), 1);
		Running->AddSprite(sSprite::Create("Running_2", TextureLoc, FBounds2D(FVector2(37, 2), FVector2(59, 32))), 1);
		Running->AddSprite(sSprite::Create("Running_3", TextureLoc, FBounds2D(FVector2(69, 2), FVector2(91, 32))), 1);
		Running->AddSprite(sSprite::Create("Running_4", TextureLoc, FBounds2D(FVector2(101, 4), FVector2(123, 32))), 1);
		Running->AddSprite(sSprite::Create("Running_5", TextureLoc, FBounds2D(FVector2(132, 6), FVector2(156, 32))), 1);
		Running->AddSprite(sSprite::Create("Running_6", TextureLoc, FBounds2D(FVector2(164, 6), FVector2(188, 32))), 1);
		Running->AddSprite(sSprite::Create("Running_7", TextureLoc, FBounds2D(FVector2(196, 3), FVector2(220, 32))), 1);
		Running->AddSprite(sSprite::Create("Running_8", TextureLoc, FBounds2D(FVector2(228, 2), FVector2(252, 32))), 1);
		Running->AddSprite(sSprite::Create("Running_9", TextureLoc, FBounds2D(FVector2(260, 2), FVector2(284, 32))), 1);
		Running->AddSprite(sSprite::Create("Running_10", TextureLoc, FBounds2D(FVector2(292, 4), FVector2(316, 32))), 1);
		Running->AddSprite(sSprite::Create("Running_11", TextureLoc, FBounds2D(FVector2(324, 6), FVector2(348, 32))), 1);
		Running->AddSprite(sSprite::Create("Running_12", TextureLoc, FBounds2D(FVector2(356, 6), FVector2(380, 32))), 1);

		StoreSpriteSheet(Running);
	}

	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Main Characters//Pink Man//Jump (32x32).png";
		sSpriteSheet::SharedPtr Jump = sSpriteSheet::Create("Jump");
		Jump->AddSprite(sSprite::Create("Jump_1", TextureLoc, FBounds2D(FVector2(5, 2), FVector2(27, 32))), 1);

		StoreSpriteSheet(Jump);
	}

	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Main Characters//Pink Man//Fall (32x32).png";
		sSpriteSheet::SharedPtr Fall = sSpriteSheet::Create("Fall");
		Fall->AddSprite(sSprite::Create("Fall_1", TextureLoc, FBounds2D(FVector2(4, 4), FVector2(28, 32))), 1);

		StoreSpriteSheet(Fall);
	}

	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Main Characters//Pink Man//Hit (32x32).png";
		std::size_t FPS = 10; // 10 // 60
		sSpriteSheet::SharedPtr Hit = sSpriteSheet::Create("Hit", false);
		Hit->FPS = FPS;
		Hit->AddSprite(sSprite::Create("Hit_1", TextureLoc, FBounds2D(FVector2(7, 2), FVector2(21, 32))), 1);
		Hit->AddSprite(sSprite::Create("Hit_2", TextureLoc, FBounds2D(FVector2(39, 2), FVector2(58, 32))), 1);
		Hit->AddSprite(sSprite::Create("Hit_3", TextureLoc, FBounds2D(FVector2(70, 3), FVector2(91, 32))), 1);
		Hit->AddSprite(sSprite::Create("Hit_4", TextureLoc, FBounds2D(FVector2(100, 5), FVector2(125, 32))), 1);
		Hit->AddSprite(sSprite::Create("Hit_5", TextureLoc, FBounds2D(FVector2(131, 6), FVector2(158, 32))), 1);
		Hit->AddSprite(sSprite::Create("Hit_6", TextureLoc, FBounds2D(FVector2(164, 5), FVector2(189, 32))), 1);
		Hit->AddSprite(sSprite::Create("Hit_7", TextureLoc, FBounds2D(FVector2(197, 4), FVector2(220, 32))), 1);

		StoreSpriteSheet(Hit);
	}

	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Main Characters//Desappearing (96x96).png";
		std::size_t FPS = 14; // 10 // 60
		sSpriteSheet::SharedPtr Hit = sSpriteSheet::Create("Dead", false);
		Hit->FPS = FPS;
		Hit->AddSprite(sSprite::Create("Dead_1", TextureLoc, FBounds2D(FVector2(38, 34), FVector2(59, 63))), 1);
		Hit->AddSprite(sSprite::Create("Dead_2", TextureLoc, FBounds2D(FVector2(134, 34), FVector2(155, 63))), 1);
		Hit->AddSprite(sSprite::Create("Dead_3", TextureLoc, FBounds2D(FVector2(232, 39), FVector2(248, 55))), 1);
		Hit->AddSprite(sSprite::Create("Dead_4", TextureLoc, FBounds2D(FVector2(316, 27), FVector2(356, 66))), 1);
		Hit->AddSprite(sSprite::Create("Dead_5", TextureLoc, FBounds2D(FVector2(398, 14), FVector2(467, 81))), 1);
		Hit->AddSprite(sSprite::Create("Dead_6", TextureLoc, FBounds2D(FVector2(486, 6), FVector2(570, 89))), 1);
		Hit->AddSprite(sSprite::Create("Dead_7", TextureLoc, FBounds2D(FVector2(577, 0), FVector2(671, 94))), 1);

		StoreSpriteSheet(Hit);
	}

	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Items//Fruits//Apple.png";
		std::size_t FPS = 3; // 10 // 60
		sSpriteSheet::SharedPtr Sprite = sSpriteSheet::Create("Apple");
		Sprite->FPS = FPS;
		Sprite->AddSprite(sSprite::Create("Apple_1", TextureLoc, FBounds2D(FVector2(10, 6), FVector2(22, 21))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_2", TextureLoc, FBounds2D(FVector2(42, 6), FVector2(54, 21))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_3", TextureLoc, FBounds2D(FVector2(74, 6), FVector2(86, 21))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_4", TextureLoc, FBounds2D(FVector2(106, 6), FVector2(118, 21))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_5", TextureLoc, FBounds2D(FVector2(138, 6), FVector2(150, 21))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_6", TextureLoc, FBounds2D(FVector2(170, 5), FVector2(182, 22))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_7", TextureLoc, FBounds2D(FVector2(203, 4), FVector2(213, 23))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_8", TextureLoc, FBounds2D(FVector2(235, 4), FVector2(245, 23))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_9", TextureLoc, FBounds2D(FVector2(267, 4), FVector2(277, 23))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_10", TextureLoc, FBounds2D(FVector2(298, 5), FVector2(310, 22))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_11", TextureLoc, FBounds2D(FVector2(330, 5), FVector2(342, 22))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_12", TextureLoc, FBounds2D(FVector2(361, 6), FVector2(375, 21))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_13", TextureLoc, FBounds2D(FVector2(392, 7), FVector2(408, 20))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_14", TextureLoc, FBounds2D(FVector2(424, 7), FVector2(440, 20))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_15", TextureLoc, FBounds2D(FVector2(456, 7), FVector2(472, 20))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_16", TextureLoc, FBounds2D(FVector2(489, 6), FVector2(503, 21))), 1);
		Sprite->AddSprite(sSprite::Create("Apple_17", TextureLoc, FBounds2D(FVector2(521, 6), FVector2(535, 21))), 1);

		StoreSpriteSheet(Sprite);
	}

	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Items//Fruits//Cherries.png";
		std::size_t FPS = 3; // 10 // 60
		sSpriteSheet::SharedPtr Sprite = sSpriteSheet::Create("Cherries");
		Sprite->FPS = FPS;
		Sprite->AddSprite(sSprite::Create("Cherries_1", TextureLoc, FBounds2D(FVector2(9, 8), FVector2(23, 22))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_2", TextureLoc, FBounds2D(FVector2(41, 8), FVector2(55, 22))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_3", TextureLoc, FBounds2D(FVector2(73, 8), FVector2(87, 22))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_4", TextureLoc, FBounds2D(FVector2(105, 8), FVector2(119, 22))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_5", TextureLoc, FBounds2D(FVector2(137, 8), FVector2(151, 22))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_6", TextureLoc, FBounds2D(FVector2(167, 7), FVector2(183, 23))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_7", TextureLoc, FBounds2D(FVector2(202, 6), FVector2(214, 24))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_8", TextureLoc, FBounds2D(FVector2(234, 6), FVector2(246, 24))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_9", TextureLoc, FBounds2D(FVector2(266, 6), FVector2(278, 24))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_10", TextureLoc, FBounds2D(FVector2(297, 7), FVector2(311, 23))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_11", TextureLoc, FBounds2D(FVector2(329, 7), FVector2(343, 23))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_12", TextureLoc, FBounds2D(FVector2(360, 8), FVector2(376, 22))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_13", TextureLoc, FBounds2D(FVector2(391, 9), FVector2(409, 21))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_14", TextureLoc, FBounds2D(FVector2(423, 9), FVector2(441, 21))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_15", TextureLoc, FBounds2D(FVector2(455, 9), FVector2(473, 21))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_16", TextureLoc, FBounds2D(FVector2(488, 8), FVector2(504, 22))), 1);
		Sprite->AddSprite(sSprite::Create("Cherries_17", TextureLoc, FBounds2D(FVector2(520, 8), FVector2(536, 22))), 1);

		StoreSpriteSheet(Sprite);
	}

	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Items//Fruits//Collected.png";
		std::size_t FPS = 5; // 10 // 60
		sSpriteSheet::SharedPtr Sprite = sSpriteSheet::Create("Collected", false);
		Sprite->FPS = FPS;
		Sprite->AddSprite(sSprite::Create("Sprite_1", TextureLoc, FBounds2D(FVector2(12, 11), FVector2(20, 20))), 1);
		Sprite->AddSprite(sSprite::Create("Sprite_2", TextureLoc, FBounds2D(FVector2(43, 10), FVector2(53, 21))), 1);
		Sprite->AddSprite(sSprite::Create("Sprite_3", TextureLoc, FBounds2D(FVector2(73, 8), FVector2(87, 23))), 1);
		Sprite->AddSprite(sSprite::Create("Sprite_4", TextureLoc, FBounds2D(FVector2(103, 6), FVector2(121, 25))), 1);
		Sprite->AddSprite(sSprite::Create("Sprite_5", TextureLoc, FBounds2D(FVector2(133, 4), FVector2(155, 27))), 1);

		StoreSpriteSheet(Sprite);
	}

	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Traps//Saw//On (38x38).png";
		std::size_t FPS = 4; // 10 // 60
		sSpriteSheet::SharedPtr Sprite = sSpriteSheet::Create("Saw");
		Sprite->FPS = FPS;
		Sprite->AddSprite(sSprite::Create("Saw_1", TextureLoc, FBounds2D(FVector2(0, 0), FVector2(38, 38))), 1);
		Sprite->AddSprite(sSprite::Create("Saw_2", TextureLoc, FBounds2D(FVector2(38, 0), FVector2(76, 38))), 1);
		Sprite->AddSprite(sSprite::Create("Saw_3", TextureLoc, FBounds2D(FVector2(76, 0), FVector2(114, 38))), 1);
		Sprite->AddSprite(sSprite::Create("Saw_4", TextureLoc, FBounds2D(FVector2(114, 0), FVector2(152, 38))), 1);
		Sprite->AddSprite(sSprite::Create("Saw_5", TextureLoc, FBounds2D(FVector2(152, 0), FVector2(190, 38))), 1);
		Sprite->AddSprite(sSprite::Create("Saw_6", TextureLoc, FBounds2D(FVector2(190, 0), FVector2(228, 38))), 1);
		Sprite->AddSprite(sSprite::Create("Saw_7", TextureLoc, FBounds2D(FVector2(228, 0), FVector2(266, 38))), 1);
		Sprite->AddSprite(sSprite::Create("Saw_8", TextureLoc, FBounds2D(FVector2(266, 0), FVector2(303, 38))), 1);

		StoreSpriteSheet(Sprite);
	}

	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Traps//Rock Head//Idle.png";
		std::size_t FPS = 4; // 10 // 60
		sSpriteSheet::SharedPtr Sprite = sSpriteSheet::Create("Rock_Head_Idle");
		Sprite->FPS = FPS;
		Sprite->AddSprite(sSprite::Create("Idle", TextureLoc, FBounds2D(FVector2(5, 4), FVector2(37, 37))), 1);

		StoreSpriteSheet(Sprite);
	}
	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Traps//Rock Head//Blink (42x42).png";
		std::size_t FPS = 60; // 10 // 60
		sSpriteSheet::SharedPtr Sprite = sSpriteSheet::Create("Rock_Head_Blink");
		Sprite->FPS = FPS;
		Sprite->AddSprite(sSprite::Create("Rock_Head_Blink_1", TextureLoc, FBounds2D(FVector2(5, 4), FVector2(37, 36))), 1);
		Sprite->AddSprite(sSprite::Create("Rock_Head_Blink_2", TextureLoc, FBounds2D(FVector2(47, 4), FVector2(79, 36))), 1);
		Sprite->AddSprite(sSprite::Create("Rock_Head_Blink_3", TextureLoc, FBounds2D(FVector2(89, 4), FVector2(121, 36))), 1);
		Sprite->AddSprite(sSprite::Create("Rock_Head_Blink_4", TextureLoc, FBounds2D(FVector2(131, 4), FVector2(163, 36))), 1);

		StoreSpriteSheet(Sprite);
	}
	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Traps//Rock Head//Bottom Hit (42x42).png";
		std::size_t FPS = 8;
		sSpriteSheet::SharedPtr Sprite = sSpriteSheet::Create("Rock_Head_Bottom_Hit", false);
		Sprite->FPS = FPS;
		Sprite->AddSprite(sSprite::Create("Rock_Head_Bottom_Hit_1", TextureLoc, FBounds2D(FVector2(0, 9), FVector2(42, 37))), 1);
		Sprite->AddSprite(sSprite::Create("Rock_Head_Bottom_Hit_2", TextureLoc, FBounds2D(FVector2(42, 9), FVector2(84, 37))), 1);
		Sprite->AddSprite(sSprite::Create("Rock_Head_Bottom_Hit_3", TextureLoc, FBounds2D(FVector2(86, 7), FVector2(124, 36))), 1);
		Sprite->AddSprite(sSprite::Create("Rock_Head_Bottom_Hit_4", TextureLoc, FBounds2D(FVector2(130, 5), FVector2(164, 36))), 1);

		StoreSpriteSheet(Sprite);
	}
	{
		std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Traps//Rock Head//Top Hit (42x42).png";
		std::size_t FPS = 8;
		sSpriteSheet::SharedPtr Sprite = sSpriteSheet::Create("Rock_Head_Top_Hit", false);
		Sprite->FPS = FPS;
		Sprite->AddSprite(sSprite::Create("Rock_Head_Top_Hit_1", TextureLoc, FBounds2D(FVector2(0, 4), FVector2(42, 32))), 1);
		Sprite->AddSprite(sSprite::Create("Rock_Head_Top_Hit_2", TextureLoc, FBounds2D(FVector2(42, 4), FVector2(84, 32))), 1);
		Sprite->AddSprite(sSprite::Create("Rock_Head_Top_Hit_3", TextureLoc, FBounds2D(FVector2(86, 4), FVector2(124, 33))), 1);
		Sprite->AddSprite(sSprite::Create("Rock_Head_Top_Hit_4", TextureLoc, FBounds2D(FVector2(130, 4), FVector2(164, 35))), 1);

		StoreSpriteSheet(Sprite);
	}
	//{
	//	std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Traps//Rock Head//Left Hit (42x42).png";
	//	std::size_t FPS = 4; // 10 // 60
	//	sSpriteSheet::SharedPtr Sprite = sSpriteSheet::Create("Rock_Head_Left_Hit");
	//	Sprite->FPS = FPS;
	//	Sprite->AddSprite(sSprite::Create("Rock_Head_Left_Hit_1", TextureLoc, FBounds2D(FVector2(89, 1), FVector2(118, 39))), 1);
	//	Sprite->AddSprite(sSprite::Create("Rock_Head_Left_Hit_2", TextureLoc, FBounds2D(FVector2(131, 3), FVector2(162, 37))), 1);

	//	StoreSpriteSheet(Sprite);
	//}
	//{
	//	std::wstring TextureLoc = L"..//Content//Pixel Adventure 1//Free//Traps//Rock Head//Right Hit (42x42).png";
	//	std::size_t FPS = 4; // 10 // 60
	//	sSpriteSheet::SharedPtr Sprite = sSpriteSheet::Create("Rock_Head_Right_Hit");
	//	Sprite->FPS = FPS;
	//	Sprite->AddSprite(sSprite::Create("Rock_Head_Right_Hit_1", TextureLoc, FBounds2D(FVector2(92, 1), FVector2(121, 39))), 1);
	//	Sprite->AddSprite(sSprite::Create("Rock_Head_Right_Hit_2", TextureLoc, FBounds2D(FVector2(132, 3), FVector2(163, 37))), 1);

	//	StoreSpriteSheet(Sprite);
	//}
}
